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

#define DEBUG_PREFIX "AggregateQueryMaker"

#include "AggregateQueryMaker.h"

#include "core/meta/Meta.h"
#include "core/support/Debug.h"
#include "core-impl/collections/aggregate/AggregateCollection.h"
#include "core-impl/collections/support/MemoryCustomValue.h"
#include "core-impl/collections/support/MemoryQueryMakerHelper.h"

#include <QMetaEnum>
#include <QMetaObject>

using namespace Collections;

AggregateQueryMaker::AggregateQueryMaker( AggregateCollection *collection, const QList<QueryMaker*> &queryMakers )
    : QueryMaker()
    , m_collection( collection )
    , m_builders( queryMakers )
    , m_queryDoneCount( 0 )
    , m_maxResultSize( -1 )
    , m_queryType( QueryMaker::None )
    , m_orderDescending( false )
    , m_orderField( 0 )
    , m_orderByNumberField( false )
    , m_queryDoneCountMutex()
{
    foreach( QueryMaker *b, m_builders )
    {
        connect( b, SIGNAL(queryDone()), this, SLOT(slotQueryDone()) );
        connect( b, SIGNAL(newResultReady(Meta::TrackList)), this, SLOT(slotNewResultReady(Meta::TrackList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::ArtistList)), this, SLOT(slotNewResultReady(Meta::ArtistList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::AlbumList)), this, SLOT(slotNewResultReady(Meta::AlbumList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::GenreList)), this, SLOT(slotNewResultReady(Meta::GenreList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::ComposerList)), this, SLOT(slotNewResultReady(Meta::ComposerList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::YearList)), this, SLOT(slotNewResultReady(Meta::YearList)), Qt::QueuedConnection );
        connect( b, SIGNAL(newResultReady(Meta::LabelList)), this, SLOT(slotNewResultReady(Meta::LabelList)), Qt::QueuedConnection );
    }
}

AggregateQueryMaker::~AggregateQueryMaker()
{
    qDeleteAll( m_returnFunctions );
    qDeleteAll( m_returnValues );
    qDeleteAll( m_builders );
}

void
AggregateQueryMaker::run()
{
    foreach( QueryMaker *b, m_builders )
        b->run();
}

void
AggregateQueryMaker::abortQuery()
{
    foreach( QueryMaker *b, m_builders )
        b->abortQuery();
}

QueryMaker*
AggregateQueryMaker::setQueryType( QueryType type )
{
    m_queryType = type;
    if( type != QueryMaker::Custom )
    {
        foreach( QueryMaker *b, m_builders )
            b->setQueryType( type );
        return this;
    }
    else
    {
        // we cannot forward custom queries as there is no way to integrate the results
        // delivered by the QueryMakers. Instead we ask for tracks that match the criterias,
        // and then generate the custom result similar to MemoryQueryMaker.
        // And yes, this means that we will load all tracks when we simply want the count of tracks
        // in the collection. It might be necessary to add some specific logic for that case.
        // On second thought, there is no way around loading all objects, as we want to operate on distinct
        // elements (for some value of distinct) in AggregateCollection. We can only figure out what the union
        // of all elements is after loading them in memory
        foreach( QueryMaker *b, m_builders )
            b->setQueryType( QueryMaker::Track );
        return this;
    }
}

QueryMaker*
AggregateQueryMaker::addReturnValue( qint64 value )
{
    //do not forward this call, see comment in setQueryType()
    m_returnValues.append( CustomValueFactory::returnValue( value ) );
    return this;
}

QueryMaker*
AggregateQueryMaker::addReturnFunction( ReturnFunction function, qint64 value )
{
    //do not forward this call, see comment in setQueryType()
    m_returnFunctions.append( CustomValueFactory::returnFunction( function, value ) );
    return this;
}

QueryMaker*
AggregateQueryMaker::orderBy( qint64 value, bool descending )
{
    m_orderField = value;
    m_orderDescending = descending;
    //copied from MemoryQueryMaker. TODO: think of a sensible place to put this code
    switch( value )
    {
        case Meta::valYear:
        case Meta::valTrackNr:
        case Meta::valDiscNr:
        case Meta::valBpm:
        case Meta::valLength:
        case Meta::valBitrate:
        case Meta::valSamplerate:
        case Meta::valFilesize:
        case Meta::valFormat:
        case Meta::valCreateDate:
        case Meta::valScore:
        case Meta::valRating:
        case Meta::valFirstPlayed:
        case Meta::valLastPlayed:
        case Meta::valPlaycount:
        case Meta::valModified:
        {
            m_orderByNumberField = true;
            break;
        }
        default:
            m_orderByNumberField = false;
    }
    foreach( QueryMaker *b, m_builders )
        b->orderBy( value, descending );
    return this;
}

QueryMaker*
AggregateQueryMaker::addFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
    foreach( QueryMaker *b, m_builders )
        b->addFilter( value, filter, matchBegin, matchEnd );
    return this;
}

QueryMaker*
AggregateQueryMaker::excludeFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
    foreach( QueryMaker *b, m_builders )
        b->excludeFilter( value, filter, matchBegin, matchEnd );
    return this;
}

QueryMaker*
AggregateQueryMaker::addNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    foreach( QueryMaker *b, m_builders )
        b->addNumberFilter( value, filter, compare);
    return this;
}

QueryMaker*
AggregateQueryMaker::excludeNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    foreach( QueryMaker *b, m_builders )
        b->excludeNumberFilter( value, filter, compare );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::TrackPtr &track )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( track );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::ArtistPtr &artist, QueryMaker::ArtistMatchBehaviour behaviour )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( artist, behaviour );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::AlbumPtr &album )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( album );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::GenrePtr &genre )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( genre );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::ComposerPtr &composer )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( composer );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::YearPtr &year )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( year );
    return this;
}

QueryMaker*
AggregateQueryMaker::addMatch( const Meta::LabelPtr &label )
{
    foreach( QueryMaker *b, m_builders )
        b->addMatch( label );
    return this;
}

QueryMaker*
AggregateQueryMaker::limitMaxResultSize( int size )
{
    //forward the call so the m_builders do not have to do work
    //that we definitely know is unnecessary (like returning more than size results)
    //we have to limit the combined result of all m_builders nevertheless
    m_maxResultSize = size;
    foreach( QueryMaker *b, m_builders )
        b->limitMaxResultSize( size );
    return this;
}

QueryMaker*
AggregateQueryMaker::beginAnd()
{
    foreach( QueryMaker *b, m_builders )
        b->beginAnd();
    return this;
}

QueryMaker*
AggregateQueryMaker::beginOr()
{
    foreach( QueryMaker *b, m_builders )
        b->beginOr();
    return this;
}

QueryMaker*
AggregateQueryMaker::endAndOr()
{
    foreach( QueryMaker *b, m_builders )
        b->endAndOr();
    return this;
}

QueryMaker*
AggregateQueryMaker::setAlbumQueryMode( AlbumQueryMode mode )
{
    foreach( QueryMaker *b, m_builders )
        b->setAlbumQueryMode( mode );
    return this;
}

QueryMaker*
AggregateQueryMaker::setLabelQueryMode( LabelQueryMode mode )
{
    foreach( QueryMaker *b, m_builders )
        b->setLabelQueryMode( mode );
    return this;
}

void
AggregateQueryMaker::slotQueryDone()
{
    m_queryDoneCountMutex.lock();
    m_queryDoneCount++;
    if ( m_queryDoneCount == m_builders.size() )
    {
        //make sure we don't give control to code outside this class while holding the lock
        m_queryDoneCountMutex.unlock();
        handleResult();
        emit queryDone();
    }
    else
    {
        m_queryDoneCountMutex.unlock();
    }
}

template <class PointerType>
void AggregateQueryMaker::emitProperResult( const QList<PointerType>& list )
{
   QList<PointerType> resultList = list;

    if ( m_maxResultSize >= 0 && resultList.count() > m_maxResultSize )
        resultList = resultList.mid( 0, m_maxResultSize );

    emit newResultReady( list );
}

void
AggregateQueryMaker::handleResult()
{
    //copied from MemoryQueryMaker::handleResult()
    switch( m_queryType )
    {
        case QueryMaker::Custom :
        {
            QStringList result;
            Meta::TrackList tracks;
            foreach( KSharedPtr<Meta::AggregateTrack> track, m_tracks )
            {
                tracks.append( Meta::TrackPtr::staticCast( track ) );
            }
            if( !m_returnFunctions.empty() )
            {
                //no sorting necessary
                foreach( CustomReturnFunction *function, m_returnFunctions )
                {
                    result.append( function->value( tracks ) );
                }
            }
            else if( !m_returnValues.empty() )
            {
                if( m_orderField )
                {
                    if( m_orderByNumberField )
                        tracks = MemoryQueryMakerHelper::orderListByNumber( tracks, m_orderField, m_orderDescending );
                    else
                        tracks = MemoryQueryMakerHelper::orderListByString( tracks, m_orderField, m_orderDescending );
                }

                int count = 0;
                foreach( const Meta::TrackPtr &track, tracks )
                {
                    if ( m_maxResultSize >= 0 && count == m_maxResultSize )
                        break;

                    foreach( CustomReturnValue *value, m_returnValues )
                    {
                        result.append( value->value( track ) );
                    }
                    count++;
                }
            }
            emit newResultReady( result );
            break;
        }
        case QueryMaker::Track :
        {
            Meta::TrackList tracks;
            foreach( KSharedPtr<Meta::AggregateTrack> track, m_tracks )
            {
                tracks.append( Meta::TrackPtr::staticCast( track ) );
            }

            if( m_orderField )
            {
                if( m_orderByNumberField )
                    tracks = MemoryQueryMakerHelper::orderListByNumber( tracks, m_orderField, m_orderDescending );
                else
                    tracks = MemoryQueryMakerHelper::orderListByString( tracks, m_orderField, m_orderDescending );
            }

            emitProperResult<Meta::TrackPtr>( tracks );
            break;
        }
        case QueryMaker::Album :
        {
            Meta::AlbumList albums;
            foreach( KSharedPtr<Meta::AggregateAlbum> album, m_albums )
            {
                albums.append( Meta::AlbumPtr::staticCast( album ) );
            }

            albums = MemoryQueryMakerHelper::orderListByName<Meta::AlbumPtr>( albums, m_orderDescending );

            emitProperResult<Meta::AlbumPtr>( albums );
            break;
        }
        case QueryMaker::Artist :
        case QueryMaker::AlbumArtist :
        {
            Meta::ArtistList artists;
            foreach( KSharedPtr<Meta::AggregateArtist> artist, m_artists )
            {
                artists.append( Meta::ArtistPtr::staticCast( artist ) );
            }

            artists = MemoryQueryMakerHelper::orderListByName<Meta::ArtistPtr>( artists, m_orderDescending );
            emitProperResult<Meta::ArtistPtr>( artists );
            break;
        }
        case QueryMaker::Composer :
        {
            Meta::ComposerList composers;
            foreach( KSharedPtr<Meta::AggregateComposer> composer, m_composers )
            {
                composers.append( Meta::ComposerPtr::staticCast( composer ) );
            }

            composers = MemoryQueryMakerHelper::orderListByName<Meta::ComposerPtr>( composers, m_orderDescending );

            emitProperResult<Meta::ComposerPtr>( composers );
            break;
        }
        case QueryMaker::Genre :
        {
            Meta::GenreList genres;
            foreach( KSharedPtr<Meta::AggregateGenre> genre, m_genres )
            {
                genres.append( Meta::GenrePtr::staticCast( genre ) );
            }

            genres = MemoryQueryMakerHelper::orderListByName<Meta::GenrePtr>( genres, m_orderDescending );

            emitProperResult<Meta::GenrePtr>( genres );
            break;
        }
        case QueryMaker::Year :
        {
            Meta::YearList years;
            foreach( KSharedPtr<Meta::AggreagateYear> year, m_years )
            {
                years.append( Meta::YearPtr::staticCast( year ) );
            }

            //years have to be ordered as numbers, but orderListByNumber does not work for Meta::YearPtrs
            if( m_orderField == Meta::valYear )
            {
                years = MemoryQueryMakerHelper::orderListByYear( years, m_orderDescending );
            }

            emitProperResult<Meta::YearPtr>( years );
            break;
        }
        case QueryMaker::Label :
        {
            Meta::LabelList labels;
            foreach( KSharedPtr<Meta::AggregateLabel> label, m_labels )
            {
                labels.append( Meta::LabelPtr::staticCast( label ) );
            }

            labels = MemoryQueryMakerHelper::orderListByName<Meta::LabelPtr>( labels, m_orderDescending );
            emitProperResult<Meta::LabelPtr>( labels );
            break;
        }
        case QueryMaker::None :
            //nothing to do
            break;
    }
    m_tracks.clear();
    m_albums.clear();
    m_artists.clear();
    m_composers.clear();
    m_genres.clear();
    m_years.clear();
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::TrackList &tracks )
{
    foreach( const Meta::TrackPtr &track, tracks )
    {
        m_tracks.insert( KSharedPtr<Meta::AggregateTrack>( m_collection->getTrack( track ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::ArtistList &artists )
{
    foreach( const Meta::ArtistPtr &artist, artists )
    {
        m_artists.insert( KSharedPtr<Meta::AggregateArtist>( m_collection->getArtist( artist ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::AlbumList &albums )
{
    foreach( const Meta::AlbumPtr &album, albums )
    {
        m_albums.insert( KSharedPtr<Meta::AggregateAlbum>( m_collection->getAlbum( album ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::GenreList &genres )
{
    foreach( const Meta::GenrePtr &genre, genres )
    {
        m_genres.insert( KSharedPtr<Meta::AggregateGenre>( m_collection->getGenre( genre ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::ComposerList &composers )
{
    foreach( const Meta::ComposerPtr &composer, composers )
    {
        m_composers.insert( KSharedPtr<Meta::AggregateComposer>( m_collection->getComposer( composer ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::YearList &years )
{
    foreach( const Meta::YearPtr &year, years )
    {
        m_years.insert( KSharedPtr<Meta::AggreagateYear>( m_collection->getYear( year ) ) );
    }
}

void
AggregateQueryMaker::slotNewResultReady( const Meta::LabelList &labels )
{
    foreach( const Meta::LabelPtr &label, labels )
    {
        m_labels.insert( KSharedPtr<Meta::AggregateLabel>( m_collection->getLabel( label ) ) );
    }
}
