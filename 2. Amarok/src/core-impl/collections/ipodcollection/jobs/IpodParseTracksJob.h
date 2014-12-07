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

#ifndef IPODPARSETRACKSJOB_H
#define IPODPARSETRACKSJOB_H

#include "core/meta/forward_declarations.h"

#include <ThreadWeaver/Job>

class IpodCollection;

/**
 * A job designed to parse iPod tracks and playlists in a thread so that main thread is
 * not blocked with it. It is guaranteed by IpodCollection that is doesn't destory itself
 * while this job is alive. Memory management of this job is up to the caller of it.
 */
class IpodParseTracksJob : public ThreadWeaver::Job
{
    Q_OBJECT

    public:
        explicit IpodParseTracksJob( IpodCollection *collection );

    public slots:
        /**
         * Aborts the job as soon as it is safely possible
         */
        void abort();

    protected:
        void run();

    signals:
        // signals for progress operation:
        void incrementProgress();
        void endProgressOperation( QObject *obj );
        void totalSteps( int steps ); // not used, defined to keep QObject::conect warning quiet

    private:
        /**
         * Go through iPod playlists and create Amarok playlists for them.
         *
         * @param staleTracks list of track from iTunes database whose associated file
         *                    no longer exists
         * @param knownPaths a set of absolute local paths of all track from iTunes
         *                   database; used for orphaned tracks detection
         */
        void parsePlaylists( const Meta::TrackList &staleTracks,
                             const QSet<QString> &knownPaths );
        Meta::TrackList findOrphanedTracks( const QSet<QString> &knownPaths );

        IpodCollection *m_coll;
        bool m_aborted;
};

#endif // IPODPARSETRACKSJOB_H
