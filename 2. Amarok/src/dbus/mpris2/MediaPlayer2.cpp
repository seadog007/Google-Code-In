/***********************************************************************
 * Copyright 2010  Canonical Ltd
 *   (author: Aurelien Gateau <aurelien.gateau@canonical.com>)
 * Copyright 2012  Eike Hein <hein@kde.org>
 * Copyright 2012  Alex Merry <alex.merry@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "MediaPlayer2.h"

#include "App.h"
#include "core/support/Debug.h"
#include "EngineController.h"

#include <QWidget>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KWindowSystem>

using namespace Amarok;

MediaPlayer2::MediaPlayer2(QObject* parent)
    : DBusAbstractAdaptor(parent)
{
}

MediaPlayer2::~MediaPlayer2()
{
}

bool MediaPlayer2::CanRaise() const
{
    return true;
}

void MediaPlayer2::Raise() const
{
    MainWindow *window = App::instance()->mainWindow();
    if( !window )
    {
        warning() << "No window!";
        return;
    }
    window->show();
    KWindowSystem::forceActiveWindow( window->winId() );
}

bool MediaPlayer2::CanQuit() const
{
    return true;
}

void MediaPlayer2::Quit() const
{
    App::instance()->quit();
}

bool MediaPlayer2::CanSetFullscreen() const
{
    return false;
}

bool MediaPlayer2::Fullscreen() const
{
    return false;
}

bool MediaPlayer2::HasTrackList() const
{
    return false;
}

QString MediaPlayer2::Identity() const
{
    return KCmdLineArgs::aboutData()->programName();
}

QString MediaPlayer2::DesktopEntry() const
{
    // Amarok desktop file is installed in $prefix/share/applications/kde4/
    // rather than in $prefix/share/applications. The standard way to
    // represent this dir is with a "kde4-" prefix. See:
    // http://standards.freedesktop.org/menu-spec/1.0/go01.html#term-desktop-file-id
    return QLatin1String("kde4-amarok");
}

QStringList MediaPlayer2::SupportedUriSchemes() const
{
    return QStringList() << "file" << "http";
}

QStringList MediaPlayer2::SupportedMimeTypes() const
{
    // FIXME: this is likely to change when
    // Phonon::BackendCapabilities::notifier()'s capabilitiesChanged signal
    // is emitted (and so a propertiesChanged D-Bus signal should be emitted)
    return The::engineController()->supportedMimeTypes();
}

