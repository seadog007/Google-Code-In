/****************************************************************************************
 * Copyright (c) 2007-2010 Bart Cerneels <bart.cerneels@kde.org>                        *
 * Copyright (c) 2007-2008 Nikolaj Hald Nielsen <nhn@kde.org>                           *
 * Copyright (c) 2007 Henry de Valence <hdevalence@gmail.com>                           *
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

#define DEBUG_PREFIX "PodcastCategory"

#include "PodcastCategory.h"

#include "amarokconfig.h"
#include "amarokurls/AmarokUrl.h"
#include "App.h"
#include "browsers/InfoProxy.h"
#include "core/support/Debug.h"
#include "core/meta/support/MetaUtility.h"
#include "PaletteHandler.h"
#include "PodcastModel.h"
#include "PlaylistBrowserView.h"
#include "widgets/PrettyTreeRoles.h"

#include <QModelIndexList>
#include <QTextBrowser>

#include <KAction>
#include <KIcon>
#include <KStandardDirs>
#include <KUrlRequesterDialog>
#include <KGlobal>
#include <KLocale>
#include <KToolBar>

namespace The
{
    PlaylistBrowserNS::PodcastCategory* podcastCategory()
    {
        return PlaylistBrowserNS::PodcastCategory::instance();
    }
}

using namespace PlaylistBrowserNS;

QString PodcastCategory::s_configGroup( "Podcast View" );

PodcastCategory* PodcastCategory::s_instance = 0;

PodcastCategory*
PodcastCategory::instance()
{
    return s_instance ? s_instance : new PodcastCategory( 0 );
}

void
PodcastCategory::destroy()
{
    if( s_instance )
    {
        delete s_instance;
        s_instance = 0;
    }
}

PodcastCategory::PodcastCategory( QWidget *parent )
    : PlaylistBrowserCategory( Playlists::PodcastChannelPlaylist,
                               "podcasts",
                               s_configGroup,
                               The::podcastModel(),
                               parent )
{
    setPrettyName( i18n( "Podcasts" ) );
    setShortDescription( i18n( "List of podcast subscriptions and episodes" ) );
    setIcon( KIcon( "podcast-amarok" ) );

    setLongDescription( i18n( "Manage your podcast subscriptions and browse individual episodes. "
                              "Downloading episodes to the disk is also done here, or you can tell "
                              "Amarok to do this automatically." ) );

    setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_podcasts.png" ) );

    // set background
    if( AmarokConfig::showBrowserBackgroundImage() )
        setBackgroundImage( imagePath() );

    QAction *addPodcastAction = new QAction( KIcon( "list-add-amarok" ), i18n("&Add Podcast"),
                                             m_toolBar );
    addPodcastAction->setPriority( QAction::NormalPriority );
    m_toolBar->insertAction( m_separator, addPodcastAction );
    connect( addPodcastAction, SIGNAL(triggered(bool)), The::podcastModel(), SLOT(addPodcast()) );

    QAction *updateAllAction = new QAction( KIcon("view-refresh-amarok"), QString(), m_toolBar );
    updateAllAction->setToolTip( i18n("&Update All") );
    updateAllAction->setPriority( QAction::LowPriority );
    m_toolBar->insertAction( m_separator, updateAllAction );
    connect( updateAllAction, SIGNAL(triggered(bool)),
             The::podcastModel(), SLOT(refreshPodcasts()) );


    QAction *importOpmlAction = new QAction( KIcon("document-import")
                                             , i18n( "Import OPML File" )
                                             , m_toolBar
                                         );
    importOpmlAction->setToolTip( i18n( "Import OPML File" ) );
    importOpmlAction->setPriority( QAction::LowPriority );
    m_toolBar->addAction( importOpmlAction );
    connect( importOpmlAction, SIGNAL(triggered()), SLOT(slotImportOpml()) );

    PlaylistBrowserView *view = static_cast<PlaylistBrowserView*>( playlistView() );
    connect( view, SIGNAL(currentItemChanged(QModelIndex)), SLOT(showInfo(QModelIndex)) );

    //transparency
//    QPalette p = m_podcastTreeView->palette();
//    QColor c = p.color( QPalette::Base );
//    c.setAlpha( 0 );
//    p.setColor( QPalette::Base, c );
//
//    c = p.color( QPalette::AlternateBase );
//    c.setAlpha( 77 );
//    p.setColor( QPalette::AlternateBase, c );
//
//    m_podcastTreeView->setPalette( p );
//
//    QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
//    sizePolicy1.setHorizontalStretch(0);
//    sizePolicy1.setVerticalStretch(0);
//    sizePolicy1.setHeightForWidth(m_podcastTreeView->sizePolicy().hasHeightForWidth());
//    m_podcastTreeView->setSizePolicy(sizePolicy1);
}

PodcastCategory::~PodcastCategory()
{
}

void
PodcastCategory::showInfo( const QModelIndex &index )
{
    if( !index.isValid() )
        return;

    const int row = index.row();
    QString description;
    QString title( index.data( Qt::DisplayRole ).toString() );
    QString subtitle( index.sibling( row, SubtitleColumn ).data( Qt::DisplayRole ).toString() );
    KUrl imageUrl( qvariant_cast<KUrl>(
        index.sibling( row, ImageColumn ).data( Qt::DisplayRole )
    ) );
    QString author( index.sibling( row, AuthorColumn ).data( Qt::DisplayRole ).toString() );
    QStringList keywords( qvariant_cast<QStringList>(
        index.sibling( row, KeywordsColumn ).data( Qt::DisplayRole )
    ) );
    bool isEpisode = index.sibling( row, IsEpisodeColumn ).data( Qt::DisplayRole ).toBool();
    QString authorAndPubDate;
    
    if( !author.isEmpty() )
    {
        authorAndPubDate = QString( "<b>%1</b> %2 " )
            .arg( i18n( "By" ) )
            .arg( Qt::escape( author ) );
    }

    if( !subtitle.isEmpty() )
    {
        description += QString( "<h1 class=\"subtitle\">%1</h1>" )
            .arg( Qt::escape( subtitle ) );
    }

    if( !imageUrl.isEmpty() )
    {
        description += QString( "<p style=\"float:right;\"><img src=\"%1\" onclick=\""
            "if (this.style.width=='150px') {"
                "this.style.width='auto';"
                "this.style.marginLeft='0em';"
                "this.style.cursor='-webkit-zoom-out';"
                "this.parentNode.style.float='inherit';"
                "this.parentNode.style.textAlign='center';"
            "} else {"
                "this.style.width='150px';"
                "this.style.marginLeft='1em';"
                "this.style.cursor='-webkit-zoom-in';"
                "this.parentNode.style.float='right';"
                "this.parentNode.style.textAlign='inherit';"
            "}\""
            " style=\"width: 150px; margin-left: 1em;"
            " margin-right: 0em; cursor: -webkit-zoom-in;\""
            "/></p>" )
            .arg( Qt::escape( imageUrl.url() ) );
    }

    if( isEpisode )
    {
        QDateTime pubDate( index.sibling( row, DateColumn ).data( Qt::DisplayRole ).toDateTime() );
        
        if( pubDate.isValid() )
        {
            authorAndPubDate += QString( "<b>%1</b> %2" )
                .arg( i18nc( "Podcast published on date", "On" ) )
                .arg( KGlobal::locale()->formatDateTime( pubDate, KLocale::FancyShortDate ) );
        }
    }

    if( !authorAndPubDate.isEmpty() )
    {
        description += QString( "<p>%1</p>" )
            .arg( authorAndPubDate );
    }

    if( isEpisode )
    {
        int fileSize = index.sibling( row, FilesizeColumn ).data( Qt::DisplayRole ).toInt();

        if( fileSize != 0 )
        {
            description += QString( "<p><b>%1</b> %2</p>" )
                .arg( i18n( "File Size:" ) )
                .arg( Meta::prettyFilesize( fileSize ) );
        }

    }
    else
    {
        QDate subsDate( index.sibling( row, DateColumn ).data( Qt::DisplayRole ).toDate() );
        
        if( subsDate.isValid() )
        {
            description += QString( "<p><b>%1</b> %2</p>" )
                .arg( i18n( "Subscription Date:" ) )
                .arg( KGlobal::locale()->formatDate( subsDate, KLocale::FancyShortDate ) );
        }
    }

    if( !keywords.isEmpty() )
    {
        description += QString( "<p><b>%1</b> %2</p>" )
            .arg( i18n( "Keywords:" ) )
            .arg( Qt::escape( keywords.join( ", " ) ) );
    }

    description += index.data( PrettyTreeRoles::ByLineRole ).toString();

    description = QString(
        "<html>"
        "    <head>"
        "        <title>%1</title>"
        "        <style type=\"text/css\">"
        "body {color: %3;}"
        "::selection {background-color: %4;}"
        "h1 {text-align:center; font-size: 1.2em;}"
        "h1.subtitle {text-align:center; font-size: 1em; font-weight: normal;}"
        "        </style>"
        "    </head>"
        "    <body>"
        "        <h1>%1</h1>"
        "        %2"
        "    </body>"
        "</html>")
        .arg( Qt::escape( title ) )
        .arg( description )
        .arg( App::instance()->palette().brush( QPalette::Text ).color().name() )
        .arg( PaletteHandler::highlightColor().name() );
    
    QVariantMap map;
    map["service_name"] = title;
    map["main_info"] = description;
    The::infoProxy()->setInfo( map );
}

void
PodcastCategory::slotImportOpml()
{
    AmarokUrl( "amarok://service-podcastdirectory/addOpml" ).run();
}

#include "PodcastCategory.moc"
