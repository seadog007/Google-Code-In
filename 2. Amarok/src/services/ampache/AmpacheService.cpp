/****************************************************************************************
 * Copyright (c) 2007 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#define DEBUG_PREFIX "AmpacheService"

#include "AmpacheService.h"

#include "AmpacheConfig.h"
#include "AmpacheAccountLogin.h"

#include "core/support/Amarok.h"
#include "core/support/Components.h"
#include "core/interfaces/Logger.h"
#include "browsers/SingleCollectionTreeItemModel.h"
#include "core-impl/collections/support/CollectionManager.h"
#include <config.h>
#include "core/support/Debug.h"

#ifdef HAVE_LIBLASTFM
  #include "LastfmInfoParser.h"
#endif

#include <KLocale>

AMAROK_EXPORT_SERVICE_PLUGIN( ampache, AmpacheServiceFactory )

AmpacheServiceFactory::AmpacheServiceFactory( QObject *parent, const QVariantList &args )
    : ServiceFactory( parent, args )
{
    KPluginInfo pluginInfo( "amarok_service_ampache.desktop", "services" );
    pluginInfo.setConfig( config() );
    m_info = pluginInfo;
}

void AmpacheServiceFactory::init()
{
    //read config and create the needed number of services
    AmpacheConfig config;
    AmpacheServerList servers = config.servers();
    m_initialized = true;

    for( int i = 0; i < servers.size(); i++ )
    {
        AmpacheServerEntry server = servers.at( i );
        ServiceBase* service = new AmpacheService( this, "Ampache (" + server.name + ')', server.url, server. username, server.password );
        emit newService( service );
    }
}

QString
AmpacheServiceFactory::name()
{
    return "Ampache";
}

KConfigGroup
AmpacheServiceFactory::config()
{
    return Amarok::config( "Service_Ampache" );
}

bool
AmpacheServiceFactory::possiblyContainsTrack(const KUrl & url) const
{
    AmpacheConfig config;
    foreach( const AmpacheServerEntry &server, config.servers() )
    {
        if ( url.url().contains( server.url, Qt::CaseInsensitive ) )
            return true;
    }

    return false;
}


AmpacheService::AmpacheService( AmpacheServiceFactory* parent, const QString & name, const QString &url, const QString &username, const QString &password )
    : ServiceBase( name,  parent )
    , m_infoParser( 0 )
    , m_collection( 0 )
    , m_ampacheLogin(new AmpacheAccountLogin(url, username, password, this))
{
    DEBUG_BLOCK
    connect(m_ampacheLogin, SIGNAL(loginSuccessful()), this, SLOT(onLoginSuccessful()));
    setShortDescription( i18n( "Amarok frontend for your Ampache server" ) );
    setIcon( KIcon( "view-services-ampache-amarok" ) );
    setLongDescription( i18n( "Use Amarok as a seamless frontend to your Ampache server. This lets you browse and play all the Ampache contents from within Amarok." ) );
    setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_ampache.png" ) );
#ifdef HAVE_LIBLASTFM
    m_infoParser = new LastfmInfoParser();
#endif
}

AmpacheService::~AmpacheService()
{
    CollectionManager::instance()->removeUnmanagedCollection( m_collection );
    delete m_collection;
    m_ampacheLogin->deleteLater();
}

void
AmpacheService::polish()
{
    m_bottomPanel->hide();
    setInfoParser( m_infoParser );

    /*if ( !m_authenticated )
        authenticate( );*/
}

void
AmpacheService::reauthenticate()
{
    m_ampacheLogin->reauthenticate();
    // it would make sense here to clean the complete cache
    // information from a server might get outdated.
}


void
AmpacheService::onLoginSuccessful()
{
    m_collection = new Collections::AmpacheServiceCollection( this, m_ampacheLogin->server(), m_ampacheLogin->sessionId() );
    // connect( m_collection, SIGNAL(authenticationNeeded()), SLOT(authenticate()) );

    CollectionManager::instance()->addUnmanagedCollection( m_collection, CollectionManager::CollectionDisabled );
    QList<CategoryId::CatMenuId> levels;
    levels << CategoryId::Artist << CategoryId::Album;
    setModel( new SingleCollectionTreeItemModel( m_collection, levels ) );
    setServiceReady( true );
}

#include "AmpacheService.moc"
