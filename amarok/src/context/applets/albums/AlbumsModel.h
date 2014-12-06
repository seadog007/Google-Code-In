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

#ifndef AMAROK_ALBUMSMODEL_H
#define AMAROK_ALBUMSMODEL_H

#include "core/meta/forward_declarations.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

/**
 * This Model is used to get the right mime type/data for entries in the albums treeview
 */
class AlbumsModel : public QStandardItemModel
{
    Q_OBJECT

public:
    AlbumsModel( QObject *parent = 0 );
    virtual ~AlbumsModel() {}
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QMimeData* mimeData( const QModelIndexList &indices ) const;
    virtual QStringList mimeTypes() const;
    int rowHeight() const;

private slots:
    void updateRowHeight();

private:
    Meta::TrackList tracksForIndex( const QModelIndex &index ) const;
    int m_rowHeight;
};

class AlbumsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY( Mode mode READ mode WRITE setMode )
    Q_ENUMS( Mode )

public:
    AlbumsProxyModel( QObject *parent );
    ~AlbumsProxyModel() {}

    enum Mode { SortByCreateDate, SortByYear };

    Mode mode() const;
    void setMode( Mode mode );

protected:
    /**
     * Determine if album @param left is less than album @param right.
     *
     * If @param left and @param right both reference albums and @c m_mode
     * is set to @c SortByCreateDate, @c lessThan will return @c true if
     * and only the album referenced by @param left has a track that was
     * added <em>more recently</em> than all of the tracks in the album
     * referenced by @param right.
     */
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

private:
    Mode m_mode;
};


#endif
