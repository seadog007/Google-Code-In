/****************************************************************************************
 * Copyright (c) 2008 Edward Toroshchin <edward.hades@gmail.com>                        *
 * Copyright (c) 2009 Jeff Mitchell <mitchell@kde.org>                                  *
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

#ifndef MYSQLEMBEDDEDSTORAGE_H
#define MYSQLEMBEDDEDSTORAGE_H

#include "../amarok_sqlcollection_export.h"
#include <core/collections/support/SqlStorage.h>
#include <core-impl/collections/db/sql/mysql-shared/MySqlStorage.h>

/**
 * Implements a MySqlCollection using a MySQL Embedded Server
 */

class AMAROK_SQLCOLLECTION_MYSQLE_EXPORT MySqlEmbeddedStorage : public MySqlStorage
{
    public:
        /** Creates a new SqlStorage.
         *  @param storageLocation The directory for storing the mysql database, will use the default defined by Amarok/KDE if not set.
         *  Note: Currently it is not possible to open two storages to different locations
         *  in one process.
         *  The first caller wins.
         */
        MySqlEmbeddedStorage( const QString &storageLocation = QString() );
        virtual ~MySqlEmbeddedStorage();

        virtual QString type() const;
};

#endif // MYSQLEMBEDDEDSTORAGE_H
