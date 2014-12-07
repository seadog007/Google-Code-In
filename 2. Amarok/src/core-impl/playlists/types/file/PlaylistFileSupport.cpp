/****************************************************************************************
 * Copyright (c) 2007 Ian Monroe <ian@monroe.nu>                                        *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) version 3 or        *
 * any later version accepted by the membership of KDE e.V. (or its successor approved  *
 * by the membership of KDE e.V.), which shall act as a proxy defined in Section 14 of  *
 * version 3 of the license.                                                            *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "core/playlists/PlaylistFormat.h"
#include "core/interfaces/Logger.h"
#include "core/support/Components.h"
#include "core/support/Amarok.h"
#include "core-impl/playlists/types/file/PlaylistFileSupport.h"
#include "core/support/Debug.h"
#include "core-impl/playlists/types/file/asx/ASXPlaylist.h"
#include "core-impl/playlists/types/file/xspf/XSPFPlaylist.h"
#include "core-impl/playlists/types/file/pls/PLSPlaylist.h"
#include "core-impl/playlists/types/file/m3u/M3UPlaylist.h"
#include "playlistmanager/file/PlaylistFileProvider.h"

#include "amarokconfig.h"


#include <KLocale>
#include <KTemporaryFile>
#include <KUrl>
#include <KMessageBox>

#include <QFile>
#include <QFileInfo>

using namespace Playlists;

PlaylistFilePtr
Playlists::loadPlaylistFile( const KUrl &url, PlaylistFileProvider *provider )
{
    // note: this function can be called from out of process, so don't do any
    // UI stuff from this thread.
    if( !url.isValid() )
    {
        error() << "url is not valid!";
        return PlaylistFilePtr();
    }

    if( url.isLocalFile() )
    {
        if( !QFileInfo( url.toLocalFile() ).exists() )
        {
            error() << QString("Could not load local playlist file %1!").arg( url.toLocalFile() );
            return PlaylistFilePtr();
        }
    }

    PlaylistFormat format = Playlists::getFormat( url );
    PlaylistFilePtr playlist;
    switch( format )
    {
        case ASX:
            playlist = new ASXPlaylist( url, provider );
            break;
        case PLS:
            playlist = new PLSPlaylist( url, provider );
            break;
        case M3U:
            playlist = new M3UPlaylist( url, provider );
            break;
        case XSPF:
            playlist = new XSPFPlaylist( url, provider );
            break;
        default:
            debug() << "Could not load playlist file " << url;
            break;
    }

    return playlist;
}

bool
Playlists::exportPlaylistFile( const Meta::TrackList &list, const KUrl &path, bool relative,
                    const QList<int> &queued )
{
    PlaylistFormat format = Playlists::getFormat( path );
    bool result = false;
    PlaylistFilePtr playlist;

    switch( format )
    {
        case ASX:
            playlist = new ASXPlaylist( path.toLocalFile() );
            break;
        case PLS:
            playlist = new PLSPlaylist( path.toLocalFile() );
            break;
        case M3U:
            playlist = new M3UPlaylist( path.toLocalFile() );
            break;
        case XSPF:
            playlist = new XSPFPlaylist( path.toLocalFile() );
            break;
        default:
            debug() << "Could not export playlist file " << path;
            break;
    }

    if( playlist )
    {
        playlist->addTracks( list );
        playlist->setQueue( queued );
        result = playlist->save( relative );
    }
    else
    {
        KMessageBox::error( 0,
                            i18n( "The used file extension is not valid for playlists." ),
                            i18n( "Unknown playlist format" ) );
    }

    return result;
}

bool
Playlists::canExpand( Meta::TrackPtr track )
{
    if( !track )
        return false;

    return Playlists::getFormat( track->uidUrl() ) != Playlists::NotPlaylist;
}

PlaylistPtr
Playlists::expand( Meta::TrackPtr track )
{
   return Playlists::PlaylistPtr::dynamicCast( loadPlaylistFile( track->uidUrl() ) );
}

KUrl
Playlists::newPlaylistFilePath( const QString &fileExtension )
{
    int trailingNumber = 1;
    KLocalizedString fileName = ki18n("Playlist_%1");
    KUrl url( Amarok::saveLocation( "playlists" ) );
    url.addPath( fileName.subs( trailingNumber ).toString() );

    while( QFileInfo( url.path() ).exists() )
        url.setFileName( fileName.subs( ++trailingNumber ).toString() );

    return KUrl( QString( "%1.%2" ).arg( url.path() ).arg( fileExtension ) );
}
