/****************************************************************************************
 * Copyright (c) 2005 Eyal Lotem <eyal.lotem@gmail.com>                                 *
 * Copyright (c) 2007 Seb Ruiz <ruiz@kde.org>                                           *
 * Copyright (c) 2009 Pascal Pollet <pascal@bongosoft.de>                               *
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

#ifndef PIXMAPVIEWER_H
#define PIXMAPVIEWER_H

#include <QScrollArea>
#include <QWidget>
#include <QString>
#include <QPixmap>

class QMouseEvent;
class QPixmap;

class PixmapViewer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged )

public:
    PixmapViewer( QWidget *parent, const QPixmap &pixmap, int screenNumber );
    virtual ~PixmapViewer();

    qreal zoomFactor() const;

public slots:
    void setZoomFactor( qreal f );

signals:
    void zoomFactorChanged( qreal );

protected:
    void paintEvent( QPaintEvent *event );
    void wheelEvent( QWheelEvent *event );

private:
    const QPixmap m_pixmap;
    qreal m_zoomFactor;
};

#endif
