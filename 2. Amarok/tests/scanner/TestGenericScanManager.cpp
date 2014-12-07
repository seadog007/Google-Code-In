/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2010-2013 Ralf Engels <ralf-engels@gmx.de>                             *
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

#include "TestGenericScanManager.h"

#include "amarokconfig.h"
#include "MetaTagLib.h"
#include "scanner/GenericScanManager.h"

#include "config-amarok-test.h"

#include <qtest_kde.h>

#include <QImage>
#include <QScopedPointer>
#include <QTest>

QTEST_KDEMAIN_CORE( TestGenericScanManager )

TestGenericScanManager::TestGenericScanManager()
    : QObject()
{
    // QString help = i18n("Amarok"); // prevent a bug when the scanner is the first thread creating a translator
}

void
TestGenericScanManager::initTestCase()
{
    // setenv( "LC_ALL", "", 1 ); // this breakes the test
    // Amarok does not force LC_ALL=C but obviously the test does it which
    // will prevent scanning of files with umlauts.

    // that is the original mp3 file that we use to generate the "real" tracks
    m_sourcePath = QDir::toNativeSeparators( QString( AMAROK_TEST_DIR ) + "/data/audio/Platz 01.mp3" );
    QVERIFY( QFile::exists( m_sourcePath ) );

    m_scanManager = new GenericScanManager( this );

    connect( m_scanManager, SIGNAL(started(GenericScanManager::ScanType)),
             SLOT(slotStarted(GenericScanManager::ScanType)) );
    connect( m_scanManager, SIGNAL(directoryCount(int)), SLOT(slotDirectoryCount(int)) );
    connect( m_scanManager, SIGNAL(directoryScanned(QSharedPointer<CollectionScanner::Directory>)),
             SLOT(slotDirectoryScanned(QSharedPointer<CollectionScanner::Directory>)) );
    connect( m_scanManager, SIGNAL(succeeded()), SLOT(slotSucceeded()) );
    connect( m_scanManager, SIGNAL(failed(QString)), SLOT(slotFailed(QString)) );

    AmarokConfig::setScanRecursively( true );
    AmarokConfig::setMonitorChanges( false );

    // switch on writing back so that we can create the test files with all the information
    AmarokConfig::setWriteBack( true );
    AmarokConfig::setWriteBackStatistics( true );
    AmarokConfig::setWriteBackCover( true );
}

void
TestGenericScanManager::cleanupTestCase()
{ }

void
TestGenericScanManager::init()
{
    m_tmpCollectionDir = new KTempDir();
    QVERIFY( m_tmpCollectionDir->exists() );

    QStringList collectionFolders;
    collectionFolders << m_tmpCollectionDir->name();

    m_started = false;
    m_finished = false;

    m_scannedDirsCount = 0;
    m_scannedTracksCount = 0;
    m_scannedCoversCount = 0;
}

void
TestGenericScanManager::cleanup()
{
    m_scanManager->abort();
    delete m_tmpCollectionDir;
}

void
TestGenericScanManager::testScanSingle()
{
    createSingleTrack();
    fullScanAndWait();

    QCOMPARE( m_scannedDirsCount, 3 );
    QCOMPARE( m_scannedTracksCount, 1 );
    QCOMPARE( m_scannedCoversCount, 0 );
}

void
TestGenericScanManager::testScanDirectory()
{
    createAlbum();
    fullScanAndWait();

    // -- check the commit
    QCOMPARE( m_scannedDirsCount, 3 );
    QCOMPARE( m_scannedTracksCount, 9 );
    QCOMPARE( m_scannedCoversCount, 0 );
}

void
TestGenericScanManager::testRestartScanner()
{
#ifndef QT_NO_DEBUG
    createAlbum();

    // the scanner crashes at a special file:
    Meta::FieldHash values;
    values.clear();
    values.insert( Meta::valUniqueId, QVariant("c6c29f50279ab9523a0f44928bc1e96b") );
    values.insert( Meta::valUrl, QVariant("Thriller/crash_amarok_here.ogg") );
    createTrack( values );

    fullScanAndWait();

    // -- check the commit
    QCOMPARE( m_scannedDirsCount, 4 );
    QCOMPARE( m_scannedTracksCount, 9 );
    QCOMPARE( m_scannedCoversCount, 0 );

#else
    QSKIP( "Collection scanner only crashes in debug build.", SkipAll );
#endif
}

void
TestGenericScanManager::testAlbumImage()
{
    createSingleTrack();
    createAlbum();

    // put an image into the album directory
    QString imageSourcePath = QDir::toNativeSeparators( QString( AMAROK_TEST_DIR ) + "/data/playlists/no-playlist.png" );
    QVERIFY( QFile::exists( imageSourcePath ) );
    QString targetPath;
    targetPath = m_tmpCollectionDir->name() + "Pop/Thriller/cover.png";
    QVERIFY( QFile::copy( m_sourcePath, targetPath ) );

    // set an embedded image
    targetPath = m_tmpCollectionDir->name() + "Various Artists/Big Screen Adventures/28 - Theme From Armageddon.mp3";
    Meta::Tag::setEmbeddedCover( targetPath, QImage( 200, 200, QImage::Format_RGB32 ) );

    fullScanAndWait();

    // -- check the commit
    QCOMPARE( m_scannedDirsCount, 5 );
    QCOMPARE( m_scannedTracksCount, 10 );
    QCOMPARE( m_scannedCoversCount, 1 );
}

void
TestGenericScanManager::slotStarted( GenericScanManager::ScanType type )
{
    Q_UNUSED( type );

    QVERIFY( !m_started );
    QVERIFY( !m_finished );

    m_started = true;
}

void
TestGenericScanManager::slotDirectoryCount( int count )
{
    Q_UNUSED( count );

    QVERIFY( m_started );
    QVERIFY( !m_finished );
}

void
TestGenericScanManager::slotDirectoryScanned( QSharedPointer<CollectionScanner::Directory> dir )
{
    QVERIFY( m_started );
    QVERIFY( !m_finished );

    m_scannedDirsCount += 1;
    m_scannedTracksCount += dir->tracks().count();
    m_scannedCoversCount += dir->covers().count();
}

void
TestGenericScanManager::slotSucceeded()
{
    QVERIFY( m_started );
    QVERIFY( !m_finished );

    m_finished = true;
}

void
TestGenericScanManager::slotFailed( const QString& message )
{
    Q_UNUSED( message );

    QVERIFY( m_started );
    QVERIFY( !m_finished );

    m_finished = true;
}

void
TestGenericScanManager::fullScanAndWait()
{
    QList<KUrl> urls;
    urls << KUrl::fromPath( m_tmpCollectionDir->name() );

    m_scanManager->requestScan( urls );
    waitScannerFinished();

    QVERIFY( m_started );
    QVERIFY( m_finished );
}

void
TestGenericScanManager::waitScannerFinished()
{
    QVERIFY( m_scanManager->isRunning() );
    QVERIFY2( QTest::kWaitForSignal( m_scanManager, SIGNAL(succeeded()), 60*1000 ),
              "ScanManager didn't finish scan within timeout" );
    QVERIFY( !m_scanManager->isRunning() );
}

void
TestGenericScanManager::createTrack( const Meta::FieldHash &values )
{
    // -- copy the file from our original
    QVERIFY( values.contains( Meta::valUrl ) );
    const QString targetPath = m_tmpCollectionDir->name() + values.value( Meta::valUrl ).toString();
    QVERIFY( QDir( m_tmpCollectionDir->name() ).mkpath( QFileInfo( values.value( Meta::valUrl ).toString() ).path() ) );

    QVERIFY( QFile::copy( m_sourcePath, targetPath ) );

    // -- set all the values that we need
    Meta::Tag::writeTags( targetPath, values, true );
}

void
TestGenericScanManager::createSingleTrack()
{
    Meta::FieldHash values;

    values.insert( Meta::valUniqueId, QVariant("794b1bd040d5dd9b5b45c1494d84cc82") );
    values.insert( Meta::valUrl, QVariant("Various Artists/Big Screen Adventures/28 - Theme From Armageddon.mp3") );
    values.insert( Meta::valFormat, QVariant("1") );
    values.insert( Meta::valTitle, QVariant("Theme From Armageddon") );
    values.insert( Meta::valArtist, QVariant("Soundtrack & Theme Orchestra") );
    values.insert( Meta::valAlbumArtist, QVariant("Theme Orchestra") );
    values.insert( Meta::valAlbum, QVariant("Big Screen Adventures") );
    values.insert( Meta::valComposer, QVariant("Unknown Composer") );
    values.insert( Meta::valComment, QVariant("Amazon.com Song ID: 210541237") );
    values.insert( Meta::valGenre, QVariant("Broadway & Vocalists") );
    values.insert( Meta::valYear, QVariant(2009) );
    values.insert( Meta::valTrackNr, QVariant(28) );
    // values.insert( Meta::valBitrate, QVariant(216) ); // the bitrate can not be set. it's computed
    // values.insert( Meta::valLength, QVariant(184000) ); // also can't be set
    // values.insert( Meta::valSamplerate, QVariant(44100) ); // again
    // values.insert( Meta::valFilesize, QVariant(5094892) ); // again
    values.insert( Meta::valScore, QVariant(0.875) );
    values.insert( Meta::valPlaycount, QVariant(5) );
    // TODO: set an embedded cover

    createTrack( values );
}

void
TestGenericScanManager::createAlbum()
{
    Meta::FieldHash values;

    values.insert( Meta::valUniqueId, QVariant("1dc7022c52a3e4c51b46577da9b3c8ff") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 01 - Michael Jackson - Track01.mp3") );
    values.insert( Meta::valTitle, QVariant("Wanna Be Startin' Somethin'") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(1) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("1dc708934a3e4c51b46577da9b3ab11") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 02 - Michael Jackson - Track02.mp3") );
    values.insert( Meta::valTitle, QVariant("Baby Be Mine") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(2) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("15a6b1bf79747fdc8e9c6b6f06203017") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 03 - Michael Jackson - Track03.mp3") );
    values.insert( Meta::valTitle, QVariant("The Girl Is Mine") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(3) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("4aba4c8b1d1893c03c112cc3c01221e9") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 04 - Michael Jackson - Track04.mp3") );
    values.insert( Meta::valTitle, QVariant("Thriller") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(4) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("cb44d2a3d8053829b04672723bf0bd6e") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 05 - Michael Jackson - Track05.mp3") );
    values.insert( Meta::valTitle, QVariant("Beat It") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(5) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("eba1858eeeb3c6d97fe3385200114d86") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 06 - Michael Jackson - Track06.mp3") );
    values.insert( Meta::valTitle, QVariant("Billy Jean") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(6) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("4623850290998486b0f7b39a2719904e") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 07 - Michael Jackson - Track07.mp3") );
    values.insert( Meta::valTitle, QVariant("Human Nature") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(7) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("6d9a7de13af1e16bb13a6208e44b046d") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 08 - Michael Jackson - Track08.mp3") );
    values.insert( Meta::valTitle, QVariant("P.Y.T. (Pretty Young Thing)") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(8) );
    createTrack( values );

    values.clear();
    values.insert( Meta::valUniqueId, QVariant("91cf9a7c0d255399f9f6babfacae432b") );
    values.insert( Meta::valUrl, QVariant("Pop/Thriller/Thriller - 09 - Michael Jackson - Track09.mp3") );
    values.insert( Meta::valTitle, QVariant("The Lady In My Life") );
    values.insert( Meta::valArtist, QVariant("Michael Jackson") );
    values.insert( Meta::valAlbum, QVariant("Thriller") );
    values.insert( Meta::valYear, QVariant(1982) );
    values.insert( Meta::valTrackNr, QVariant(9) );
    createTrack( values );
}

