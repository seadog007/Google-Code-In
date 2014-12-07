/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2010 Bart Cerneels <bart.cerneels@kde.org>                             *
 * Copyright (c) 2011 Ralf Engels <ralf-engels@gmx.de>                                  *
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

#ifndef DYNAMICVIEW_H
#define DYNAMICVIEW_H

#include "widgets/PrettyTreeView.h"

#include <QMutex>

class PopupDropper;
class KAction;
class QKeyEvent;
class QMouseEvent;
class QContextMenuEvent;

namespace PlaylistBrowserNS {

class DynamicView : public Amarok::PrettyTreeView
{
Q_OBJECT
public:
    explicit DynamicView( QWidget *parent = 0 );
    ~DynamicView();

signals:
    void currentItemChanged( const QModelIndex &current );

public slots:
    void addPlaylist();
    void addToSelected();
    void cloneSelected();
    void editSelected();
    void removeSelected();

protected slots:
    void expandRecursive(const QModelIndex &index);
    void collapseRecursive(const QModelIndex &index);

protected:
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void mouseDoubleClickEvent( QMouseEvent *event );

    virtual void contextMenuEvent( QContextMenuEvent* event );
};

} // namespace PlaylistBrowserNS

#endif // DYNAMICVIEW_H
