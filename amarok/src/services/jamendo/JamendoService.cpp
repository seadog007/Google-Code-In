/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#include "JamendoService.h"

#include "browsers/CollectionTreeItem.h"
#include "browsers/CollectionTreeView.h"
#include "browsers/SingleCollectionTreeItemModel.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "core/support/Debug.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "EngineController.h"
#include "JamendoInfoParser.h"
#include "ServiceSqlRegistry.h"
#include "widgets/SearchWidget.h"

#include <KAction>
#include <KFileDialog>
#include <KMenuBar>
#include <KRun>
#include <KShell>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <threadweaver/ThreadWeaver.h>

#include <QDesktopServices>
#include <QToolBar>
#include <QToolButton>

#include <typeinfo>

using namespace Meta;

AMAROK_EXPORT_SERVICE_PLUGIN( jamendo, JamendoServiceFactory )

JamendoServiceFactory::JamendoServiceFactory( QObject *parent, const QVariantList &args )
    : ServiceFactory( parent, args )
{
    KPluginInfo pluginInfo(  "amarok_service_jamendo.desktop", "services" );
    pluginInfo.setConfig( config() );
    m_info = pluginInfo;
}

void JamendoServiceFactory::init()
{
    ServiceBase* service = new JamendoService( this, "Jamendo.com" );
    m_initialized = true;
    emit newService( service );
}

QString
JamendoServiceFactory::name()
{
    return "Jamendo.com";
}

KConfigGroup
JamendoServiceFactory::config()
{
    return Amarok::config( "Service_Jamendo" );
}

JamendoService::JamendoService( JamendoServiceFactory* parent, const QString & name)
    : ServiceBase( name, parent )
    , m_currentAlbum( 0 )
    , m_xmlParser( 0 )
{
    setShortDescription(  i18n( "A archive of free, Creative Commons licensed music" ) );
    setIcon( KIcon( "view-services-jamendo-amarok" ) );

    setLongDescription( i18n( "Jamendo.com puts artists and music lovers in touch with each other. The site allows artists to upload their own albums to share them with the world and users to download all of them for free. Listen to and download all Jamendo.com contents from within Amarok." ) );

setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_jamendo.png" ) );

    ServiceMetaFactory * metaFactory = new JamendoMetaFactory( "jamendo", this );
    ServiceSqlRegistry * registry = new ServiceSqlRegistry( metaFactory );
    m_collection = new Collections::ServiceSqlCollection( "jamendo", "Jamendo.com", metaFactory, registry );
    CollectionManager::instance()->addUnmanagedCollection( m_collection, CollectionManager::CollectionDisabled );
    setServiceReady( true );
}

JamendoService::~JamendoService()
{
    DEBUG_BLOCK
    
    //if currently running, stop it or we will get crashes
    if( m_xmlParser ) {
        m_xmlParser->requestAbort();
        delete m_xmlParser;
        m_xmlParser = 0;
    }
}

void
JamendoService::polish()
{
    generateWidgetInfo();
    if ( m_polished )
        return;

    KHBox * bottomPanelLayout = new KHBox;
    bottomPanelLayout->setParent( m_bottomPanel );

    m_updateListButton = new QPushButton;
    m_updateListButton->setParent( bottomPanelLayout );
    m_updateListButton->setText( i18nc( "Fetch new information from the website", "Update" ) );
    m_updateListButton->setObjectName( "updateButton" );
    m_updateListButton->setIcon( KIcon( "view-refresh-amarok" ) );

    m_downloadButton = new QPushButton;
    m_downloadButton->setParent( bottomPanelLayout );
    m_downloadButton->setText( i18n( "Download" ) );
    m_downloadButton->setObjectName( "downloadButton" );
    m_downloadButton->setIcon( KIcon( "download-amarok" ) );
    m_downloadButton->setEnabled( false );

    connect( m_updateListButton, SIGNAL(clicked()), this, SLOT(updateButtonClicked()) );
    connect( m_downloadButton, SIGNAL(clicked()), this, SLOT(download()) );

    setInfoParser( new JamendoInfoParser() );

    QList<CategoryId::CatMenuId> levels;
    levels << CategoryId::Genre << CategoryId::Artist << CategoryId::Album;

    setModel( new SingleCollectionTreeItemModel( m_collection, levels ) );

    connect( m_contentView, SIGNAL(itemSelected(CollectionTreeItem*)), this, SLOT(itemSelected(CollectionTreeItem*)) );

    QMenu *filterMenu = new QMenu( 0 );

//     QAction *action = filterMenu->addAction( i18n("Artist") );
//     connect( action, SIGNAL(triggered(bool)), SLOT(sortByArtist()) );
// 
//     action = filterMenu->addAction( i18n( "Artist / Album" ) );
//     connect( action, SIGNAL(triggered(bool)), SLOT(sortByArtistAlbum()) );
// 
//     action = filterMenu->addAction( i18n( "Album" ) );
//     connect( action, SIGNAL(triggered(bool)), SLOT(sortByAlbum()) );

    QAction *action = filterMenu->addAction( i18n( "Genre / Artist" ) );
    connect( action, SIGNAL(triggered(bool)), SLOT(sortByGenreArtist()) );

    action = filterMenu->addAction( i18n( "Genre / Artist / Album" ) );
    connect( action, SIGNAL(triggered(bool)), SLOT(sortByGenreArtistAlbum()) );

    KAction *filterMenuAction = new KAction( KIcon( "preferences-other" ), i18n( "Sort Options" ), this );
    filterMenuAction->setMenu( filterMenu );

    m_searchWidget->toolBar()->addSeparator();
    m_searchWidget->toolBar()->addAction( filterMenuAction );

    QToolButton *tbutton = qobject_cast< QToolButton* >( m_searchWidget->toolBar()->widgetForAction( filterMenuAction ) );
    if( tbutton )
        tbutton->setPopupMode( QToolButton::InstantPopup );

//     m_menubar->show();

    m_polished = true;
}

void
JamendoService::updateButtonClicked()
{
    m_updateListButton->setEnabled( false );

    debug() << "JamendoService: start downloading xml file";

    KTemporaryFile tempFile;
    tempFile.setSuffix( ".gz" );
    tempFile.setAutoRemove( false );  //file will be removed in JamendoXmlParser
    if( !tempFile.open() )
        return; //error
    m_tempFileName = tempFile.fileName();
    m_listDownloadJob = KIO::file_copy(
                /* Deprecated */ KUrl( "http://imgjam.com/data/dbdump_artistalbumtrack.xml.gz" ),
                KUrl( m_tempFileName ), 0700 , KIO::HideProgressInfo | KIO::Overwrite );

    Amarok::Components::logger()->newProgressOperation( m_listDownloadJob, i18n( "Downloading Jamendo.com database..." ), this, SLOT(listDownloadCancelled()) );

    connect( m_listDownloadJob, SIGNAL(result(KJob*)),
            this, SLOT(listDownloadComplete(KJob*)) );
}

void
JamendoService::listDownloadComplete(KJob * downloadJob)
{
    if( downloadJob != m_listDownloadJob )
        return ; //not the right job, so let's ignore it
    debug() << "JamendoService: xml file download complete";

    m_listDownloadJob = 0;
    //testing
    if ( downloadJob->error() != 0 )
    {
        //TODO: error handling here
        m_updateListButton->setEnabled( true ); // otherwise button will remain inactive in case of error
        return;
    }

    Amarok::Components::logger()->shortMessage( i18n( "Updating the local Jamendo database."  ) );
    debug() << "JamendoService: create xml parser";

    if( m_xmlParser == 0 )
        m_xmlParser = new JamendoXmlParser( m_tempFileName );
    connect( m_xmlParser, SIGNAL(doneParsing()), SLOT(doneParsing()) );

    ThreadWeaver::Weaver::instance()->enqueue( m_xmlParser );
    downloadJob->deleteLater();
}

void
JamendoService::listDownloadCancelled()
{
    m_listDownloadJob->kill();
    m_listDownloadJob = 0;
    debug() << "Aborted xml download";

    m_updateListButton->setEnabled( true );
}

void
JamendoService::doneParsing()
{
    debug() << "JamendoService: done parsing";
    m_updateListButton->setEnabled( true );
    // model->setGenre("All");
    //delete sender
    sender()->deleteLater();
    m_xmlParser = 0;
    m_collection->emitUpdated();
}

void
JamendoService::itemSelected( CollectionTreeItem * selectedItem )
{
    DEBUG_BLOCK

    //we only enable the download button if there is only one item selected and it happens to
    //be an album or a track
    DataPtr dataPtr = selectedItem->data();

    if ( typeid( *dataPtr.data() ) == typeid( JamendoTrack ) )
    {
        debug() << "is right type (track)";
        JamendoTrack * track = static_cast<JamendoTrack *> ( dataPtr.data() );
        m_currentAlbum = static_cast<JamendoAlbum *> ( track->album().data() );
        m_downloadButton->setEnabled( true );
    }
    else if ( typeid( * dataPtr.data() ) == typeid( JamendoAlbum ) )
    {
        m_currentAlbum = static_cast<JamendoAlbum *> ( dataPtr.data() );
        debug() << "is right type (album) named " << m_currentAlbum->name();
        m_downloadButton->setEnabled( true );
    }
    else
    {
        debug() << "is wrong type";
        m_downloadButton->setEnabled( false );
    }
    return;
}

void
JamendoService::download() // SLOT
{
    DEBUG_BLOCK

    if ( !m_polished )
        polish();

    CollectionTreeView *treeView = static_cast<CollectionTreeView*>( view() );
    treeView->copySelectedToLocalCollection();
}


#include "JamendoService.moc"

