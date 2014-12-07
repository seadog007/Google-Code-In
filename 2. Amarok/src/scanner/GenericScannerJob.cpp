/****************************************************************************************
 * Copyright (c) 2003-2008 Mark Kretschmann <kretschmann@kde.org>                       *
 * Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 * Copyright (c) 2007 Casey Link <unnamedrambler@gmail.com>                             *
 * Copyright (c) 2008-2009 Jeff Mitchell <mitchell@kde.org>                             *
 * Copyright (c) 2010-2011 Ralf Engels <ralf-engels@gmx.de>                             *
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org>                             *
 * Copyright (c) 2013 Ralf Engels <ralf-engels@gmx.de>                                  *
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

#define DEBUG_PREFIX "GenericScannerJob"

#include "GenericScannerJob.h"

#include "App.h"
#include "GenericScanManager.h"
#include "core/support/Debug.h"
#include "collectionscanner/ScanningState.h"

#include <KStandardDirs>
#include <KProcess>

#include <QFile>
#include <QSharedMemory>
#include <QUuid>

static const int MAX_RESTARTS = 40;
static const int SHARED_MEMORY_SIZE = 1024 * 1024; // 1 MB shared memory

GenericScannerJob::GenericScannerJob( GenericScanManager* manager,
                                      QStringList scanDirsRequested,
                                      GenericScanManager::ScanType type,
                                      bool recursive, bool detectCharset )
    : ThreadWeaver::Job( 0 )
    , m_manager( manager )
    , m_type( type )
    , m_scanDirsRequested( scanDirsRequested )
    , m_input( 0 )
    , m_restartCount( 0 )
    , m_abortRequested( false )
    , m_scanner( 0 )
    , m_scannerStateMemory( 0 )
    , m_recursive( recursive )
    , m_charsetDetect( detectCharset )
{
    connect( this, SIGNAL(done(ThreadWeaver::Job*)),
             this, SLOT(deleteLater()) ); // auto delete
}

GenericScannerJob::GenericScannerJob( GenericScanManager* manager,
                                      QIODevice *input,
                                      GenericScanManager::ScanType type )
    : ThreadWeaver::Job( 0 )
    , m_manager( manager )
    , m_type( type )
    , m_input( input )
    , m_restartCount( 0 )
    , m_abortRequested( false )
    , m_scanner( 0 )
    , m_scannerStateMemory( 0 )
    , m_recursive( true )
    , m_charsetDetect( false )
{
    connect( this, SIGNAL(done(ThreadWeaver::Job*)),
             this, SLOT(deleteLater()) ); // auto delete
}


GenericScannerJob::~GenericScannerJob()
{
    delete m_scanner;
    delete m_scannerStateMemory;

    if( !m_batchfilePath.isEmpty() )
        QFile( m_batchfilePath ).remove();
}

void
GenericScannerJob::run()
{
    // -- initialize the input
    // - from io device
    if( m_input )
    {
        m_reader.setDevice( m_input );
    }
    // - from process
    else
    {
        if( !createScannerProcess() )
            return;
    }

    emit started( m_type );

    // -- read the input and loop
    bool finished = false;
    do
    {
        // -- check if we were aborted, have finished or need to wait for new data
        {
            QMutexLocker locker( &m_mutex );
            if( m_abortRequested )
            {
                debug() << "Aborting ScannerJob";
                emit failed( i18n( "Abort for scanner requested" ) );
                closeScannerProcess();
                return;
            }
        }

        if( m_scanner )
        {
            if( m_reader.atEnd() )
                getScannerOutput();

            if( m_scanner->exitStatus() != QProcess::NormalExit )
            {
                if( !restartScannerProcess() )
                    return;
            }
        }

         // -- scan as many directory tags as we added to the data
         finished = parseScannerOutput();

    } while( !finished &&
             (!m_reader.hasError() || m_reader.error() == QXmlStreamReader::PrematureEndOfDocumentError) );

    {
        QMutexLocker locker( &m_mutex );
        if( !finished && m_reader.hasError() )
        {
            warning() << "Aborting ScannerJob with error" << m_reader.errorString();
            emit failed( i18n( "Aborting scanner with error: %1", m_reader.errorString() ) );
            closeScannerProcess();
            return;
        }
        else
        {
            emit succeeded();
            closeScannerProcess();
            return;
        }
    }
}

void
GenericScannerJob::abort()
{
    QMutexLocker locker( &m_mutex );
    m_abortRequested = true;
}

QString
GenericScannerJob::scannerPath()
{
    // Defined in the tests so we use the recently built scanner for testing
    const QString overridePath = qApp->property( "overrideUtilitiesPath" ).toString();
    QString path;
    if( overridePath.isEmpty() ) // Not running a test
    {
        path = KStandardDirs::locate( "exe", "amarokcollectionscanner" );

        // If the binary is not in $PATH, then search in the application folder too
        if( path.isEmpty() )
            path = App::applicationDirPath() + QDir::separator() + "amarokcollectionscanner";
    }
    else
    {
        // Running a test, use the path + append collectionscanner
        path = overridePath + QDir::separator() + "collectionscanner" + QDir::separator() + "amarokcollectionscanner";
    }

    if( !QFile::exists( path ) )
    {
        error() << "Cannot find amarokcollectionscanner! Check your install";
        emit failed( i18n( "Could not find amarokcollectionscanner!" ) );
        return QString();
    }
    return path;
}

bool
GenericScannerJob::createScannerProcess( bool restart )
{
    // -- create the shared memory
    if( !m_scannerStateMemory && !restart )
    {
        QString sharedMemoryKey = "AmarokScannerMemory"+QUuid::createUuid().toString();
        m_scannerStateMemory = new QSharedMemory( sharedMemoryKey );
        if( !m_scannerStateMemory->create( SHARED_MEMORY_SIZE ) )
        {
            warning() << "Unable to create shared memory for collection scanner";
            warning() << "Shared Memory error: " << m_scannerStateMemory->errorString();
            delete m_scannerStateMemory;
            m_scannerStateMemory = 0;
        }
    }

    // -- create the scanner process
    KProcess *scanner = new KProcess(); //not parented since in a different thread
    scanner->setOutputChannelMode( KProcess::OnlyStdoutChannel );

    // debug() << "creating options";
    *scanner << scannerPath() << "--idlepriority";

    if( m_type != GenericScanManager::FullScan ) // we don't need a batch file for a full scan
        m_batchfilePath = m_manager->getBatchFile( m_scanDirsRequested );

    if( m_type != GenericScanManager::FullScan )
        *scanner << "-i";

    if( !m_batchfilePath.isEmpty() )
        *scanner << "--batch" << m_batchfilePath;

    if( m_recursive )
        *scanner << "-r";

    if( m_charsetDetect )
        *scanner << "-c";

    if( restart )
        *scanner << "-s";

    // debug() << "creating shared memory";
    if( m_scannerStateMemory )
        *scanner << "--sharedmemory" << m_scannerStateMemory->key();

    *scanner << m_scanDirsRequested;

    // debug() << "starting";
    scanner->start();
    if( !scanner->waitForStarted( 5000 ) )
    {
        delete scanner;

        warning() << "Unable to start Amarok collection scanner.";
        emit failed( i18n("Unable to start Amarok collection scanner." ) );
        return false;
    }
    // debug() << "finished";

    m_scanner = scanner;
    return true;
}

bool
GenericScannerJob::restartScannerProcess()
{
    if( m_scanner->exitStatus() == QProcess::NormalExit )
        return true; // all shiny. no need to restart

    m_restartCount++;
    warning() << __PRETTY_FUNCTION__ << scannerPath().toLocal8Bit().data()
              << "crashed, restart count is " << m_restartCount;

    // -- try to determine the offending file
    QStringList badFiles;
    if( m_scannerStateMemory )
    {
        using namespace CollectionScanner;
        ScanningState scanningState;
        scanningState.setKey( m_scannerStateMemory->key() );
        scanningState.readFull();

        badFiles << scanningState.badFiles();
        // yes, the last file is also bad, CollectionScanner only adds it after restart
        badFiles << scanningState.lastFile();

        debug() << __PRETTY_FUNCTION__ << "lastDirectory" << scanningState.lastDirectory();
        debug() << __PRETTY_FUNCTION__ << "lastFile" << scanningState.lastFile();
    }
    else
        debug() << __PRETTY_FUNCTION__ << "m_scannerStateMemory is null";

    // -- delete the old scanner
    delete m_scanner;
    m_scanner = 0;

    if( m_restartCount >= MAX_RESTARTS )
    {
        debug() << __PRETTY_FUNCTION__ << "Following files made amarokcollectionscanner (or TagLib) crash:";
        foreach( const QString &file, badFiles )
            debug() << __PRETTY_FUNCTION__ << file;

        // TODO: this message doesn't seem to be propagated to the UI
        QString text = i18n( "The collection scan had to be aborted. Too many crashes (%1) "
                "were encountered during the scan. Following files caused the crashes:\n\n%2",
                m_restartCount, badFiles.join( "\n" ) );

        emit failed( text );
        return false;
    }

    createScannerProcess( true );

    return (m_scanner != 0);
}

void
GenericScannerJob::closeScannerProcess()
{
    if( !m_scanner )
        return;

    m_scanner->close();
    m_scanner->waitForFinished(); // waits at most 3 seconds
    delete m_scanner;
    m_scanner = 0;
}


bool
GenericScannerJob::parseScannerOutput()
{
// DEBUG_BLOCK;
    while( !m_reader.atEnd() )
    {
        // -- check if we were aborted, have finished or need to wait for new data
        {
            QMutexLocker locker( &m_mutex );
            if( m_abortRequested )
                return false;
        }

        m_reader.readNext();
        if( m_reader.hasError() )
        {
            return false;
        }
        else if( m_reader.isStartElement() )
        {
            QStringRef name = m_reader.name();
            if( name == "scanner" )
            {
                int totalCount = m_reader.attributes().value( "count" ).toString().toInt();
                emit directoryCount( totalCount );
            }
            else if( name == "directory" )
            {
                QSharedPointer<CollectionScanner::Directory> dir( new CollectionScanner::Directory( &m_reader ) );
                emit directoryScanned( dir );
            }
            else
            {
                warning() << "Unexpected xml start element"<<name<<"in input";
                m_reader.skipCurrentElement();
            }

        }
        else if( m_reader.isEndElement() )
        {
            if( m_reader.name() == "scanner" ) // ok. finished
                return true;
        }
        else if( m_reader.isEndDocument() )
        {
            return true;
        }
    }

    return false;
}

void
GenericScannerJob::getScannerOutput()
{
    if( !m_scanner->waitForReadyRead( -1 ) )
        return;
    QByteArray newData = m_scanner->readAll();
    m_incompleteTagBuffer += newData;

    int index = m_incompleteTagBuffer.lastIndexOf( "</scanner>" );
    if( index >= 0 )
    {
        // append new data (we need to be locked. the reader is probalby not thread save)
        m_reader.addData( QString( m_incompleteTagBuffer.left( index + 10 ) ) );
        m_incompleteTagBuffer = m_incompleteTagBuffer.mid( index + 10 );
    }
    else
    {
        index = m_incompleteTagBuffer.lastIndexOf( "</directory>" );
        if( index >= 0 )
        {
            // append new data (we need to be locked. the reader is probalby not thread save)
            m_reader.addData( QString( m_incompleteTagBuffer.left( index + 12 ) ) );
            m_incompleteTagBuffer = m_incompleteTagBuffer.mid( index + 12 );
        }
    }

}

#include "GenericScannerJob.moc"
