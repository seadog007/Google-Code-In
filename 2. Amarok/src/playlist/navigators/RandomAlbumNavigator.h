/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2008 Soren Harward <stharward@gmail.com>                               *
 * Copyright (c) 2010 Nanno Langstraat <langstr@gmail.com>                              *
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

#ifndef RANDOMALBUMNAVIGATOR_H
#define RANDOMALBUMNAVIGATOR_H

#include "AlbumNavigator.h"

namespace Playlist
{
    /**
        Navigator for playing back albums in random order

        @author Nikolaj Hald Nielsen <nhn@kde.org>
        @author Soren Harward <stharward@gmail.com>
     */

    class RandomAlbumNavigator : public AlbumNavigator
    {
        Q_OBJECT

        public:
            RandomAlbumNavigator();

        private:
            //! Override from 'AlbumNavigator'
            void notifyAlbumsInserted( const QList<AlbumId> insertedAlbums );

            //! Override from 'NonlinearTrackNavigator'
            void planOne();
    };
}

#endif
