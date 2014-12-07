/****************************************************************************************
 * Copyright (c) 2008 Bonne Eggleston <b.eggleston@gmail.com>                           *
 * Copyright (c) 2009 Seb Ruiz <ruiz@kde.org>                                           *
 * Copyright (c) 2009 Louis Bayle <louis.bayle@gmail.com>                               *
 * Copyright (c) 2010 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef AMAROK_PLAYLISTVIEWCOMMON_H
#define AMAROK_PLAYLISTVIEWCOMMON_H

#include <QAction>
#include <QModelIndex>
#include <QPoint>

namespace Playlist
{
    class ViewCommon
    {
        public:

            ViewCommon();
            ~ViewCommon();

            void editTrackInformation();
            void trackMenu( QWidget *, const QModelIndex *, const QPoint &pos );
            QList<QAction*> actionsFor( QWidget *parent, const QModelIndex *index );

            QList<QAction*> trackActionsFor( QWidget *parent, const QModelIndex *index );
            QList<QAction*> albumActionsFor( const QModelIndex *index );
            QList<QAction*> multiSourceActionsFor( QWidget *parent, const QModelIndex *index );
            QList<QAction*> editActionsFor( QWidget *parent, const QModelIndex *index );

        private:

            /** Sets the parent to \c parent for all actions that don't already have one set.
                This is needed because ActionsCapability expects actions without parent to be freed by the caller.
            */
            QList<QAction*> parentCheckActions( QObject *parent, QList<QAction*> actions );
            QAction* m_stopAfterTrackAction;
            QAction* m_cueTrackAction;
            QAction* m_removeTracTrackAction;
            QAction* m_findInSourceAction;
    };

}

#endif
