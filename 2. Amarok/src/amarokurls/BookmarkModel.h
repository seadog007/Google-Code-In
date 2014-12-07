/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include "BookmarkViewItem.h"

#include <KSharedPtr>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


class BookmarkGroup;

typedef KSharedPtr<BookmarkViewItem> BookmarkViewItemPtr;

class BookmarkGroup;
typedef KSharedPtr<BookmarkGroup> BookmarkGroupPtr;
typedef QList<BookmarkGroupPtr> BookmarkGroupList;


//#define BOOKMARK_DB_VERSION 1


/**
    @author Nikolaj Hald Nielsen <nhn@kde.org>
*/

class BookmarkModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum Column
    {
        Name = 0,
        Command,
        Url,
        Description
    };

    static BookmarkModel * instance();

    ~BookmarkModel();

    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const;
    virtual QModelIndex index( int row, int column,
                               const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    virtual Qt::DropActions supportedDropActions() const
    {
        return Qt::MoveAction;
    }

    virtual QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    void reloadFromDb();
    void editBookmark( int id );

    QModelIndex createIndex( int row, int column, const BookmarkViewItemPtr &item ) const;
    //only use the above method
    QModelIndex createIndex( int, int, void * ptr = 0 ) const
    {
        Q_UNUSED( ptr );
        Q_ASSERT( 0 );
        return QModelIndex();
    }

    QModelIndex createIndex( int, int, quint32 ) const
    {
        Q_ASSERT( 0 );
        return QModelIndex();
    }

public slots:
    void createNewGroup();
    void createNewBookmark();
    void deleteBookmark( const QString &name );
    void renameBookmark( const QString &oldName , const QString &newName );

    /**
     * Sets the bookmark's (whose name is @param name) url argument named @param key
     * to @param value. Overrides any possible previous value.
     */
    void setBookmarkArg(const QString &name, const QString &key, const QString &value);

signals:
    void editIndex( const QModelIndex &index );

private:
    BookmarkModel();

    void checkTables();
    void createTables();
    void deleteTables();
    void upgradeTables( int from );

    bool setBookmarkArgRecursively( BookmarkGroupPtr group, const QString &name, const QString &key, const QString &value );
    bool deleteBookmarkRecursively( BookmarkGroupPtr group, const QString &name );
    bool renameBookmarkRecursively( BookmarkGroupPtr group, const QString &oldName, const QString &newName );

    static BookmarkModel * s_instance;

    BookmarkGroupPtr m_root;
    mutable QHash<quint32, BookmarkViewItemPtr> m_viewItems; ///the hash of the pointer mapped to the KSharedPtr

};


#endif
