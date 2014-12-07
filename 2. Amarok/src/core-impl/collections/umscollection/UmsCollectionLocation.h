/****************************************************************************************
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org>                             *
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

#ifndef UMSCOLLECTIONLOCATION_H
#define UMSCOLLECTIONLOCATION_H

#include "core/collections/CollectionLocation.h"
#include "UmsCollection.h"

#include <KJob>
#include <KCompositeJob>

#include <QList>
#include <QPair>

class UmsTransferJob;

class UmsCollectionLocation : public Collections::CollectionLocation
{
    Q_OBJECT
    public:
        UmsCollectionLocation( UmsCollection *umsCollection );
        ~UmsCollectionLocation();

        /* CollectionLocation methods */
        virtual QString prettyLocation() const;
        virtual QStringList actualLocation() const;
        virtual bool isWritable() const;
        virtual bool isOrganizable() const;

        virtual void copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources,
                                           const Transcoding::Configuration &configuration );
        virtual void removeUrlsFromCollection( const Meta::TrackList &sources );

    protected slots:
        void slotRemoveOperationFinished(); // hides intentionally parent methods

    private slots:
        /**
         * Needed for removal of source tracks during move operation
         */
        void slotTrackTransferred( const KUrl &sourceTrackUrl );

    private:
        UmsCollection *m_umsCollection;
        QHash<KUrl, Meta::TrackPtr> m_sourceUrlToTrackMap;
};

class UmsTransferJob : public KCompositeJob
{
    Q_OBJECT
    public:
        UmsTransferJob( UmsCollectionLocation *location, const Transcoding::Configuration &configuration );

        void addCopy( const KUrl &from, const KUrl &to );
        void addTranscode( const KUrl &from, const KUrl &to );
        virtual void start();

    signals:
        void sourceFileTransferDone( KUrl source );
        void fileTransferDone( KUrl destination );

    public slots:
        void slotCancel();

    private slots:
        void startNextJob();
        void slotChildJobPercent( KJob *job, unsigned long percentage );

        //reimplemented from KCompositeJob
        virtual void slotResult( KJob *job );

    private:
        UmsCollectionLocation *m_location;
        Transcoding::Configuration m_transcodingConfiguration;
        bool m_abort;

        typedef QPair<KUrl,KUrl> KUrlPair;
        QList<KUrlPair> m_copyList;
        QList<KUrlPair> m_transcodeList;
        int m_totalTracks; // total number of tracks in whole transfer
};

#endif // UMSCOLLECTIONLOCATION_H
