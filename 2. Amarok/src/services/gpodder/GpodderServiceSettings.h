/****************************************************************************************
 * Copyright (c) 2007 Shane King <kde@dontletsstart.com>                                *
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

#ifndef GPODDERSERVICESETTINGS_H
#define GPODDERSERVICESETTINGS_H

#include "core/podcasts/PodcastMeta.h"
#include "GpodderServiceConfig.h"
#include <mygpo-qt/ApiRequest.h>

#include <kcmodule.h>

#include <QNetworkReply>

namespace Ui { class GpodderConfigWidget; }

class QListWidgetItem;

class GpodderServiceSettings : public KCModule
{
    Q_OBJECT

public:
    explicit GpodderServiceSettings( QWidget *parent = 0,
                                     const QVariantList &args = QVariantList() );

    virtual ~GpodderServiceSettings();

    virtual void save();
    virtual void load();
    virtual void defaults();

private slots:
    void testLogin();

    void finished();
    void onError( QNetworkReply::NetworkError code );
    void onParseError( );

    void deviceCreationFinished();
    void deviceCreationError( QNetworkReply::NetworkError code );
    void settingsChanged();

private:
    Ui::GpodderConfigWidget *m_configDialog;
    GpodderServiceConfig m_config;

    mygpo::DeviceListPtr m_devices;
    mygpo::AddRemoveResultPtr m_result;
    bool m_enableProvider;
    QNetworkReply *m_createDevice;
};

#endif // GPODDERSERVICESETTINGS_H
