/****************************************************************************************
 * Copyright (c) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>                     *
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

#ifndef MEDIADEVICECOLLECTION_H
#define MEDIADEVICECOLLECTION_H

#include "core/collections/Collection.h"
#include "core-impl/collections/mediadevicecollection/MediaDeviceCollectionLocation.h"
#include "core-impl/collections/mediadevicecollection/support/ConnectionAssistant.h"
#include "core-impl/collections/mediadevicecollection/support/mediadevicecollection_export.h"
#include "core-impl/collections/support/MemoryCollection.h"

#include <KIcon>

#include <QtGlobal>
#include <QSharedPointer>

namespace Collections {

class MediaDeviceCollection;

/**
 * HACK: Base and Factory are separate because Q_OBJECT does not work directly with templates.
 * Templates used to reduce duplicated code in subclasses.
 */
class MEDIADEVICECOLLECTION_EXPORT MediaDeviceCollectionFactoryBase : public Collections::CollectionFactory
{
    Q_OBJECT

    public:
        virtual ~MediaDeviceCollectionFactoryBase();
        virtual void init();

    protected:
        MediaDeviceCollectionFactoryBase( QObject *parent, const QVariantList &args,
                                          ConnectionAssistant* assistant );

    protected slots:
        virtual void slotDeviceDetected( MediaDeviceInfo* info ); // detected type of device, connect it

    private slots:
        void slotDeviceDisconnected( const QString &udi );

    private:
        virtual MediaDeviceCollection* createCollection( MediaDeviceInfo* info ) = 0;

        ConnectionAssistant* m_assistant;
        QMap<QString, MediaDeviceCollection*> m_collectionMap;
};

template <class CollType>
class MediaDeviceCollectionFactory : public MediaDeviceCollectionFactoryBase
{
    protected:
        MediaDeviceCollectionFactory( QObject *parent, const QVariantList &args,
                                      ConnectionAssistant *assistant )
            : MediaDeviceCollectionFactoryBase( parent, args, assistant )
        {}

        virtual ~MediaDeviceCollectionFactory() {}

    private:
        virtual MediaDeviceCollection* createCollection( MediaDeviceInfo* info )
        {
            return new CollType( info );
        }
};


class MEDIADEVICECOLLECTION_EXPORT MediaDeviceCollection : public Collections::Collection
{
    Q_OBJECT

    public:
        /** Collection-related methods */

        virtual ~MediaDeviceCollection();

        /**
         * url-based methods can be abstracted via use of Amarok URLs
         * subclasses simply define a protocol prefix, e.g. ipod
         */
        virtual bool possiblyContainsTrack( const KUrl &url ) const { Q_UNUSED(url); return false;} // TODO: NYI
        virtual Meta::TrackPtr trackForUrl( const KUrl &url ) { Q_UNUSED(url); return Meta::TrackPtr();  } // TODO: NYI

        virtual QueryMaker* queryMaker();
        virtual void startFullScanDevice();

        // NOTE: incrementalscan and stopscan not implemented, might be used by UMS later though

        /**
            The protocol of uids coming from this collection.
            @return A string of the protocol, without the ://

            This has to be overridden for every device type, e.g. ipod://
        */
        virtual QString uidUrlProtocol() const { return QString(); } // TODO: NYI
        virtual QString collectionId() const; // uses udi

        virtual QString prettyName() const = 0; // NOTE: must be overridden based on device type
        virtual KIcon icon() const = 0; // NOTE: must be overridden based on device type

        virtual bool hasCapacity() const;
        virtual float usedCapacity() const;
        virtual float totalCapacity() const;

        // NOTE: location will have same method calls always, no need to redo each time
        virtual CollectionLocation* location() { return new MediaDeviceCollectionLocation( this ); }

        /** Capability-related methods */
        virtual bool hasCapabilityInterface( Capabilities::Capability::Type type ) const;
        virtual Capabilities::Capability* createCapabilityInterface( Capabilities::Capability::Type type );

        /** MediaDeviceCollection methods */
        QString udi() const { return m_udi; }

        /** MediaDeviceCollection-specific */
        Meta::MediaDeviceHandler* handler();
        void init() { m_handler->init(); } // tell handler to start connection
        void emitCollectionReady();

        virtual QAction *ejectAction() const;

        QSharedPointer<MemoryCollection> memoryCollection() const { return m_mc; }
        void collectionUpdated() { emit updated(); }

    signals:
        void collectionReady( Collections::Collection* );
        /** collectionDisconnected is called when ConnectionAssistant
          is told it is to be disconnected.  This could be
          because another part of Amarok (e.g. applet) told it to
          or because the MediaDeviceMonitor noticed it disconnect
        */
        void collectionDisconnected( const QString &udi );
        void deletingCollection();

        void attemptConnectionDone( bool success );

        void copyTracksCompleted( bool success );

    public slots:
        void slotAttemptConnectionDone( bool success );

        virtual void eject();
        void deleteCollection();

    protected:
        MediaDeviceCollection();

        QString m_udi;
        Meta::MediaDeviceHandler *m_handler;

        mutable QAction *m_ejectAction;

        QSharedPointer<MemoryCollection> m_mc;

};

} //namespace Collections

#endif
