/****************************************************************************************
 * Copyright (c) 2004 Mark Kretschmann <kretschmann@kde.org>                            *
 * Copyright (c) 2004 Stefan Bogner <bochi@online.ms>                                   *
 * Copyright (c) 2004 Max Howell <max.howell@methylblue.com>                            *
 * Copyright (c) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>                        *
 * Copyright (c) 2009 Martin Sandsmark <sandsmark@samfundet.no>                         *
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

#define DEBUG_PREFIX "CoverFetcher"

#include "CoverFetcher.h"

#include "amarokconfig.h"
#include "core/interfaces/Logger.h"
#include "core/meta/Meta.h"
#include "core/support/Amarok.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"
#include "CoverFetchQueue.h"
#include "CoverFoundDialog.h"
#include "CoverFetchUnit.h"

#include <KLocale>
#include <KUrl>

#include <QBuffer>
#include <QImageReader>

CoverFetcher* CoverFetcher::s_instance = 0;

CoverFetcher*
CoverFetcher::instance()
{
    return s_instance ? s_instance : new CoverFetcher();
}

void CoverFetcher::destroy()
{
    if( s_instance )
    {
        delete s_instance;
        s_instance = 0;
    }
}

CoverFetcher::CoverFetcher()
    : QObject()
    , m_limit( 10 )
{
    DEBUG_BLOCK
    setObjectName( "CoverFetcher" );
    qRegisterMetaType<CoverFetchUnit::Ptr>("CoverFetchUnit::Ptr");

    m_queue = new CoverFetchQueue( this );
    connect( m_queue, SIGNAL(fetchUnitAdded(CoverFetchUnit::Ptr)),
                      SLOT(slotFetch(CoverFetchUnit::Ptr)) );
    s_instance = this;

    connect( The::networkAccessManager(), SIGNAL(requestRedirected(QNetworkReply*,QNetworkReply*)),
             this, SLOT(fetchRequestRedirected(QNetworkReply*,QNetworkReply*)) );
}

CoverFetcher::~CoverFetcher()
{
}

void
CoverFetcher::manualFetch( Meta::AlbumPtr album )
{
    debug() << QString("Adding interactive cover fetch for: '%1' from %2")
        .arg( album->name() )
        .arg( Amarok::config("Cover Fetcher").readEntry("Interactive Image Source", "LastFm") );
    switch( fetchSource() )
    {
    case CoverFetch::LastFm:
        m_queue->add( album, CoverFetch::Interactive, fetchSource() );
        break;

    case CoverFetch::Discogs:
    case CoverFetch::Google:
        queueQueryForAlbum( album );
        break;

    default:
        break;
    }
}

void
CoverFetcher::queueAlbum( Meta::AlbumPtr album )
{
    if( m_queue->size() > m_limit )
        m_queueLater.append( album );
    else
        m_queue->add( album, CoverFetch::Automatic );
    debug() << "Queueing automatic cover fetch for:" << album->name();
}

void
CoverFetcher::queueAlbums( Meta::AlbumList albums )
{
    foreach( Meta::AlbumPtr album, albums )
    {
        if( m_queue->size() > m_limit )
            m_queueLater.append( album );
        else
            m_queue->add( album, CoverFetch::Automatic );
    }
}

void
CoverFetcher::queueQuery( Meta::AlbumPtr album, const QString &query, int page )
{
    m_queue->addQuery( query, fetchSource(), page, album );
    debug() << QString( "Queueing cover fetch query: '%1' (page %2)" ).arg( query ).arg( page );
}

void
CoverFetcher::queueQueryForAlbum( Meta::AlbumPtr album )
{
    QString query( album->name() );
    if( album->hasAlbumArtist() )
        query += ' ' + album->albumArtist()->name();
    queueQuery( album, query, 0 );
}

void
CoverFetcher::slotFetch( CoverFetchUnit::Ptr unit )
{
    if( !unit )
        return;

    const CoverFetchPayload *payload = unit->payload();
    const CoverFetch::Urls urls = payload->urls();

    // show the dialog straight away if fetch is interactive
    if( !m_dialog && unit->isInteractive() )
    {
        showCover( unit, QImage() );
    }
    else if( urls.isEmpty() )
    {
        finish( unit, NotFound );
        return;
    }

    const KUrl::List uniqueUrls = urls.uniqueKeys();
    foreach( const KUrl &url, uniqueUrls )
    {
        if( !url.isValid() )
            continue;

        QNetworkReply *reply = The::networkAccessManager()->getData( url, this,
                               SLOT(slotResult(KUrl,QByteArray,NetworkAccessManagerProxy::Error)) );
        m_urls.insert( url, unit );

        if( payload->type() == CoverFetchPayload::Art )
        {
            if( unit->isInteractive() )
                Amarok::Components::logger()->newProgressOperation( reply, i18n( "Fetching Cover" ) );
            else
                return; // only one is needed when the fetch is non-interactive
        }
    }
}

void
CoverFetcher::slotResult( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e )
{
    DEBUG_BLOCK
    if( !m_urls.contains( url ) )
        return;
    debug() << "Data dump from the result: " << data;

    const CoverFetchUnit::Ptr unit( m_urls.take( url ) );
    if( !unit )
    {
        m_queue->remove( unit );
        return;
    }

    if( e.code != QNetworkReply::NoError )
    {
        finish( unit, Error, i18n("There was an error communicating with cover provider: %1", e.description) );
        return;
    }

    const CoverFetchPayload *payload = unit->payload();
    switch( payload->type() )
    {
    case CoverFetchPayload::Info:
        m_queue->add( unit->album(), unit->options(), payload->source(), data );
        m_queue->remove( unit );
        break;

    case CoverFetchPayload::Search:
        m_queue->add( unit->options(), fetchSource(), data );
        m_queue->remove( unit );
        break;

    case CoverFetchPayload::Art:
        handleCoverPayload( unit, data, url );
        break;
    }
}

void
CoverFetcher::handleCoverPayload( const CoverFetchUnit::Ptr &unit, const QByteArray &data, const KUrl &url )
{
    if( data.isEmpty() )
    {
        finish( unit, NotFound );
        return;
    }

    QBuffer buffer;
    buffer.setData( data );
    buffer.open( QIODevice::ReadOnly );
    QImageReader reader( &buffer );
    if( !reader.canRead() )
    {
        finish( unit, Error, reader.errorString() );
        return;
    }

    QSize imageSize = reader.size();
    const CoverFetchArtPayload *payload = static_cast<const CoverFetchArtPayload*>( unit->payload() );
    const CoverFetch::Metadata &metadata = payload->urls().value( url );

    if( payload->imageSize() == CoverFetch::ThumbSize )
    {
        if( imageSize.isEmpty() )
        {
            imageSize.setWidth( metadata.value( QLatin1String("width") ).toInt() );
            imageSize.setHeight( metadata.value( QLatin1String("height") ).toInt() );
        }
        imageSize.scale( 120, 120, Qt::KeepAspectRatio );
        reader.setScaledSize( imageSize );
        // This will force the JPEG decoder to use JDCT_IFAST
        reader.setQuality( 49 );
    }

    if( unit->isInteractive() )
    {
        QImage image;
        if( reader.read( &image ) )
        {
            showCover( unit, image, metadata );
            m_queue->remove( unit );
            return;
        }
    }
    else
    {
        QImage image;
        if( reader.read( &image ) )
        {
            m_selectedImages.insert( unit, image );
            finish( unit );
            return;
        }
    }
    finish( unit, Error, reader.errorString() );
}

void
CoverFetcher::slotDialogFinished()
{
    const CoverFetchUnit::Ptr unit = m_dialog.data()->unit();
    switch( m_dialog.data()->result() )
    {
    case KDialog::Accepted:
        m_selectedImages.insert( unit, m_dialog.data()->image() );
        finish( unit );
        break;

    case KDialog::Rejected:
        finish( unit, Cancelled );
        break;

    default:
        finish( unit, Error );
    }

    /*
     * Remove all manual fetch jobs from the queue if the user accepts, cancels,
     * or closes the cover found dialog. This way, the dialog will not reappear
     * if there are still covers yet to be retrieved.
     */
    QList< CoverFetchUnit::Ptr > units = m_urls.values();
    foreach( const CoverFetchUnit::Ptr &unit, units )
    {
        if( unit->isInteractive() )
            abortFetch( unit );
    }

    m_dialog.data()->delayedDestruct();
}

void
CoverFetcher::fetchRequestRedirected( QNetworkReply *oldReply,
                                      QNetworkReply *newReply )
{
    KUrl oldUrl = oldReply->request().url();
    KUrl newUrl = newReply->request().url();

    // Since we were redirected we have to check if the redirect
    // was for one of our URLs and if the new URL is not handled
    // already.
    if( m_urls.contains( oldUrl ) && !m_urls.contains( newUrl ) )
    {
        // Get the unit for the old URL.
        CoverFetchUnit::Ptr unit = m_urls.value( oldUrl );

        // Add the unit with the new URL and remove the old one.
        m_urls.insert( newUrl, unit );
        m_urls.remove( oldUrl );

        // If the unit is an interactive one we have to incidate that we're
        // still fetching the cover.
        if( unit->isInteractive() )
            Amarok::Components::logger()->newProgressOperation( newReply, i18n( "Fetching Cover" ) );
    }
}

void
CoverFetcher::showCover( const CoverFetchUnit::Ptr &unit,
                         const QImage &cover,
                         const CoverFetch::Metadata &data )
{
    if( !m_dialog )
    {
        const Meta::AlbumPtr album = unit->album();
        if( !album )
        {
            finish( unit, Error );
            return;
        }

        m_dialog = new CoverFoundDialog( unit, data, static_cast<QWidget*>( parent() ) );
        connect( m_dialog.data(), SIGNAL(newCustomQuery(Meta::AlbumPtr,QString,int)),
                           SLOT(queueQuery(Meta::AlbumPtr,QString,int)) );
        connect( m_dialog.data(), SIGNAL(accepted()), SLOT(slotDialogFinished()) );
        connect( m_dialog.data(), SIGNAL(rejected()), SLOT(slotDialogFinished()) );

        if( fetchSource() == CoverFetch::LastFm )
            queueQueryForAlbum( album );
        m_dialog.data()->setQueryPage( 1 );

        m_dialog.data()->show();
        m_dialog.data()->raise();
        m_dialog.data()->activateWindow();
    }
    else
    {
        if( !cover.isNull() )
        {
            typedef CoverFetchArtPayload CFAP;
            const CFAP *payload = dynamic_cast< const CFAP* >( unit->payload() );
            if( payload )
                m_dialog.data()->add( cover, data, payload->imageSize() );
        }
    }
}

void
CoverFetcher::abortFetch( CoverFetchUnit::Ptr unit )
{
    m_queue->remove( unit );
    m_queueLater.removeAll( unit->album() );
    m_selectedImages.remove( unit );
    KUrl::List urls = m_urls.keys( unit );
    foreach( const KUrl &url, urls )
        m_urls.remove( url );
    The::networkAccessManager()->abortGet( urls );
}

void
CoverFetcher::finish( const CoverFetchUnit::Ptr unit,
                      CoverFetcher::FinishState state,
                      const QString &message )
{
    Meta::AlbumPtr album = unit->album();
    const QString albumName = album ? album->name() : QString();

    switch( state )
    {
    case Success:
        if( !albumName.isEmpty() )
        {
            const QString text = i18n( "Retrieved cover successfully for '%1'.", albumName );
            Amarok::Components::logger()->shortMessage( text );
            debug() << "Finished successfully for album" << albumName;
        }
        album->setImage( m_selectedImages.take( unit ) );
        abortFetch( unit );
        break;

    case Error:
        if( !albumName.isEmpty() )
        {
            const QString text = i18n( "Fetching cover for '%1' failed.", albumName );
            Amarok::Components::logger()->shortMessage( text );
            QString debugMessage;
            if( !message.isEmpty() )
                debugMessage = '[' + message + ']';
            debug() << "Finished with errors for album" << albumName << debugMessage;
        }
        m_errors += message;
        break;

    case Cancelled:
        if( !albumName.isEmpty() )
        {
            const QString text = i18n( "Canceled fetching cover for '%1'.", albumName );
            Amarok::Components::logger()->shortMessage( text );
            debug() << "Finished, cancelled by user for album" << albumName;
        }
        break;

    case NotFound:
        if( !albumName.isEmpty() )
        {
            const QString text = i18n( "Unable to find a cover for '%1'.", albumName );
            //FIXME: Not visible behind cover manager
            Amarok::Components::logger()->shortMessage( text );
            m_errors += text;
            debug() << "Finished due to cover not found for album" << albumName;
        }
        break;
    }

    m_queue->remove( unit );

    if( !m_queueLater.isEmpty() )
    {
        const int diff = m_limit - m_queue->size();
        if( diff > 0 )
        {
            for( int i = 0; i < diff && !m_queueLater.isEmpty(); ++i )
            {
                Meta::AlbumPtr album = m_queueLater.takeFirst();
                // automatic fetching only uses Last.fm as source
                m_queue->add( album, CoverFetch::Automatic, CoverFetch::LastFm );
            }
        }
    }

    emit finishedSingle( static_cast< int >( state ) );
}

CoverFetch::Source
CoverFetcher::fetchSource() const
{
    const KConfigGroup config = Amarok::config( "Cover Fetcher" );
    const QString sourceEntry = config.readEntry( "Interactive Image Source", "LastFm" );
    CoverFetch::Source source;
    if( sourceEntry == "LastFm" )
        source = CoverFetch::LastFm;
    else if( sourceEntry == "Google" )
        source = CoverFetch::Google;
    else
        source = CoverFetch::Discogs;
    return source;
}

#include "CoverFetcher.moc"

