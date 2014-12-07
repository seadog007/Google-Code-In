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

#ifndef MTPCOLLECTION_H
#define MTPCOLLECTION_H

#include <libmtp.h>

#include "MtpHandler.h"

#include "MediaDeviceCollection.h"
#include "core/support/Debug.h"

#include <QtGlobal>

#include <KIcon>

class MediaDeviceInfo;

namespace Collections {

class MtpCollection;

class MtpCollectionFactory : public MediaDeviceCollectionFactory<MtpCollection>
{
    Q_OBJECT
    public:
        MtpCollectionFactory( QObject *parent, const QVariantList &args );
        virtual ~MtpCollectionFactory();

};

class MtpCollection : public MediaDeviceCollection
{
    Q_OBJECT
	public:

    MtpCollection( MediaDeviceInfo* );
    virtual ~MtpCollection();

    virtual QString collectionId() const;
    virtual QString prettyName() const;
    virtual KIcon icon() const { return KIcon("multimedia-player"); }

    //void writeDatabase();
};

} //namespace Collections

#endif
