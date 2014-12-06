/****************************************************************************************
 * Copyright (c) 2009 Simon Esneault <simon.esneault@gmail.com>                         *
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

#ifndef PHOTOSSCROLLWIDGET_H
#define PHOTOSSCROLLWIDGET_H

#include "context/engines/photos/PhotosInfo.h"
#include "NetworkAccessManagerProxy.h"

#include <QGraphicsWidget>

#define PHOTOS_MODE_AUTOMATIC   0
#define PHOTOS_MODE_INTERACTIVE 1
#define PHOTOS_MODE_FADING      2

//forward
class QPixmap;
class QPropertyAnimation;
class QGraphicsSceneHoverEvent;
class DragPixmapItem;

/**
* \brief A widget to present the photos
* 3 possible animation :
*  - Interactive : the sliding is done on mouse hover
*  - Automatic : the photos are presented in an infinite loop, always scrolling
*  - Fading, the photos are presented one by one, fading ...
* \sa QGraphicsWidget
*
* \author Simon Esneault <simon.esneault@gmail.com>
*/

class PhotosScrollWidget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animValue READ animValue WRITE animate)
    public:

        PhotosScrollWidget( QGraphicsItem* parent = 0 );
        ~PhotosScrollWidget();

        void setPhotosInfoList( const PhotosInfo::List &list );

        void setMode( int );

        void clear();

        int count() const;

        qreal animValue() const;
        bool isAnimating() const;

    public slots:
        void animate( qreal anim );
        void automaticAnimBegin();
        void automaticAnimEnd();

       /**
        * Reimplement resize in order to correctly repositioned the stack of pixmap
        */
        virtual void resize( qreal, qreal );

    signals:
        void photoAdded();

    protected:

       /**
        * Reimplement mouse interaction event
        */
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
        //virtual void keyPressEvent(QKeyEvent* event);
        //virtual void wheelEvent(QGraphicsSceneWheelEvent* event);

    private slots:
        void photoFetched( const KUrl&, QByteArray, NetworkAccessManagerProxy::Error );

    private:
        void addPhoto( const PhotosInfoPtr &item, const QPixmap &photo );

        float   m_speed;      // if negative, go to left, if positive go to right,
        int     m_margin;     // margin between the photos
        int     m_scrollmax;  // length of the whole stack
        int     m_actualpos;  //
        int     m_currentPix; // index of the current pix
        int     m_lastPix;    // index of the lat pix
        int     m_interval;   // time in ms between to change
        int     m_mode;       //
        int     m_delta;
        int     m_deltastart;
        QHash<KUrl, PhotosInfoPtr> m_infoHash;
        QPropertyAnimation      *m_animation;   // animation
        QList<int>               m_timerlist;
        PhotosInfo::List         m_currentlist; // contain the list of the current PhotosItem in the widget
        QList<DragPixmapItem *>  m_pixmaplist;  // contain the list of dragpixmap item
        QTimer                  *m_timer;       // our magnificent timer
};

#endif // PHOTOSSCROLLWIDGET_H
