/****************************************************************************************
 * Copyright (c) 2007 Alexandre Pereira de Oliveira <aleprj@gmail.com>                  *
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#ifndef SINGLECOLLECTIONTREEITEMMODEL_H
#define SINGLECOLLECTIONTREEITEMMODEL_H

#include "amarok_export.h"
#include "CollectionTreeItemModelBase.h"
#include "core/meta/forward_declarations.h"

#include <QAbstractItemModel>
#include <QMap>
#include <QPair>

class CollectionTreeItem;
class Collection;
//typedef QPair<Collection*, CollectionTreeItem* > CollectionRoot;


class AMAROK_EXPORT SingleCollectionTreeItemModel: public CollectionTreeItemModelBase 
{
    Q_OBJECT

    public:
        SingleCollectionTreeItemModel( Collections::Collection *collection,
                                       const QList<CategoryId::CatMenuId> &levelType );

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual bool canFetchMore( const QModelIndex &parent ) const;
        virtual void fetchMore( const QModelIndex &parent );
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    protected:
        virtual void filterChildren();
        virtual int levelModifier() const { return 1; }

    private:

        Collections::Collection* m_collection;
};

#endif
