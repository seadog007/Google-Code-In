/****************************************************************************************
 * Copyright (c) 2007 Trolltech ASA <copyright@trolltech.com>                           *
 * Copyright (c) 2008 Urs Wolfer <uwolfer@kde.org>                                      *
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

#define DEBUG_PREFIX "NetworkAccessManagerProxy"

#include "NetworkAccessManagerProxy.h"
#ifdef DEBUG_BUILD_TYPE
#include "NetworkAccessViewer.h"
#endif // DEBUG_BUILD_TYPE

#include "core/support/Debug.h"

#include "Version.h"

#include <KProtocolManager>

#include <QMetaMethod>
#include <QNetworkReply>
#include <QWeakPointer>

NetworkAccessManagerProxy *NetworkAccessManagerProxy::s_instance = 0;

NetworkAccessManagerProxy *NetworkAccessManagerProxy::instance()
{
    if( s_instance == 0 )
        s_instance = new NetworkAccessManagerProxy();
    return s_instance;
}

void NetworkAccessManagerProxy::destroy()
{
    if( s_instance )
    {
        delete s_instance;
        s_instance = 0;
    }
}

class NetworkAccessManagerProxy::NetworkAccessManagerProxyPrivate
{
public:
    NetworkAccessManagerProxyPrivate( NetworkAccessManagerProxy *parent )
        : userAgent( QString( "Amarok/" ) + AMAROK_VERSION )
#ifdef DEBUG_BUILD_TYPE
        , viewer( 0 )
#endif // DEBUG_BUILD_TYPE
        , q_ptr( parent )
    {}

    ~NetworkAccessManagerProxyPrivate() {}

    void _replyFinished()
    {
        Q_Q( NetworkAccessManagerProxy );
        QNetworkReply *reply = static_cast<QNetworkReply*>( q->sender() );

        KUrl url = reply->request().url();
        QList<CallBackData*> callbacks = urlMap.values( url );
        urlMap.remove( url );
        QByteArray data = reply->readAll();
        data.detach(); // detach so the bytes are not deleted before methods are invoked
        foreach( const CallBackData *cb, callbacks )
        {
            // There may have been a redirect.
            KUrl redirectUrl = q->getRedirectUrl( reply );

            // Check if there's no redirect.
            if( redirectUrl.isEmpty() )
            {
                QByteArray sig = QMetaObject::normalizedSignature( cb->method );
                sig.remove( 0, 1 ); // remove first char, which is the member code (see qobjectdefs.h)
                                    // and let Qt's meta object system handle the rest.
                if( cb->receiver )
                {
                    bool success( false );
                    const QMetaObject *mo = cb->receiver.data()->metaObject();
                    int methodIndex = mo->indexOfSlot( sig );
                    if( methodIndex != -1 )
                    {
                        Error err = { reply->error(), reply->errorString() };
                        QMetaMethod method = mo->method( methodIndex );
                        success = method.invoke( cb->receiver.data(),
                                                cb->type,
                                                Q_ARG( KUrl, reply->request().url() ),
                                                Q_ARG( QByteArray, data ),
                                                Q_ARG( NetworkAccessManagerProxy::Error, err ) );
                    }

                    if( !success )
                    {
                        debug() << QString( "Failed to invoke method %1 of %2" )
                            .arg( QString(sig) ).arg( mo->className() );
                    }
                }
            }
            else
            {
                debug() << "the server is redirecting the request to: " << redirectUrl;

                // Let's try to fetch the data again, but this time from the new url.
                QNetworkReply *newReply = q->getData( redirectUrl, cb->receiver.data(), cb->method, cb->type );

                emit q->requestRedirected( url, redirectUrl );
                emit q->requestRedirected( reply, newReply );
            }
        }

        qDeleteAll( callbacks );
        reply->deleteLater();
    }

    class CallBackData
    {
    public:
        CallBackData( QObject *rec, QNetworkReply *rep, const char *met, Qt::ConnectionType t )
            : receiver( rec )
            , reply( rep )
            , method( met )
            , type( t )
        {}

        ~CallBackData()
        {
            if( reply )
                reply.data()->deleteLater();
        }

        QWeakPointer<QObject> receiver;
        QWeakPointer<QNetworkReply> reply;
        const char *method;
        Qt::ConnectionType type;
    };

    QMultiHash<KUrl, CallBackData*> urlMap;
    QString userAgent;
#ifdef DEBUG_BUILD_TYPE
    NetworkAccessViewer *viewer;
#endif // DEBUG_BUILD_TYPE

private:
    NetworkAccessManagerProxy *const q_ptr;
    Q_DECLARE_PUBLIC( NetworkAccessManagerProxy )
};

NetworkAccessManagerProxy::NetworkAccessManagerProxy( QObject *parent )
    : KIO::Integration::AccessManager( parent )
    , d( new NetworkAccessManagerProxyPrivate( this ) )
{
    setCache(0);   // disable QtWebKit cache to just use KIO one..
    qRegisterMetaType<NetworkAccessManagerProxy::Error>();
}

NetworkAccessManagerProxy::~NetworkAccessManagerProxy()
{
    delete d;
    s_instance = 0;
}

#ifdef DEBUG_BUILD_TYPE
NetworkAccessViewer *
NetworkAccessManagerProxy::networkAccessViewer()
{
    return d->viewer;
}

void
NetworkAccessManagerProxy::setNetworkAccessViewer( NetworkAccessViewer *viewer )
{
    if( viewer )
    {
        if( d->viewer )
            delete d->viewer;
        d->viewer = viewer;
    }
}
#endif // DEBUG_BUILD_TYPE

QNetworkReply *
NetworkAccessManagerProxy::getData( const KUrl &url, QObject *receiver, const char *method,
                                    Qt::ConnectionType type )
{
    if( !url.isValid() )
    {
        const QMetaObject *mo = receiver->metaObject();
        debug() << QString( "Error: URL '%1' is invalid (from %2)" ).arg( url.url() ).arg( mo->className() );
        return 0;
    }

    QNetworkReply *r = get( QNetworkRequest(url) );
    typedef NetworkAccessManagerProxyPrivate::CallBackData PrivateCallBackData;
    PrivateCallBackData *cbm = new PrivateCallBackData( receiver, r, method, type );
    d->urlMap.insert( url, cbm );
    connect( r, SIGNAL(finished()), this, SLOT(_replyFinished()), type );
    return r;
}

int
NetworkAccessManagerProxy::abortGet( const KUrl::List &urls )
{
    int removed = 0;
    const QSet<KUrl> &urlSet = urls.toSet();
    foreach( const KUrl &url, urlSet )
        removed += abortGet( url );
    return removed;
}

int
NetworkAccessManagerProxy::abortGet( const KUrl &url )
{
    if( !d->urlMap.contains(url) )
        return 0;

    qDeleteAll( d->urlMap.values( url ) );
    int removed = d->urlMap.remove( url );
    return removed;
}

KUrl
NetworkAccessManagerProxy::getRedirectUrl( QNetworkReply *reply )
{
    KUrl targetUrl;

    // Get the original URL.
    KUrl originalUrl = reply->request().url();

    // Get the redirect attribute.
    QVariant redirectAttribute = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );

    // Get the redirect URL from the attribute.
    KUrl redirectUrl = KUrl( redirectAttribute.toUrl() );

    // If the redirect URL is valid and if it differs from the original
    // URL then we return the redirect URL. Otherwise an empty URL will
    // be returned.
    if( !redirectUrl.isEmpty() && redirectUrl != originalUrl )
    {
        targetUrl = redirectUrl;
    }

    return targetUrl;
}

void
NetworkAccessManagerProxy::slotError( QObject *obj )
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>( obj );
    if( !reply )
        return;
    KUrl url = reply->request().url();
    d->urlMap.remove( url );
    reply->deleteLater();
}

QNetworkReply *
NetworkAccessManagerProxy::createRequest( Operation op, const QNetworkRequest &req, QIODevice *outgoingData )
{
    QNetworkRequest request = req;
    request.setAttribute( QNetworkRequest::HttpPipeliningAllowedAttribute, true );
    if ( request.hasRawHeader( "User-Agent" ) )
        request.setRawHeader( "User-Agent", d->userAgent.toLocal8Bit() + ' ' + request.rawHeader( "User-Agent" ) );
    else
        request.setRawHeader( "User-Agent", d->userAgent.toLocal8Bit() );

    KIO::CacheControl cc = KProtocolManager::cacheControl();
    switch (cc)
    {
    case KIO::CC_CacheOnly:      // Fail request if not in cache.
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        break;

    case KIO::CC_Refresh:        // Always validate cached entry with remote site.
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
        break;

    case KIO::CC_Reload:         // Always fetch from remote site
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        break;

    case KIO::CC_Cache:          // Use cached entry if available.
    case KIO::CC_Verify:         // Validate cached entry with remote site if expired.
    default:
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        break;
    }

    QNetworkReply *reply = KIO::Integration::AccessManager::createRequest( op, request, outgoingData );

#ifdef DEBUG_BUILD_TYPE
    if( d->viewer )
        d->viewer->addRequest( op, request, outgoingData, reply );
#endif // DEBUG_BUILD_TYPE
    return reply;
}

namespace The
{
    NetworkAccessManagerProxy *networkAccessManager()
    {
        return NetworkAccessManagerProxy::instance();
    }
}

#include "NetworkAccessManagerProxy.moc"
