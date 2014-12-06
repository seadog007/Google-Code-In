/****************************************************************************************
 * Copyright (c) 2010 Maximilian Kossick <maximilian.kossick@googlemail.com>       *
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

#ifndef TESTAGGREGATEMETA_H
#define TESTAGGREGATEMETA_H

#include <QtTest/QTest>

class TestAggregateMeta : public QObject
{
    Q_OBJECT
public:
    TestAggregateMeta();

private slots:
    void testHasCapabilityOnSingleTrack();
    void testCreateCapabilityOnSingleTrack();

    void testHasCapabilityOnSingleAlbum();
    void testCreateCapabilityOnSingleAlbum();

    void testHasCapabilityOnSingleArtist();
    void testCreateCapabilityOnSingleArtist();

    void testHasCapabilityOnSingleGenre();
    void testCreateCapabilityOnSingleGenre();

    void testHasCapabilityOnSingleComposer();
    void testCreateCapabilityOnSingleComposer();

    void testHasCapabilityOnSingleYear();
    void testCreateCapabilityOnSingleYear();

    void testEditableCapabilityOnMultipleTracks();

    void testPrettyUrl();
};

#endif // TESTAGGREGATEMETA_H
