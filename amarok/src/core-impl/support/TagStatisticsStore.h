/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#ifndef TAGSTATISTICSPROVIDER_H
#define TAGSTATISTICSPROVIDER_H

#include "core-impl/support/PersistentStatisticsStore.h"

#include <QString>

class AMAROK_EXPORT TagStatisticsStore : public PersistentStatisticsStore
{
    public:
        TagStatisticsStore( Meta::Track *track );

    protected:
        virtual void save();

    private:
        QString m_name;
        QString m_artist;
        QString m_album;
};

#endif // PERMANENTURLSTATISTICSPROVIDER_H
