/****************************************************************************************
 * Copyright (c) 2007-2008 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#define DEBUG_PREFIX "CollectionManager"

#include "CollectionManager.h"

#include "core/capabilities/CollectionScanCapability.h"
#include "core/collections/Collection.h"
#include "core/collections/MetaQueryMaker.h"
#include "core/collections/support/SqlStorage.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "core/support/SmartPointerList.h"
#include "core-impl/meta/file/FileTrackProvider.h"
#include "core-impl/meta/stream/Stream.h"
#include "core-impl/meta/timecode/TimecodeTrackProvider.h"

#include <QCoreApplication>
#include <QList>
#include <QMetaEnum>
#include <QMetaObject>
#include <QPair>
#include <QTimer>
#include <QReadWriteLock>

typedef QPair<Collections::Collection*, CollectionManager::CollectionStatus> CollectionPair;

/** This wrapper will be used by the collection manager to present one static SqlStorage object even when the user switches the actual database.
On the other hand nobody except the owning collection should hold a reference to the SqlStorage anyway. */
class SqlStorageWrapper : public SqlStorage
{
public:
    SqlStorageWrapper()
        : SqlStorage()
        , m_sqlStorage( 0 )
    {}

    virtual int sqlDatabasePriority() const { return ( m_sqlStorage ? m_sqlStorage->sqlDatabasePriority() : 0 ); }
    virtual QString type() const  { return ( m_sqlStorage ? m_sqlStorage->type() : "SqlStorageWrapper" ); }
    virtual QString escape( const QString &text ) const  { return ( m_sqlStorage ? m_sqlStorage->escape( text ) : text ); }
    virtual QStringList query( const QString &query )  { return ( m_sqlStorage ? m_sqlStorage->query( query ) : QStringList() ); }
    virtual int insert( const QString &statement, const QString &table )  { return ( m_sqlStorage ? m_sqlStorage->insert( statement, table ) : 0 ); }
    virtual QString boolTrue() const  { return ( m_sqlStorage ? m_sqlStorage->boolTrue() : "1" ); }
    virtual QString boolFalse() const  { return ( m_sqlStorage ? m_sqlStorage->boolFalse() : "0" ); }
    virtual QString idType() const  { return ( m_sqlStorage ? m_sqlStorage->idType() : "WRAPPER_NOT_IMPLEMENTED" ); }
    virtual QString textColumnType( int length ) const { return ( m_sqlStorage ? m_sqlStorage->textColumnType( length ) : "WRAPPER_NOT_IMPLEMENTED" ); }
    virtual QString exactTextColumnType( int length ) const { return ( m_sqlStorage ? m_sqlStorage->exactTextColumnType( length ) : "WRAPPER_NOT_IMPLEMENTED" ); }
    virtual QString exactIndexableTextColumnType( int length ) const { return ( m_sqlStorage ? m_sqlStorage->exactIndexableTextColumnType( length ) : "WRAPPER_NOT_IMPLEMENTED" ); };
    virtual QString longTextColumnType() const { return ( m_sqlStorage ? m_sqlStorage->longTextColumnType() : "WRAPPER_NOT_IMPLEMENTED" ); }
    virtual QString randomFunc() const { return ( m_sqlStorage ? m_sqlStorage->randomFunc() : "WRAPPER_NOT_IMPLEMENTED" ); }

    virtual QStringList getLastErrors() const
    { return m_sqlStorage ? m_sqlStorage->getLastErrors() : QStringList(); }
    virtual void clearLastErrors()
    { if( m_sqlStorage ) m_sqlStorage->clearLastErrors(); }

    void setSqlStorage( SqlStorage *sqlStorage ) { m_sqlStorage = sqlStorage; }
private:
    SqlStorage *m_sqlStorage;
};

/** Private structure of the collection manager */
struct CollectionManager::Private
{
    QList<CollectionPair> collections;
    QList<Plugins::PluginFactory*> factories; // factories belong to PluginManager

    SqlStorage *sqlDatabase;
    SqlStorageWrapper *sqlStorageWrapper;

    QList<Collections::Collection*> unmanagedCollections;

    QList<Collections::TrackProvider*> trackProviders;
    TimecodeTrackProvider *timecodeTrackProvider;
    Collections::TrackProvider *fileTrackProvider; // special case

    Collections::Collection *primaryCollection;

    QReadWriteLock lock; ///< protects all other variables against threading issues
};

CollectionManager *CollectionManager::s_instance = 0;

CollectionManager *
CollectionManager::instance()
{
    if( !s_instance ) {
        s_instance = new CollectionManager();
        s_instance->init();
    }

    return s_instance;
}

void
CollectionManager::destroy()
{
    if( s_instance ) {
        delete s_instance;
        s_instance = 0;
    }
}

CollectionManager::CollectionManager()
    : QObject()
    , d( new Private )
{
    DEBUG_BLOCK
    // ensure this object is created in a main thread
    Q_ASSERT( thread() == QCoreApplication::instance()->thread() );

    setObjectName( "CollectionManager" );
    qRegisterMetaType<SqlStorage *>( "SqlStorage*" );
    d->sqlDatabase = 0;
    d->primaryCollection = 0;
    d->sqlStorageWrapper = new SqlStorageWrapper();
    // special-cased in trackForUrl(), don't add to d->trackProviders yet
    d->fileTrackProvider = new FileTrackProvider();
}

CollectionManager::~CollectionManager()
{
    DEBUG_BLOCK

    {
        QWriteLocker locker( &d->lock );

        //not deleting SqlStorageWrapper here as somebody might be caching it
        //Amarok really needs a proper state management...
        d->sqlStorageWrapper->setSqlStorage( 0 );
        delete d->timecodeTrackProvider;
        delete d->fileTrackProvider;
        d->collections.clear();
        d->unmanagedCollections.clear();
        d->trackProviders.clear();

        // Hmm, qDeleteAll from Qt 4.8 crashes with our SmartPointerList, do it manually. Bug 285951
        while (!d->factories.isEmpty() )
            delete d->factories.takeFirst();
    }

    delete d;
}

void
CollectionManager::init()
{
    //register the timecode track provider now, as it needs to get added before loading
    //the stored playlist... Since it can have playable urls that might also match other providers, it needs to get added first.
    d->timecodeTrackProvider = new TimecodeTrackProvider();
    addTrackProvider( d->timecodeTrackProvider );
}

void
CollectionManager::setFactories( const QList<Plugins::PluginFactory*> &factories )
{
    using Collections::CollectionFactory;


    QSet<Plugins::PluginFactory*> newFactories = factories.toSet();
    QSet<Plugins::PluginFactory*> oldFactories;

    {
        QReadLocker locker( &d->lock );
        oldFactories = d->factories.toSet();
    }

    // remove old factories
    foreach( Plugins::PluginFactory* pFactory, oldFactories - newFactories )
    {

        CollectionFactory *factory = qobject_cast<CollectionFactory*>( pFactory );
        if( !factory )
            continue;

        disconnect( factory, SIGNAL(newCollection(Collections::Collection*)),
                    this, SLOT(slotNewCollection(Collections::Collection*)) );
        {
            QWriteLocker locker( &d->lock );
            d->factories.removeAll( factory );
        }
    }

    // create new factories
    foreach( Plugins::PluginFactory* pFactory, newFactories - oldFactories )
    {
        CollectionFactory *factory = qobject_cast<CollectionFactory*>( pFactory );
        if( !factory )
            continue;

        const KPluginInfo info = factory->info();
        const QString pluginName = info.pluginName();
        const bool useMySqlServer = Amarok::config( "MySQL" ).readEntry( "UseServer", false );

        // the sql collection is a core collection. It cannot be switched off
        // and should be first.
        bool primaryCollection = false;
        if( (useMySqlServer && (pluginName == QLatin1String("amarok_collection-mysqlservercollection"))) ||
            (!useMySqlServer && (pluginName == QLatin1String("amarok_collection-mysqlecollection"))) )
        {
            primaryCollection = true;
        }

        connect( factory, SIGNAL(newCollection(Collections::Collection*)),
                 this, SLOT(slotNewCollection(Collections::Collection*)) );
        {
                QWriteLocker locker( &d->lock );
                if( primaryCollection )
                    d->factories.prepend( factory );
                else
                    d->factories.append( factory );
        }

        debug() << "initializing" << pluginName;
        factory->init();
    }

    d->factories = factories;
}


void
CollectionManager::startFullScan()
{
    QReadLocker locker( &d->lock );

    foreach( const CollectionPair &pair, d->collections )
    {
        QScopedPointer<Capabilities::CollectionScanCapability> csc( pair.first->create<Capabilities::CollectionScanCapability>());
        if( csc )
            csc->startFullScan();
    }
}

void
CollectionManager::startIncrementalScan( const QString &directory )
{
    QReadLocker locker( &d->lock );

    foreach( const CollectionPair &pair, d->collections )
    {
        QScopedPointer<Capabilities::CollectionScanCapability> csc( pair.first->create<Capabilities::CollectionScanCapability>());
        if( csc )
            csc->startIncrementalScan( directory );
    }
}

void
CollectionManager::stopScan()
{
    QReadLocker locker( &d->lock );

    foreach( const CollectionPair &pair, d->collections )
    {
        QScopedPointer<Capabilities::CollectionScanCapability> csc( pair.first->create<Capabilities::CollectionScanCapability>());
        if( csc )
            csc->stopScan();
    }
}

void
CollectionManager::checkCollectionChanges()
{
    startIncrementalScan( QString() );
}

Collections::QueryMaker*
CollectionManager::queryMaker() const
{
    QReadLocker locker( &d->lock );

    QList<Collections::Collection*> colls;
    foreach( const CollectionPair &pair, d->collections )
    {
        if( pair.second & CollectionQueryable )
        {
            colls << pair.first;
        }
    }
    return new Collections::MetaQueryMaker( colls );
}

void
CollectionManager::slotNewCollection( Collections::Collection* newCollection )
{
    DEBUG_BLOCK

    if( !newCollection )
    {
        debug() << "Warning, newCollection in slotNewCollection is 0";
        return;
    }
    {
        QWriteLocker locker( &d->lock );
        foreach( const CollectionPair &p, d->collections )
        {
            if( p.first == newCollection )
            {
                debug() << "Warning, newCollection is already being managed";
                return;
            }
        }
    }

    const QMetaObject *mo = metaObject();
    const QMetaEnum me = mo->enumerator( mo->indexOfEnumerator( "CollectionStatus" ) );
    const QString &value = KGlobal::config()->group( "CollectionManager" ).readEntry( newCollection->collectionId() );
    int enumValue = me.keyToValue( value.toLocal8Bit().constData() );
    CollectionStatus status;
    enumValue == -1 ? status = CollectionEnabled : status = (CollectionStatus) enumValue;
    CollectionPair pair( newCollection, status );

    {
        QWriteLocker locker( &d->lock );
        d->collections.append( pair );
        d->trackProviders.append( newCollection );
        connect( newCollection, SIGNAL(remove()), SLOT(slotRemoveCollection()), Qt::QueuedConnection );
        connect( newCollection, SIGNAL(updated()), SLOT(slotCollectionChanged()), Qt::QueuedConnection );

        // by convention, collections that provide a SQL database have a Qt property called "sqlStorage"
        int propertyIndex = newCollection->metaObject()->indexOfProperty( "sqlStorage" );
        if( propertyIndex != -1 )
        {
            SqlStorage *sqlStorage = newCollection->property( "sqlStorage" ).value<SqlStorage*>();
            if( sqlStorage )
            {
                //let's cheat a bit and assume that sqlStorage and the primaryCollection are always the same
                //it is true for now anyway
                if( d->sqlDatabase )
                {
                    if( d->sqlDatabase->sqlDatabasePriority() < sqlStorage->sqlDatabasePriority() )
                    {
                        d->sqlDatabase = sqlStorage;
                        d->primaryCollection = newCollection;
                        d->sqlStorageWrapper->setSqlStorage( sqlStorage );
                    }
                }
                else
                {
                    d->sqlDatabase = sqlStorage;
                    d->primaryCollection = newCollection;
                    d->sqlStorageWrapper->setSqlStorage( sqlStorage );
                }
            }
            else
            {
                warning() << "Collection " << newCollection->collectionId() << " has sqlStorage property but did not provide a SqlStorage pointer";
            }
        }
    }

    if( status & CollectionViewable )
    {
        emit collectionAdded( newCollection );
        emit collectionAdded( newCollection, status );
    }
}

void
CollectionManager::slotRemoveCollection()
{
    Collections::Collection* collection = qobject_cast<Collections::Collection*>( sender() );
    if( collection )
    {
        CollectionStatus status = collectionStatus( collection->collectionId() );
        CollectionPair pair( collection, status );

        {
            QWriteLocker locker( &d->lock );
            d->collections.removeAll( pair );
            d->trackProviders.removeAll( collection );

            // if the collection had a sql storage, find a new database that could provide
            // one.
            QVariant v = collection->property( "sqlStorage" );
            if( v.isValid() )
            {
                SqlStorage *sqlDb = v.value<SqlStorage*>();
                if( sqlDb && sqlDb == d->sqlDatabase )
                {
                    SqlStorage *newSqlDatabase = 0;
                    foreach( const CollectionPair &pair, d->collections )
                    {
                        QVariant variant = pair.first->property( "sqlStorage" );
                        if( !variant.isValid() )
                            continue;
                        SqlStorage *sqlDb = variant.value<SqlStorage*>();
                        if( sqlDb )
                        {
                            if( newSqlDatabase )
                            {
                                if( newSqlDatabase->sqlDatabasePriority() < sqlDb->sqlDatabasePriority() )
                                    newSqlDatabase = sqlDb;
                            }
                            else
                                newSqlDatabase = sqlDb;
                        }
                    }
                    d->sqlDatabase = newSqlDatabase;
                    d->sqlStorageWrapper->setSqlStorage( newSqlDatabase );
                }
            }
        }

        emit collectionRemoved( collection->collectionId() );
        QTimer::singleShot( 500, collection, SLOT(deleteLater()) ); // give the tree some time to update itself until we really delete the collection pointers.
    }
}

void
CollectionManager::slotCollectionChanged()
{
    Collections::Collection *collection = dynamic_cast<Collections::Collection*>( sender() );
    if( collection )
    {
        CollectionStatus status = collectionStatus( collection->collectionId() );
        if( status & CollectionViewable )
        {
            emit collectionDataChanged( collection );
        }
    }
}

QList<Collections::Collection*>
CollectionManager::viewableCollections() const
{
    QReadLocker locker( &d->lock );

    QList<Collections::Collection*> result;
    foreach( const CollectionPair &pair, d->collections )
    {
        if( pair.second & CollectionViewable )
        {
            result << pair.first;
        }
    }
    return result;
}

QList<Collections::Collection*>
CollectionManager::queryableCollections() const
{
    QReadLocker locker( &d->lock );

    QList<Collections::Collection*> result;
    foreach( const CollectionPair &pair, d->collections )
        if( pair.second & CollectionQueryable )
            result << pair.first;
    return result;

}

Collections::Collection*
CollectionManager::primaryCollection() const
{
    QReadLocker locker( &d->lock );

    return d->primaryCollection;
}

SqlStorage*
CollectionManager::sqlStorage() const
{
    QReadLocker locker( &d->lock );

    return d->sqlStorageWrapper;
}

Meta::TrackList
CollectionManager::tracksForUrls( const KUrl::List &urls )
{
    DEBUG_BLOCK

    debug() << "adding " << urls.size() << " tracks";

    Meta::TrackList tracks;
    foreach( const KUrl &url, urls )
    {
        Meta::TrackPtr track = trackForUrl( url );
        if( track )
            tracks.append( track );
    }
    return tracks;
}

Meta::TrackPtr
CollectionManager::trackForUrl( const KUrl &url )
{
    QReadLocker locker( &d->lock );

    // TODO:
    // might be a podcast, in that case we'll have additional meta information
    // might be a lastfm track, another stream
    if( !url.isValid() )
        return Meta::TrackPtr( 0 );

    foreach( Collections::TrackProvider *provider, d->trackProviders )
    {
        if( provider->possiblyContainsTrack( url ) )
        {
            Meta::TrackPtr track = provider->trackForUrl( url );
            if( track )
                return track;
        }
    }

    // TODO: create specific TrackProviders for these:
    static const QSet<QString> remoteProtocols = QSet<QString>()
            << "http" << "https" << "mms" << "smb"; // consider unifying with TrackLoader::tracksLoaded()
    if( remoteProtocols.contains( url.protocol() ) )
        return Meta::TrackPtr( new MetaStream::Track( url ) );

    /* TODO: add fileTrackProvider to normal providers once tested that the reorder
     * doesn't change behaviour */
    if( d->fileTrackProvider->possiblyContainsTrack( url ) )
    {
        Meta::TrackPtr track = d->fileTrackProvider->trackForUrl( url );
        if( track )
            return track;
    }

    return Meta::TrackPtr( 0 );
}


void
CollectionManager::addUnmanagedCollection( Collections::Collection *newCollection, CollectionStatus defaultStatus )
{
    QWriteLocker locker( &d->lock );

    // TODO: what happens if a collection is managed and then added as unmanaged
    //  or the other way round.
    if( newCollection && d->unmanagedCollections.indexOf( newCollection ) == -1 )
    {
        const QMetaObject *mo = metaObject();
        const QMetaEnum me = mo->enumerator( mo->indexOfEnumerator( "CollectionStatus" ) );
        const QString &value = KGlobal::config()->group( "CollectionManager" ).readEntry( newCollection->collectionId() );
        int enumValue = me.keyToValue( value.toLocal8Bit().constData() );
        CollectionStatus status;
        enumValue == -1 ? status = defaultStatus : status = (CollectionStatus) enumValue;
        d->unmanagedCollections.append( newCollection );
        CollectionPair pair( newCollection, status );
        d->collections.append( pair );
        d->trackProviders.append( newCollection );
        if( status & CollectionViewable )
        {
            emit collectionAdded( newCollection );
            emit collectionAdded( newCollection, status );
        }
        emit trackProviderAdded( newCollection );
    }
}

void
CollectionManager::removeUnmanagedCollection( Collections::Collection *collection )
{
    //do not remove it from collection if it is not in unmanagedCollections
    if( collection && d->unmanagedCollections.removeAll( collection ) )
    {
        CollectionPair pair( collection, collectionStatus( collection->collectionId() ) );
        d->collections.removeAll( pair );
        d->trackProviders.removeAll( collection );
        emit collectionRemoved( collection->collectionId() );
    }
}

CollectionManager::CollectionStatus
CollectionManager::collectionStatus( const QString &collectionId ) const
{
    QReadLocker locker( &d->lock );

    foreach( const CollectionPair &pair, d->collections )
    {
        if( pair.first->collectionId() == collectionId )
        {
            return pair.second;
        }
    }
    return CollectionDisabled;
}

QHash<Collections::Collection*, CollectionManager::CollectionStatus>
CollectionManager::collections() const
{
    QReadLocker locker( &d->lock );

    QHash<Collections::Collection*, CollectionManager::CollectionStatus> result;
    foreach( const CollectionPair &pair, d->collections )
    {
        result.insert( pair.first, pair.second );
    }
    return result;
}

void
CollectionManager::addTrackProvider( Collections::TrackProvider *provider )
{
    {
        QWriteLocker locker( &d->lock );
        d->trackProviders.append( provider );
    }
    emit trackProviderAdded( provider );
}

void
CollectionManager::removeTrackProvider( Collections::TrackProvider *provider )
{
    QWriteLocker locker( &d->lock );
    d->trackProviders.removeAll( provider );
}

Collections::TrackProvider *
CollectionManager::fileTrackProvider()
{
    QReadLocker locker( &d->lock );
    return d->fileTrackProvider;
}
