/****************************************************************************************
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

#define DEBUG_PREFIX "AbstractDirectoryWatcher"

#include "AbstractDirectoryWatcher.h"

#include "amarokconfig.h"
#include "core/support/Debug.h"

#include <KDirWatch>

#include <QTimer>
#include <QFileInfo>
#include <QMutexLocker>

static const int WATCH_INTERVAL = 60 * 1000; // = 60 seconds
static const int DELAYED_SCAN_INTERVAL = 2 * 1000; // = 2 seconds

AbstractDirectoryWatcher::AbstractDirectoryWatcher()
    : ThreadWeaver::Job( 0 )
    , m_delayedScanTimer( 0 )
    , m_watcher( 0 )
    , m_aborted( false )
    , m_blocked( false )
{
    m_delayedScanTimer = new QTimer( this );
    m_delayedScanTimer->setSingleShot( true );
    connect( m_delayedScanTimer, SIGNAL(timeout()), this, SLOT(delayTimeout()) );

    // -- create a new watcher
    m_watcher = new KDirWatch( this );

    connect( m_watcher, SIGNAL(dirty(QString)),
             this, SLOT(delayedScan(QString)) );
    connect( m_watcher, SIGNAL(created(QString)),
             this, SLOT(delayedScan(QString)) );
    connect( m_watcher, SIGNAL(deleted(QString)),
             this, SLOT(delayedScan(QString)) );

    m_watcher->startScan( false );
}

void
AbstractDirectoryWatcher::run()
{
    // TODO: re-create the watcher if scanRecursively has changed
    QSet<QString> oldWatchDirs;

    forever {
        m_mutex.lock();
        m_waitCondition.wait( &m_mutex, WATCH_INTERVAL );

        if( m_aborted )
            break;

        // -- start scan
        if( AmarokConfig::monitorChanges() )
        {
            if( m_watcher->isStopped() )
            {
                // Check if directories changed while we didn't have a watcher
                QList<KUrl> urls;
                foreach( const QString &path, collectionFolders() )
                {
                    urls.append( KUrl::fromPath( path ) );
                }
                emit requestScan( urls, GenericScanManager::PartialUpdateScan );
                m_watcher->startScan( true );
            }


            // -- update the KDirWatch with the current set of directories
            QSet<QString> dirs = collectionFolders().toSet();

            // - add new
            QSet<QString> newDirs = dirs - oldWatchDirs;
            foreach( const QString& dir, newDirs )
            {
                m_watcher->addDir( dir,
                                   AmarokConfig::scanRecursively() ?
                                   KDirWatch::WatchSubDirs : KDirWatch::WatchDirOnly );
            }

            // - remove old
            QSet<QString> removeDirs = oldWatchDirs - dirs;
            foreach( const QString& dir, removeDirs )
            {
                m_watcher->removeDir( dir );
            }

            oldWatchDirs = dirs;

        }
        else
        {
            if( !m_watcher->isStopped() )
                m_watcher->stopScan();
        }

        m_mutex.unlock();
    }

}

void
AbstractDirectoryWatcher::abort()
{
    m_aborted = true;
    m_waitCondition.wakeAll();
}

void
AbstractDirectoryWatcher::setBlockScanning( bool block )
{
    m_blocked = block;

    // send out the old requests
    if( !m_blocked )
        delayTimeout();
}

void
AbstractDirectoryWatcher::delayTimeout()
{
    QMutexLocker locker( &m_dirsMutex );

    if( m_blocked || m_aborted )
        return;

    if( m_scanDirsRequested.isEmpty() )
        return;

    emit requestScan( m_scanDirsRequested.toList(), GenericScanManager::PartialUpdateScan );
    m_scanDirsRequested.clear();
}

void
AbstractDirectoryWatcher::delayedScan( const QString &path )
{
    QFileInfo info( path );
    if( info.isDir() )
        addDirToList( path );
    else
        addDirToList( info.path() );

    m_delayedScanTimer->start( DELAYED_SCAN_INTERVAL );
}

void
AbstractDirectoryWatcher::addDirToList( const QString &directory )
{
    QMutexLocker locker( &m_dirsMutex );

    debug() << "addDirToList for"<<directory;

    m_scanDirsRequested.insert( directory );
}


#include "AbstractDirectoryWatcher.moc"

