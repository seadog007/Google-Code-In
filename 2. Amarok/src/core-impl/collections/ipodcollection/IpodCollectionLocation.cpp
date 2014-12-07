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

#include "IpodCollectionLocation.h"

#include "jobs/IpodDeleteTracksJob.h"
#include "core/interfaces/Logger.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"

#include <ThreadWeaver/Weaver>

#include <QDir>
#include <QFile>

#include <gpod/itdb.h>

IpodCollectionLocation::IpodCollectionLocation( QWeakPointer<IpodCollection> parentCollection )
    : CollectionLocation()  // we implement collection(), we need not pass parentCollection
    , m_coll( parentCollection )
{
}

IpodCollectionLocation::~IpodCollectionLocation()
{
}

Collections::Collection*
IpodCollectionLocation::collection() const
{
    // overridden to avoid dangling pointers
    return m_coll.data();
}

QString
IpodCollectionLocation::prettyLocation() const
{
    if( m_coll )
        return m_coll.data()->prettyName();
    // match string with IpodCopyTracksJob::slotDisplaySorryDialog()
    return i18n( "Disconnected iPod/iPad/iPhone" );
}

bool
IpodCollectionLocation::isWritable() const
{
    if( !m_coll )
        return false;
    return m_coll.data()->isWritable(); // no infinite loop, IpodCollection iplements this
}

void
IpodCollectionLocation::copyUrlsToCollection( const QMap<Meta::TrackPtr,KUrl> &sources,
                                              const Transcoding::Configuration &configuration )
{
    if( !isWritable() )
        return;  // mostly unreachable, CollectionLocation already checks this and issues a warning
    ensureDirectoriesExist();

    IpodCopyTracksJob *job = new IpodCopyTracksJob( sources, m_coll, configuration, isGoingToRemoveSources() );
    int trackCount = sources.size();
    Amarok::Components::logger()->newProgressOperation( job,
        operationInProgressText( configuration, trackCount ), trackCount, job, SLOT(abort()) );

    qRegisterMetaType<IpodCopyTracksJob::CopiedStatus>( "IpodCopyTracksJob::CopiedStatus" );
    connect( job, SIGNAL(signalTrackProcessed(Meta::TrackPtr,Meta::TrackPtr,IpodCopyTracksJob::CopiedStatus)),
             this, SLOT(slotCopyTrackProcessed(Meta::TrackPtr,Meta::TrackPtr,IpodCopyTracksJob::CopiedStatus)) );
    connect( job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(slotCopyOperationFinished()) );
    connect( job, SIGNAL(done(ThreadWeaver::Job*)), job, SLOT(deleteLater()) );
    ThreadWeaver::Weaver::instance()->enqueue( job );
}

void
IpodCollectionLocation::removeUrlsFromCollection( const Meta::TrackList &sources )
{
    if( !isWritable() )
        return;

    IpodDeleteTracksJob *job = new IpodDeleteTracksJob( sources, m_coll );
    connect( job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(slotRemoveOperationFinished()) );
    connect( job, SIGNAL(done(ThreadWeaver::Job*)), job, SLOT(deleteLater()) );
    ThreadWeaver::Weaver::instance()->enqueue( job );
}

void
IpodCollectionLocation::setDestinationPlaylist( Playlists::PlaylistPtr destPlaylist, const QMap<Meta::TrackPtr, int> &trackPlaylistPositions )
{
    m_destPlaylist = destPlaylist;
    m_trackPlaylistPositions = trackPlaylistPositions;
}

void
IpodCollectionLocation::slotCopyTrackProcessed( Meta::TrackPtr srcTrack, Meta::TrackPtr destTrack,
                                                IpodCopyTracksJob::CopiedStatus status )
{
    if( status == IpodCopyTracksJob::Success )
        // we do not include track found by matching meta-data here for safety reasons
        source()->transferSuccessful( srcTrack );

    if( m_destPlaylist && ( status == IpodCopyTracksJob::Success || status == IpodCopyTracksJob::Duplicate )
        && destTrack && m_trackPlaylistPositions.contains( srcTrack ) )
        // add this track to iPod playlist
    {
        m_destPlaylist->addTrack( destTrack, m_trackPlaylistPositions.value( srcTrack ) );
    }
}

void IpodCollectionLocation::ensureDirectoriesExist()
{
    QByteArray mountPoint = m_coll ? QFile::encodeName( m_coll.data()->mountPoint() ) : QByteArray();
    if( mountPoint.isEmpty() )
        return;

    gchar *musicDirChar = itdb_get_music_dir( mountPoint.constData() );
    QString musicDirPath = QFile::decodeName( musicDirChar );
    g_free( musicDirChar );
    if( musicDirPath.isEmpty() )
        return;

    QDir musicDir( musicDirPath );
    if( !musicDir.exists() && !musicDir.mkpath( "." ) /* try to create it */ )
    {
        warning() << __PRETTY_FUNCTION__ << "failed to create" << musicDirPath << "directory.";
        return;
    }

    QChar fillChar( '0' );
    for( int i = 0; i < 20; i++ )
    {
        QString name = QString( "F%1" ).arg( i, /* min-width */ 2, /* base */ 10, fillChar );
        if( musicDir.exists( name ) )
            continue;
        QString toCreatePath = QString( "%1/%2" ).arg( musicDirPath, name );
        if( musicDir.mkdir( name ) )
            debug() << __PRETTY_FUNCTION__ << "created" << toCreatePath << "directory.";
        else
            warning() << __PRETTY_FUNCTION__ << "failed to create" << toCreatePath << "directory.";
    }
}

#include "IpodCollectionLocation.moc"
