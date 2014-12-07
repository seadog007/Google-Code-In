/****************************************************************************************
 * Copyright (c) 2008-2010 Casey Link <unnamedrambler@gmail.com>                             *
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

#ifndef AMAROK_FILECOLLECTIONLOCATION_H
#define AMAROK_FILECOLLECTIONLOCATION_H

#include "amarok_export.h"
#include "core/collections/CollectionLocation.h"

#include <QSet>
#include <QMap>
#include <QString>

class KJob;

namespace Collections {

class AMAROK_EXPORT FileCollectionLocation : public CollectionLocation
{
    Q_OBJECT
    public:
        FileCollectionLocation();
        virtual ~FileCollectionLocation();

        virtual QString prettyLocation() const;
        virtual bool isWritable() const;
        virtual bool isOrganizable() const;
        virtual void removeUrlsFromCollection( const Meta::TrackList& sources );
        virtual void showRemoveDialog( const Meta::TrackList &tracks );
    public slots:
        void slotRemoveJobFinished( KJob *job );
    private:
        void startRemoveJobs();

        QMap<KJob*, Meta::TrackPtr> m_removejobs;
        Meta::TrackList m_removetracks;
};

} //namespace Collections

#endif
