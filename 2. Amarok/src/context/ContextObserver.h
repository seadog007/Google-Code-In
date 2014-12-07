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

#ifndef CONTEXT_OBSERVER_H
#define CONTEXT_OBSERVER_H

#include "amarok_export.h"
#include "Context.h"

#include <QSet>


class ContextSubject;

class AMAROK_EXPORT ContextObserver
{
public:
    virtual void message( const Context::ContextState& ) {}
    
protected:
    ContextObserver();
    ContextObserver( ContextSubject* );
    virtual ~ContextObserver();

private:
    ContextSubject *m_subject;
};

class ContextSubject
{
public:
    void attach( ContextObserver *observer );
    void detach( ContextObserver *observer );
    
protected:
    ContextSubject();
    virtual ~ContextSubject();
    
    void messageNotify( const Context::ContextState& message );
    
private:
    QSet<ContextObserver*> m_observers;
};

#endif
