/***************************************************************************
 *   Copyright (c) 2009 Sven Krohlas <sven@asbest-online.de>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef TESTMETAFILETRACK_H
#define TESTMETAFILETRACK_H

#include "core-impl/meta/file/File.h"

#include <QtCore/QObject>
#include <QtCore/QString>

class KTempDir;

class TestMetaFileTrack : public QObject
{
Q_OBJECT

public:
    TestMetaFileTrack();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();

    // methods inherited from Meta::Base
    void testNameAndSetTitle();
    void testPrettyName();
    void testSortableName();

    // methods inherited from Meta::Track
    void testPlayableUrl();
    void testPrettyUrl();
    void testUidUrl();

    void testIsPlayable();
    void testIsEditable();

    void testSetGetAlbum();
    void testSetGetArtist();
    void testSetGetGenre();
    void testSetGetComposer();
    void testSetGetYear();
    void testSetGetComment();
    void testSetGetScore();
    void testSetGetRating();
    void testSetGetTrackNumber();
    void testSetGetDiscNumber();
    void testLength();
    void testFilesize();
    void testSampleRate();
    void testBitrate();
    void testSetGetLastPlayed();
    void testSetGetFirstPlayed();
    void testSetGetPlayCount();
    void testReplayGain();
    void testType();
    void testCreateDate();

private:
    MetaFile::TrackPtr m_track;
    KTempDir *m_tmpDir;
    QString m_tmpFileName;
    QString m_origTrackPath;
};

#endif // TESTMETAFILETRACK_H
