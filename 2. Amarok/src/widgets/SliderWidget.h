/****************************************************************************************
 * Copyright (c) 2003-2009 Mark Kretschmann <kretschmann@kde.org>                       *
 * Copyright (c) 2005 Gabor Lehel <illissius@gmail.com>                                 *
 * Copyright (c) 2008 Dan Meltzer <parallelgrapefruit@gmail.com>                        *
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

#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QList>
#include <QPixmap>
#include <QSlider>
#include <QVector>

class QPalette;
class QTimer;
class BookmarkTriangle;


namespace Amarok
{
    class Slider : public QSlider
    {
        Q_OBJECT

        public:
            explicit Slider( Qt::Orientation, uint max = 0, QWidget* parent = 0 );

            virtual void setValue( int );

        signals:
            //we emit this when the user has specifically changed the slider
            //so connect to it if valueChanged() is too generic
            //Qt also emits valueChanged( int )
            void sliderReleased( int );

        protected:
            virtual void wheelEvent( QWheelEvent* );
            virtual void mouseMoveEvent( QMouseEvent* );
            virtual void mouseReleaseEvent( QMouseEvent* );
            virtual void mousePressEvent( QMouseEvent* );
            virtual void slideEvent( QMouseEvent* );
            QRect sliderHandleRect( const QRect &slider, qreal percent ) const;
            virtual void resizeEvent( QResizeEvent * ) { m_needsResize = true; }

            void paintCustomSlider( QPainter *p, bool paintMoodbar = false );

            bool m_sliding;
            bool m_usingCustomStyle;

            static const int s_borderWidth = 6;
            static const int s_borderHeight = 6;

            static const int s_sliderInsertX = 5;
            static const int s_sliderInsertY = 5;

        private:

            bool m_outside;
            int  m_prevValue;
            bool m_needsResize;

            QPixmap m_topLeft;
            QPixmap m_topRight;
            QPixmap m_top;
            QPixmap m_bottomRight;
            QPixmap m_right;
            QPixmap m_bottomLeft;
            QPixmap m_bottom;
            QPixmap m_left;

            Q_DISABLE_COPY( Slider )
    };

    class VolumeSlider: public Slider
    {
        Q_OBJECT

        public:
            explicit VolumeSlider( uint max, QWidget *parent, bool customStyle = true );

            // VolumePopupButton needs to access this
            virtual void wheelEvent( QWheelEvent *e );

        protected:
            virtual void paintEvent( QPaintEvent* );
            virtual void mousePressEvent( QMouseEvent* );
            virtual void contextMenuEvent( QContextMenuEvent* );

        private:
            Q_DISABLE_COPY( VolumeSlider )
    };

    class TimeSlider : public Amarok::Slider
    {
        Q_OBJECT

        public:
            TimeSlider( QWidget *parent );

            void setSliderValue( int value );
            void drawTriangle( const QString &name, int milliSeconds, bool showPopup = false);
            void clearTriangles();

        protected:
            virtual void paintEvent( QPaintEvent* );
            virtual void mousePressEvent( QMouseEvent* );
            virtual void resizeEvent( QResizeEvent * event );
            virtual void sliderChange( SliderChange change );
            virtual bool event( QEvent * event );

        private slots:
            void slotTriangleClicked( int );
            void slotTriangleFocused( int );

        private:
            Q_DISABLE_COPY( TimeSlider )

            QList<BookmarkTriangle*> m_triangles;
            int m_knobX; // The position of the current indicator.
    };
}

#endif

