/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef AMAROKURLACTION_H
#define AMAROKURLACTION_H

#include "AmarokUrl.h"
#include "BookmarkGroup.h"


#include <QAction>


class AmarokUrlAction : public QAction
{
    Q_OBJECT
public:
    AmarokUrlAction( const QIcon & icon, AmarokUrlPtr url, QObject * parent );
    AmarokUrlAction( AmarokUrlPtr url, QObject * parent );

private slots:
    void run();
    
private:
    AmarokUrlPtr m_url;
};

#endif // AMAROKURLACTION_H
