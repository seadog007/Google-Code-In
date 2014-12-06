/****************************************************************************************
 * Copyright (c) 2008 Leo Franchi <lfranchi@kde.org>                                    *
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

#ifndef AMAROK_CONTEXT_TOOLBAR_VIEW
#define AMAROK_CONTEXT_TOOLBAR_VIEW

#include <QGraphicsView>
#include <QWeakPointer>

class QGraphicsScene;
class QWidget;

namespace Plasma
{
    class Applet;
    class Containment;
}

namespace Context
{

class AppletToolbar;
class AppletToolbarAppletItem;
class AppletItemOverlay;

/**
 * The applet toolbar is a separate QGV on the same QGS that the ContextView uses. This is because we want to be able to show
 *  the add applet menu in the main CV, but we also want to manage the toolbar differently. So the toolbar is actually in the
 *  same scene as the applets but shifted a few thousand pixels so it is invisible in the main CV. The ToolbarView however is
 *  positioned directly above the toolbar in the scene.
 */
class ToolbarView : public QGraphicsView
{
    Q_OBJECT

    public:
        explicit ToolbarView( Plasma::Containment* cont, QGraphicsScene* scene, QWidget* parent = 0 );
        ~ToolbarView();

    signals:
        void hideAppletExplorer();
        void showAppletExplorer();
        
    protected:
        void resizeEvent( QResizeEvent * event );
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dragLeaveEvent(QDragLeaveEvent *event);
    
    private slots:
        void applyStyleSheet();
        void toggleConfigMode();
        void appletRemoved( Plasma::Applet* );
        void appletAdded( Plasma::Applet*, int);
        void refreshOverlays();
        void recreateOverlays();
        void refreshSycoca();

    private:
        int m_height;
        QWeakPointer<AppletToolbar> m_toolbar;
        QList< AppletItemOverlay* > m_moveOverlays;
        Plasma::Containment* m_cont;
};    
    
}


#endif
