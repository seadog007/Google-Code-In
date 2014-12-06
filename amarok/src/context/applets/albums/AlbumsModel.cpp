/****************************************************************************************
 * Copyright (c) 2008 Andreas Muetzel <andreas.muetzel@gmx.net>                         *
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

#define DEBUG_PREFIX "AlbumsModel"

#include "AlbumsModel.h"
#include "AlbumsDefs.h"
#include "AlbumItem.h"
#include "AmarokMimeData.h"
#include "core/support/Debug.h"
#include "TrackItem.h"

#include <KGlobalSettings>
#include <KStringHandler>

#include <QFontMetrics>

AlbumsModel::AlbumsModel( QObject *parent )
    : QStandardItemModel( parent )
    , m_rowHeight( 0 )
{
    connect( KGlobalSettings::self(), SIGNAL(appearanceChanged()), SLOT(updateRowHeight()) );
    updateRowHeight();
}

int
AlbumsModel::rowHeight() const
{
    return m_rowHeight;
}

void
AlbumsModel::updateRowHeight()
{
    QFont font;
    m_rowHeight = QFontMetrics( font ).height();
}

QVariant
AlbumsModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    if( role == Qt::SizeHintRole )
    {
        const QStandardItem *item = itemFromIndex( index );
        int h = 4;
        h += (item->type() != AlbumType) ? m_rowHeight : static_cast<const AlbumItem *>( item )->iconSize();
        return QSize( -1, h );
    }
    return itemFromIndex( index )->data( role );
}

QMimeData*
AlbumsModel::mimeData( const QModelIndexList &indices ) const
{
    DEBUG_BLOCK
    if( indices.isEmpty() )
        return 0;

    Meta::TrackList tracks;
    foreach( const QModelIndex &index, indices )
        tracks << tracksForIndex( index );
    tracks = tracks.toSet().toList();

    // http://doc.trolltech.com/4.4/qabstractitemmodel.html#mimeData
    // If the list of indexes is empty, or there are no supported MIME types,
    // 0 is returned rather than a serialized empty list.
    if( tracks.isEmpty() )
        return 0;

    AmarokMimeData *mimeData = new AmarokMimeData();
    mimeData->setTracks( tracks );
    return mimeData;
}

Meta::TrackList
AlbumsModel::tracksForIndex( const QModelIndex &index ) const
{
    Meta::TrackList tracks;
    if( !index.isValid() )
        return tracks;

    if( hasChildren( index ) )
    {
        for( int i = 0, rows = rowCount( index ); i < rows; ++i )
            tracks << tracksForIndex( index.child( i, 0 ) );
    }
    else if( QStandardItem *item = itemFromIndex( index ) )
    {
        if( item->type() == TrackType )
        {
            TrackItem* trackItem = static_cast<TrackItem*>( item );
            if( trackItem )
                tracks << trackItem->track();
        }
    }
    return tracks;
}

QStringList
AlbumsModel::mimeTypes() const
{
    QStringList types;
    types << AmarokMimeData::TRACK_MIME;
    return types;
}

AlbumsProxyModel::AlbumsProxyModel( QObject *parent )
    : QSortFilterProxyModel( parent )
    , m_mode( SortByCreateDate )
{}

bool
AlbumsProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
    const QStandardItemModel *model = static_cast<QStandardItemModel*>( sourceModel() );
    const QStandardItem *leftItem = model->itemFromIndex( left );
    int type = leftItem->type();
    if( type == AlbumType && m_mode == SortByCreateDate )
    {
        const AlbumItem *leftAlbum = static_cast<const AlbumItem *>( leftItem );
        const AlbumItem *rightAlbum = static_cast<const AlbumItem *>( model->itemFromIndex( right ) );
        QDateTime leftMaxCreateDate, rightMaxCreateDate;
        foreach( Meta::TrackPtr track, leftAlbum->album()->tracks() )
            if( track->createDate() > leftMaxCreateDate )
                leftMaxCreateDate = track->createDate();
        foreach( Meta::TrackPtr track, rightAlbum->album()->tracks() )
            if( track->createDate() > rightMaxCreateDate )
                rightMaxCreateDate = track->createDate();
        return leftMaxCreateDate > rightMaxCreateDate;
    }
    else if( type == AlbumType || type == TrackType )
        return leftItem->operator<( *model->itemFromIndex( right ) );
    else
        return KStringHandler::naturalCompare( leftItem->text(), model->itemFromIndex(right)->text() ) < 0;
}

bool
AlbumsProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
    const QStandardItemModel *model = static_cast<QStandardItemModel*>( sourceModel() );
    const QModelIndex &srcIndex     = sourceModel()->index( sourceRow, 0, sourceParent );
    const QStandardItem *item       = model->itemFromIndex( srcIndex );

    if( item->data( NameRole ).toString().contains( filterRegExp() ) )
        return true;

    if( item->type() == AlbumType )
    {
        for( int i = 0, count = model->rowCount( srcIndex ); i < count; ++i )
        {
            const QModelIndex &kid = srcIndex.child( i, 0 );
            if( kid.data( NameRole ).toString().contains( filterRegExp() ) )
                return true;
        }
    }
    return false;
}

AlbumsProxyModel::Mode
AlbumsProxyModel::mode() const
{
    return m_mode;
}

void
AlbumsProxyModel::setMode( Mode mode )
{
    m_mode = mode;
}

#include "AlbumsModel.moc"
