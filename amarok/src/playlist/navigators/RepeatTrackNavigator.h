/****************************************************************************************
 * Copyright (c) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>                        *
 * Copyright (c) 2008 Soren Harward <stharward@gmail.com>                               *
 * Copyright (c) 2010 Nanno Langstraat <langstr@gmail.com>                              *
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

#ifndef REPEATTRACKNAVIGATOR_H
#define REPEATTRACKNAVIGATOR_H

#include "StandardTrackNavigator.h"

namespace Playlist
{
    /**
     * Repeats the selected track over and over, unless the user intervenes
     */
    class RepeatTrackNavigator : public StandardTrackNavigator
    {
        Q_OBJECT

        public:
            RepeatTrackNavigator();

            quint64 likelyNextTrack() { return m_trackid ? m_trackid : bestFallbackItem(); }
            // likelyLastTrack(): The user explicitly asks for change. Let parent 'StandardTrackNavigator' handle that.
            quint64 requestNextTrack() { return likelyNextTrack(); }
            quint64 requestUserNextTrack() { return StandardTrackNavigator::requestNextTrack(); }  // The user explicitly asks for change. Use the 'next' behaviour of parent 'StandardTrackNavigator'.
            // requestLastTrack(): The user explicitly asks for change. Let parent 'StandardTrackNavigator' handle that.

        private slots:
            void recvActiveTrackChanged( const quint64 id ) { m_trackid = id; }

        private:
            quint64 m_trackid;
    };

}

#endif
