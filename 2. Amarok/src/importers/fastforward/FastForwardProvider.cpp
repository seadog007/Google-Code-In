/****************************************************************************************
 * Copyright (c) 2013 Konrad Zemek <konrad.zemek@gmail.com>                             *
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

#include "FastForwardProvider.h"

#include "FastForwardTrack.h"
#include "core/support/Debug.h"
#include "importers/ImporterManager.h"
#include "importers/ImporterSqlConnection.h"

using namespace StatSyncing;

FastForwardProvider::FastForwardProvider( const QVariantMap &config,
                                          ImporterManager *importer )
    : ImporterProvider( config, importer )
{
    if( config.value( "dbDriver" ).toString() == "QSQLITE" )
    {
        m_connection = ImporterSqlConnectionPtr(
                    new ImporterSqlConnection( config.value( "dbPath" ).toString() ) );
    }
    else
    {
        m_connection = ImporterSqlConnectionPtr( new ImporterSqlConnection(
             m_config.value( "dbDriver" ).toString(),
             m_config.value( "dbHost" ).toString(),
             m_config.value( "dbPort" ).toUInt(),
             m_config.value( "dbName" ).toString(),
             m_config.value( "dbUser" ).toString(),
             m_config.value( "dbPass" ).toString()
        ) );
    }
}

FastForwardProvider::~FastForwardProvider()
{
}

qint64
FastForwardProvider::reliableTrackMetaData() const
{
    return Meta::valTitle | Meta::valArtist | Meta::valAlbum | Meta::valComposer
            | Meta::valYear | Meta::valTrackNr | Meta::valDiscNr;
}

qint64
FastForwardProvider::writableTrackStatsData() const
{
    return Meta::valRating | Meta::valFirstPlayed | Meta::valLastPlayed
            | Meta::valPlaycount | Meta::valLabel;
}

QSet<QString>
FastForwardProvider::artists()
{
    QSet<QString> result;
    foreach( const QVariantList &row, m_connection->query( "SELECT name FROM artist" ) )
        result.insert( row[0].toString() );

    return result;
}

TrackList
FastForwardProvider::artistTracks( const QString &artistName )
{
    const QString query = "SELECT t.url, t.title, al.name, ar.name, c.name, y.name, "
            "t.track, t.discnumber, s.rating, s.createdate, s.accessdate, s.playcounter "
            "FROM tags t "
            "INNER JOIN artist ar ON ar.id = t.artist "
            "LEFT JOIN album al ON al.id = t.album "
            "LEFT JOIN composer c ON c.id = t.composer "
            "LEFT JOIN year y ON y.id = t.year "
            "LEFT JOIN statistics s ON s.url = t.url "
            "WHERE ar.name = :artist";

    QVariantMap bindValues;
    bindValues.insert( ":artist", artistName );

    const QList<qint64> fields = QList<qint64>() << Meta::valTitle << Meta::valAlbum
                << Meta::valArtist << Meta::valComposer << Meta::valYear
                << Meta::valTrackNr << Meta::valDiscNr << Meta::valRating
                << Meta::valFirstPlayed << Meta::valLastPlayed << Meta::valPlaycount;

    TrackList result;
    foreach( const QVariantList &row, m_connection->query( query, bindValues ) )
    {
        const QString trackUrl = row[0].toString();

        Meta::FieldHash metadata;
        for( int i = 0; i < fields.size(); ++i )
            metadata.insert( fields[i], row[i + 1] );

        const QString lblQuery = "SELECT l.name FROM labels l "
                                 "INNER JOIN tags_labels tl ON tl.labelid = l.id "
                                 "WHERE tl.url = :url";

        QVariantMap lblBindValues;
        lblBindValues.insert( ":url", trackUrl );

        QSet<QString> labels;
        foreach( const QVariantList &row, m_connection->query( lblQuery, lblBindValues ) )
            labels.insert( row[0].toString() );

        result << TrackPtr( new FastForwardTrack( trackUrl, m_connection, metadata,
                                                  labels ) );
    }

    return result;
}
