/****************************************************************************************
 * Copyright (c) 2006-2007 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
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

#define DEBUG_PREFIX "MountPointManager"

#include "MountPointManager.h"

#include "MediaDeviceCache.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "core/collections/support/SqlStorage.h"
#include "core-impl/collections/db/sql/device/massstorage/MassStorageDeviceHandler.h"
#include "core-impl/collections/db/sql/device/nfs/NfsDeviceHandler.h"
#include "core-impl/collections/db/sql/device/smb/SmbDeviceHandler.h"

#include <KConfigGroup>
#include <Solid/Predicate>
#include <Solid/Device>

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QList>
#include <QStringList>
#include <QTimer>

MountPointManager::MountPointManager( QObject *parent, SqlStorage *storage )
    : QObject( parent )
    , m_storage( storage )
    , m_ready( false )
{
    DEBUG_BLOCK
    setObjectName( "MountPointManager" );

    if ( !Amarok::config( "Collection" ).readEntry( "DynamicCollection", true ) )
    {
        debug() << "Dynamic Collection deactivated in amarokrc, not loading plugins, not connecting signals";
        m_ready = true;
        handleMusicLocation();
        return;
    }

    connect( MediaDeviceCache::instance(), SIGNAL(deviceAdded(QString)), SLOT(deviceAdded(QString)) );
    connect( MediaDeviceCache::instance(), SIGNAL(deviceRemoved(QString)), SLOT(deviceRemoved(QString)) );

    createDeviceFactories();
}

void
MountPointManager::handleMusicLocation()
{
    // For users who were using QDesktopServices::MusicLocation exclusively up
    // to v2.2.2, which did not store the location into config.
    // and also for versions up to 2.7-git that did wrote the Use MusicLocation entry

    KConfigGroup folders = Amarok::config( "Collection Folders" );
    const QString entryKey( "Use MusicLocation" );
    if( !folders.hasKey( entryKey ) )
        return; // good, already solved, nothing to do

    // write the music location as another collection folder in this case
    if( folders.readEntry( entryKey, false ) )
    {
        const KUrl musicUrl = QDesktopServices::storageLocation( QDesktopServices::MusicLocation );
        const QString musicDir = musicUrl.toLocalFile( KUrl::RemoveTrailingSlash );
        const QDir dir( musicDir );
        if( dir.exists() && dir.isReadable() )
        {
            QStringList currentFolders = collectionFolders();
            if( !currentFolders.contains( musicDir ) )
                setCollectionFolders( currentFolders << musicDir );
        }
    }

    folders.deleteEntry( entryKey ); // get rid of it for good
}

MountPointManager::~MountPointManager()
{
    DEBUG_BLOCK

    m_handlerMapMutex.lock();
    foreach( DeviceHandler *dh, m_handlerMap )
        delete dh;
    m_handlerMapMutex.unlock();

    // DeviceHandlerFactories are memory managed using QObject parentship
}


void
MountPointManager::createDeviceFactories()
{
    DEBUG_BLOCK
    QList<DeviceHandlerFactory*> factories;
    factories << new MassStorageDeviceHandlerFactory( this );
    factories << new NfsDeviceHandlerFactory( this );
    factories << new SmbDeviceHandlerFactory( this );
    foreach( DeviceHandlerFactory *factory, factories )
    {
        debug() << "Initializing DeviceHandlerFactory of type:" << factory->type();
        if( factory->canCreateFromMedium() )
            m_mediumFactories.append( factory );
        else if (factory->canCreateFromConfig() )
            m_remoteFactories.append( factory );
        else //FIXME max: better error message
            debug() << "Unknown DeviceHandlerFactory";
    }

    Solid::Predicate predicate = Solid::Predicate( Solid::DeviceInterface::StorageAccess );
    QList<Solid::Device> devices = Solid::Device::listFromQuery( predicate );
    foreach( const Solid::Device &device, devices )
        createHandlerFromDevice( device, device.udi() );

    m_ready = true;
    handleMusicLocation();
}

int
MountPointManager::getIdForUrl( const KUrl &url )
{
    int mountPointLength = 0;
    int id = -1;
    m_handlerMapMutex.lock();
    foreach( DeviceHandler *dh, m_handlerMap )
    {
        if ( url.path().startsWith( dh->getDevicePath() ) && mountPointLength < dh->getDevicePath().length() )
        {
            id = m_handlerMap.key( dh );
            mountPointLength = dh->getDevicePath().length();
        }
    }
    m_handlerMapMutex.unlock();
    if ( mountPointLength > 0 )
    {
        return id;
    }
    else
    {
        //default fallback if we could not identify the mount point.
        //treat -1 as mount point / in all other methods
        return -1;
    }
}

bool
MountPointManager::isMounted( const int deviceId ) const
{
    m_handlerMapMutex.lock();
    const bool result = m_handlerMap.contains( deviceId );
    m_handlerMapMutex.unlock();
    return result;
}

QString
MountPointManager::getMountPointForId( const int id ) const
{
    QString mountPoint;
    if ( isMounted( id ) )
    {
        m_handlerMapMutex.lock();
        mountPoint = m_handlerMap[id]->getDevicePath();
        m_handlerMapMutex.unlock();
    }
    else
        //TODO better error handling
        mountPoint = '/';
    return mountPoint;
}

QString
MountPointManager::getAbsolutePath( const int deviceId, const QString& relativePath ) const
{
    // TODO: someone who clearly understands KUrl should clean this up.
    KUrl rpath;
    rpath.setPath( relativePath );
    KUrl absolutePath;

    // debug() << "id is " << deviceId << ", relative path is " << relativePath;
    if ( deviceId == -1 )
    {
#ifdef Q_OS_WIN32
        absolutePath.setPath( rpath.toLocalFile() );
#else
        absolutePath.setPath( "/" );
        absolutePath.addPath( rpath.path() );
#endif
        absolutePath.cleanPath();
        // debug() << "Deviceid is -1, using relative Path as absolute Path, returning " << absolutePath.path();
    }
    else
    {
        m_handlerMapMutex.lock();
        if ( m_handlerMap.contains( deviceId ) )
        {
            m_handlerMap[deviceId]->getURL( absolutePath, rpath );
            m_handlerMapMutex.unlock();
        }
        else
        {
            m_handlerMapMutex.unlock();
            const QStringList lastMountPoint = m_storage->query(
                                                                QString( "SELECT lastmountpoint FROM devices WHERE id = %1" )
                                                                .arg( deviceId ) );
            if ( lastMountPoint.count() == 0 )
            {
                //hmm, no device with that id in the DB...serious problem
                warning() << "Device " << deviceId << " not in database, this should never happen!";
                return getAbsolutePath( -1, relativePath );
            }
            else
            {
                absolutePath.setPath( lastMountPoint.first() );
                absolutePath.addPath( rpath.path() );
                absolutePath.cleanPath();
                //debug() << "Device " << deviceId << " not mounted, using last mount point and returning " << absolutePath.path();
            }
        }
    }

    #ifdef Q_OS_WIN32
        return absolutePath.toLocalFile();
    #else
        return absolutePath.path();
    #endif
}

QString
MountPointManager::getRelativePath( const int deviceId, const QString& absolutePath ) const
{
    QMutexLocker locker(&m_handlerMapMutex);
    if ( deviceId != -1 && m_handlerMap.contains( deviceId ) )
    {
        //FIXME max: returns garbage if the absolute path is actually not under the device's mount point
        return KUrl::relativePath( m_handlerMap[deviceId]->getDevicePath(), absolutePath );
    }
    else
    {
        //TODO: better error handling
#ifdef Q_OS_WIN32
        return KUrl( absolutePath ).toLocalFile();
#else
        return KUrl::relativePath( "/", absolutePath );
#endif
    }
}

IdList
MountPointManager::getMountedDeviceIds() const
{
    m_handlerMapMutex.lock();
    IdList list( m_handlerMap.keys() );
    m_handlerMapMutex.unlock();
    list.append( -1 );
    return list;
}

QStringList
MountPointManager::collectionFolders() const
{
    if( !m_ready )
    {
        debug() << "requested collectionFolders from MountPointManager that is not yet ready";
        return QStringList();
    }

    //TODO max: cache data
    QStringList result;
    KConfigGroup folders = Amarok::config( "Collection Folders" );
    const IdList ids = getMountedDeviceIds();

    foreach( int id, ids )
    {
        const QStringList rpaths = folders.readEntry( QString::number( id ), QStringList() );
        foreach( const QString &strIt, rpaths )
        {
            const KUrl url = ( strIt == "./" ) ? getMountPointForId( id ) : getAbsolutePath( id, strIt );
            const QString absPath = url.toLocalFile( KUrl::RemoveTrailingSlash );
            if ( !result.contains( absPath ) )
                result.append( absPath );
        }
    }

    return result;
}

void
MountPointManager::setCollectionFolders( const QStringList &folders )
{
    typedef QMap<int, QStringList> FolderMap;
    KConfigGroup folderConf = Amarok::config( "Collection Folders" );
    FolderMap folderMap;

    foreach( const QString &folder, folders )
    {
        int id = getIdForUrl( folder );
        const QString rpath = getRelativePath( id, folder );
        if( folderMap.contains( id ) ) {
            if( !folderMap[id].contains( rpath ) )
                folderMap[id].append( rpath );
        }
        else
            folderMap[id] = QStringList( rpath );
    }
    //make sure that collection folders on devices which are not in foldermap are deleted
    IdList ids = getMountedDeviceIds();
    foreach( int deviceId, ids )
    {
        if( !folderMap.contains( deviceId ) )
        {
            folderConf.deleteEntry( QString::number( deviceId ) );
        }
    }
    QMapIterator<int, QStringList> i( folderMap );
    while( i.hasNext() )
    {
        i.next();
        folderConf.writeEntry( QString::number( i.key() ), i.value() );
    }
}

void
MountPointManager::deviceAdded( const QString &udi )
{
    DEBUG_BLOCK
    Solid::Predicate predicate = Solid::Predicate( Solid::DeviceInterface::StorageAccess );
    QList<Solid::Device> devices = Solid::Device::listFromQuery( predicate );
    //Looking for a specific udi in predicate seems flaky/buggy; the foreach loop barely
    //takes any time, so just be safe
    bool found = false;
    debug() << "looking for udi " << udi;
    foreach( const Solid::Device &device, devices )
    {
        if( device.udi() == udi )
        {
            createHandlerFromDevice( device, udi );
            found = true;
        }
    }
    if( !found )
        debug() << "Did not find device from Solid for udi " << udi;
}

void
MountPointManager::deviceRemoved( const QString &udi )
{
    DEBUG_BLOCK
    m_handlerMapMutex.lock();
    foreach( DeviceHandler *dh, m_handlerMap )
    {
        if( dh->deviceMatchesUdi( udi ) )
        {
            int key = m_handlerMap.key( dh );
            m_handlerMap.remove( key );
            delete dh;
            debug() << "removed device " << key;
            m_handlerMapMutex.unlock();
            //we found the medium which was removed, so we can abort the loop
            emit deviceRemoved( key );
            return;
        }
    }
    m_handlerMapMutex.unlock();
}

void MountPointManager::createHandlerFromDevice( const Solid::Device& device, const QString &udi )
{
    DEBUG_BLOCK
    if ( device.isValid() )
    {
        debug() << "Device added and mounted, checking handlers";
        foreach( DeviceHandlerFactory *factory, m_mediumFactories )
        {
            if( factory->canHandle( device ) )
            {
                debug() << "found handler for " << udi;
                DeviceHandler *handler = factory->createHandler( device, udi, m_storage );
                if( !handler )
                {
                    debug() << "Factory " << factory->type() << "could not create device handler";
                    break;
                }
                int key = handler->getDeviceID();
                m_handlerMapMutex.lock();
                if( m_handlerMap.contains( key ) )
                {
                    debug() << "Key " << key << " already exists in handlerMap, replacing";
                    delete m_handlerMap[key];
                    m_handlerMap.remove( key );
                }
                m_handlerMap.insert( key, handler );
                m_handlerMapMutex.unlock();
//                 debug() << "added device " << key << " with mount point " << volumeAccess->mountPoint();
                emit deviceAdded( key );
                break;  //we found the added medium and don't have to check the other device handlers
            }
            else
                debug() << "Factory can't handle device " << udi;
        }
    }
    else
        debug() << "Device not valid!";
}
