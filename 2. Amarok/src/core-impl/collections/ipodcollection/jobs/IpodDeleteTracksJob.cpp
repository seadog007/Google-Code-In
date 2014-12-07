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

#include "IpodDeleteTracksJob.h"

#include "core/interfaces/Logger.h"
#include "core/support/Components.h"
#include "core-impl/collections/ipodcollection/IpodMeta.h"

#include <QFile>

#include <gpod/itdb.h>

IpodDeleteTracksJob::IpodDeleteTracksJob( const Meta::TrackList &sources,
                                          const QWeakPointer<IpodCollection> &collection )
    : Job()
    , m_sources( sources )
    , m_coll( collection )
{
}

void
IpodDeleteTracksJob::run()
{
    if( !m_coll )
        return;
    int trackCount = m_sources.size();
    QString operationText = i18np( "Removing one track from iPod",
                                   "Removing %1 tracks from iPod", trackCount );
    Amarok::Components::logger()->newProgressOperation( this, operationText, trackCount );
    itdb_start_sync( m_coll.data()->m_itdb );

    QListIterator<Meta::TrackPtr> it( m_sources );
    while( it.hasNext() )
    {
        if( !m_coll )
            break;

        /* delete the file first, then remove from database. Dangling entry in iTunes db
         * is better than dangling file */
        Meta::TrackPtr track = it.next();
        QFile file( track->playableUrl().path() );  // iPod files are always local, QFile suffices
        bool success = true;
        if( file.exists() )
            success = file.remove();
        if( success )
            m_coll.data()->removeTrack( track );

        incrementProgress();
    }

    emit endProgressOperation( this );
    if( m_coll )
        itdb_stop_sync( m_coll.data()->m_itdb );
}

#include "IpodDeleteTracksJob.moc"
