/****************************************************************************************
 * Copyright (c) 2010 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef CONTEXTDOCK_H
#define CONTEXTDOCK_H

#include "widgets/AmarokDockWidget.h"

#include <QWeakPointer>

class KVBox;
class QResizeEvent;

namespace Context {
    class ContextScene;
    class ContextView;
    class ToolbarView;
}

namespace Plasma { class Containment; }

class ContextDock : public AmarokDockWidget
{
    Q_OBJECT

public:
    ContextDock( QWidget *parent );

    void polish();

protected slots:
    void createContextView( Plasma::Containment *containment );

private:
    KVBox * m_mainWidget;

    QWeakPointer<Context::ContextScene> m_corona;
    QWeakPointer<Context::ContextView>  m_contextView;
    QWeakPointer<Context::ToolbarView>  m_contextToolbarView;
};

#endif // CONTEXTDOCK_H
