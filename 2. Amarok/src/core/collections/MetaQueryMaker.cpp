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

#include "core/collections/MetaQueryMaker.h"

using namespace Collections;

MetaQueryMaker::MetaQueryMaker( const QList<Collections::Collection*> &collections )
    : QueryMaker()
    , m_queryDoneCount( 0 )
    , m_queryDoneCountMutex()
{
    foreach( Collections::Collection *c, collections )
    {
        QueryMaker *b = c->queryMaker();
        builders.append( b );
        connect( b, SIGNAL(queryDone()), this, SLOT(slotQueryDone()) );
        //relay signals directly
        // actually this is wrong. We would need to combine the results
        // to prevent duplicate album name results.
        // On the other hand we need duplicate AlbumPtr results.
        // Summary: be carefull when using this class. (Ralf)
        connect( b, SIGNAL(newResultReady(Meta::TrackList)), this, SIGNAL(newResultReady(Meta::TrackList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::ArtistList)), this, SIGNAL(newResultReady(Meta::ArtistList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::AlbumList)), this, SIGNAL(newResultReady(Meta::AlbumList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::GenreList)), this, SIGNAL(newResultReady(Meta::GenreList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::ComposerList)), this, SIGNAL(newResultReady(Meta::ComposerList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::YearList)), this, SIGNAL(newResultReady(Meta::YearList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(QStringList)), this, SIGNAL(newResultReady(QStringList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::LabelList)), this, SIGNAL(newResultReady(Meta::LabelList)), Qt::DirectConnection );
    }
}

MetaQueryMaker::MetaQueryMaker( const QList<QueryMaker*> &queryMakers )
    : QueryMaker()
    , builders( queryMakers )
    , m_queryDoneCount( 0 )
    , m_queryDoneCountMutex()
{
    foreach( QueryMaker *b, builders )
    {
        connect( b, SIGNAL(queryDone()), this, SLOT(slotQueryDone()) );
        //relay signals directly
        connect( b, SIGNAL(newResultReady(Meta::TrackList)), this, SIGNAL(newResultReady(Meta::TrackList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::ArtistList)), this, SIGNAL(newResultReady(Meta::ArtistList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::AlbumList)), this, SIGNAL(newResultReady(Meta::AlbumList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::GenreList)), this, SIGNAL(newResultReady(Meta::GenreList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::ComposerList)), this, SIGNAL(newResultReady(Meta::ComposerList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::YearList)), this, SIGNAL(newResultReady(Meta::YearList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(QStringList)), this, SIGNAL(newResultReady(QStringList)), Qt::DirectConnection );
        connect( b, SIGNAL(newResultReady(Meta::LabelList)), this, SIGNAL(newResultReady(Meta::LabelList)), Qt::DirectConnection );
    }
}

MetaQueryMaker::~MetaQueryMaker()
{
    foreach( QueryMaker *b, builders )
        delete b;
}

void
MetaQueryMaker::run()
{
    foreach( QueryMaker *b, builders )
        b->run();
}

void
MetaQueryMaker::abortQuery()
{
    foreach( QueryMaker *b, builders )
        b->abortQuery();
}

QueryMaker*
MetaQueryMaker::setQueryType( QueryType type )
{
    foreach( QueryMaker *qm, builders )
        qm->setQueryType( type );
    return this;
}

QueryMaker*
MetaQueryMaker::addReturnValue( qint64 value )
{
    foreach( QueryMaker *b, builders )
        b->addReturnValue( value );
    return this;
}

QueryMaker*
MetaQueryMaker::addReturnFunction( ReturnFunction function, qint64 value )
{
    foreach( QueryMaker *qm, builders )
        qm->addReturnFunction( function, value );
    return this;
}

/* Ok. That doesn't work. First connecting the signals directly and then
   doing "orderBy" directly */
QueryMaker*
MetaQueryMaker::orderBy( qint64 value, bool descending )
{
    foreach( QueryMaker *b, builders )
        b->orderBy( value, descending );
    return this;
}

QueryMaker*
MetaQueryMaker::addFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
    foreach( QueryMaker *b, builders )
        b->addFilter( value, filter, matchBegin, matchEnd );
    return this;
}

QueryMaker*
MetaQueryMaker::excludeFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd )
{
    foreach( QueryMaker *b, builders )
        b->excludeFilter( value, filter, matchBegin, matchEnd );
    return this;
}

QueryMaker*
MetaQueryMaker::addNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    foreach( QueryMaker *b, builders )
        b->addNumberFilter( value, filter, compare);
    return this;
}

QueryMaker*
MetaQueryMaker::excludeNumberFilter( qint64 value, qint64 filter, QueryMaker::NumberComparison compare )
{
    foreach( QueryMaker *b, builders )
        b->excludeNumberFilter( value, filter, compare );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::TrackPtr &track )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( track );
    return this;
}


QueryMaker*
MetaQueryMaker::addMatch( const Meta::ArtistPtr &artist, QueryMaker::ArtistMatchBehaviour behaviour )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( artist, behaviour );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::AlbumPtr &album )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( album );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::GenrePtr &genre )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( genre );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::ComposerPtr &composer )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( composer );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::YearPtr &year )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( year );
    return this;
}

QueryMaker*
MetaQueryMaker::addMatch( const Meta::LabelPtr &label )
{
    foreach( QueryMaker *b, builders )
        b->addMatch( label );
    return this;
}

QueryMaker*
MetaQueryMaker::limitMaxResultSize( int size )
{
    foreach( QueryMaker *b, builders )
        b->limitMaxResultSize( size );
    return this;
}

QueryMaker*
MetaQueryMaker::beginAnd()
{
    foreach( QueryMaker *b, builders )
        b->beginAnd();
    return this;
}

QueryMaker*
MetaQueryMaker::beginOr()
{
    foreach( QueryMaker *b, builders )
        b->beginOr();
    return this;
}

QueryMaker*
MetaQueryMaker::endAndOr()
{
    foreach( QueryMaker *b, builders )
        b->endAndOr();
    return this;
}

QueryMaker*
MetaQueryMaker::setAlbumQueryMode( AlbumQueryMode mode )
{
    foreach( QueryMaker *qm, builders )
        qm->setAlbumQueryMode( mode );
    return this;
}

QueryMaker*
MetaQueryMaker::setLabelQueryMode( LabelQueryMode mode )
{
    foreach( QueryMaker *qm, builders )
        qm->setLabelQueryMode( mode );
    return this;
}

void
MetaQueryMaker::slotQueryDone()
{
    m_queryDoneCountMutex.lock();
    m_queryDoneCount++;
    if ( m_queryDoneCount == builders.size() )
    {
        //make sure we don't give control to code outside this class while holding the lock
        m_queryDoneCountMutex.unlock();
        emit queryDone();
    }
    else
        m_queryDoneCountMutex.unlock();
}

#include "MetaQueryMaker.moc"

