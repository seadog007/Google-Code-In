/****************************************************************************************
 * Copyright (c) 2010 Ralf Engels <ralf-engels@gmx.de>                                  *
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

#include "SqlBatchImporter.h"
#include "SqlBatchImporterConfig.h"

#include "core/collections/Collection.h"
#include "core/support/Debug.h"
#include "core/capabilities/CollectionImportCapability.h"
#include "core-impl/collections/support/CollectionManager.h"

#include <KLocale>

#include <QFile>

SqlBatchImporter::SqlBatchImporter( QObject *parent )
    : QObject( parent )
    , m_config( 0 )
    , m_count( 0 )
    , m_importing( false )
{
    connect( this, SIGNAL(importSucceeded()), SLOT(importingFinished()) );
    connect( this, SIGNAL(importFailed()), SLOT(importingFinished()) );
    connect( this, SIGNAL(trackAdded(Meta::TrackPtr)), SLOT(trackImported(Meta::TrackPtr)) );
    connect( this, SIGNAL(trackMatchFound(Meta::TrackPtr,QString)), SLOT(trackMatched(Meta::TrackPtr,QString)) );
}

SqlBatchImporter::~SqlBatchImporter()
{
}

SqlBatchImporterConfig*
SqlBatchImporter::configWidget( QWidget *parent )
{
    if( !m_config )
        m_config = new SqlBatchImporterConfig( parent );
    return m_config;
}

void
SqlBatchImporter::import()
{
    DEBUG_BLOCK

    Q_ASSERT( m_config );
    if( !m_config )
    {
        error() << "No configuration exists, bailing out of import";
        return;
    }

    int numStarted = 0;
    // search for a collection with the CollectionImportCapability
    foreach( Collections::Collection *coll, CollectionManager::instance()->collections().keys() )
    {
        debug() << "Collection: "<<coll->prettyName() << "id:"<<coll->collectionId();
        QScopedPointer<Capabilities::CollectionImportCapability> cic( coll->create<Capabilities::CollectionImportCapability>());

        if( cic ) {

            QFile *file = new QFile( m_config->inputFilePath() );
            if( file->open( QIODevice::ReadOnly ) )
            {
                debug() << "importing db";
                cic->import( file, this );
                numStarted++;
            } else {
                debug() << "could not open";
                emit importError( i18n( "Could not open file \"%1\".", m_config->inputFilePath() ) );
                delete file;
            }
        }
    }

    if( !numStarted )
        emit importFailed();
}

int
SqlBatchImporter::importedCount() const
{
    return m_count;
}

void
SqlBatchImporter::trackImported( Meta::TrackPtr track )
{
    Q_UNUSED( track )
    ++m_count;
}

void
SqlBatchImporter::trackMatched( Meta::TrackPtr track, QString oldUrl )
{
    Q_UNUSED( track )
    Q_UNUSED( oldUrl )
    ++m_count;
}

bool
SqlBatchImporter::importing() const
{
    return m_importing;
}

void
SqlBatchImporter::startImporting()
{
    DEBUG_BLOCK
    m_importing = true;
    import();
}

void
SqlBatchImporter::importingFinished()
{
    m_importing = false;
}
