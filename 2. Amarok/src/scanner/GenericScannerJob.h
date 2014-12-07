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

#ifndef GENERIC_SCANNERJOB_H
#define GENERIC_SCANNERJOB_H

#include "GenericScanManager.h"
#include "collectionscanner/Directory.h"

#include <threadweaver/Job.h>
#include <KUrl>

#include <QObject>
#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QXmlStreamReader>

namespace CollectionScanner {
    class Directory;
}
class KProcess;
class QSharedMemory;
template<class T>
class QSharedPointer;

/** This is the job that does all the hard work with scanning.
    It will receive new data from the scanner process, parse it and call the
    ScanResultProcessor.

    The job will delete itself when finished or aborted. (This is important
    as a separate process should not be killed by another process out of order)

    Design Decision: The ScannerJob should parse the xml from the amarok collection
      scanner while it's being produced. In case the collection scanner crashes
      the scanner process will be restarted and the xml output from the scanner
      seamlessly appended.
*/
class GenericScannerJob : public ThreadWeaver::Job
{
    Q_OBJECT

    public:
        /** Creates the parse job.
            The constructor itself should be called from the UI thread.
            @param input An input io device for the scanner. The input must remain valid as long as the scanner is working (TODO: is this smart?)
        */
        GenericScannerJob( GenericScanManager* manager,
                           QIODevice *input,
                           GenericScanManager::ScanType type );

        GenericScannerJob( GenericScanManager* manager,
                           QStringList scanDirsRequested,
                           GenericScanManager::ScanType type,
                           bool recursive = true,
                           bool detectCharset = false );

        ~GenericScannerJob();

        /* ThreadWeaver::Job virtual methods */
        virtual void run();
        virtual void abort();

    signals:
        void started( GenericScanManager::ScanType type );

        /** Gives the estimated count of directories that this scan will have.
         *  This signal might not be emitted or emitted multiple times if the
         *  count is updated.
        */
        void directoryCount( int count );

        /**
         * Emitted once we get the complete data for a directory.
         *
         * @param dir The directory structure with all containing tracks. It is
         * memory-managed using QSharedPointer - you are not allowed to convert it to a
         * plain pointer unless you can guarantee another QSharedPointer instance pointing
         * to the same object exist for the time of the existence of the plain pointer.
         */
        void directoryScanned( QSharedPointer<CollectionScanner::Directory> dir );

        void succeeded();
        void failed( QString message );
        // and the ThreadWeaver::Job also emits done

    private:
        /** Returns the path to the collection scanner */
        QString scannerPath();

        /** Tries to create the scanner process.
         *  If unable to create the scanner a failed signal will
         *  be emitted by this method.
         *  @returns true if it managed to start.
        */
        bool createScannerProcess( bool restart = false );

        /** Tries to restart the scanner process.
         *  If unable to restart the scanner a failed signal will
         *  be emitted by this method.
         *  @returns true if it managed to restart.
        */
        bool restartScannerProcess();

        void closeScannerProcess();

        /** Parses the currently available input from m_reader.
         *  @returns true if parsing the file is finished successfully.
        */
        bool parseScannerOutput();

        /** Wait for the scanner to produce some output or die */
        void getScannerOutput();

        GenericScanManager* m_manager;
        GenericScanManager::ScanType m_type;
        QStringList m_scanDirsRequested;
        QIODevice *m_input;

        int m_restartCount;
        bool m_abortRequested;
        QString m_incompleteTagBuffer; // strings received via addNewXmlData but not terminated by either a </directory> or a </scanner>

        KProcess *m_scanner;
        QString m_batchfilePath;
        QSharedMemory *m_scannerStateMemory; // a persistent storage of the current scanner state in case it needs to be restarted.
        bool m_recursive;
        bool m_charsetDetect;

        QXmlStreamReader m_reader;

        QMutex m_mutex; // only protects m_abortRequested and the abort reason
};

#endif // SCANNERJOB_H
