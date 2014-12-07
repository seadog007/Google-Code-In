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

#ifndef TRASHCOLLECTIONLOCATION_H
#define TRASHCOLLECTIONLOCATION_H

#include "core/collections/CollectionLocation.h"

class KJob;

namespace Collections {

/**
  * Utility class that allows moving tracks to the KIO trash using standard
  * CollectionLocation API. It is not intented to be a collection, but more
  * as a black hole destination.
  */
class TrashCollectionLocation : public CollectionLocation
{
    Q_OBJECT

public:
    TrashCollectionLocation();
    ~TrashCollectionLocation();

    QString prettyLocation() const;
    bool isWritable() const;

protected:
    void copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources,
                               const Transcoding::Configuration &configuration );
    void showDestinationDialog( const Meta::TrackList &tracks, bool removeSources,
                                const Transcoding::Configuration &configuration );

private slots:
    void slotTrashJobFinished( KJob *job );

private:
    bool m_trashConfirmed;
    QHash<KJob*, Meta::TrackList> m_trashJobs;
};

} //namespace Collections

#endif // TRASHCOLLECTIONLOCATION_H
