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

#ifndef STATSYNCING_COLLECTIONPROVIDER_H
#define STATSYNCING_COLLECTIONPROVIDER_H

#include "core/meta/forward_declarations.h"
#include "statsyncing/Provider.h"

#include <QSemaphore>

namespace Collections {
    class Collection;
}

namespace StatSyncing
{
    /**
     * Provider that has Collections::Colections as a back-end.
     */
    class CollectionProvider : public Provider
    {
        Q_OBJECT

        public:
            /**
             * Construct provider that has @param collection as a back-end.
             */
            CollectionProvider( Collections::Collection *collection );
            virtual ~CollectionProvider();

            virtual QString id() const;
            virtual QString prettyName() const;
            virtual KIcon icon() const;
            virtual qint64 reliableTrackMetaData() const;
            virtual qint64 writableTrackStatsData() const;
            virtual Preference defaultPreference();
            virtual QSet<QString> artists();
            virtual TrackList artistTracks( const QString &artistName );

        signals:
            /// hacks to create and start QueryMaker in main eventloop
            void startArtistSearch();
            void startTrackSearch( QString artistName );

        private slots:
            /// @see startArtistSearch
            void slotStartArtistSearch();
            void slotStartTrackSearch( QString artistName );

            void slotNewResultReady( Meta::ArtistList list );
            void slotNewResultReady( Meta::TrackList list );
            void slotQueryDone();

        private:
            Q_DISABLE_COPY(CollectionProvider)

            /// collection can disappear at any time, use weak pointer to notice it
            QWeakPointer<Collections::Collection> m_coll;
            QSet<QString> m_foundArtists;
            QString m_currentArtistName;
            TrackList m_foundTracks;
            /**
             * Semaphore for the simplified producer-consumer pattern, where
             * slotNewResultReady( ArtistList ) along with slotQueryDone() is producer
             * and artists() is consumer, or
             * slotNewResultReady( TrackList ) along with slotQueryDone() is producer
             * and artistTracks() is consumer.
             */
            QSemaphore m_queryMakerSemaphore;
    };

} // namespace StatSyncing

#endif // STATSYNCING_COLLECTIONPROVIDER_H
