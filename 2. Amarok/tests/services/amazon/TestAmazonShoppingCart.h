/****************************************************************************************
 * Copyright (c) 2012 Sven Krohlas <sven@asbest-online.de>                              *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Pulic License for more details.              *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef TESTAMAZONSHOPPINGCART_H
#define TESTAMAZONSHOPPINGCART_H

#include <QObject>

class TestAmazonShoppingCart : public QObject
{
    Q_OBJECT
public:
    TestAmazonShoppingCart();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testAdd();
    void testClear();
    void testStringList();
    void testPrice();
    void testRemove();
    void testCheckoutUrl();
};

#endif // TESTAMAZONSHOPPINGCART_H
