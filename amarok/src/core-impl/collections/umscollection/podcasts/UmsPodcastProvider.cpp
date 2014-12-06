/****************************************************************************************
 * Copyright (c) 2010 Bart Cerneels <bart.cerneels@kde.org>                             *
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
#include "UmsPodcastProvider.h"
#include "core/support/Debug.h"

#include <KDialog>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KMimeType>

#include <QAction>
#include <QDirIterator>
#include <QLabel>
#include <QListWidget>
#include <QObject>
#include <QVBoxLayout>

using namespace Podcasts;

UmsPodcastProvider::UmsPodcastProvider( KUrl scanDirectory )
        : m_scanDirectory( scanDirectory )
        , m_deleteEpisodeAction( 0 )
        , m_deleteChannelAction( 0 )
{

}

UmsPodcastProvider::~UmsPodcastProvider()
{

}

bool
UmsPodcastProvider::possiblyContainsTrack( const KUrl &url ) const
{
    Q_UNUSED( url )
    return false;
}

Meta::TrackPtr
UmsPodcastProvider::trackForUrl( const KUrl &url )
{
    Q_UNUSED( url )
    return Meta::TrackPtr();
}

PodcastEpisodePtr
UmsPodcastProvider::episodeForGuid( const QString &guid )
{
    Q_UNUSED( guid )
    return PodcastEpisodePtr();
}

void
UmsPodcastProvider::addPodcast( const KUrl &url )
{
    Q_UNUSED( url );
}

PodcastChannelPtr
UmsPodcastProvider::addChannel( PodcastChannelPtr channel )
{
    UmsPodcastChannelPtr umsChannel = UmsPodcastChannelPtr(
            new UmsPodcastChannel( channel, this ) );
    m_umsChannels << umsChannel;

    emit playlistAdded( Playlists::PlaylistPtr( umsChannel.data() ) );
    return PodcastChannelPtr( umsChannel.data() );
}

PodcastEpisodePtr
UmsPodcastProvider::addEpisode( PodcastEpisodePtr episode )
{
    KUrl localFilePath = episode->playableUrl();
    if( !localFilePath.isLocalFile() )
        return PodcastEpisodePtr();

    KUrl destination = KUrl( m_scanDirectory );
    destination.addPath( Amarok::vfatPath( episode->channel()->prettyName() ) );
    KIO::mkdir( destination );
    destination.addPath( Amarok::vfatPath( localFilePath.fileName() ) );

    debug() << QString( "Copy episode \"%1\" to %2" ).arg( localFilePath.path())
            .arg( destination.path() );
    KIO::FileCopyJob *copyJob = KIO::file_copy( localFilePath, destination );
    connect( copyJob, SIGNAL(result(KJob*)), SLOT(slotCopyComplete(KJob*)) );
    copyJob->start();
    //we have not copied the data over yet so we can't return an episode yet
    //TODO: return a proxy for the episode we are still copying.
    return PodcastEpisodePtr();
}

void
UmsPodcastProvider::slotCopyComplete( KJob *job )
{
    KIO::FileCopyJob *copyJob = dynamic_cast<KIO::FileCopyJob *>( job );
    if( !copyJob )
        return;

    KUrl localFilePath = copyJob->destUrl();
    MetaFile::Track *fileTrack = new MetaFile::Track( localFilePath );

    UmsPodcastEpisodePtr umsEpisode = addFile( MetaFile::TrackPtr( fileTrack ) );
}

PodcastChannelList
UmsPodcastProvider::channels()
{
    return UmsPodcastChannel::toPodcastChannelList( m_umsChannels );
}

void
UmsPodcastProvider::removeSubscription( PodcastChannelPtr channel )
{
    UmsPodcastChannelPtr umsChannel = UmsPodcastChannelPtr::dynamicCast( channel );
    if( umsChannel.isNull() )
    {
        error() << "trying to remove a podcast channel of the wrong type";
        return;
    }

    if( !m_umsChannels.contains( umsChannel ) )
    {
        error() << "trying to remove a podcast channel that is not in the list";
        return;
    }

    m_umsChannels.removeAll( umsChannel );
}

void
UmsPodcastProvider::configureProvider()
{
}

void
UmsPodcastProvider::configureChannel( PodcastChannelPtr channel )
{
    Q_UNUSED( channel );
}

QString
UmsPodcastProvider::prettyName() const
{
    return i18nc( "Podcasts on a media device", "Podcasts on %1", QString("TODO: replace me") );
}

KIcon
UmsPodcastProvider::icon() const
{
    return KIcon("drive-removable-media-usb-pendrive");
}

Playlists::PlaylistList
UmsPodcastProvider::playlists()
{
    Playlists::PlaylistList playlists;
    foreach( UmsPodcastChannelPtr channel, m_umsChannels )
        playlists << Playlists::PlaylistPtr::dynamicCast( channel );
    return playlists;
}

QActionList
UmsPodcastProvider::episodeActions( PodcastEpisodeList episodes )
{
    QActionList actions;
    if( episodes.isEmpty() )
        return actions;

    if( m_deleteEpisodeAction == 0 )
    {
        m_deleteEpisodeAction = new QAction( KIcon( "edit-delete" ), i18n( "&Delete Episode" ), this );
        m_deleteEpisodeAction->setProperty( "popupdropper_svg_id", "delete" );
        connect( m_deleteEpisodeAction, SIGNAL(triggered()), SLOT(slotDeleteEpisodes()) );
    }
    // set the episode list as data that we'll retrieve in the slot
    m_deleteEpisodeAction->setData( QVariant::fromValue( episodes ) );
    actions << m_deleteEpisodeAction;

    return actions;
}

void
UmsPodcastProvider::slotDeleteEpisodes()
{
    DEBUG_BLOCK
    QAction *action = qobject_cast<QAction *>( QObject::sender() );
    if( action == 0 )
        return;

    //get the list of episodes to apply to, then clear that data.
    PodcastEpisodeList episodes =
            action->data().value<PodcastEpisodeList>();
    action->setData( QVariant() );

    UmsPodcastEpisodeList umsEpisodes;
    foreach( PodcastEpisodePtr episode, episodes )
    {
        UmsPodcastEpisodePtr umsEpisode =
                UmsPodcastEpisode::fromPodcastEpisodePtr( episode );
        if( !umsEpisode )
        {
            error() << "Could not cast to UmsPodcastEpisode";
            continue;
        }

        PodcastChannelPtr channel = umsEpisode->channel();
        if( !channel )
        {
            error() << "episode did not have a valid channel";
            continue;
        }

        UmsPodcastChannelPtr umsChannel =
                UmsPodcastChannel::fromPodcastChannelPtr( channel );
        if( !umsChannel )
        {
            error() << "Could not cast to UmsPodcastChannel";
            continue;
        }

        umsEpisodes << umsEpisode;
    }

    deleteEpisodes( umsEpisodes );
}

void
UmsPodcastProvider::deleteEpisodes( UmsPodcastEpisodeList umsEpisodes )
{
    KUrl::List urlsToDelete;
    foreach( UmsPodcastEpisodePtr umsEpisode, umsEpisodes )
        urlsToDelete << umsEpisode->playableUrl();

    KDialog dialog;
    dialog.setCaption( i18n( "Confirm Delete" ) );
    dialog.setButtons( KDialog::Ok | KDialog::Cancel );
    QLabel label( i18np( "Are you sure you want to delete this episode?",
                         "Are you sure you want to delete these %1 episodes?",
                         urlsToDelete.count() )
                    , &dialog
                  );
    QListWidget listWidget( &dialog );
    listWidget.setSelectionMode( QAbstractItemView::NoSelection );
    foreach( const KUrl &url, urlsToDelete )
    {
        new QListWidgetItem( url.toLocalFile(), &listWidget );
    }

    QWidget *widget = new QWidget( &dialog );
    QVBoxLayout *layout = new QVBoxLayout( widget );
    layout->addWidget( &label );
    layout->addWidget( &listWidget );
    dialog.setButtonText( KDialog::Ok, i18n( "Yes, delete from %1.",
                                             QString("TODO: replace me") ) );

    dialog.setMainWidget( widget );
    if( dialog.exec() != QDialog::Accepted )
        return;

    KIO::DeleteJob *deleteJob = KIO::del( urlsToDelete, KIO::HideProgressInfo );

    //keep track of these episodes until the job is done
    m_deleteJobMap.insert( deleteJob, umsEpisodes );

    connect( deleteJob, SIGNAL(result(KJob*)),
             SLOT(deleteJobComplete(KJob*)) );
}

void
UmsPodcastProvider::deleteJobComplete( KJob *job )
{
    DEBUG_BLOCK
    if( job->error() )
    {
        error() << "problem deleting episode(s): " << job->errorString();
        return;
    }

    UmsPodcastEpisodeList deletedEpisodes = m_deleteJobMap.take( job );
    foreach( UmsPodcastEpisodePtr deletedEpisode, deletedEpisodes )
    {
        PodcastChannelPtr channel = deletedEpisode->channel();
        UmsPodcastChannelPtr umsChannel =
                UmsPodcastChannel::fromPodcastChannelPtr( channel );
        if( !umsChannel )
        {
            error() << "Could not cast to UmsPodcastChannel";
            continue;
        }

        umsChannel->removeEpisode( deletedEpisode );
        if( umsChannel->m_umsEpisodes.isEmpty() )
        {
            debug() << "channel is empty now, remove it";
            m_umsChannels.removeAll( umsChannel );
            emit( playlistRemoved( Playlists::PlaylistPtr::dynamicCast( umsChannel ) ) );
        }
    }
}

QActionList
UmsPodcastProvider::channelActions( PodcastChannelList channels )
{
    QActionList actions;
    if( channels.isEmpty() )
        return actions;

    if( m_deleteChannelAction == 0 )
    {
        m_deleteChannelAction = new QAction( KIcon( "edit-delete" ), i18n( "&Delete "
                "Channel and Episodes" ), this );
        m_deleteChannelAction->setProperty( "popupdropper_svg_id", "delete" );
        connect( m_deleteChannelAction, SIGNAL(triggered()), SLOT(slotDeleteChannels()) );
    }
    // set the episode list as data that we'll retrieve in the slot
    m_deleteChannelAction->setData( QVariant::fromValue( channels ) );
    actions << m_deleteChannelAction;

    return actions;
}

void
UmsPodcastProvider::slotDeleteChannels()
{
    DEBUG_BLOCK
    QAction *action = qobject_cast<QAction *>( QObject::sender() );
    if( action == 0 )
        return;

    //get the list of episodes to apply to, then clear that data.
    PodcastChannelList channels =
            action->data().value<PodcastChannelList>();
    action->setData( QVariant() );

    foreach( PodcastChannelPtr channel, channels )
    {
        UmsPodcastChannelPtr umsChannel =
                UmsPodcastChannel::fromPodcastChannelPtr( channel );
        if( !umsChannel )
        {
            error() << "Could not cast to UmsPodcastChannel";
            continue;
        }

        deleteEpisodes( umsChannel->m_umsEpisodes );
        //slot deleteJobComplete() will emit signal once all tracks are gone.
    }
}

QActionList
UmsPodcastProvider::playlistActions( const Playlists::PlaylistList &playlists )
{
    PodcastChannelList channels;
    foreach( const Playlists::PlaylistPtr &playlist, playlists )
    {
        PodcastChannelPtr channel = PodcastChannelPtr::dynamicCast( playlist );
        if( channel )
            channels << channel;
    }

    return channelActions( channels );
}

QActionList
UmsPodcastProvider::trackActions( const QMultiHash<Playlists::PlaylistPtr, int> &playlistTracks )
{
    PodcastEpisodeList episodes;
    foreach( const Playlists::PlaylistPtr &playlist, playlistTracks.uniqueKeys() )
    {
        PodcastChannelPtr channel = PodcastChannelPtr::dynamicCast( playlist );
        if( !channel )
            continue;

        PodcastEpisodeList channelEpisodes = channel->episodes();
        QList<int> trackPositions = playlistTracks.values( playlist );
        qSort( trackPositions );
        foreach( int trackPosition, trackPositions )
        {
            if( trackPosition >= 0 && trackPosition < channelEpisodes.count() )
                episodes << channelEpisodes.at( trackPosition );
        }
    }

    return episodeActions( episodes );
}

void
UmsPodcastProvider::completePodcastDownloads()
{

}

void
UmsPodcastProvider::updateAll() //slot
{
}

void
UmsPodcastProvider::update( Podcasts::PodcastChannelPtr channel ) //slot
{
    Q_UNUSED( channel );
}

void
UmsPodcastProvider::downloadEpisode( Podcasts::PodcastEpisodePtr episode ) //slot
{
    Q_UNUSED( episode );
}

void
UmsPodcastProvider::deleteDownloadedEpisode( Podcasts::PodcastEpisodePtr episode ) //slot
{
    Q_UNUSED( episode );
}

void
UmsPodcastProvider::slotUpdated() //slot
{

}

void
UmsPodcastProvider::scan()
{
    if( m_scanDirectory.isEmpty() )
        return;
    m_dirList.clear();
    debug() << "scan directory for podcasts: " <<
            m_scanDirectory.toLocalFile( KUrl::AddTrailingSlash );
    QDirIterator it( m_scanDirectory.toLocalFile(), QDirIterator::Subdirectories );
    while( it.hasNext() )
        addPath( it.next() );
}

int
UmsPodcastProvider::addPath( const QString &path )
{
    DEBUG_BLOCK
    int acc = 0;
    debug() << path;
    KMimeType::Ptr mime = KMimeType::findByFileContent( path, &acc );
    if( !mime || mime->name() == KMimeType::defaultMimeType() )
    {
        debug() << "Trying again with findByPath:" ;
        mime = KMimeType::findByPath( path, 0, true, &acc );
        if( mime->name() == KMimeType::defaultMimeType() )
            return 0;
    }
    debug() << "Got type: " << mime->name() << ", with accuracy: " << acc;

    QFileInfo info( path );
    if( info.isDir() )
    {
        if( m_dirList.contains( path ) )
            return 0;
        m_dirList << info.canonicalPath();
        return 1;
    }
    else if( info.isFile() )
    {
//        foreach( const QString &mimetype, m_handler->mimetypes() )
//        {
//            if( mime->is( mimetype ) )
//            {
                addFile( MetaFile::TrackPtr( new MetaFile::Track(
                        KUrl( info.canonicalFilePath() ) ) ) );
                return 2;
//            }
//        }
    }

    return 0;
}

UmsPodcastEpisodePtr
UmsPodcastProvider::addFile( MetaFile::TrackPtr metafileTrack )
{
    DEBUG_BLOCK
    debug() << metafileTrack->playableUrl().url();
    debug() << "album: " << metafileTrack->album()->name();
    debug() << "title: " << metafileTrack->name();
    if( metafileTrack->album()->name().isEmpty() )
    {
        debug() << "Can't figure out channel without album tag.";
        return UmsPodcastEpisodePtr();
    }

    if( metafileTrack->name().isEmpty() )
    {
        debug() << "Can not use a track without a title.";
        return UmsPodcastEpisodePtr();
    }

    //see if there is already a UmsPodcastEpisode for this track
    UmsPodcastChannelPtr channel;
    UmsPodcastEpisodePtr episode;

    foreach( UmsPodcastChannelPtr c, m_umsChannels )
    {
        if( c->name() == metafileTrack->album()->name() )
        {
            channel = c;
            break;
        }
    }

    if( channel )
    {
        foreach( UmsPodcastEpisodePtr e, channel->umsEpisodes() )
        {
            if( e->title() == metafileTrack->name() )
            {
                episode = e;
                break;
            }
        }
    }
    else
    {
        debug() << "there is no channel for this episode yet";
        channel = UmsPodcastChannelPtr( new UmsPodcastChannel( this ) );
        channel->setTitle( metafileTrack->album()->name() );
        m_umsChannels << channel;
        emit playlistAdded( Playlists::PlaylistPtr( channel.data() ) );
    }

    if( episode.isNull() )
    {
        debug() << "this episode was not found in an existing channel";
        episode = UmsPodcastEpisodePtr( new UmsPodcastEpisode( channel ) );
        episode->setLocalFile( metafileTrack );

        channel->addUmsEpisode( episode );
    }

    episode->setLocalFile( metafileTrack );

    return episode;
}
