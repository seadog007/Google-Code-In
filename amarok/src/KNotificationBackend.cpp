/****************************************************************************************
 * Copyright (c) 2009-2011 Kevin Funk <krf@electrostorm.net>                            *
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

#include "KNotificationBackend.h"

#include "EngineController.h"
#include "SvgHandler.h"
#include "core/meta/Meta.h"
#include "core/support/Debug.h"

#include <KIconLoader>
#include <KLocalizedString>
#include <KNotification>
#include <KWindowSystem>

using namespace Amarok;

KNotificationBackend *
KNotificationBackend::s_instance = 0;

KNotificationBackend *
KNotificationBackend::instance()
{
    if( !s_instance )
        s_instance = new KNotificationBackend();
    return s_instance;
}

void
KNotificationBackend::destroy()
{
    delete s_instance;
    s_instance = 0;
}

KNotificationBackend::KNotificationBackend()
    : m_enabled( false )
{
    EngineController *engine = The::engineController();
    connect( engine, SIGNAL(trackPlaying(Meta::TrackPtr)), SLOT(showCurrentTrack()) );
    connect( engine, SIGNAL(trackMetadataChanged(Meta::TrackPtr)), SLOT(showCurrentTrack()) );
    connect( engine, SIGNAL(albumMetadataChanged(Meta::AlbumPtr)), SLOT(showCurrentTrack()) );

    if( engine->isPlaying() )
        showCurrentTrack();
}

KNotificationBackend::~KNotificationBackend()
{
    if( m_notify )
        m_notify.data()->close();
}

void
KNotificationBackend::setEnabled( bool enable )
{
    m_enabled = enable;
}

bool
KNotificationBackend::isEnabled() const
{
    return m_enabled;
}

bool
KNotificationBackend::isFullscreenWindowActive() const
{
    // Get information of the active window.
    KWindowInfo activeWindowInfo = KWindowSystem::windowInfo( KWindowSystem::activeWindow(), NET::WMState );

    // Check if it is running in fullscreen mode.
    return activeWindowInfo.hasState( NET::FullScreen );
}

void
KNotificationBackend::show( const QString &title, const QString &body, const QPixmap &pixmap )
{
    QPixmap icon;
    if( pixmap.isNull() )
    {
        KIconLoader loader;
        icon = loader.loadIcon( QString("amarok"), KIconLoader::Desktop );
    }
    else
        icon = pixmap;

    KNotification *notify = new KNotification( "message" );
    notify->setTitle( title );
    notify->setText( body );
    notify->setPixmap( icon );
    notify->sendEvent();
}

void
KNotificationBackend::showCurrentTrack( bool force )
{
    if( !m_enabled && !force )
        return;

    EngineController *engine = The::engineController();
    Meta::TrackPtr track = engine->currentTrack();
    if( !track )
    {
        warning() << __PRETTY_FUNCTION__ << "null track!";
        return;
    }

    const QString title = i18n( "Now playing" );
    const QString text = engine->prettyNowPlaying();
    Meta::AlbumPtr album = track->album();
    const QPixmap pixmap = album ? The::svgHandler()->imageWithBorder( album, 80 ) : QPixmap();

    KNotification *notify = m_notify.data();
    if( !notify )
        notify = new KNotification( "trackChange" );
    notify->setTitle( title );
    notify->setText( text );
    notify->setPixmap( pixmap );

    if( m_notify ) // existing notification already shown
        notify->update();
    notify->sendEvent(); // (re)start timeout in both cases
    m_notify = notify;
}
