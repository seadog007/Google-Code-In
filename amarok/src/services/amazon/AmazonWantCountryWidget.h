/****************************************************************************************
 * Copyright (c) 2012 Edward "hades" Toroshchin <amarok@hades.name>                     *
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

#ifndef AMAZONWANTCOUNTRYWIDGET_H
#define AMAZONWANTCOUNTRYWIDGET_H

#include <QWidget>

namespace Ui{ class AmazonWantCountryWidget; }

class AmazonWantCountryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AmazonWantCountryWidget(QWidget *parent = 0);

protected:
    Ui::AmazonWantCountryWidget* ui;

signals:
    void countrySelected();

public slots:

private slots:
    void storeCountry();
    void adjustButtonState();
};

#endif // AMAZONWANTCOUNTRYWIDGET_H
