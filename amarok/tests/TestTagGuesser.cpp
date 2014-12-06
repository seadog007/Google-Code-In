/***************************************************************************
 *   Copyright (c) 2010 Casey Link <unnamedrambler@gmail.com>              *
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

#include "TestTagGuesser.h"

#include "dialogs/TagGuesser.h"
#include "MetaValues.h"

#include <QMap>

#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( TestTagGuesser )

TestTagGuesser::TestTagGuesser()
{
}

void TestTagGuesser::init()
{
  mTagGuesser = new TagGuesser;
}

void TestTagGuesser::cleanup()
{
  delete mTagGuesser;
}


void TestTagGuesser::testStandard()
{
  mTagGuesser->setFilename( "01 - Artist - Title.mp3" );
  mTagGuesser->setSchema( "%track% - %artist% - %title%.%ignore%" );
  QVERIFY( mTagGuesser->guess() );
  
  QMap<qint64,QString> tags = mTagGuesser->tags();
  QCOMPARE( tags[Meta::valArtist], QString( "Artist" ) );
  QCOMPARE( tags[Meta::valTitle], QString( "Title" ) );
  QCOMPARE( tags[Meta::valTrackNr], QString( "01" ) );
}

void TestTagGuesser::testDotInFilename()
{
  // based off bug 225743
  // https://bugs.kde.org/show_bug.cgi?id=225743
  mTagGuesser->setFilename( "03.Moloko - Sing It back.mp3" );
  mTagGuesser->setSchema( "%track%.%artist% - %title%.%ignore%" );
  QVERIFY( mTagGuesser->guess() );
  
  QMap<qint64,QString> tags = mTagGuesser->tags();
  QCOMPARE( tags[Meta::valArtist], QString( "Moloko" ) );
  QCOMPARE( tags[Meta::valTitle], QString( "Sing It back" ) );
  QCOMPARE( tags[Meta::valTrackNr], QString( "03" ) );
}
