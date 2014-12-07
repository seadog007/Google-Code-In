/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2008 Ian Monroe <imonroe@kde.org>                                      *
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

#include "UserPlaylistModel.h"
#include "playlistmanager/PlaylistManager.h"
#include "core/playlists/PlaylistProvider.h"

#include "AmarokMimeData.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "core/support/Debug.h"
#include "SvgHandler.h"

#include <KIcon>

#include <QAbstractListModel>

#include <typeinfo>

//Playlist & Track index differentiator macros
//QModelIndex::intenalId() is a qint64 to support 64-bit pointers in a union with the ID
#define TRACK_MASK (0x1<<31)
#define IS_TRACK(x) ((x.internalId()) & (TRACK_MASK))?true:false
#define SET_TRACK_MASK(x) ((x) | (TRACK_MASK))
#define REMOVE_TRACK_MASK(x) ((x) & ~(TRACK_MASK))

namespace The
{
    PlaylistBrowserNS::UserModel* userPlaylistModel()
    {
        return PlaylistBrowserNS::UserModel::instance();
    }
}

PlaylistBrowserNS::UserModel *PlaylistBrowserNS::UserModel::s_instance = 0;

PlaylistBrowserNS::UserModel *PlaylistBrowserNS::UserModel::instance()
{
    if( s_instance == 0 )
        s_instance = new UserModel();

    return s_instance;
}

void
PlaylistBrowserNS::UserModel::destroy()
{
    if( s_instance )
    {
        delete s_instance;
        s_instance = 0;
    }
}

PlaylistBrowserNS::UserModel::UserModel()
    : PlaylistBrowserModel( PlaylistManager::UserPlaylist )
{
    s_instance = this;
}

PlaylistBrowserNS::UserModel::~UserModel()
{
}

bool
PlaylistBrowserNS::UserModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
    Q_UNUSED( role )

    switch( idx.column() )
    {
        case PlaylistBrowserModel::PlaylistItemColumn:
        {
            QString newName = value.toString().trimmed();
            if( newName.isEmpty() )
                return false;
            Playlists::PlaylistPtr playlist = m_playlists.value( idx.internalId() );
            // we emit dataChanged signals later
            return The::playlistManager()->rename( playlist, newName );
        }
        case PlaylistBrowserModel::LabelColumn:
        {
            debug() << "changing group of item " << idx.internalId() << " to " << value.toString();
            Playlists::PlaylistPtr item = m_playlists.value( idx.internalId() );
            item->setGroups( value.toStringList() );
            // we emit dataChanged signals later
            return true;
        }
        default:
            return false;
    }

    return true;
}

bool
PlaylistBrowserNS::UserModel::removeRows( int row, int count, const QModelIndex &parent )
{
    if( row < 0 || row > rowCount( parent ) )
        return false;

    if( !parent.isValid() )
    {
      Playlists::PlaylistList playlistToRemove;
      for( int i = row; i < row + count; i++ )
      {
        if( m_playlists.count() > i )
        {
            Playlists::PlaylistPtr playlist = m_playlists[i];
            debug() << "Removing " << playlist->name();
            playlistToRemove << playlist;
        }
      }
      if( playlistToRemove.isEmpty() )
        return false;

      return The::playlistManager()->deletePlaylists( playlistToRemove );
    }
    int playlistRow = REMOVE_TRACK_MASK(parent.internalId());

    //don't try to get a playlist beyond the last item in the list
    if( playlistRow >=  m_playlists.count() )
    {
        error() << "Tried to remove from non existing playlist:";
        error() << playlistRow << " while there are only " << m_playlists.count();
        return false;
    }

    Playlists::PlaylistPtr playlist = m_playlists.value( playlistRow );

    //if we are trying to delete more tracks then what the playlist has, return.
    //count will be at least 1 to delete one track
    if( row + count - 1 >= playlist->tracks().count() )
    {
        error() << "Tried to remove a track using an index that is not there:";
        error() << "row: " << row << " count: " << count << " number of tracks: "
                << playlist->tracks().count();
        return false;
    }

    beginRemoveRows( parent, row, row + count - 1 );
    //ignore notifications while removing tracks
    unsubscribeFrom( playlist );
    for( int i = row; i < row + count; i++ )
        //deleting a track moves the next track up, so use the same row number each time
        playlist->removeTrack( row );
    subscribeTo( playlist );
    endRemoveRows();

    return true;
}

bool
PlaylistBrowserNS::UserModel::dropMimeData ( const QMimeData *data, Qt::DropAction action, int row,
        int column, const QModelIndex &parent ) //reimplemented
{
    Q_UNUSED( column )

    //let the base class handle the regular actions.
    if( PlaylistBrowserModel::dropMimeData( data, action, row, column, parent ) )
        return true;

    if( data->hasUrls() )
    {
        foreach( const QUrl &url, data->urls() )
            The::playlistManager()->import( url.toString() );
    }

    return false;
}

#include "UserPlaylistModel.moc"
