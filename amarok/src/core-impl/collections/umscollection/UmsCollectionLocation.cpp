/****************************************************************************************
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#include "UmsCollectionLocation.h"

#include "core/support/Components.h"
#include "core/support/Debug.h"
#include "core/interfaces/Logger.h"
#include "core/transcoding/TranscodingController.h"
#include "core-impl/meta/file/File.h"
#include "transcoding/TranscodingJob.h"

#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <kio/job.h>
#include <KUrl>
#include <kio/jobclasses.h>

#include <QDir>

UmsCollectionLocation::UmsCollectionLocation( UmsCollection *umsCollection )
    : CollectionLocation( umsCollection )
    , m_umsCollection( umsCollection )
{
}

UmsCollectionLocation::~UmsCollectionLocation()
{
}

QString
UmsCollectionLocation::prettyLocation() const
{
    return m_umsCollection->musicPath().toLocalFile( KUrl::RemoveTrailingSlash );
}

QStringList
UmsCollectionLocation::actualLocation() const
{
    return QStringList() << prettyLocation();
}

bool
UmsCollectionLocation::isWritable() const
{
    const QFileInfo info( m_umsCollection->musicPath().toLocalFile() );
    return info.isWritable();
}

bool
UmsCollectionLocation::isOrganizable() const
{
    return isWritable();
}

void
UmsCollectionLocation::copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources,
                                             const Transcoding::Configuration &configuration )
{
    //TODO: disable scanning until we are done with copying

    UmsTransferJob *transferJob = new UmsTransferJob( this, configuration );
    QMapIterator<Meta::TrackPtr, KUrl> i( sources );
    while( i.hasNext() )
    {
        i.next();
        Meta::TrackPtr track = i.key();
        KUrl destination;
        bool isJustCopy = configuration.isJustCopy( track );
        if( isJustCopy )
            destination = m_umsCollection->organizedUrl( track );
        else
            destination = m_umsCollection->organizedUrl( track, Amarok::Components::
                transcodingController()->format( configuration.encoder() )->fileExtension() );
        debug() << "destination is " << destination.toLocalFile();
        QDir dir( destination.directory() );
        if( !dir.exists() && !dir.mkpath( "." ) )
        {
            error() << "could not create directory to copy into.";
            abort();
        }
        m_sourceUrlToTrackMap.insert( i.value(), track ); // needed for slotTrackTransferred()
        if( isJustCopy )
            transferJob->addCopy( i.value(), destination );
        else
            transferJob->addTranscode( i.value(), destination );
    }

    connect( transferJob, SIGNAL(sourceFileTransferDone(KUrl)),
             this, SLOT(slotTrackTransferred(KUrl)) );
    connect( transferJob, SIGNAL(fileTransferDone(KUrl)),
             m_umsCollection, SLOT(slotTrackAdded(KUrl)) );
    connect( transferJob, SIGNAL(finished(KJob*)),
             this, SLOT(slotCopyOperationFinished()) );

    QString loggerText = operationInProgressText( configuration, sources.count(), m_umsCollection->prettyName() );
    Amarok::Components::logger()->newProgressOperation( transferJob, loggerText, transferJob,
                                                        SLOT(slotCancel()) );
    transferJob->start();
}

void
UmsCollectionLocation::removeUrlsFromCollection( const Meta::TrackList &sources )
{
    KUrl::List sourceUrls;
    foreach( const Meta::TrackPtr track, sources )
    {
        KUrl trackUrl = track->playableUrl();
        m_sourceUrlToTrackMap.insert( trackUrl, track );
        sourceUrls.append( trackUrl );
    }

    QString loggerText = i18np( "Removing one track from %2",
                                "Removing %1 tracks from %2", sourceUrls.count(),
                                m_umsCollection->prettyName() );
    KIO::DeleteJob *delJob = KIO::del( sourceUrls, KIO::HideProgressInfo );
    Amarok::Components::logger()->newProgressOperation( delJob, loggerText, delJob, SLOT(kill()) );

    connect( delJob, SIGNAL(finished(KJob*)), SLOT(slotRemoveOperationFinished()) );
}

void
UmsCollectionLocation::slotTrackTransferred( const KUrl &sourceTrackUrl )
{
    Meta::TrackPtr sourceTrack = m_sourceUrlToTrackMap.value( sourceTrackUrl );
    if( !sourceTrack )
        warning() << __PRETTY_FUNCTION__ << ": I don't know about" << sourceTrackUrl;
    else
        // this is needed for example for "move" operation to actually remove source tracks
        source()->transferSuccessful( sourceTrack );
}

void UmsCollectionLocation::slotRemoveOperationFinished()
{
    foreach( Meta::TrackPtr track, m_sourceUrlToTrackMap )
    {
        KUrl trackUrl = track->playableUrl();
        if( !trackUrl.isLocalFile() // just pretend it was deleted
            || !QFileInfo( trackUrl.toLocalFile() ).exists() )
        {
            // good, the file was deleted. following is needed to trigger
            // CollectionLocation's functionality to remove empty dirs:
            transferSuccessful( track );
            m_umsCollection->slotTrackRemoved( track );
        }
    }
    CollectionLocation::slotRemoveOperationFinished();
}

UmsTransferJob::UmsTransferJob( UmsCollectionLocation* location,
                                const Transcoding::Configuration &configuration )
    : KCompositeJob( location )
    , m_location( location )
    , m_transcodingConfiguration( configuration )
    , m_abort( false )
{
    setCapabilities( KJob::Killable );
}

void
UmsTransferJob::addCopy( const KUrl &from, const KUrl &to )
{
    m_copyList << KUrlPair( from, to );
}

void
UmsTransferJob::addTranscode( const KUrl &from, const KUrl &to )
{
    m_transcodeList << KUrlPair( from, to );
}

void
UmsTransferJob::start()
{
    DEBUG_BLOCK;
    if( m_copyList.isEmpty() && m_transcodeList.isEmpty() )
        return;

    m_totalTracks = m_transcodeList.size() + m_copyList.size();
    startNextJob();
}

void
UmsTransferJob::slotCancel()
{
    m_abort = true;
}

void
UmsTransferJob::startNextJob()
{
    if( m_abort )
    {
        emitResult();
        return;
    }

    KJob *job;
    if( !m_transcodeList.isEmpty() )
    {
        KUrlPair urlPair = m_transcodeList.takeFirst();
        job = new Transcoding::Job( urlPair.first, urlPair.second, m_transcodingConfiguration );
    }
    else if( !m_copyList.isEmpty() )
    {
        KUrlPair urlPair = m_copyList.takeFirst();
        job = KIO::file_copy( urlPair.first, urlPair.second, -1, KIO::HideProgressInfo );
    }
    else
    {
        emitResult();
        return;
    }

    connect( job, SIGNAL(percent(KJob*,ulong)),
             SLOT(slotChildJobPercent(KJob*,ulong)) );
    addSubjob( job );
    job->start();  // no-op for KIO job, but matters for transcoding job
}

void
UmsTransferJob::slotChildJobPercent( KJob *job, unsigned long percentage )
{
    Q_UNUSED(job)
    // the -1 is for the current track that is being processed but already removed from transferList
    int alreadyTransferred = m_totalTracks - m_transcodeList.size() - m_copyList.size() - 1;
    emitPercent( alreadyTransferred * 100.0 + percentage, 100.0 * m_totalTracks );
}

void
UmsTransferJob::slotResult( KJob *job )
{
    removeSubjob( job );

    if( job->error() == KJob::NoError )
    {
        KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob *>( job );
        Transcoding::Job *transcodingJob = dynamic_cast<Transcoding::Job *>( job );
        if( copyJob )
        {
            emit sourceFileTransferDone( copyJob->srcUrl() );
            emit fileTransferDone( copyJob->destUrl() );
        }
        else if( transcodingJob )
        {
            emit sourceFileTransferDone( transcodingJob->srcUrl() );
            emit fileTransferDone( transcodingJob->destUrl() );
        }
        else
            Debug::warning() << __PRETTY_FUNCTION__ << "invalid job passed to me!";
    }
    else
        Debug::warning() << __PRETTY_FUNCTION__ << "job failed with" << job->error();

    // transcoding job currently doesn't emit percentage, so emit it at least once for track
    emitPercent( m_totalTracks - ( m_transcodeList.size() + m_copyList.size() ),
                 m_totalTracks );
    startNextJob();
}
