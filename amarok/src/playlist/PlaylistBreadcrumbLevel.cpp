/****************************************************************************************
 * Copyright (c) 2009 Téo Mrnjavac <teo@kde.org>                                        *
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

#include "PlaylistBreadcrumbLevel.h"

#include "PlaylistDefines.h"

#include <KLocale>

namespace Playlist
{

BreadcrumbLevel::BreadcrumbLevel( QString internalColumnName )
    : m_name( internalColumnName )
{
    Column col = columnForName( internalColumnName );

    m_icon = KIcon( iconName( col ) );
    m_prettyName = columnName( col );
}

BreadcrumbLevel::~BreadcrumbLevel()
{}

const QString &
BreadcrumbLevel::name()
{
    return m_name;
}

const QString &
BreadcrumbLevel::prettyName()
{
    return m_prettyName;
}

const KIcon &
BreadcrumbLevel::icon()
{
    return m_icon;
}

}   //namespace Playlist
