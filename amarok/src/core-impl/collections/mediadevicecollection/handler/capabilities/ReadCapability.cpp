/****************************************************************************************
 * Copyright (c) 2009 Alejandro Wainzinger <aikawarazuni@gmail.com>                     *
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

#include "ReadCapability.h"

Handler::ReadCapability::~ReadCapability()
{
    // nothing to do here
}

bool
Handler::ReadCapability::libIsCompilation( const Meta::MediaDeviceTrackPtr &track )
{
    Q_UNUSED( track )
    // provide default implementation so that MTP collection doesn't need to override.
    return false;
}

qreal Handler::ReadCapability::libGetReplayGain( const Meta::MediaDeviceTrackPtr &track )
{
    Q_UNUSED( track )
    return 0.0;
}

#include "ReadCapability.moc"
