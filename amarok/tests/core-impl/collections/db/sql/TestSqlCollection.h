/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
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

#ifndef TESTSQLCOLLECTION_H
#define TESTSQLCOLLECTION_H

#include <QtTest/QtTest>

#include <KTempDir>

class SqlMountPointManagerMock;
class SqlStorage;

namespace Collections {
    class SqlCollection;
}

class TestSqlCollection : public QObject
{
    Q_OBJECT

public:
    TestSqlCollection();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDeviceAddedWithTracks();
    void testDeviceAddedWithoutTracks();
    void testDeviceRemovedWithTracks();
    void testDeviceRemovedWithoutTracks();

private:
    Collections::SqlCollection *m_collection;
    SqlMountPointManagerMock *m_mpmMock;
    SqlStorage *m_storage;
    KTempDir *m_tmpDir;
};

#endif // TESTSQLCOLLECTION_H
