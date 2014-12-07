/****************************************************************************************
 * Copyright (c) 2008 Casey Link <unnamedrambler@gmail.com>                             *
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

#include "Mp3tunesServiceCollectionLocation.h"

#include "Mp3tunesWorkers.h"
#include "core/interfaces/Logger.h"
#include "core/support/Components.h"

#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>

#include "core/support/Debug.h"

using namespace Collections;

Mp3tunesServiceCollectionLocation::Mp3tunesServiceCollectionLocation( Mp3tunesServiceCollection *parentCollection )
    : ServiceCollectionLocation( parentCollection )
    , m_collection( parentCollection )
{
    DEBUG_BLOCK
}

Mp3tunesServiceCollectionLocation::~Mp3tunesServiceCollectionLocation()
{
}

QString Mp3tunesServiceCollectionLocation::prettyLocation() const
{
    return i18n( "MP3tunes Locker" );
}

bool Mp3tunesServiceCollectionLocation::isWritable() const
{
    return true;
}

void Mp3tunesServiceCollectionLocation::copyUrlsToCollection (
        const QMap<Meta::TrackPtr, KUrl> &sources,
        const Transcoding::Configuration &configuration )
{
    DEBUG_BLOCK
    Q_UNUSED( configuration ); // TODO: we might support transcoding here

    QStringList urls;
    QString error;
    debug() << "sources has " << sources.count();
    foreach( const Meta::TrackPtr &track, sources.keys() )
    {
        debug() << "copying " << sources[ track ] << " to mp3tunes locker";
        debug() << "file is at " << sources[ track ].pathOrUrl();

        QString supported_types = "mp3 mp4 m4a m4b m4p aac wma ogg";
        
        if( supported_types.contains( track->type() ) )
        {   

            debug() << "Added " << sources[ track ].pathOrUrl() << " to queue.";
            urls.push_back( sources[ track ].pathOrUrl() );

        } 
        else 
        {
            error = i18n( "Only the following types of tracks can be uploaded to MP3tunes: mp3, mp4, m4a, m4p, aac, wma, and ogg. " );
            debug() << "File type not supprted " << track->type();
        }
    }
    if( !error.isEmpty() )
        Amarok::Components::logger()->longMessage( error );
    Mp3tunesSimpleUploader * uploadWorker = new Mp3tunesSimpleUploader( m_collection->locker(), urls );
    connect( uploadWorker, SIGNAL(uploadComplete()), this, SLOT(slotCopyOperationFinished()) );
    ThreadWeaver::Weaver::instance()->enqueue( uploadWorker );
}
