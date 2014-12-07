/****************************************************************************************
 * Copyright (c) 2011 Sven Krohlas <sven@asbest-online.de>                              *
 * The Amazon store in based upon the Magnatune store in Amarok,                        *
 * Copyright (c) 2006,2007 Nikolaj Hald Nielsen <nhn@kde.org>                           *
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

#ifndef AMAZONCOLLECTION_H
#define AMAZONCOLLECTION_H

#include "ServiceCollection.h"

namespace Collections {

class AmazonCollection : public Collections::ServiceCollection
{
public:
    AmazonCollection( ServiceBase * service, const QString &id, const QString &prettyName );

    /**
    * Returns a reference to the artist -> ID map.
    * The artist consists if the artist name.
    */
    QMap<QString, int> &artistIDMap();

    /**
    * Returns a reference to the album -> ID map.
    * The album is represented by the album ASIN.
    */
    QMap<QString, int> &albumIDMap();

    /**
    * Returns a reference to the track -> ID map.
    * The track is represented by the track ASIN.
    */
    QMap<QString, int> &trackIDMap();

    /**
    * Clears the collection from any content.
    */
    void clear();

private:
    QMap<QString, int> m_artistIDMap;
    QMap<QString, int> m_albumIDMap;
    QMap<QString, int> m_trackIDMap;
};

}
#endif // AMAZONCOLLECTION_H
