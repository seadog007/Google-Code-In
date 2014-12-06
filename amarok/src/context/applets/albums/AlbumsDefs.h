/****************************************************************************************
 * Copyright (c) 2010 Rick W. Chen <stuffcorpse@archlinux.us>                           *
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

#ifndef AMAROK_ALBUMSDEFS_H
#define AMAROK_ALBUMSDEFS_H

#include <QStandardItem>

enum AlbumsModelItemTypes
{
    AlbumType = QStandardItem::UserType,
    TrackType
};

enum AlbumsModelCustomRoles
{
    NameRole = Qt::UserRole + 1,
    AlbumCompilationRole,
    AlbumMaxTrackNumberRole,
    AlbumLengthRole,
    AlbumYearRole,
    TrackArtistRole,
    TrackNumberRole,
    TrackLengthRole
};

#endif /* AMAROK_ALBUMSDEFS_H */
