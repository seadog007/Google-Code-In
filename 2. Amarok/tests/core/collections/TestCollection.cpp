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

#include "TestCollection.h"

#include "core/capabilities/ActionsCapability.h"
#include "core/capabilities/BookmarkThisCapability.h"
#include "core/collections/Collection.h"
#include "core/collections/CollectionLocation.h"

#include <KIcon>
#include <qtest_kde.h>

using namespace Collections;

/**
 * Ad-hoc mock to test location() method of Collection
 */
class CollectionMock : public Collection
{
    public:
        /**
         * Mock implementations of pure virtual methods of class Collections::Collection
         * to enable creation of an instance of this mock class
         *
         * NOT TO BE USED ANYWHERE IN THE TEST
         */
        virtual QueryMaker *queryMaker()
        {
            Q_ASSERT_X( false, __PRETTY_FUNCTION__, "should not be called");
            return 0;
        }

        virtual QString collectionId() const
        {
            Q_ASSERT_X( false, __PRETTY_FUNCTION__, "should not be called");
            return QString();
        }

        virtual QString prettyName() const
        {
            Q_ASSERT_X( false, __PRETTY_FUNCTION__, "should not be called");
            return QString();
        }

        virtual KIcon icon() const
        {
            Q_ASSERT_X( false, __PRETTY_FUNCTION__, "should not be called");
            return KIcon();
        }
};

/**
 * Ad-hoc mock to reimplement isWritable() and isOrganizable() methods of
 * CollectionLocation to incorporate multiple test cases
 */
class CollectionLocationMock : public CollectionLocation
{
    public:
        virtual bool isWritable() const
        {
            QFETCH( bool, isWritable );
            return isWritable;
        }

        virtual bool isOrganizable() const
        {
            QFETCH( bool, isOrganizable );
            return isOrganizable;
        }
};

/**
 * Ad-hoc mock to test isWritable() and isOrganizable() methods of Collection
 * with multiple test cases
 */
class TestingCollectionMock : public CollectionMock
{
    public:
        virtual Collections::CollectionLocation *location()
        {
            return new CollectionLocationMock();
        }
};


QTEST_KDEMAIN_CORE( TestCollection )

void
TestCollection::initTestCase()
{
    m_collection1 = new CollectionMock();
    m_collection2 = new TestingCollectionMock();
    m_trackProvider = new TrackProvider();
}

void
TestCollection::cleanupTestCase()
{
    delete( m_collection1 );
    delete( m_collection2 );
    delete( m_trackProvider );
}

// TrackProvider
void
TestCollection::testTrackForUrl()
{
    // Always returns a shared pointer pointing to null by default
    KUrl url;
    QVERIFY( m_trackProvider->trackForUrl( url ).isNull() );
}

// Collection
void
TestCollection::testLocation()
{
    CollectionLocation *collectionLocation = m_collection1->location();
    QVERIFY( collectionLocation );
    delete( collectionLocation );
}

void
TestCollection::testIsWritable_data()
{
    QTest::addColumn<bool>( "isWritable" );

    QTest::newRow( "true value" ) << true;
    QTest::newRow( "false value" ) << false;
}

void
TestCollection::testIsWritable()
{
    CollectionLocationMock *collectionLocationMock = new CollectionLocationMock();
    QCOMPARE( m_collection2->isWritable(), collectionLocationMock->isWritable() );
    delete( collectionLocationMock );
}

void
TestCollection::testIsOrganizable_data()
{
    QTest::addColumn<bool>( "isOrganizable" );

    QTest::newRow( "true value" ) << true;
    QTest::newRow( "false value" ) << false;
}

void
TestCollection::testIsOrganizable()
{
    CollectionLocationMock *collectionLocationMock = new CollectionLocationMock();
    QCOMPARE( m_collection2->isOrganizable(), collectionLocationMock->isOrganizable() );
    delete( collectionLocationMock );
}
