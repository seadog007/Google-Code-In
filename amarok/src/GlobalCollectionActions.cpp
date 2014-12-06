/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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
 
#include "GlobalCollectionActions.h"
#include "core/meta/Meta.h"


namespace The
{
    static GlobalCollectionActions* s_GlobalCollectionActions_instance = 0;

    GlobalCollectionActions* globalCollectionActions()
    {
        if( !s_GlobalCollectionActions_instance )
            s_GlobalCollectionActions_instance = new GlobalCollectionActions();

        return s_GlobalCollectionActions_instance;
    }
}

GlobalCollectionActions::GlobalCollectionActions()
{}


GlobalCollectionActions::~GlobalCollectionActions()
{}

void
GlobalCollectionActions::addGenreAction( GlobalCollectionGenreAction * action )
{
    m_genreActions.append( action );
}

void
GlobalCollectionActions::addArtistAction( GlobalCollectionArtistAction * action )
{
    m_artistActions.append( action );
}

void
GlobalCollectionActions::addAlbumAction( GlobalCollectionAlbumAction * action )
{
    m_albumActions.append( action );
}

void
GlobalCollectionActions::addTrackAction( GlobalCollectionTrackAction * action )
{
    m_trackActions.append( action );
}

void
GlobalCollectionActions::addYearAction( GlobalCollectionYearAction * action )
{
    m_yearActions.append( action );
}

void
GlobalCollectionActions::addComposerAction( GlobalCollectionComposerAction * action )
{
    m_composerActions.append( action );
}

QList< QAction * > GlobalCollectionActions::actionsFor( Meta::DataPtr item )
{

    Meta::GenrePtr genrePtr = Meta::GenrePtr::dynamicCast( item );
    if ( genrePtr )
        return actionsFor( genrePtr );

    Meta::ArtistPtr artistPtr = Meta::ArtistPtr::dynamicCast( item );
    if ( artistPtr )
        return actionsFor( artistPtr );

    Meta::AlbumPtr albumPtr = Meta::AlbumPtr::dynamicCast( item );
    if ( albumPtr )
        return actionsFor( albumPtr );

    Meta::TrackPtr trackPtr = Meta::TrackPtr::dynamicCast( item );
    if ( trackPtr )
        return actionsFor( trackPtr );

    Meta::YearPtr yearPtr = Meta::YearPtr::dynamicCast( item );
    if ( yearPtr )
        return actionsFor( yearPtr );

    Meta::ComposerPtr composerPtr = Meta::ComposerPtr::dynamicCast( item );
    if ( composerPtr )
        return actionsFor( composerPtr );

    QList< QAction * > emptyList;
    return emptyList;
}


QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::GenrePtr genre )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionGenreAction * genreAction, m_genreActions )
    {
        genreAction->setGenre( genre );
        returnList.append( genreAction );
    }

    return returnList;
}

QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::ArtistPtr artist )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionArtistAction * artistAction, m_artistActions )
    {
        artistAction->setArtist( artist );
        returnList.append( artistAction );
    }

    return returnList;
}

QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::AlbumPtr album )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionAlbumAction * albumAction, m_albumActions )
    {
        albumAction->setAlbum( album );
        returnList.append( albumAction );
    }

    return returnList;
}

QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::TrackPtr track )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionTrackAction * trackAction, m_trackActions )
    {
        trackAction->setTrack( track );
        returnList.append( trackAction );
    }

    return returnList;
}

QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::YearPtr year )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionYearAction * yearAction, m_yearActions )
    {
        yearAction->setYear( year );
        returnList.append( yearAction );
    }

    return returnList;
}

QList< QAction * >
GlobalCollectionActions::actionsFor( Meta::ComposerPtr composer )
{
    QList< QAction * > returnList;
    foreach( GlobalCollectionComposerAction * composerAction, m_composerActions )
    {
        composerAction->setComposer( composer );
        returnList.append( composerAction );
    }

    return returnList;
}

GlobalCollectionAction::GlobalCollectionAction( const QString &text, QObject * parent )
    : QAction( text, parent )
{}

GlobalCollectionGenreAction::GlobalCollectionGenreAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void
GlobalCollectionGenreAction::setGenre( Meta::GenrePtr genre )
{
    m_currentGenre = genre;
}

Meta::GenrePtr GlobalCollectionGenreAction::genre()
{
    return m_currentGenre;
}

GlobalCollectionArtistAction::GlobalCollectionArtistAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void
GlobalCollectionArtistAction::setArtist( Meta::ArtistPtr artist )
{
    m_currentArtist = artist;
}

Meta::ArtistPtr GlobalCollectionArtistAction::artist()
{
    return m_currentArtist;
}

GlobalCollectionAlbumAction::GlobalCollectionAlbumAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void GlobalCollectionAlbumAction::setAlbum( Meta::AlbumPtr album )
{
    m_currentAlbum = album;
}

Meta::AlbumPtr GlobalCollectionAlbumAction::album()
{
    return m_currentAlbum;
}

GlobalCollectionTrackAction::GlobalCollectionTrackAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void GlobalCollectionTrackAction::setTrack( Meta::TrackPtr track )
{
    m_currentTrack = track;
}

Meta::TrackPtr
GlobalCollectionTrackAction::track()
{
    return m_currentTrack;
}

GlobalCollectionYearAction::GlobalCollectionYearAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void
GlobalCollectionYearAction::setYear( Meta::YearPtr year )
{
    m_currentYear = year;
}

Meta::YearPtr
GlobalCollectionYearAction::year()
{
    return m_currentYear;
}

GlobalCollectionComposerAction::GlobalCollectionComposerAction( const QString &text, QObject * parent )
    : GlobalCollectionAction( text, parent )
{}

void
GlobalCollectionComposerAction::setComposer(Meta::ComposerPtr composer)
{
    m_currentComposer = composer;
}

Meta::ComposerPtr
GlobalCollectionComposerAction::composer()
{
    return m_currentComposer;
}


