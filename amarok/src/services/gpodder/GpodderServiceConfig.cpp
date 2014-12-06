/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2010 Stefan Derkits <stefan@derkits.at>                                *
 * Copyright (c) 2010 Christian Wagner <christian.wagner86@gmx.at>                      *
 * Copyright (c) 2010 Felix Winter <ixos01@gmail.com>                                   *
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

#define DEBUG_PREFIX "GPodderConfig"

#include "GpodderServiceConfig.h"

#include "App.h"
#include "core/support/Debug.h"

#include <KDialog>
#include <KWallet/Wallet>

#include <QLabel>

GpodderServiceConfig::GpodderServiceConfig()
    : m_username( "" )
    , m_password( "" )
    , m_enableProvider( false )
    , m_ignoreWallet( false )
    , m_isDataLoaded( false )
    , m_askDiag( 0 )
    , m_wallet( 0 )
{
    DEBUG_BLOCK
    
    load();
}

GpodderServiceConfig::~GpodderServiceConfig()
{
    DEBUG_BLOCK

    if( m_askDiag )
        m_askDiag->deleteLater();

    if( m_wallet )
        m_wallet->deleteLater();
}

void
GpodderServiceConfig::load()
{
    DEBUG_BLOCK
    debug() << "Load config";

    KConfigGroup config = KGlobal::config()->group( configSectionName() );

    m_enableProvider = config.readEntry( "enableProvider", false );
    m_ignoreWallet = config.readEntry( "ignoreWallet", false );

    //We only want to load the wallet if the user has enabled features that require a user/pass
    tryToOpenWallet();

    if( m_wallet )
    {
        if( !m_wallet->hasFolder( "Amarok" ) )
            m_wallet->createFolder( "Amarok" );

        // do a one-time transfer
        // can remove at some point in the future, post-2.2
        m_wallet->setFolder( "Amarok" );

        if( m_wallet->readPassword( "gpodder_password", m_password ) != 0 )
            debug() << "Failed to read gpodder.net password from kwallet!";
        else
        {
            QByteArray rawUsername;

            if( m_wallet->readEntry( "gpodder_username", rawUsername ) != 0 )
                debug() << "Failed to read gpodder.net username from kwallet.. :(";
            else
                m_username = QString::fromUtf8( rawUsername );
        }
    }
    else if( m_ignoreWallet )
    {
        m_username = config.readEntry( "username", QString() );
        m_password = config.readEntry( "password", QString() );
    }
    else
        debug() << "Failed to load the data.";

    m_isDataLoaded = !( m_username.isEmpty() || m_password.isEmpty() );
}

void
GpodderServiceConfig::save()
{
    DEBUG_BLOCK

    debug() << "Save config";

    KConfigGroup config = KGlobal::config()->group( configSectionName() );

    config.writeEntry( "enableProvider", m_enableProvider );
    config.writeEntry( "ignoreWallet", m_ignoreWallet );

    //Whenever this function is called, we'll assume the user wants to
    //change something, so blow away the subscription timestamp key
    config.writeEntry( "subscriptionTimestamp", 0 );

    //Maybe the wallet had already closed or m_enableProvider and m_ignoreWallet
    //could had changed also. So we try to reopen the wallet if it's not open.
    tryToOpenWallet();

    if( m_wallet )
    {
        m_wallet->setFolder( "Amarok" );

        if( m_wallet->writeEntry( "gpodder_username", m_username.toUtf8() ) != 0 )
            debug() << "Failed to save gpodder.net username to kwallet!";

        if( m_wallet->writePassword( "gpodder_password", m_password ) != 0 )
            debug() << "Failed to save gpodder.net pw to kwallet!";
    }
    else if( m_ignoreWallet )
    {
        config.writeEntry( "username", m_username );
        config.writeEntry( "password", m_password );
    }
    else
    {
        if( m_enableProvider )
        {
            debug() << "Couldnt access the wallet to save the gpodder.net credentials";
            askAboutMissingKWallet();
        }
        else
            debug() << "There isn't valid credentials to be saved";
    }

    config.sync();
}

void
GpodderServiceConfig::askAboutMissingKWallet()
{
    if ( !m_askDiag )
    {
        m_askDiag = new KDialog( 0 );

        m_askDiag->setCaption( i18n( "gpodder.net credentials" ) );
        m_askDiag->setMainWidget( new QLabel( i18n( "No running KWallet found. Would you like Amarok to save your gpodder.net credentials in plaintext?" ), m_askDiag ) );
        m_askDiag->setButtons( KDialog::Yes | KDialog::No );
        m_askDiag->setModal( true );

        connect( m_askDiag, SIGNAL(yesClicked()), this, SLOT(textDialogYes()) );
        connect( m_askDiag, SIGNAL(noClicked()), this, SLOT(textDialogNo()) );
    }

    m_askDiag->exec();
}

void GpodderServiceConfig::tryToOpenWallet()
{
    DEBUG_BLOCK

    //We only want to load the wallet if the user has enabled features
    //that require a user/pass
    if( ( m_enableProvider ) && ( !m_ignoreWallet ) )
    {
        debug() << "Opening wallet";

        //Open wallet unless explicitly told not to
        m_wallet = KWallet::Wallet::openWallet(
                       KWallet::Wallet::NetworkWallet(),
                       0, KWallet::Wallet::Synchronous );
    }
    else
    {
        debug() << "The wallet was ignored or is not needed.";
        m_wallet = 0;
    }
}

void
GpodderServiceConfig::reset()
{
    debug() << "Reset config";

    m_username = "";
    m_password = "";
    m_enableProvider = false;
    m_ignoreWallet = false;
}

void
GpodderServiceConfig::textDialogYes() //SLOT
{
    DEBUG_BLOCK

    if ( !m_ignoreWallet )
    {
        KConfigGroup config = KGlobal::config()->group( configSectionName() );

        m_ignoreWallet = true;
        config.writeEntry( "ignoreWallet ", m_ignoreWallet );

        config.sync();
    }
}

void
GpodderServiceConfig::textDialogNo() //SLOT
{
    DEBUG_BLOCK

    if ( m_ignoreWallet )
    {
        KConfigGroup config = KGlobal::config()->group( configSectionName() );

        m_ignoreWallet = false;
        config.writeEntry( "ignoreWallet ", m_ignoreWallet );

        config.sync();
    }
}

#include "GpodderServiceConfig.moc"
