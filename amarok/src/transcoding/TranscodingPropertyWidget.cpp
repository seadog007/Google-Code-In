/****************************************************************************************
 * Copyright (c) 2010 Téo Mrnjavac <teo@kde.org>                                        *
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

#include "TranscodingPropertyWidget.h"

#include "TranscodingPropertySliderWidget.h"

#include "core/support/Debug.h"

namespace Transcoding
{

PropertyWidget *
PropertyWidget::create( Property &property, QWidget * parent )
{
    switch( property.type() )
    {
    case Property::TRADEOFF:
        return new PropertySliderWidget( property, parent );
    default:
        debug() << "Muy bad!";
        return 0;
    }
}

} //namespace Transcoding
