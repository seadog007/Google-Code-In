/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
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

#ifndef FILETRACKPROVIDER_H
#define FILETRACKPROVIDER_H

#include "amarok_export.h"
#include "core/collections/Collection.h"

/**
 * A simple track provider that contructs MetaFile::Tracks for local and
 * existing urls. (no remote protocols supported, just "file" protocol.)
 */
class AMAROK_EXPORT FileTrackProvider : public Collections::TrackProvider
{
    public:
        FileTrackProvider();
        virtual ~FileTrackProvider();

        virtual bool possiblyContainsTrack( const KUrl &url ) const;
        virtual Meta::TrackPtr trackForUrl( const KUrl &url );

    private:
        Q_DISABLE_COPY( FileTrackProvider )
};

#endif // FILETRACKPROVIDER_H
