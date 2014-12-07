/****************************************************************************************
* Copyright (c) 2009 Thomas Luebking <thomas.luebking@web.de>                          *
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

#ifndef TRACKACTIONBUTTON_H
#define TRACKACTIONBUTTON_H

class QAction;

#include "IconButton.h"
#include <QImage>
#include <QIcon>

class TrackActionButton : public IconButton
{
    Q_OBJECT

public:
    explicit TrackActionButton( QWidget *parent = 0, const QAction *act = 0 );
    void setAction( const QAction *act );
    QSize sizeHint() const;
protected:
    bool eventFilter( QObject *o, QEvent *e );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void reloadContent( const QSize &sz );
private slots:
    void updateAction();
    void init();
private:
    struct
    {
        QImage image[3];
        QIcon icon;
    } m_icon;
    const QAction *m_action;
};


#endif  // end include guard
