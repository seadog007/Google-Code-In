/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef JAMENDODATABASEHANDLER_H
#define JAMENDODATABASEHANDLER_H

#include "JamendoMeta.h"

#include <QStringList>
#include <QMap>

/**
* This class wraps the database operations needed by the JamendoBrowser
*
* @author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class JamendoDatabaseHandler
{
public:
    /**
     * Private constructor (singleton pattern)
     * @return Pointer to new object
     */
    JamendoDatabaseHandler();

    ~JamendoDatabaseHandler();

    /**
     * Creates the tables needed to store Jamendo info
     */
    void createDatabase();

    /**
     * Destroys Jamendo tables
     */
    void destroyDatabase();

    /**
     * Inserts a new track into the Jamendo database
     * @param track pointer to the track to insert
     * @return the database id of the newly inserted track
     */
    int insertTrack( Meta::ServiceTrack *track );

    /**
     * inserts a new album into the Jamendo database
     * @param album pointer to the album to insert
     * @return the database id of the newly inserted album
     */
    int insertAlbum( Meta::ServiceAlbum *album );

    /**
     * inserts a new artist into the Jamendo database
     * @param artist pointer to the artist to insert
     * @return the database id of the newly inserted artist
     */
    int insertArtist( Meta::ServiceArtist *artist );

    /**
     * inserts a new genre into the Jamendo database
     * @param genre pointer to the genre to insert
     * @return the database id of the newly inserted genre
     */
    int insertGenre( Meta::ServiceGenre *genre );

    /**
     * Begins a database transaction. Must be followed by a later call to commit()
     */
    void begin();

    /**
     * Completes (executes) a database transaction. Must be preceded by a call to begin()
     */
    void commit();

    /**
     * Remove all genres that are applied to too few albums in an attempt to weed out the worst mistags and
     * speed up queries a bit!
     * @param minCount cutoff value...
     */
    void trimGenres( int minCount );
};

#endif
