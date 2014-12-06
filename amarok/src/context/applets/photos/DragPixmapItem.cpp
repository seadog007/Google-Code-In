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

#define DEBUG_PREFIX "DragPixmapItem"

#include "DragPixmapItem.h"

#include "core/support/Debug.h"

#include <KIcon>
#include <KLocale>

#include <QApplication>
#include <QDesktopServices>
#include <QDrag>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QPoint>

DragPixmapItem::DragPixmapItem( QGraphicsItem* parent )
    : QGraphicsPixmapItem( parent )
    , m_dragPos( QPoint() )
{
    setAcceptDrops( true );
    setCursor( Qt::PointingHandCursor );
}

void DragPixmapItem::SetClickableUrl( const KUrl &url )
{
    m_url = url;
}

void DragPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
//    DEBUG_BLOCK

    if (event->button() == Qt::LeftButton)
         m_dragPos = event->pos().toPoint();
}

void DragPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    DEBUG_BLOCK
    
    if ( event->button() == Qt::LeftButton )
    {
        if ( !m_url.isEmpty() )
        {
            QDesktopServices::openUrl( m_url );
            debug() << "DragPixmapItem: clicked photos url "<<m_url;
        }
    }
}

void DragPixmapItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if ( !( event->buttons() & Qt::LeftButton ) )
        return;
    if ( ( event->pos().toPoint() - m_dragPos ).manhattanLength() < QApplication::startDragDistance() )
        return;

    QMimeData *data = new QMimeData;
    data->setImageData( this->pixmap().toImage() );

    QDrag *drag = new QDrag( event->widget() );
    drag->setMimeData( data );
    drag->setPixmap( pixmap().scaledToWidth( 140 ) );
    drag->setDragCursor( KIcon( "insert-image" ).pixmap( 24, 24 ), Qt::CopyAction );
    drag->exec( Qt::CopyAction );
}

#include "DragPixmapItem.moc"
