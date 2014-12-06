/****************************************************************************************
 * Copyright (c) 2012 Jasneet Singh Bhatti <jazneetbhatti@gmail.com>                    *
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

#include "TestTrackForUrlWorker.h"

#include "config-amarok-test.h"
#include "core/meta/Meta.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "mocks/MockTrackForUrlWorker.h"

#include <KUrl>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/Weaver>
#include <qtest_kde.h>

#include <QMetaType>
#include <QSignalSpy>

QTEST_KDEMAIN_CORE( TestTrackForUrlWorker )

void
TestTrackForUrlWorker::initTestCase()
{
    // To make queued signals/slots work with custom payload
    qRegisterMetaType<Meta::TrackPtr>( "Meta::TrackPtr" );
    qRegisterMetaType<ThreadWeaver::Job*>( "ThreadWeaver::Job*" );
}

QString
TestTrackForUrlWorker::dataPath( const QString &relPath )
{
    return QDir::toNativeSeparators( QString( AMAROK_TEST_DIR ) + '/' + relPath );
}

void
TestTrackForUrlWorker::testCompleteJobKUrl_data()
{
    testCompleteJobInternal_data();
}

void
TestTrackForUrlWorker::testCompleteJobKUrl()
{
    KUrl url;

    MockTrackForUrlWorker *trackForUrlWorker = new MockTrackForUrlWorker( url );
    QVERIFY( trackForUrlWorker );

    testCompleteJobInternal( trackForUrlWorker );
}

void TestTrackForUrlWorker::testCompleteJobQString_data()
{
    testCompleteJobInternal_data();
}

void
TestTrackForUrlWorker::testCompleteJobQString()
{
    QString url;

    MockTrackForUrlWorker *trackForUrlWorker = new MockTrackForUrlWorker( url );
    QVERIFY( trackForUrlWorker );

    testCompleteJobInternal( trackForUrlWorker );
}

void
TestTrackForUrlWorker::testCompleteJobInternal_data()
{
    QTest::addColumn<Meta::TrackPtr>( "track" );

    QTest::newRow( "track 1" ) << CollectionManager::instance()->trackForUrl( dataPath( "data/audio/album/Track01.ogg" ) );
    QTest::newRow( "track 2" ) << CollectionManager::instance()->trackForUrl( dataPath( "data/audio/album/Track02.ogg" ) );
    QTest::newRow( "track 3" ) << CollectionManager::instance()->trackForUrl( dataPath( "data/audio/album/Track03.ogg" ) );
}

void
TestTrackForUrlWorker::testCompleteJobInternal( MockTrackForUrlWorker *trackForUrlWorker )
{
    // Connect finishedLookup with setEmittedTrack() that will store the emitted track
    connect( trackForUrlWorker, SIGNAL(finishedLookup(Meta::TrackPtr)),
             this, SLOT(setEmittedTrack(Meta::TrackPtr)) );

    QSignalSpy spyFinishedLookup( trackForUrlWorker, SIGNAL(finishedLookup(Meta::TrackPtr)) );

    // Enqueue the job for execution and verify that it emits done when finished, which triggers completeJob
    ThreadWeaver::Weaver::instance()->enqueue( trackForUrlWorker );
    bool receivedDone = QTest::kWaitForSignal( trackForUrlWorker, SIGNAL(done(ThreadWeaver::Job*)), 1000 );
    QVERIFY( receivedDone );

    // Verify that finishedLookup was emitted
    QCOMPARE( spyFinishedLookup.count(), 1 );

    // Verify that the track emitted with finishedLookup is indeed the track set by run()
    QFETCH( Meta::TrackPtr, track );
    QCOMPARE( m_emittedTrack, track );

    // Check for emission of the destroyed signal after deferred delete ( deleteLater )
    bool receivedDestroyed = QTest::kWaitForSignal( trackForUrlWorker, SIGNAL(destroyed()), 1000 );
    QVERIFY( receivedDestroyed );
}

void
TestTrackForUrlWorker::setEmittedTrack( Meta::TrackPtr track )
{
    m_emittedTrack = track;
}

#include "TestTrackForUrlWorker.moc"
