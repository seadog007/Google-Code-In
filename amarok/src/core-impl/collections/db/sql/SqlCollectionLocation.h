/****************************************************************************************
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2008 Jason A. Donenfeld <Jason@zx2c4.com>                              *
 * Copyright (c) 2010 Casey Link <unnamedrambler@gmail.com>                             *
 * Copyright (c) 2010 Teo Mrnjavac <teo@kde.org>                                        *
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

#ifndef AMAROK_SQLCOLLECTIONLOCATION_H
#define AMAROK_SQLCOLLECTIONLOCATION_H

#include "amarok_sqlcollection_export.h"
#include "core/collections/CollectionLocation.h"

#include <KJob>
#include <KCompositeJob>
#include <QSet>
#include <QMap>
#include <QString>

class OrganizeCollectionDelegateFactory;

namespace Collections {

class SqlCollection;

class SqlCollectionLocation;

/**
 * @class TransferJob
 * A simple class that provides KJob functionality (progress reporting, aborting, etc) for sqlcollectionlocation.
 * It calls SqlCollectionLocation::startNextJob()
 */
class TransferJob : public KCompositeJob
{
    Q_OBJECT
    public:
        TransferJob( SqlCollectionLocation * location, const Transcoding::Configuration & configuration );

        void start();
        virtual bool addSubjob( KJob* job );

        void emitInfo( const QString &message );
    public slots:
        /**
         * A move or copy job finished
         */
        void slotJobFinished( KJob *job );
    protected slots:
        void slotResult( KJob *job );
        void doWork();
        void propagateProcessedAmount( KJob *job, KJob::Unit unit, qulonglong amount);
    protected:
        virtual bool doKill();
    private:
        SqlCollectionLocation* m_location;
        bool m_killed;
        Transcoding::Configuration m_transcodeFormat;
};

class AMAROK_SQLCOLLECTION_EXPORT SqlCollectionLocation : public CollectionLocation
{
    Q_OBJECT

    public:
        SqlCollectionLocation( SqlCollection *collection );
        virtual ~SqlCollectionLocation();

        virtual QString prettyLocation() const;
        virtual QStringList actualLocation() const;
        virtual bool isWritable() const;
        virtual bool isOrganizable() const;

        bool remove( const Meta::TrackPtr &track );
        virtual bool insert( const Meta::TrackPtr &track, const QString &url );

        //dependency injectors
        void setOrganizeCollectionDelegateFactory( OrganizeCollectionDelegateFactory *fac );

    protected:
        virtual void showDestinationDialog( const Meta::TrackList &tracks,
                                            bool removeSources,
                                            const Transcoding::Configuration &configuration );
        virtual void copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources,
                                           const Transcoding::Configuration & configuration );
        virtual void removeUrlsFromCollection( const Meta::TrackList &sources );

    private slots:
        void slotDialogAccepted();
        void slotDialogRejected();
        void slotJobFinished( KJob *job );
        void slotRemoveJobFinished( KJob *job );
        void slotTransferJobFinished( KJob *job );
        void slotTransferJobAborted();

    private:
        KUrl moodFile( const KUrl &track ) const;
        void migrateLabels( const QMap<Meta::TrackPtr, QString> &trackMap );
        bool startNextJob( const Transcoding::Configuration configuration );
        bool startNextRemoveJob();

        Collections::SqlCollection *m_collection;
        OrganizeCollectionDelegateFactory *m_delegateFactory;
        QMap<Meta::TrackPtr, QString> m_destinations;
        QMap<Meta::TrackPtr, KUrl> m_sources;
        Meta::TrackList m_removetracks;
        QHash<Meta::TrackPtr, KUrl> m_originalUrls;
        bool m_overwriteFiles;
        QMap<KJob*, Meta::TrackPtr> m_jobs;
        QMap<KJob*, Meta::TrackPtr> m_removejobs;
        TransferJob* m_transferjob;
        friend class TransferJob; // so the transfer job can run the jobs
};

class SqlCollectionLocationFactory
{
    public:
        virtual SqlCollectionLocation* createSqlCollectionLocation() const = 0;
        virtual ~SqlCollectionLocationFactory() {}
};

} //namespace Collections

class AMAROK_SQLCOLLECTION_EXPORT OrganizeCollectionDelegate : public QObject
{
    Q_OBJECT
public:
    OrganizeCollectionDelegate() : QObject() {}
    virtual ~ OrganizeCollectionDelegate() {}

    virtual void setTracks( const Meta::TrackList &tracks ) = 0;
    virtual void setFolders( const QStringList &folders ) = 0;
    virtual void setIsOrganizing( bool organizing ) = 0;
    virtual void setTranscodingConfiguration( const Transcoding::Configuration &configuration ) = 0;
    virtual void setCaption( const QString &caption ) = 0;

    virtual void show() = 0;

    virtual bool overwriteDestinations() const = 0;
    virtual QMap<Meta::TrackPtr, QString> destinations() const = 0;

signals:
    void accepted();
    void rejected();
};

class OrganizeCollectionDelegateFactory
{
public:
    virtual OrganizeCollectionDelegate* createDelegate() = 0;
    virtual ~OrganizeCollectionDelegateFactory() {}
};

#endif
