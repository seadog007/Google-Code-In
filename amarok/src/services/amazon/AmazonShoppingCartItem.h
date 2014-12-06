/****************************************************************************************
 * Copyright (c) 2011 Sven Krohlas <sven@asbest-online.de>                              *
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

#ifndef AMAZONSHOPPINGCARTITEM_H
#define AMAZONSHOPPINGCARTITEM_H

#include <QString>

class AmazonShoppingCartItem
{
public:
    AmazonShoppingCartItem( QString asin, QString price, QString prettyName );

    /**
    * Returns the Amazon internal ID of the item.
    */
    QString asin() const;

    /**
    * Returns the pretty name of the item.
    */
    QString prettyName() const;

    /**
    * Returns the price of the item.
    */
    QString price() const;

private:
    QString m_asin;
    QString m_prettyName;
    QString m_price;
};

#endif // AMAZONSHOPPINGCARTITEM_H
