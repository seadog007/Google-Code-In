/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2007 Casey Link <unnamedrambler@gmail.com>                             *
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

#define DEBUG_PREFIX "ServiceAlbumCoverDownloader"

#include "ServiceAlbumCoverDownloader.h"

#include "core/support/Amarok.h"
#include "amarokconfig.h"
#include "core/support/Debug.h"
#include "covermanager/CoverCache.h"

#include <QDir>
#include <QImage>

using namespace Meta;


Meta::ServiceAlbumWithCover::ServiceAlbumWithCover( const QString &name )
    : ServiceAlbum( name )
    , m_hasFetchedCover( false )
    , m_isFetchingCover ( false )
{}

Meta::ServiceAlbumWithCover::ServiceAlbumWithCover( const QStringList &resultRow )
    : ServiceAlbum( resultRow )
    , m_hasFetchedCover( false )
    , m_isFetchingCover ( false )
{}

Meta::ServiceAlbumWithCover::~ServiceAlbumWithCover()
{
    CoverCache::invalidateAlbum( this );
}

QImage
ServiceAlbumWithCover::image( int size ) const
{
    if( size > 1000 )
    {
        debug() << "Giant image detected, are you sure you want this?";
        return Meta::Album::image( size );
    }

    const QString artist = hasAlbumArtist() ?
        albumArtist()->name() :
        QLatin1String("NULL"); //no need to translate, only used as a caching key/temp filename

    const QString coverName = QString( "%1_%2_%3_cover.png" ).arg( downloadPrefix(), artist, name() );
    const QString saveLocation = Amarok::saveLocation( "albumcovers/cache/" );
    const QDir cacheCoverDir = QDir( saveLocation );

    //make sure that this dir exists
    if( !cacheCoverDir.exists() )
        cacheCoverDir.mkpath( saveLocation );

    if( size <= 1 )
        size = 100;

    const QString sizeKey = QString::number( size ) + QLatin1Char('@');
    const QString cacheCoverPath = cacheCoverDir.filePath( sizeKey + coverName );

    if( QFile::exists( cacheCoverPath ) )
    {
        return QImage( cacheCoverPath );
    }
    else if( m_hasFetchedCover && !m_cover.isNull() )
    {
        QImage image( m_cover.scaled( size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
        image.save( cacheCoverPath, "PNG" );
        return image;
    }
    else if( !m_isFetchingCover && !coverUrl().isEmpty() )
    {
        m_isFetchingCover = true;

        ( new ServiceAlbumCoverDownloader )->downloadCover(
            ServiceAlbumWithCoverPtr(const_cast<ServiceAlbumWithCover*>(this)) );
    }

    return Meta::Album::image( size );
}

void
ServiceAlbumWithCover::setImage( const QImage& image )
{
    m_cover = image;
    m_hasFetchedCover = true;
    m_isFetchingCover = false;
    CoverCache::invalidateAlbum( this );

    notifyObservers();
}

void
ServiceAlbumWithCover::imageDownloadCanceled() const
{
    m_hasFetchedCover = true;
    m_isFetchingCover = false;
}


///////////////////////////////////////////////////////////////////////////////
// Class ServiceAlbumCoverDownloader
///////////////////////////////////////////////////////////////////////////////

ServiceAlbumCoverDownloader::ServiceAlbumCoverDownloader()
    : m_albumDownloadJob( 0 )
{
    m_tempDir = new KTempDir();
    m_tempDir->setAutoRemove( true );
}

ServiceAlbumCoverDownloader::~ServiceAlbumCoverDownloader()
{
    delete m_tempDir;
}

void
ServiceAlbumCoverDownloader::downloadCover( ServiceAlbumWithCoverPtr album )
{
    m_album = album;

    KUrl downloadUrl( album->coverUrl() );

    m_coverDownloadPath = m_tempDir->name() + downloadUrl.fileName();

    debug() << "Download Cover: " << downloadUrl.url() << " to: " << m_coverDownloadPath;

    m_albumDownloadJob = KIO::file_copy( downloadUrl, KUrl( m_coverDownloadPath ), -1, KIO::Overwrite | KIO::HideProgressInfo );

    connect( m_albumDownloadJob, SIGNAL(result(KJob*)), SLOT(coverDownloadComplete(KJob*)) );
    connect( m_albumDownloadJob, SIGNAL(canceled(KJob*)), SLOT(coverDownloadCanceled(KJob*)) );
}

void
ServiceAlbumCoverDownloader::coverDownloadComplete( KJob * downloadJob )
{
    if( !m_album ) // album was removed in between
    {
        debug() << "Bad album pointer";
        return;
    }

    if( !downloadJob || downloadJob->error() != 0 )
    {
        debug() << "Download Job failed!";

        //we could not download, so inform album
        m_album->imageDownloadCanceled();
        return;
    }

    if ( downloadJob != m_albumDownloadJob )
        return; //not the right job, so let's ignore it

    const QImage cover = QImage( m_coverDownloadPath );
    if ( cover.isNull() )
    {
        debug() << "file not a valid image";
        //the file wasn't an image, so inform album
        m_album->imageDownloadCanceled();
        return;
    }

    m_album->setImage( cover );

    downloadJob->deleteLater();

    deleteLater();
}

void
ServiceAlbumCoverDownloader::coverDownloadCanceled( KJob *downloadJob )
{
    Q_UNUSED( downloadJob );
    DEBUG_BLOCK

    if( !m_album ) // album was removed in between
        return;

    debug() << "Cover download cancelled";
    m_album->imageDownloadCanceled();
    deleteLater();
}

#include "ServiceAlbumCoverDownloader.moc"

