/****************************************************************************************
 * Copyright (c) 2009 Casey Link <unnamedrambler@gmail.com>                             *
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

#ifndef TRACKFORURLWORKER_H
#define TRACKFORURLWORKER_H

#include "core/amarokcore_export.h"
#include "core/support/Amarok.h"
#include "core/meta/forward_declarations.h"

#include <KUrl>

#include <threadweaver/Job.h>

namespace Amarok
{
/**
 * Derive from this class and implement the run() method to set mTrack.
 * @author Casey Link
 */
class AMAROK_CORE_EXPORT TrackForUrlWorker : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    TrackForUrlWorker( const KUrl &url );
    TrackForUrlWorker( const QString &url );
    ~TrackForUrlWorker();

    virtual void run() = 0;
signals:
    void finishedLookup( const Meta::TrackPtr &track );

protected:
    KUrl m_url;
    Meta::TrackPtr m_track;

private slots:
    void completeJob();


};

}
#endif // TRACKFORURLWORKER_H
