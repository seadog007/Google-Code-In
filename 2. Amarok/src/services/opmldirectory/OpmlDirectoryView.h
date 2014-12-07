/****************************************************************************************
 * Copyright (c) 2010 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#ifndef OPMLDIRECTORYVIEW_H
#define OPMLDIRECTORYVIEW_H

#include "widgets/PrettyTreeView.h"

class QContextMenuEvent;
class QKeyEvent;

class OpmlDirectoryView : public Amarok::PrettyTreeView
{
    Q_OBJECT
    public:
        explicit OpmlDirectoryView( QWidget *parent = 0 );

        virtual void contextMenuEvent( QContextMenuEvent *event );
        virtual void keyPressEvent( QKeyEvent *event );

    protected:
        //reimplemented to allow only leaf nodes to be selected
        virtual QItemSelectionModel::SelectionFlags selectionCommand( const QModelIndex &index,
                                                                    const QEvent *event = 0 ) const;

};

#endif // OPMLDIRECTORYVIEW_H
