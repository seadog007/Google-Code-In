/****************************************************************************************
 * Copyright (c) 2010 Nikhil Marathe <nsm.nikhil@gmail.com>                             *
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

#ifndef UPNPCOLLECTIONBASE_H
#define UPNPCOLLECTIONBASE_H

#include "core/collections/Collection.h"

#include <QSet>
#include <QHostInfo>
#include <QPointer>
#include <QtGlobal>
#include <QSharedPointer>

#include <KIcon>
#include <KDirNotify>
#include <solid/device.h>

#include "deviceinfo.h"

namespace KIO {
    class Slave;
    class Job;
    class SimpleJob;
}
class KJob;

class QTimer;

namespace Collections {

class UpnpMemoryQueryMaker;

/**
 * UPnP Collections are of two types.
 * If a MediaServer only supports the Browse() action
 * a directory walking, recursive listing UpnpBrowseCollection
 * is used.
 * If a server also supports Search(), a more efficient,
 * UpnpSearchCollection is used.
 * Certain things are common to both, removal,
 * track creation from the UDSEntry, collection identification,
 */
class UpnpCollectionBase : public Collections::Collection
{
  Q_OBJECT
  public:
    UpnpCollectionBase( const DeviceInfo& dev );
    virtual ~UpnpCollectionBase();
    void removeCollection() { emit remove(); }

    virtual QString collectionId() const;
    virtual QString prettyName() const;
    bool possiblyContainsTrack( const KUrl &url ) const;

  private slots:
    void slotSlaveError( KIO::Slave *slave, int err, const QString &msg );
    void slotSlaveConnected( KIO::Slave *slave );
    void slotRemoveJob( KJob *job );
  protected:
    void addJob( KIO::SimpleJob *job );
    //const Solid::Device m_device;
    const DeviceInfo m_device;
    KIO::Slave *m_slave;
    bool m_slaveConnected;
    QSet<KIO::SimpleJob*> m_jobSet;
    int m_continuousJobFailureCount;
};

} //namespace Collections

#endif
