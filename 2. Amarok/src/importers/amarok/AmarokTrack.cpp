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

#include "AmarokTrack.h"

#include "importers/ImporterSqlConnection.h"

#include <QStringList>

using namespace StatSyncing;

AmarokTrack::AmarokTrack( const qint64 urlId, const ImporterSqlConnectionPtr &connection,
                          const Meta::FieldHash &metadata, const QSet<QString> &labels )
    : SimpleWritableTrack( metadata, labels )
    , m_connection( connection )
    , m_urlId( urlId )
{
}

AmarokTrack::~AmarokTrack()
{
}

void
AmarokTrack::doCommit( const qint64 fields )
{
    bool ok = true;
    m_connection->transaction();

    QStringList updates;
    QVariantMap bindValues;
    if( fields & Meta::valFirstPlayed )
    {
        updates << "createdate = :createdate";
        bindValues.insert( ":createdate", m_statistics.value( Meta::valFirstPlayed ) );
    }
    if( fields & Meta::valLastPlayed )
    {
        updates << "accessdate = :accessdate";
        bindValues.insert( ":accessdate", m_statistics.value( Meta::valLastPlayed ) );
    }
    if( fields & Meta::valRating )
    {
        updates << "rating = :rating";
        bindValues.insert( ":rating", m_statistics.value( Meta::valRating ) );
    }
    if( fields & Meta::valPlaycount )
    {
        updates << "playcount = :playcount";
        bindValues.insert( ":playcount", m_statistics.value( Meta::valPlaycount ) );
    }

    if( !updates.isEmpty() )
    {
        const QString query = "UPDATE statistics SET " + updates.join(", ") +
                              " WHERE url = :url";

        bindValues.insert( ":url", m_urlId );
        m_connection->query( query, bindValues, &ok );
        if( !ok )
        {
            m_connection->rollback();
            return;
        }
    }

    if( fields & Meta::valLabel )
    {
        // Try to insert all labels. Since the 'label' field's unique, nothing will happen
        // if the label already exists
        foreach( const QString &label, m_labels )
        {
            QVariantMap bindValues;
            bindValues.insert( ":label", label );
            m_connection->query( "INSERT IGNORE INTO labels (label) VALUES ( :label )",
                                 bindValues, &ok );
            if( !ok )
            {
                m_connection->rollback();
                return;
            }
        }

        // Drop all labels for the track
        {
            QVariantMap bindValues;
            bindValues.insert( ":url", m_urlId );
            m_connection->query( "DELETE QUICK FROM urls_labels WHERE url = ?", bindValues,
                                 &ok );
            if( !ok )
            {
                m_connection->rollback();
                return;
            }
        }

        // Add labels
        foreach( const QString &label, m_labels )
        {
            const QString query = "INSERT INTO urls_labels (url, label) VALUES ( :url, "
                                  "(SELECT id FROM labels WHERE label = :label ))";

            QVariantMap bindValues;
            bindValues.insert( ":url", m_urlId );
            bindValues.insert( ":label", label );

            m_connection->query( query, bindValues, &ok );
            if( !ok )
            {
                m_connection->rollback();
                return;
            }
        }
    }

    m_connection->commit();
}
