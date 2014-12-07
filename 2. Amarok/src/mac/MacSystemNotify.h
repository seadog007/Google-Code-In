/****************************************************************************************
 * Copyright (c) 2014 Daniel Meltzer <parallelgrapefruit@gmail.com>                     *
 * Based on Code from the Quassel OS X Backend.
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
 
#ifndef AMAROK_MACSYSTEMNOTIFY_H
#define AMAROK_MACSYSTEMNOTIFY_H



#include <QObject>
#include <QString>

#include "core/support/Debug.h"
#include "core/meta/forward_declarations.h"
#include "EngineController.h"

class OSXNotify : QObject
{
    Q_OBJECT

    public:
    OSXNotify( QString appName );

    protected slots:
        void show( Meta::TrackPtr );

    private:
        QString m_appName;
        Meta::TrackPtr m_currentTrack;

};

#endif
 
