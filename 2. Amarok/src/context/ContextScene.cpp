/****************************************************************************************
 * Copyright (c) 2007 Leo Franchi <lfranchi@gmail.com>                                  *
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

#define DEBUG_PREFIX "ContextScene"

#include "ContextScene.h"

#include "core/support/Amarok.h"
#include "amarokconfig.h"
#include "core/support/Debug.h"

#include <KStandardDirs>
#include <plasma/containment.h>
#include <plasma/theme.h>

#include <QGraphicsSceneDragDropEvent>


namespace Context
{

ContextScene::ContextScene( QObject * parent )
    : Plasma::Corona( parent )
{
    DEBUG_BLOCK
    setBackgroundBrush( Qt::NoBrush );
}

ContextScene::~ContextScene()
{
    DEBUG_BLOCK
}

void ContextScene::loadDefaultSetup()
{
    Plasma::Containment* c = addContainment( "amarok_containment_vertical" );
    c->setScreen( -1 );
    c->setFormFactor( Plasma::Planar );
}

} // Context namespace

#include "ContextScene.moc"

