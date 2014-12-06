/****************************************************************************************
 * Copyright (c) 2007-2009 Nikolaj Hald Nielsen <nhn@kde.org>                           *
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

#include "ScriptableServiceMeta.h"
#include "ScriptableServiceMeta_p.h"

#include "core/meta/support/PrivateMetaRegistry.h"
#include "core-impl/meta/multi/MultiTrack.h"
#include "core-impl/playlists/types/file/PlaylistFileSupport.h"
#include "services/scriptable/ScriptableService.h"
#include "services/scriptable/ScriptableServiceManager.h"

using namespace Meta;

ScriptableServiceMetaItem::ScriptableServiceMetaItem( int level )
    : m_callbackString( QString() )
    , m_level( level )
    , m_serviceName( QString() )
    , m_serviceDescription( QString() )
    , m_serviceEmblem( QPixmap() )
{}

void Meta::ScriptableServiceMetaItem::setCallbackString( const QString &callbackString )
{ 
    m_callbackString = callbackString;
}

QString Meta::ScriptableServiceMetaItem::callbackString() const
{
    return m_callbackString;
}

int Meta::ScriptableServiceMetaItem::level() const
{
    return m_level;
}

void Meta::ScriptableServiceMetaItem::setServiceName( const QString & name )
{
    m_serviceName = name;
}

void Meta::ScriptableServiceMetaItem::setServiceDescription( const QString & description )
{
    m_serviceDescription = description;
}

void Meta::ScriptableServiceMetaItem::setServiceEmblem( const QPixmap & emblem )
{
    m_serviceEmblem = emblem;
}

void Meta::ScriptableServiceMetaItem::setServiceScalableEmblem ( const QString& emblemPath )
{
    m_serviceScalableEmblem = emblemPath;
}


/* ScriptableServiceTrack */
ScriptableServiceTrack::ScriptableServiceTrack( const QString & name )
    : ServiceTrack( name )
    , ScriptableServiceMetaItem( 0 )
{}

ScriptableServiceTrack::ScriptableServiceTrack( const QStringList & resultRow )
    : ServiceTrack( resultRow )
    , ScriptableServiceMetaItem( 0 )
{}


void Meta::ScriptableServiceTrack::setAlbumName( const QString &newAlbum )
{
    Meta::AlbumPtr albumPtr = Meta::PrivateMetaRegistry::instance()->album( m_serviceName, newAlbum );
    if ( !albumPtr ) {
        ScriptableServiceInternalAlbum * intAlbum = new ScriptableServiceInternalAlbum( newAlbum );
        intAlbum->setServiceName( m_serviceName );
        intAlbum->setServiceDescription( m_serviceDescription );
        intAlbum->setServiceEmblem( m_serviceEmblem );
        intAlbum->setServiceScalableEmblem( m_serviceScalableEmblem );
        albumPtr = Meta::AlbumPtr( intAlbum );
        Meta::PrivateMetaRegistry::instance()->insertAlbum( m_serviceName, newAlbum, albumPtr );
    }

    setAlbumPtr( albumPtr );
}

void Meta::ScriptableServiceTrack::setArtistName( const QString &newArtist )
{
    Meta::ArtistPtr artistPtr = Meta::PrivateMetaRegistry::instance()->artist( m_serviceName, newArtist );
    if ( !artistPtr ) {
        ScriptableServiceInternalArtist * intArtist = new ScriptableServiceInternalArtist( newArtist );
        intArtist->setServiceName( m_serviceName );
        intArtist->setServiceDescription( m_serviceDescription );
        intArtist->setServiceEmblem( m_serviceEmblem );
        intArtist->setServiceScalableEmblem( m_serviceScalableEmblem );
        artistPtr = Meta::ArtistPtr( intArtist );
        Meta::PrivateMetaRegistry::instance()->insertArtist( m_serviceName, newArtist, artistPtr );
    }

    setArtist( artistPtr );
}

void Meta::ScriptableServiceTrack::setGenreName( const QString &newGenre )
{
    Meta::GenrePtr genrePtr = Meta::PrivateMetaRegistry::instance()->genre( m_serviceName, newGenre );
    if ( !genrePtr ) {
        ScriptableServiceInternalGenre * intGenre = new ScriptableServiceInternalGenre( newGenre );
        intGenre->setServiceName( m_serviceName );
        intGenre->setServiceDescription( m_serviceDescription );
        intGenre->setServiceEmblem( m_serviceEmblem );
        intGenre->setServiceScalableEmblem( m_serviceScalableEmblem );
        genrePtr = Meta::GenrePtr( intGenre );
        Meta::PrivateMetaRegistry::instance()->insertGenre( m_serviceName, newGenre, genrePtr );
    }

    setGenre( genrePtr );
}

void Meta::ScriptableServiceTrack::setComposerName( const QString &newComposer )
{
    Meta::ComposerPtr composerPtr = Meta::PrivateMetaRegistry::instance()->composer( m_serviceName, newComposer );
    if ( !composerPtr ) {
        ScriptableServiceInternalComposer * intComposer = new ScriptableServiceInternalComposer( newComposer);
        intComposer->setServiceName( m_serviceName );
        intComposer->setServiceDescription( m_serviceDescription );
        intComposer->setServiceEmblem( m_serviceEmblem );
        intComposer->setServiceScalableEmblem( m_serviceScalableEmblem );
        composerPtr = Meta::ComposerPtr( intComposer );
        Meta::PrivateMetaRegistry::instance()->insertComposer( m_serviceName, newComposer, composerPtr );
    }

    setComposer( composerPtr );
}

void Meta::ScriptableServiceTrack::setYearNumber( int newYear )
{
    const QString yearString = QString::number( newYear );
    
    Meta::YearPtr yearPtr = Meta::PrivateMetaRegistry::instance()->year( m_serviceName, yearString );
    if ( !yearPtr ) {
        ScriptableServiceInternalYear * intYear = new ScriptableServiceInternalYear( yearString );
        intYear->setServiceName( m_serviceName );
        intYear->setServiceDescription( m_serviceDescription );
        intYear->setServiceEmblem( m_serviceEmblem );
        intYear->setServiceScalableEmblem( m_serviceScalableEmblem );
        yearPtr = Meta::YearPtr( intYear );
        Meta::PrivateMetaRegistry::instance()->insertYear( m_serviceName, yearString, yearPtr );
    }

    setYear( yearPtr );
}

void Meta::ScriptableServiceTrack::setCustomAlbumCoverUrl( const QString & coverurl )
{
    DEBUG_BLOCK
    if ( album() ) {
        debug() << "one";
        ServiceAlbumWithCoverPtr albumWithCover = ServiceAlbumWithCoverPtr::dynamicCast( album() );
        if ( albumWithCover ) {
            debug() << "two";
            albumWithCover->setCoverUrl( coverurl );
        }
    }
}


QString Meta::ScriptableServiceTrack::sourceName()
{
    return m_serviceName;
}

QString Meta::ScriptableServiceTrack::sourceDescription()
{
    return m_serviceDescription;
}

QPixmap Meta::ScriptableServiceTrack::emblem()
{
    return m_serviceEmblem;
}

QString Meta::ScriptableServiceTrack::scalableEmblem()
{
    return  m_serviceScalableEmblem;
}

void ScriptableServiceTrack::setUidUrl( const QString &url )
{
    Meta::ServiceTrack::setUidUrl( url );

    using namespace Playlists;
    Meta::TrackPtr track( this );
    PlaylistPtr playlist = canExpand( track ) ? expand( track ) : PlaylistPtr();
    if( playlist )
        // since this is a playlist masqueurading as a single track, make a MultiTrack out of it:
        m_playableTrack = Meta::TrackPtr( new Meta::MultiTrack( playlist ) );
    else
        m_playableTrack = TrackPtr();
}

TrackPtr ScriptableServiceTrack::playableTrack() const
{
    return m_playableTrack ? m_playableTrack : TrackPtr( const_cast<ScriptableServiceTrack *>( this ) );
}


/* DynamicScriptableAlbum */
ScriptableServiceAlbum::ScriptableServiceAlbum( const QString & name )
    : ServiceAlbumWithCover( name )
    , ScriptableServiceMetaItem( 1 )
{}

ScriptableServiceAlbum::ScriptableServiceAlbum( const QStringList & resultRow )
    : ServiceAlbumWithCover( resultRow )
    , ScriptableServiceMetaItem( 1 )
{}

QString Meta::ScriptableServiceAlbum::sourceName()
{
    return m_serviceName;
}

QString Meta::ScriptableServiceAlbum::sourceDescription()
{
    return m_serviceDescription;
}

QPixmap Meta::ScriptableServiceAlbum::emblem()
{
    return m_serviceEmblem;
}

QString Meta::ScriptableServiceAlbum::scalableEmblem()
{
    return  m_serviceScalableEmblem;
}

bool Meta::ScriptableServiceAlbum::isBookmarkable() const
{
    ScriptableService * service = The::scriptableServiceManager()->service( m_serviceName );
    if ( service )
        return service->hasSearchBar();
    else
        return false;
}



/* ScriptableServiceArtist */
ScriptableServiceArtist::ScriptableServiceArtist( const QString & name )
    : ServiceArtist( name )
    , ScriptableServiceMetaItem( 2 )
    , m_genreId( 0 )
{}

ScriptableServiceArtist::ScriptableServiceArtist( const QStringList & resultRow )
    : ServiceArtist( resultRow )
    , ScriptableServiceMetaItem( 2 )
    , m_genreId( 0 )
{}


void Meta::ScriptableServiceArtist::setGenreId(int genreId)
{
    m_genreId = genreId;
}


int Meta::ScriptableServiceArtist::genreId() const
{
    return m_genreId;
}

QString Meta::ScriptableServiceArtist::sourceName()
{
    return m_serviceName;
}

QString Meta::ScriptableServiceArtist::sourceDescription()
{
    return m_serviceDescription;
}

QPixmap Meta::ScriptableServiceArtist::emblem()
{
    return m_serviceEmblem;
}

QString Meta::ScriptableServiceArtist::scalableEmblem()
{
    return  m_serviceScalableEmblem;
}

bool Meta::ScriptableServiceArtist::isBookmarkable()  const
{
    ScriptableService * service = The::scriptableServiceManager()->service( m_serviceName );
    if ( service )
        return service->hasSearchBar();
    else
        return false;
}


/* ScriptableServiceGenre */
ScriptableServiceGenre::ScriptableServiceGenre( const QString & name )
    : ServiceGenre( name )
    , ScriptableServiceMetaItem( 3 )
{}

ScriptableServiceGenre::ScriptableServiceGenre( const QStringList & resultRow )
    : ServiceGenre( resultRow )
    , ScriptableServiceMetaItem( 3 )
{}


void Meta::ScriptableServiceGenre::setDescription(const QString & description)
{
    m_description = description;
}

QString Meta::ScriptableServiceGenre::description()
{
    return m_description;
}

QString Meta::ScriptableServiceGenre::sourceName()
{
    return m_serviceName;
}

QString Meta::ScriptableServiceGenre::sourceDescription()
{
    return m_serviceDescription;
}

QPixmap Meta::ScriptableServiceGenre::emblem()
{
    return m_serviceEmblem;
}

QString Meta::ScriptableServiceGenre::scalableEmblem()
{
    return  m_serviceScalableEmblem;
}







