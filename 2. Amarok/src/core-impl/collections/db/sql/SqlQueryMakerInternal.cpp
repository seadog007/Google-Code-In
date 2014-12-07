/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#include "SqlQueryMakerInternal.h"

#include "core/collections/support/SqlStorage.h"
#include "core/support/Debug.h"
#include "SqlCollection.h"
#include "SqlMeta.h"
#include "SqlRegistry.h"

#include <QStringList>

using namespace Collections;

SqlQueryMakerInternal::SqlQueryMakerInternal( SqlCollection *collection )
    : QObject()
    , m_collection( collection )
    , m_queryType( QueryMaker::None )
{
}

SqlQueryMakerInternal::~ SqlQueryMakerInternal()
{
    disconnect();
}

void
SqlQueryMakerInternal::run()
{
    Q_ASSERT( !m_query.isEmpty() );
    if( !m_collection.isNull() )
    {
        QStringList result = m_collection.data()->sqlStorage()->query( m_query );
        handleResult( result );
    }
    else
    {
        deleteLater();
    }

}

void
SqlQueryMakerInternal::setQuery( const QString &query )
{
    //qDebug() << query;
    m_query = query;
}

void
SqlQueryMakerInternal::setQueryType( QueryMaker::QueryType type )
{
    m_queryType = type;
}

void
SqlQueryMakerInternal::handleResult( const QStringList &result )
{
    if( !result.isEmpty() )
    {
        switch( m_queryType ) {
        case QueryMaker::Custom:
            emit newResultReady( result );
            break;
        case QueryMaker::Track:
            handleTracks( result );
            break;
        case QueryMaker::Artist:
        case QueryMaker::AlbumArtist:
            handleArtists( result );
            break;
        case QueryMaker::Album:
            handleAlbums( result );
            break;
        case QueryMaker::Genre:
            handleGenres( result );
            break;
        case QueryMaker::Composer:
            handleComposers( result );
            break;
        case QueryMaker::Year:
            handleYears( result );
            break;
        case QueryMaker::Label:
            handleLabels( result );
            break;

        case QueryMaker::None:
            debug() << "Warning: queryResult with queryType == NONE";
        }
    }
    else
    {
        switch( m_queryType ) {
            case QueryMaker::Custom:
                emit newResultReady( QStringList() );
                break;
            case QueryMaker::Track:
                emit newResultReady( Meta::TrackList() );
                break;
            case QueryMaker::Artist:
            case QueryMaker::AlbumArtist:
                emit newResultReady( Meta::ArtistList() );
                break;
            case QueryMaker::Album:
                emit newResultReady( Meta::AlbumList() );
                break;
            case QueryMaker::Genre:
                emit newResultReady( Meta::GenreList() );
                break;
            case QueryMaker::Composer:
                emit newResultReady( Meta::ComposerList() );
                break;
            case QueryMaker::Year:
                emit newResultReady( Meta::YearList() );
                break;
            case QueryMaker::Label:
                emit newResultReady( Meta::LabelList() );
                break;

            case QueryMaker::None:
                debug() << "Warning: queryResult with queryType == NONE";
        }
    }

    //queryDone will be emitted in done(Job*)
}

void
SqlQueryMakerInternal::handleTracks( const QStringList &result )
{
    Meta::TrackList tracks;
    SqlRegistry* reg = m_collection.data()->registry();
    int returnCount = Meta::SqlTrack::getTrackReturnValueCount();
    int resultRows = result.size() / returnCount;
    for( int i = 0; i < resultRows; i++ )
    {
        QStringList row = result.mid( i*returnCount, returnCount );
        tracks.append( reg->getTrack( row[Meta::SqlTrack::returnIndex_trackId].toInt(), row ) );
    }
    emit newResultReady( tracks );
}

void
SqlQueryMakerInternal::handleArtists( const QStringList &result )
{
    Meta::ArtistList artists;
    SqlRegistry* reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString name = iter.next();
        QString id = iter.next();
        if( id.toInt() > 0 )
            artists.append( reg->getArtist( id.toInt(), name ) );
    }
    emit newResultReady( artists );
}

void
SqlQueryMakerInternal::handleAlbums( const QStringList &result )
{
    Meta::AlbumList albums;
    SqlRegistry* reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString name = iter.next();
        QString id = iter.next();
        QString artist = iter.next();
        albums.append( reg->getAlbum( id.toInt(), name, artist.toInt() ) );
    }
    emit newResultReady( albums );
}

void
SqlQueryMakerInternal::handleGenres( const QStringList &result )
{
    Meta::GenreList genres;
    SqlRegistry* reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString name = iter.next();
        QString id = iter.next();
        genres.append( reg->getGenre( id.toInt(), name ) );
    }
    emit newResultReady( genres );
}

void
SqlQueryMakerInternal::handleComposers( const QStringList &result )
{
    Meta::ComposerList composers;
    SqlRegistry* reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString name = iter.next();
        QString id = iter.next();
        composers.append( reg->getComposer( id.toInt(), name ) );
    }
    emit newResultReady( composers );
}

void
SqlQueryMakerInternal::handleYears( const QStringList &result )
{
    Meta::YearList years;
    SqlRegistry* reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString name = iter.next();
        QString id = iter.next();
        years.append( reg->getYear( id.toInt(), name.toInt() ) );
    }
    emit newResultReady( years );
}

void
SqlQueryMakerInternal::handleLabels( const QStringList &result )
{
    Meta::LabelList labels;
    SqlRegistry *reg = m_collection.data()->registry();
    for( QStringListIterator iter( result ); iter.hasNext(); )
    {
        QString label = iter.next();
        QString id = iter.next();
        labels.append( reg->getLabel( id.toInt(), label ) );
    }

    emit newResultReady( labels );
}

#include "SqlQueryMakerInternal.moc"
