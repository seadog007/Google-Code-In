/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2007 Adam Pigg <adam@piggz.co.uk>                                      *
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

#include "DynamicServiceQueryMaker.h"

#include "core/support/Debug.h"
#include "ServiceCollection.h"

using namespace Collections;

DynamicServiceQueryMaker::DynamicServiceQueryMaker( )
 : QueryMaker()
{
}

QueryMaker * DynamicServiceQueryMaker::addReturnValue(qint64 value)
{
    Q_UNUSED( value );
    return this;
}

QueryMaker* DynamicServiceQueryMaker::addReturnFunction( ReturnFunction function, qint64 value )
{
    AMAROK_NOTIMPLEMENTED
    Q_UNUSED( value )
    Q_UNUSED( function )
    return this;
}

QueryMaker * DynamicServiceQueryMaker::orderBy(qint64 value, bool descending)
{
    Q_UNUSED( value );
    Q_UNUSED( descending );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::TrackPtr & track)
{
    DEBUG_BLOCK
    Q_UNUSED( track );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::ArtistPtr & artist, QueryMaker::ArtistMatchBehaviour behaviour )
{
    DEBUG_BLOCK
    Q_UNUSED( artist );
    Q_UNUSED( behaviour );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::AlbumPtr & album)
{
    DEBUG_BLOCK
    Q_UNUSED( album );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::GenrePtr & genre)
{
    DEBUG_BLOCK
    Q_UNUSED( genre );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::ComposerPtr & composer)
{
    DEBUG_BLOCK
    Q_UNUSED( composer );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::YearPtr & year)
{
    DEBUG_BLOCK
    Q_UNUSED( year );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addMatch(const Meta::LabelPtr & label)
{
    DEBUG_BLOCK
    Q_UNUSED( label );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::addFilter(qint64 value, const QString & filter, bool matchBegin, bool matchEnd)
{
    Q_UNUSED( value );
    Q_UNUSED( filter );
    Q_UNUSED( matchBegin );
    Q_UNUSED( matchEnd );
    return this;
}

QueryMaker * DynamicServiceQueryMaker::excludeFilter(qint64 value, const QString & filter, bool matchBegin, bool matchEnd)
{
    Q_UNUSED( value );
    Q_UNUSED( filter );
    Q_UNUSED( matchBegin );
    Q_UNUSED( matchEnd );
    return this;
}

QueryMaker* DynamicServiceQueryMaker::addNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    AMAROK_NOTIMPLEMENTED
    Q_UNUSED( value )
    Q_UNUSED( filter )
    Q_UNUSED( compare )
    return this;
}

QueryMaker* DynamicServiceQueryMaker::excludeNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    AMAROK_NOTIMPLEMENTED
    Q_UNUSED( value )
    Q_UNUSED( filter )
    Q_UNUSED( compare )
    return this;
}

QueryMaker * DynamicServiceQueryMaker::limitMaxResultSize(int size)
{
    Q_UNUSED( size );
    return this;
}

Meta::AlbumList
DynamicServiceQueryMaker::matchAlbums( ServiceCollection *coll, const Meta::ArtistPtr &artist )
{
    if( !artist || !coll )
        return Meta::AlbumList();
    ArtistMap artistMap = coll->artistMap();
    if ( artist && artistMap.contains( artist->name() ) )
    {
        Meta::ArtistPtr artist2 = artistMap.value( artist->name() );

        Meta::AlbumList matchingAlbums;
        Meta::AlbumList albums = coll->albumMap().values();

        foreach( Meta::AlbumPtr albumPtr, albums ) {

            if ( albumPtr->albumArtist() == artist2 )
                matchingAlbums.push_back( albumPtr );
        }

        return matchingAlbums;
    }
    else
        return Meta::AlbumList();
}


#include "DynamicServiceQueryMaker.moc"

