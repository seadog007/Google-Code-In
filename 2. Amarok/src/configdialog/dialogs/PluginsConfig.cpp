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

#define DEBUG_PREFIX "PluginsConfig"

#include "PluginsConfig.h"

#include "core/support/Debug.h"
#include "services/ServiceBase.h"
#include "PluginManager.h"

#include <KPluginInfo>
#include <KPluginSelector>

#include <QVBoxLayout>

PluginsConfig::PluginsConfig( QWidget *parent )
    : ConfigDialogBase( parent )
    , m_configChanged( false )
{
    DEBUG_BLOCK
    m_selector = new KPluginSelector( this );
    m_selector->setSizePolicy( QSizePolicy:: Expanding, QSizePolicy::Expanding );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->addWidget( m_selector );

    m_selector->addPlugins( The::pluginManager()->plugins( Plugins::PluginFactory::Collection ),
                            KPluginSelector::ReadConfigFile, i18n("Collections"), "Collection" );

    m_selector->addPlugins( The::pluginManager()->plugins( Plugins::PluginFactory::Service ),
                            KPluginSelector::ReadConfigFile, i18n("Internet Services"), "Service" );

    m_selector->addPlugins( The::pluginManager()->plugins( Plugins::PluginFactory::Importer ),
                            KPluginSelector::ReadConfigFile, i18n("Statistics importers"), "Importer" );

    connect( m_selector, SIGNAL(changed(bool)), SLOT(slotConfigChanged(bool)) );
    connect( m_selector, SIGNAL(changed(bool)), parent, SLOT(updateButtons()) );
}

PluginsConfig::~PluginsConfig()
{}

void PluginsConfig::updateSettings()
{
    DEBUG_BLOCK
    if( m_configChanged )
    {
        debug() << "config changed";
        m_selector->save();

        // check if any services were disabled and needs to be removed, or any
        // that are hidden needs to be enabled
        The::pluginManager()->checkPluginEnabledStates();
    }
}

bool PluginsConfig::hasChanged()
{
    return m_configChanged;
}

bool PluginsConfig::isDefault()
{
    return false;
}

void PluginsConfig::slotConfigChanged( bool changed )
{
    m_configChanged = changed;
    if( changed )
        debug() << "config changed";
}

#include "PluginsConfig.moc"
