/****************************************************************************************
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#include "PlaylistFile.h"

#include "core/support/Debug.h"
#include "core-impl/playlists/types/file/PlaylistFileLoaderJob.h"
#include "playlistmanager/file/PlaylistFileProvider.h"
#include "playlistmanager/PlaylistManager.h"

#include <KUrl>
#include <KMimeType>

#include <ThreadWeaver/Weaver>

using namespace Playlists;

PlaylistFile::PlaylistFile( const KUrl &url, PlaylistProvider *provider )
             : Playlist()
             , m_provider( provider )
             , m_url( url )
             , m_tracksLoaded( false )
             , m_name( m_url.fileName() )
             , m_relativePaths( false )
             , m_loadingDone( 0 )
{
}

void
PlaylistFile::saveLater()
{
    PlaylistFileProvider *fileProvider = qobject_cast<PlaylistFileProvider *>( m_provider );
    if( !fileProvider )
        return;

    fileProvider->saveLater( PlaylistFilePtr( this ) );
}

void
PlaylistFile::triggerTrackLoad()
{
    if( m_tracksLoaded )
    {
        notifyObserversTracksLoaded();
        return;
    }
    PlaylistFileLoaderJob *worker = new PlaylistFileLoaderJob( PlaylistFilePtr( this ) );
    ThreadWeaver::Weaver::instance()->enqueue( worker );
    if ( !isLoadingAsync() )
        m_loadingDone.acquire(); // after loading is finished worker will release semapore
}

bool
PlaylistFile::isWritable() const
{
    if( m_url.isEmpty() )
        return false;

    return QFileInfo( m_url.path() ).isWritable();
}

int
PlaylistFile::trackCount() const
{
    if( m_tracksLoaded )
        return m_tracks.count();
    else
        return -1;
}

void
PlaylistFile::addTrack( Meta::TrackPtr track, int position )
{
    if( !track ) // playlists might contain invalid tracks. see BUG: 303056
        return;

    int trackPos = position < 0 ? m_tracks.count() : position;
    if( trackPos > m_tracks.count() )
        trackPos = m_tracks.count();
    m_tracks.insert( trackPos, track );
    // set in case no track was in the playlist before
    m_tracksLoaded = true;

    notifyObserversTrackAdded( track, trackPos );

    if( !m_url.isEmpty() )
        saveLater();
}

void
PlaylistFile::removeTrack( int position )
{
    if( position < 0 || position >= m_tracks.count() )
        return;

    m_tracks.removeAt( position );

    notifyObserversTrackRemoved( position );

    if( !m_url.isEmpty() )
        saveLater();
}

bool
PlaylistFile::save( bool relative )
{
    m_relativePaths = relative;
    QMutexLocker locker( &m_saveLock );

    //if the location is a directory append the name of this playlist.
    if( m_url.fileName( KUrl::ObeyTrailingSlash ).isNull() )
        m_url.setFileName( name() );

    QFile file( m_url.path() );

    if( !file.open( QIODevice::WriteOnly ) )
    {
        warning() << QString( "Cannot write playlist (%1)." ).arg( file.fileName() )
                  << file.errorString();
        return false;
    }

    savePlaylist( file );
    file.close();
    return true;
}

void
PlaylistFile::setName( const QString &name )
{
    //can't save to a new file if we don't know where.
    if( !m_url.isEmpty() && !name.isEmpty() )
    {
        QString exten = QString( ".%1" ).arg(extension());
        m_url.setFileName( name + ( name.endsWith( exten, Qt::CaseInsensitive ) ? "" : exten ) );
    }
}

void
PlaylistFile::addProxyTrack( const Meta::TrackPtr &proxyTrack )
{
    m_tracks << proxyTrack;
    notifyObserversTrackAdded( m_tracks.last(), m_tracks.size() - 1 );
}

KUrl
PlaylistFile::getAbsolutePath( const KUrl &url )
{
    KUrl absUrl = url;
    if( url.isRelative() )
    {
        m_relativePaths = true;
        // example: url = KUrl("../tunes/tune.ogg")
        const QString relativePath = url.path(); // "../tunes/tune.ogg"
        absUrl = m_url.directory(); // file:///playlists/
        absUrl.addPath( relativePath ); // file:///playlists/../tunes/tune.ogg
        absUrl.cleanPath(); // file:///playlists/tunes/tune.ogg
    }
    return absUrl;
}

QString
PlaylistFile::trackLocation( const Meta::TrackPtr &track ) const
{
    KUrl path = track->playableUrl();
    if( path.isEmpty() )
        return track->uidUrl();

    if( !m_relativePaths || m_url.isEmpty() || !path.isLocalFile() || !m_url.isLocalFile() )
        return path.toEncoded();

    QDir playlistDir( m_url.directory() );
    return QUrl::toPercentEncoding( playlistDir.relativeFilePath( path.path() ), "/" );
}
