/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
 * Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>                                    *
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

#define DEBUG_PREFIX "lastfm"

#include "LastFmServiceConfig.h"

#include "App.h"
#include "core/support/Components.h"
#include "core/support/Debug.h"

#include <KDialog>
#include <KWallet/Wallet>

#include <QLabel>
#include <QThread>

QWeakPointer<LastFmServiceConfig> LastFmServiceConfig::s_instance;

LastFmServiceConfigPtr
LastFmServiceConfig::instance()
{
    Q_ASSERT( QThread::currentThread() == QCoreApplication::instance()->thread() );

    LastFmServiceConfigPtr strongRef = s_instance.toStrongRef();
    if( strongRef )
        return strongRef;

    LastFmServiceConfigPtr newStrongRef( new LastFmServiceConfig() );
    s_instance = newStrongRef;
    return newStrongRef;
}

LastFmServiceConfig::LastFmServiceConfig()
    : m_askDiag( 0 )
    , m_wallet( 0 )
{
    DEBUG_BLOCK

    KConfigGroup config = KGlobal::config()->group( configSectionName() );
    m_sessionKey = config.readEntry( "sessionKey", QString() );
    m_scrobble = config.readEntry( "scrobble", defaultScrobble() );
    m_fetchSimilar = config.readEntry( "fetchSimilar", defaultFetchSimilar() );
    m_scrobbleComposer = config.readEntry( "scrobbleComposer", defaultScrobbleComposer() );
    m_useFancyRatingTags = config.readEntry( "useFancyRatingTags", defaultUseFancyRatingTags() );
    m_announceCorrections = config.readEntry( "announceCorrections", defaultAnnounceCorrections() );
    m_filterByLabel = config.readEntry( "filterByLabel", defaultFilterByLabel() );
    m_filteredLabel = config.readEntry( "filteredLabel", defaultFilteredLabel() );

    if( config.hasKey( "kWalletUsage" ) )
        m_kWalletUsage = KWalletUsage( config.readEntry( "kWalletUsage", int( NoPasswordEnteredYet ) ) );
    else
    {
        // migrate from the old config that used "ignoreWallet" key set to yes/no
        if( config.readEntry( "ignoreWallet", "" ) == "yes" )
            m_kWalletUsage = PasswordInAscii;
        else if( config.hasKey( "scrobble" ) )
            // assume password was saved in KWallet if the config was once written
            m_kWalletUsage = PasswodInKWallet;
        else
            m_kWalletUsage = NoPasswordEnteredYet; // config not yet saved, assume unused
    }

    switch( m_kWalletUsage )
    {
        case NoPasswordEnteredYet:
            break;
        case PasswodInKWallet:
            openWalletToRead();
            break;
        case PasswordInAscii:
            m_username = config.readEntry( "username", QString() );
            m_password = config.readEntry( "password", QString() );
            break;
    }
}

LastFmServiceConfig::~LastFmServiceConfig()
{
    DEBUG_BLOCK

    if( m_askDiag )
        m_askDiag->deleteLater();
    if( m_wallet )
        m_wallet->deleteLater();
}

void LastFmServiceConfig::save()
{
    KConfigGroup config = KGlobal::config()->group( configSectionName() );

    // if username and password is empty, reset to NoPasswordEnteredYet; this enables
    // going from PasswordInAscii to PasswodInKWallet
    if( m_username.isEmpty() && m_password.isEmpty() )
    {
        m_kWalletUsage = NoPasswordEnteredYet;
        config.deleteEntry( "username" ); // prevent possible stray credentials
        config.deleteEntry( "password" );
    }

    config.writeEntry( "sessionKey", m_sessionKey );
    config.writeEntry( "scrobble", m_scrobble );
    config.writeEntry( "fetchSimilar", m_fetchSimilar );
    config.writeEntry( "scrobbleComposer", m_scrobbleComposer );
    config.writeEntry( "useFancyRatingTags", m_useFancyRatingTags );
    config.writeEntry( "announceCorrections", m_announceCorrections );
    config.writeEntry( "kWalletUsage", int( m_kWalletUsage ) );
    config.writeEntry( "filterByLabel", m_filterByLabel );
    config.writeEntry( "filteredLabel", m_filteredLabel );
    config.deleteEntry( "ignoreWallet" ); // remove old settings

    switch( m_kWalletUsage )
    {
        case NoPasswordEnteredYet:
            if( m_username.isEmpty() && m_password.isEmpty() )
                break; // stay in this state
            // otherwise
        case PasswodInKWallet:
            openWalletToWrite();
            config.deleteEntry( "username" ); // prevent possible stray credentials
            config.deleteEntry( "password" );
            break;
        case PasswordInAscii:
            config.writeEntry( "username", m_username );
            config.writeEntry( "password", m_password );
            break;
    }

    config.sync();
    emit updated();
}

void
LastFmServiceConfig::openWalletToRead()
{
    if( m_wallet && m_wallet->isOpen() )
    {
        slotWalletOpenedToRead( true );
        return;
    }

    if( m_wallet )
        disconnect( m_wallet, 0, this, 0 );
    else
    {
        openWalletAsync();
        if( !m_wallet ) // can happen, see bug 322964
        {
            slotWalletOpenedToRead( false );
            return;
        }
    }
    connect( m_wallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpenedToRead(bool)) );
}

void
LastFmServiceConfig::openWalletToWrite()
{
    if( m_wallet && m_wallet->isOpen() )
    {
        slotWalletOpenedToWrite( true );
        return;
    }

    if( m_wallet )
        disconnect( m_wallet, 0, this, 0 );
    else
    {
        openWalletAsync();
        if( !m_wallet ) // can happen, see bug 322964
        {
            slotWalletOpenedToWrite( false );
            return;
        }
    }
    connect( m_wallet, SIGNAL(walletOpened(bool)), SLOT(slotWalletOpenedToWrite(bool)) );
}

void
LastFmServiceConfig::openWalletAsync()
{
    Q_ASSERT( !m_wallet );
    using namespace KWallet;
    m_wallet = Wallet::openWallet( Wallet::NetworkWallet(), 0, Wallet::Asynchronous );
}

void
LastFmServiceConfig::prepareOpenedWallet()
{
    if( !m_wallet->hasFolder( "Amarok" ) )
        m_wallet->createFolder( "Amarok" );
    m_wallet->setFolder( "Amarok" );
}

void
LastFmServiceConfig::slotWalletOpenedToRead( bool success )
{
    if( !success )
    {
        warning() << __PRETTY_FUNCTION__ << "failed to open wallet";
        QString message = i18n( "Failed to open KDE Wallet to read Last.fm credentials" );
        Amarok::Components::logger()->longMessage( message, Amarok::Logger::Warning );
        if( m_wallet )
            m_wallet->deleteLater(); // no point in having invalid wallet around
        m_wallet = 0;
        return;
    }

    Q_ASSERT( m_wallet );
    prepareOpenedWallet();

    if( m_wallet->readPassword( "lastfm_password", m_password ) > 0 )
        warning() << "Failed to read lastfm password from kwallet";
    QByteArray rawUsername;
    if( m_wallet->readEntry( "lastfm_username", rawUsername ) > 0 )
        warning() << "Failed to read last.fm username from kwallet";
    else
        m_username = QString::fromUtf8( rawUsername );
    emit updated();
}

void
LastFmServiceConfig::slotWalletOpenedToWrite( bool success )
{
    if( !success )
    {
        askAboutMissingKWallet();
        if( m_wallet )
            m_wallet->deleteLater(); // no point in having invalid wallet around
        m_wallet = 0;
        return;
    }

    Q_ASSERT( m_wallet );
    prepareOpenedWallet();

    if( m_wallet->writePassword( "lastfm_password", m_password ) > 0 )
        warning() << "Failed to save last.fm password to kwallet";
    if( m_wallet->writeEntry( "lastfm_username", m_username.toUtf8() ) > 0 )
        warning() << "Failed to save last.fm username to kwallet";

    m_kWalletUsage = PasswodInKWallet;
    KConfigGroup config = KGlobal::config()->group( configSectionName() );
    config.writeEntry( "kWalletUsage", int( m_kWalletUsage ) );
    config.sync();
}

void
LastFmServiceConfig::askAboutMissingKWallet()
{
    if ( !m_askDiag )
    {
        m_askDiag = new KDialog( 0 );
        m_askDiag->setCaption( i18n( "Last.fm credentials" ) );
        m_askDiag->setMainWidget( new QLabel( i18n( "No running KWallet found. Would you like Amarok to save your Last.fm credentials in plaintext?" ) ) );
        m_askDiag->setButtons( KDialog::Yes | KDialog::No );

        connect( m_askDiag, SIGNAL(yesClicked()), this, SLOT(slotStoreCredentialsInAscii()) );
        // maybe connect SIGNAL(noClicked()) to a message informing the user the password will
        // be forgotten on Amarok restart
    }
    m_askDiag->show();
}

void
LastFmServiceConfig::slotStoreCredentialsInAscii() //SLOT
{
    DEBUG_BLOCK
    m_kWalletUsage = PasswordInAscii;
    save();
}
