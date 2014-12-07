/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#include "AmarokMimeData.h"

#include "core/support/Debug.h"

#include <QCoreApplication>
#include <QList>
#include <QTimer>
#include <QUrl>

const QString AmarokMimeData::TRACK_MIME = "application/x-amarok-tracks";
const QString AmarokMimeData::PLAYLIST_MIME = "application/x-amarok-playlists";
const QString AmarokMimeData::PLAYLISTBROWSERGROUP_MIME = "application/x-amarok-playlistbrowsergroup";
const QString AmarokMimeData::PODCASTCHANNEL_MIME = "application/x-amarok-podcastchannel";
const QString AmarokMimeData::PODCASTEPISODE_MIME = "application/x-amarok-podcastepisode";
const QString AmarokMimeData::AMAROKURL_MIME = "application/x-amarok-amarokurl";
const QString AmarokMimeData::BOOKMARKGROUP_MIME = "application/x-amarok-bookmarkgroup";


class AmarokMimeData::Private
{
public:
    Private() : deleteQueryMakers( true ), completedQueries( 0 )
    {}

    ~Private()
    {
        if( deleteQueryMakers )
            qDeleteAll( queryMakers );
    }

    Meta::TrackList tracks;
    Playlists::PlaylistList playlists;
    QStringList playlistGroups;
    Podcasts::PodcastChannelList m_podcastChannels;
    Podcasts::PodcastEpisodeList m_podcastEpisodes;
    QList<Collections::QueryMaker*> queryMakers;
    QMap<Collections::QueryMaker*, Meta::TrackList> trackMap;
    QMap<Collections::QueryMaker*, Playlists::PlaylistList> playlistMap;
    BookmarkList bookmarks;
    BookmarkGroupList bookmarkGroups;

    bool deleteQueryMakers;
    int completedQueries;

};

AmarokMimeData::AmarokMimeData()
    : QMimeData()
    , d( new Private() )
{
    //nothing to do
}

AmarokMimeData::~AmarokMimeData()
{
    delete d;
}

QStringList
AmarokMimeData::formats() const
{
    QStringList formats( QMimeData::formats() );
    if( !d->tracks.isEmpty() || !d->queryMakers.isEmpty() || !d->playlistGroups.isEmpty() || !d->bookmarks.isEmpty() || !d->bookmarkGroups.isEmpty() )
    {
        formats.append( TRACK_MIME );
        formats.append( PLAYLIST_MIME );
        formats.append( PLAYLISTBROWSERGROUP_MIME );
        formats.append( PODCASTCHANNEL_MIME );
        formats.append( PODCASTEPISODE_MIME );
        formats.append( BOOKMARKGROUP_MIME );
        formats.append( AMAROKURL_MIME );

        if( !formats.contains( "text/uri-list" ) )
            formats.append( "text/uri-list" );
        if( !formats.contains( "text/plain" ) )
            formats.append( "text/plain" );
    }

    return formats;
}

bool
AmarokMimeData::hasFormat( const QString &mimeType ) const
{
    if( mimeType == TRACK_MIME )
        return !d->tracks.isEmpty() || !d->queryMakers.isEmpty();
    else if( mimeType == PLAYLIST_MIME )
        return !d->playlists.isEmpty() || !d->queryMakers.isEmpty();
    else if( mimeType == PLAYLISTBROWSERGROUP_MIME )
        return !d->playlistGroups.isEmpty();
    else if( mimeType == PODCASTCHANNEL_MIME )
        return !d->m_podcastChannels.isEmpty();
    else if( mimeType == PODCASTEPISODE_MIME )
        return !d->m_podcastEpisodes.isEmpty();
    else if( mimeType == BOOKMARKGROUP_MIME )
        return !d->bookmarkGroups.isEmpty();
    else if( mimeType == AMAROKURL_MIME )
        return !d->bookmarks.isEmpty();
    else if( mimeType == "text/uri-list" || mimeType == "text/plain" )
        return !d->tracks.isEmpty() || !d->playlists.isEmpty()
            || !d->m_podcastChannels.isEmpty() || !d->m_podcastEpisodes.isEmpty()
            || !d->queryMakers.isEmpty();
    else
        return QMimeData::hasFormat( mimeType );
}

Meta::TrackList
AmarokMimeData::tracks() const
{
    while( d->completedQueries < d->queryMakers.count() )
    {
        QCoreApplication::instance()->processEvents( QEventLoop::ExcludeUserInputEvents );
    }
    Meta::TrackList result = d->tracks;
    foreach( Collections::QueryMaker *qm, d->queryMakers )
    {
        if( d->trackMap.contains( qm ) )
            result << d->trackMap.value( qm );
    }
    return result;
}

void
AmarokMimeData::setTracks( const Meta::TrackList &tracks )
{
    d->tracks = tracks;
}

void
AmarokMimeData::addTracks( const Meta::TrackList &tracks )
{
    d->tracks << tracks;
}

void
AmarokMimeData::getTrackListSignal() const
{
    if( d->completedQueries < d->queryMakers.count() )
    {
        QTimer::singleShot( 0, const_cast<AmarokMimeData*>( this ), SLOT(getTrackListSignal()) );
        return;
    }
    else
    {
        Meta::TrackList result = d->tracks;
        foreach( Collections::QueryMaker *qm, d->queryMakers )
        {
            if( d->trackMap.contains( qm ) )
                result << d->trackMap.value( qm );
        }
        emit trackListSignal( result );
    }
}

Playlists::PlaylistList
AmarokMimeData::playlists() const
{
    while( d->completedQueries < d->queryMakers.count() )
    {
        QCoreApplication::instance()->processEvents( QEventLoop::AllEvents );
    }
    Playlists::PlaylistList result = d->playlists;
    return result;
}

void
AmarokMimeData::setPlaylists( const Playlists::PlaylistList &playlists )
{
    d->playlists = playlists;
}

void
AmarokMimeData::addPlaylists( const Playlists::PlaylistList &playlists )
{
    d->playlists << playlists;
}

QStringList
AmarokMimeData::playlistGroups() const
{
    return d->playlistGroups;
}

void
AmarokMimeData::setPlaylistGroups( const QStringList &groups )
{
    d->playlistGroups = groups;
}

void
AmarokMimeData::addPlaylistGroup( const QString &group )
{
    d->playlistGroups << group;
}

Podcasts::PodcastChannelList
AmarokMimeData::podcastChannels() const
{
    return d->m_podcastChannels;
}

void
AmarokMimeData::setPodcastChannels( const Podcasts::PodcastChannelList &channels )
{
    d->m_podcastChannels = channels;
}

void
AmarokMimeData::addPodcastChannels( const Podcasts::PodcastChannelList &channels )
{
    d->m_podcastChannels << channels;
}

Podcasts::PodcastEpisodeList
AmarokMimeData::podcastEpisodes() const
{
    return d->m_podcastEpisodes;
}

void
AmarokMimeData::setPodcastEpisodes( const Podcasts::PodcastEpisodeList &episodes )
{
    d->m_podcastEpisodes = episodes;
}

void
AmarokMimeData::addPodcastEpisodes( const Podcasts::PodcastEpisodeList &episodes )
{
    d->m_podcastEpisodes << episodes;
}

QList<Collections::QueryMaker*>
AmarokMimeData::queryMakers()
{
    d->deleteQueryMakers = false;
    return d->queryMakers;
}

void
AmarokMimeData::addQueryMaker( Collections::QueryMaker *queryMaker )
{
    d->queryMakers.append( queryMaker );
}

void
AmarokMimeData::setQueryMakers( const QList<Collections::QueryMaker*> &queryMakers )
{
    d->queryMakers << queryMakers;
}

BookmarkList AmarokMimeData::bookmarks() const
{
    return d->bookmarks;
}

void AmarokMimeData::setBookmarks( const BookmarkList &bookmarks )
{
    d->bookmarks = bookmarks;
}

void AmarokMimeData::addBookmarks( const BookmarkList &bookmarks )
{
    d->bookmarks << bookmarks;
}

BookmarkGroupList AmarokMimeData::bookmarkGroups() const
{
    return d->bookmarkGroups;
}

void AmarokMimeData::setBookmarkGroups( const BookmarkGroupList &groups )
{
    d->bookmarkGroups = groups;
}

void AmarokMimeData::addBookmarkGroups( const BookmarkGroupList &groups )
{
    d->bookmarkGroups << groups;
}

QVariant
AmarokMimeData::retrieveData( const QString &mimeType, QVariant::Type type ) const
{
    Meta::TrackList tracks = this->tracks();
    Playlists::PlaylistList playlists = this->playlists();
    Podcasts::PodcastChannelList channels = this->podcastChannels();
    Podcasts::PodcastEpisodeList episodes = this->podcastEpisodes();
    if( !tracks.isEmpty() )
    {
        if( mimeType == "text/uri-list" && (type == QVariant::List || type == QVariant::ByteArray) )
        {
            QList<QVariant> list;
            foreach( Meta::TrackPtr track, tracks )
            {
                list.append( QVariant( QUrl( track->playableUrl() ) ) );
            }
            foreach( Podcasts::PodcastEpisodePtr episode, episodes )
            {
                list.append( QVariant( QUrl( episode->playableUrl() ) ) );
            }
            foreach( Playlists::PlaylistPtr playlist, playlists )
            {
                list.append( QVariant( QUrl( playlist->uidUrl() ) ) );
            }
            foreach( Podcasts::PodcastChannelPtr channel, channels )
            {
                list.append( QVariant( QUrl( channel->url() ) ) );
            }
            return QVariant( list );
        }
        if( mimeType == "text/plain" && (type == QVariant::String || type == QVariant::ByteArray) )
        {
            QString result;
            foreach( Meta::TrackPtr track, tracks )
            {
                if( !result.isEmpty() )
                    result += '\n';
                result += track->artist()->prettyName();
                result += " - ";
                result += track->prettyName();
            }
            foreach( Podcasts::PodcastEpisodePtr episode, episodes )
            {
                if( !result.isEmpty() )
                    result += '\n';
                result += episode->prettyName();
                result += " - ";
                result += episode->channel()->prettyName();
            }
            foreach( Playlists::PlaylistPtr playlist, playlists )
            {
                if( !result.isEmpty() )
                    result += '\n';
                result += playlist->prettyName();
            }
            foreach( Podcasts::PodcastChannelPtr channel, channels )
            {
                if( !result.isEmpty() )
                    result += '\n';
                result += channel->prettyName();
            }
            return QVariant( result );
        }
    }
    return QMimeData::retrieveData( mimeType, type );
}

void
AmarokMimeData::startQueries()
{
    foreach( Collections::QueryMaker *qm, d->queryMakers )
    {
        qm->setQueryType( Collections::QueryMaker::Track );
        connect( qm, SIGNAL(newResultReady(Meta::TrackList)), this, SLOT(newResultReady(Meta::TrackList)), Qt::QueuedConnection );
        connect( qm, SIGNAL(queryDone()), this, SLOT(queryDone()), Qt::QueuedConnection );
        qm->run();
    }
}

void
AmarokMimeData::newResultReady( const Meta::TrackList &tracks )
{
    Collections::QueryMaker *qm = dynamic_cast<Collections::QueryMaker*>( sender() );
    if( qm )
    {
        d->trackMap.insert( qm, tracks );
    }
    else
        d->tracks << tracks;
}

void
AmarokMimeData::queryDone()
{
    d->completedQueries++;
}


#include "AmarokMimeData.moc"
