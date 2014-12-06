/****************************************************************************************
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

#include "MagnatuneAlbumDownloader.h"

#include "core/support/Amarok.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"
#include "core/interfaces/Logger.h"
#include "MagnatuneMeta.h"


#include <KLocale>
#include <KZip>

MagnatuneAlbumDownloader::MagnatuneAlbumDownloader()
    : QObject()
    , m_albumDownloadJob( 0 )
    , m_currentAlbumFileName()
{
    m_tempDir = new KTempDir();
}

MagnatuneAlbumDownloader::~MagnatuneAlbumDownloader()
{
    delete m_tempDir;
    m_tempDir = 0;
}

void
MagnatuneAlbumDownloader::downloadAlbum( MagnatuneDownloadInfo info )
{
    DEBUG_BLOCK

    m_currentAlbumInfo = info;


    KUrl downloadUrl = info.completeDownloadUrl();
    m_currentAlbumUnpackLocation = info.unpackLocation();
    debug() << "Download: " << downloadUrl.url() << " to: " << m_currentAlbumUnpackLocation;

    m_currentAlbumFileName = info.albumCode() + ".zip";

    debug() << "Using temporary location: " << m_tempDir->name() + m_currentAlbumFileName;

    m_albumDownloadJob = KIO::file_copy( downloadUrl, KUrl( m_tempDir->name() + m_currentAlbumFileName ), -1, KIO::Overwrite | KIO::HideProgressInfo );

    connect( m_albumDownloadJob, SIGNAL(result(KJob*)), SLOT(albumDownloadComplete(KJob*)) );

    QString msgText;
    if( !info.albumName().isEmpty() && !info.artistName().isEmpty() )
    {
        msgText = i18n( "Downloading '%1' by %2 from Magnatune.com", info.albumName(), info.artistName() );
    }
    else
    {
        msgText = i18n( "Downloading album from Magnatune.com" );
    }

    Amarok::Components::logger()->newProgressOperation( m_albumDownloadJob, msgText, this, SLOT(albumDownloadAborted()) );
}




void
MagnatuneAlbumDownloader::albumDownloadComplete( KJob * downloadJob )
{
    DEBUG_BLOCK

    debug() << "album download complete";

    if ( !downloadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downloadJob != m_albumDownloadJob )
        return ; //not the right job, so let's ignore it

    const QString finalAlbumPath = m_currentAlbumUnpackLocation + '/' + m_currentAlbumInfo.artistName() + '/' + m_currentAlbumInfo.albumName();

    //ok, now we have the .zip file downloaded. All we need is to unpack it to the desired location and add it to the collection.

    KZip kzip( m_tempDir->name() + m_currentAlbumFileName );

    if ( !kzip.open( QIODevice::ReadOnly ) )
    {
        Amarok::Components::logger()->shortMessage( i18n( "Magnatune download seems to have failed. Cannot read zip file" ) );
        emit( downloadComplete( false ) );
        return;
    }

    debug() << m_tempDir->name() + m_currentAlbumFileName << " opened for decompression";

    const KArchiveDirectory * directory = kzip.directory();

    Amarok::Components::logger()->shortMessage( i18n( "Uncompressing Magnatune.com download..." ) );

    //Is this really blocking with no progress status!? Why is it not a KJob?

    debug() <<  "decompressing to " << finalAlbumPath;
    directory->copyTo( m_currentAlbumUnpackLocation );

    debug() <<  "done!";
    


    //now I really want to add the album cover to the same folder where I just unzipped the album... The
    //only way of getting the actual location where the album was unpacked is using the artist and album names

    QString coverUrlString = m_currentAlbumInfo.coverUrl();

    KUrl downloadUrl( coverUrlString.replace( "_200.jpg", ".jpg") );

    debug() << "Adding cover " << downloadUrl.url() << " to collection at " << finalAlbumPath;

    m_albumDownloadJob = KIO::file_copy( downloadUrl, KUrl( finalAlbumPath + "/cover.jpg" ), -1, KIO::Overwrite | KIO::HideProgressInfo );

    connect( m_albumDownloadJob, SIGNAL(result(KJob*)), SLOT(coverAddComplete(KJob*)) );

    Amarok::Components::logger()->newProgressOperation( m_albumDownloadJob, i18n( "Adding album cover to collection" ), this, SLOT(coverAddAborted()) );

    emit( downloadComplete( true ) );


}

void
MagnatuneAlbumDownloader::albumDownloadAborted( )
{
    DEBUG_BLOCK
    
    m_albumDownloadJob->kill();
    m_albumDownloadJob = 0;
    debug() << "Aborted album download";

    emit( downloadComplete( false ) );

}

#include "MagnatuneAlbumDownloader.moc"
