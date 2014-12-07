/****************************************************************************************
 * Copyright (c) 2010 Rick W. Chen <stuffcorpse@archlinux.us>                           *
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

#include "core/support/PluginFactory.h"

Plugins::PluginFactory::PluginFactory( QObject *parent, const QVariantList &args )
    : QObject( parent )
    , m_initialized( false )
    , m_type( Unknown )
{
    Q_UNUSED( args )
}

Plugins::PluginFactory::~PluginFactory()
{}

KPluginInfo
Plugins::PluginFactory::info() const
{
    return m_info;
}

Plugins::PluginFactory::Type
Plugins::PluginFactory::pluginType() const
{
    return m_type;
}

bool
Plugins::PluginFactory::isInitialized() const
{
    return m_initialized;
}

#include "PluginFactory.moc"
