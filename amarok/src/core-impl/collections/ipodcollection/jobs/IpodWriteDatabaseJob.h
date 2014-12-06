/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
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

#ifndef IPODWRITEDATABASEJOB_H
#define IPODWRITEDATABASEJOB_H

#include <ThreadWeaver/Job>

class IpodCollection;

/**
 * A job designed to call IpodCollection::writeDatabase() in a thread so that main
 * thread is not blocked with it. It is guaranteed by IpodCollection that is doesn't
 * destory itself while this job is alive. Memory management of this job is up to
 * the caller of it.
 */
class IpodWriteDatabaseJob : public ThreadWeaver::Job
{
    Q_OBJECT

    public:
        explicit IpodWriteDatabaseJob( IpodCollection *collection );
        virtual void run();

    private:
        IpodCollection *m_coll;
};

#endif // IPODWRITEDATABASEJOB_H
