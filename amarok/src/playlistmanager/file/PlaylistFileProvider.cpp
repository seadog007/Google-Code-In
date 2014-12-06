/****************************************************************************************
 * Copyright (c) 2007 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#include "PlaylistFileProvider.h"
#include "App.h"
#include "core-impl/playlists/types/file/PlaylistFileSupport.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "core-impl/playlists/types/file/asx/ASXPlaylist.h"
#include "core-impl/playlists/types/file/m3u/M3UPlaylist.h"
#include "core-impl/playlists/types/file/pls/PLSPlaylist.h"
#include "core-impl/playlists/types/file/xspf/XSPFPlaylist.h"
#include "playlist/PlaylistModelStack.h"
#include "playlistmanager/PlaylistManager.h"

#include <KDialog>
#include <KInputDialog>
#include <KLocale>
#include <KUrl>

#include <QAction>
#include <QDir>
#include <QLabel>
#include <QString>
#include <QTimer>

using Playlist::ModelStack;

namespace Playlists {

PlaylistFileProvider::PlaylistFileProvider()
 : UserPlaylistProvider()
 , m_playlistsLoaded( false )
 , m_saveLaterTimer( 0 )
{
    //playlists are lazy loaded but we can count how many we'll load already
    QStringList keys = loadedPlaylistsConfig().keyList();
    foreach( const QString &key, keys )
    {
        KUrl url( key );
        //Don't load these from the config file, they are read from the directory anyway
        if( url.upUrl().equals( Amarok::saveLocation( "playlists" ) ) )
            continue;
        m_urlsToLoad << url;
    }
    //also add all files in the $KDEHOME/share/apps/amarok/playlists
    QDir playlistDir = QDir( Amarok::saveLocation( "playlists" ), "",
                             QDir::Name,
                             QDir::Files | QDir::Readable );
    foreach( const QString &file, playlistDir.entryList() )
    {
        KUrl url( playlistDir.path() );
        url.addPath( file );
        if( Playlists::isPlaylist( url ) )
            m_urlsToLoad << url;
    }
}

PlaylistFileProvider::~PlaylistFileProvider()
{
    DEBUG_BLOCK
    //remove all, well add them again soon
    loadedPlaylistsConfig().deleteGroup();
    //Write loaded playlists to config file
    foreach( Playlists::PlaylistFilePtr playlistFile, m_playlists )
    {
        KUrl url = playlistFile->uidUrl();
        //only save files NOT in "playlists", those are automatically loaded.
        if( url.upUrl().equals( Amarok::saveLocation( "playlists" ) ) )
            continue;

        //debug() << "storing to rc-file: " << url.url();

        loadedPlaylistsConfig().writeEntry( url.url(), playlistFile->groups() );
    }
    loadedPlaylistsConfig().sync();
}

QString
PlaylistFileProvider::prettyName() const
{
    return i18n( "Playlist Files on Disk" );
}

KIcon PlaylistFileProvider::icon() const
{
    return KIcon( "folder-documents" );
}

int
PlaylistFileProvider::playlistCount() const
{
    return m_playlists.count() + m_urlsToLoad.count();
}

Playlists::PlaylistList
PlaylistFileProvider::playlists()
{
    Playlists::PlaylistList playlists;

    if( !m_playlistsLoaded )
    {
        //trigger a lazy load the playlists
        QTimer::singleShot(0, this, SLOT(loadPlaylists()) );
        return playlists;
    }

    foreach( const Playlists::PlaylistFilePtr &playlistFile, m_playlists )
    {
        Playlists::PlaylistPtr playlist = Playlists::PlaylistPtr::dynamicCast( playlistFile );
        if( !playlist.isNull() )
            playlists << playlist;
    }
    return playlists;
}

Playlists::PlaylistPtr
PlaylistFileProvider::save( const Meta::TrackList &tracks, const QString &name )
{
    DEBUG_BLOCK

    QString filename = name.isEmpty() ? QDateTime::currentDateTime().toString( "ddd MMMM d yy hh-mm") : name;
    filename.replace( QLatin1Char('/'), QLatin1Char('-') );
    filename.replace( QLatin1Char('\\'), QLatin1Char('-') );

    Playlists::PlaylistFormat format = Playlists::getFormat( filename );
    if( format == Playlists::Unknown ) // maybe the name just had a dot in it. We just add .xspf
    {
        format = Playlists::XSPF;
        filename.append( QLatin1String( ".xspf" ) );
    }

    KUrl path( Amarok::saveLocation( "playlists" ) );
    path.addPath( Amarok::vfatPath( filename ) );
    if( QFileInfo( path.toLocalFile() ).exists() )
    {
        //TODO:request overwrite
        return Playlists::PlaylistPtr();
    }

    Playlists::PlaylistFile *playlistFile = 0;
    switch( format )
    {
        case Playlists::ASX:
            playlistFile = new Playlists::ASXPlaylist( path, this );
            break;
        case Playlists::PLS:
            playlistFile = new Playlists::PLSPlaylist( path, this );
            break;
        case Playlists::M3U:
            playlistFile = new Playlists::M3UPlaylist( path, this );
            break;
        case Playlists::XSPF:
            playlistFile = new Playlists::XSPFPlaylist( path, this );
            break;
        case Playlists::XML:
        case Playlists::RAM:
        case Playlists::SMIL:
        case Playlists::Unknown:
            // this should not happen since we set the format to XSPF above.
            return Playlists::PlaylistPtr();
    }
    playlistFile->setName( filename );
    playlistFile->addTracks( tracks );
    playlistFile->save( true );

    Playlists::PlaylistFilePtr playlistPtr( playlistFile );
    m_playlists << playlistPtr;
    //just in case there wasn't one loaded before.
    m_playlistsLoaded = true;

    Playlists::PlaylistPtr playlist( playlistFile );
    emit playlistAdded( playlist );
    return playlist;
}

bool
PlaylistFileProvider::import( const KUrl &path )
{
    DEBUG_BLOCK
    if( !path.isValid() )
    {
        error() << "path is not valid!";
        return false;
    }

    foreach( Playlists::PlaylistFilePtr playlistFile, m_playlists )
    {
        if( !playlistFile )
        {
            error() << "Could not cast down.";
            error() << "m_playlists got corrupted! " << __FILE__ << ":" << __LINE__;
            continue;
        }
        if( playlistFile->uidUrl() == path )
        {
            debug() << "Playlist " << path.path() << " was already imported";
            return false;
        }
    }

    debug() << "Importing playlist file " << path;
    if( path == Amarok::defaultPlaylistPath() )
    {
        error() << "trying to load saved session playlist at %s" << path.path();
        return false;
    }

    Playlists::PlaylistFilePtr playlistFile = Playlists::loadPlaylistFile( path, this );
    if( !playlistFile )
        return false;

    m_playlists << playlistFile;
    //just in case there wasn't one loaded before.
    m_playlistsLoaded = true;

    emit playlistAdded( PlaylistPtr( playlistFile.data() ) );
    return true;
}

void
PlaylistFileProvider::renamePlaylist( Playlists::PlaylistPtr playlist, const QString &newName )
{
    DEBUG_BLOCK
    playlist->setName( newName );
}

bool
PlaylistFileProvider::deletePlaylists( const Playlists::PlaylistList &playlists )
{
    Playlists::PlaylistFileList playlistFiles;
    foreach( Playlists::PlaylistPtr playlist, playlists )
    {
        Playlists::PlaylistFilePtr playlistFile =
                Playlists::PlaylistFilePtr::dynamicCast( playlist );
        if( !playlistFile.isNull() )
            playlistFiles << playlistFile;
    }
    return deletePlaylistFiles( playlistFiles );
}

bool
PlaylistFileProvider::deletePlaylistFiles( Playlists::PlaylistFileList playlistFiles )
{
    foreach( Playlists::PlaylistFilePtr playlistFile, playlistFiles )
    {
        m_playlists.removeAll( playlistFile );
        loadedPlaylistsConfig().deleteEntry( playlistFile->uidUrl().url() );
        QFile::remove( playlistFile->uidUrl().path() );
        emit playlistRemoved( Playlists::PlaylistPtr::dynamicCast( playlistFile ) );
    }
    loadedPlaylistsConfig().sync();

    return true;
}

void
PlaylistFileProvider::loadPlaylists()
{
    if( m_urlsToLoad.isEmpty() )
        return;

    //arbitrary number of playlists to load during one mainloop run: 5
    for( int i = 0; i < qMin( m_urlsToLoad.count(), 5 ); i++ )
    {
        KUrl url = m_urlsToLoad.takeFirst();
        QString groups = loadedPlaylistsConfig().readEntry( url.url() );
        Playlists::PlaylistFilePtr playlist = Playlists::loadPlaylistFile( url, this );
        if( !playlist )
        {
            Amarok::Components::logger()->longMessage(
                    i18n("The playlist file \"%1\" could not be loaded.", url.fileName() ),
                    Amarok::Logger::Error
                );
            continue;
        }

        if( !groups.isEmpty() && playlist->isWritable() )
            playlist->setGroups( groups.split( ',',  QString::SkipEmptyParts ) );

        m_playlists << playlist;
        emit playlistAdded( PlaylistPtr( playlist.data() ) );
    }

    //give the mainloop time to run
    if( !m_urlsToLoad.isEmpty() )
        QTimer::singleShot( 0, this, SLOT(loadPlaylists()) );
}

void
PlaylistFileProvider::saveLater( Playlists::PlaylistFilePtr playlist )
{
    //WARNING: this assumes the playlistfile uses it's m_url for uidUrl
    if( playlist->uidUrl().isEmpty() )
        return;

    if( !m_saveLaterPlaylists.contains( playlist ) )
        m_saveLaterPlaylists << playlist;

    if( !m_saveLaterTimer )
    {
        m_saveLaterTimer = new QTimer( this );
        m_saveLaterTimer->setSingleShot( true );
        m_saveLaterTimer->setInterval( 0 );
        connect( m_saveLaterTimer, SIGNAL(timeout()), SLOT(slotSaveLater()) );
    }

    m_saveLaterTimer->start();
}

void
PlaylistFileProvider::slotSaveLater() //SLOT
{
    foreach( Playlists::PlaylistFilePtr playlist, m_saveLaterPlaylists )
    {
        playlist->save( true ); //TODO: read relative type when loading
    }

    m_saveLaterPlaylists.clear();
}

KConfigGroup
PlaylistFileProvider::loadedPlaylistsConfig() const
{
    return Amarok::config( "Loaded Playlist Files" );
}

} //namespace Playlists

