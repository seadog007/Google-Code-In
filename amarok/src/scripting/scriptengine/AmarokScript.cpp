/****************************************************************************************
 * Copyright (c) 2008 Peter ZHOU <peterzhoulei@gmail.com>                               *
 * Copyright (c) 2008 Mark Kretschmann <kretschmann@kde.org>                            *
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

#include "AmarokScript.h"

#include "core/support/Amarok.h"
#include "App.h"
#include "core/support/Debug.h"
#include "scripting/scriptmanager/ScriptManager.h"

#include <KMessageBox>

#include <QScriptEngine>

AmarokScript::AmarokScript::AmarokScript( const QString &name, QScriptEngine *engine )
    : QObject( engine )
    , m_name( name )
{
    QScriptValue scriptObject = engine->newQObject( this, QScriptEngine::AutoOwnership,
                                                    QScriptEngine::ExcludeSuperClassContents );
    engine->globalObject().setProperty( "Amarok", scriptObject );
    if( ScriptManager::instance()->m_scripts.contains( name ) )
        connect( ScriptManager::instance()->m_scripts[name], SIGNAL(uninstalled()), this, SIGNAL(uninstalled()) );
}

void
AmarokScript::AmarokScript::quitAmarok()
{
    App::instance()->quit();
}

void
AmarokScript::AmarokScript::debug( const QString& text ) const
{
    ::debug() << "SCRIPT" << m_name << ": " << text;
}

int
AmarokScript::AmarokScript::alert( const QString& text, const QString& type ) const
{
    //Ok = 1, Cancel = 2, Yes = 3, No = 4, Continue = 5
    if( type == "error" )
    {
        KMessageBox::error( 0, text );
        return -1;
    }
    else if( type == "sorry" )
    {
        KMessageBox::sorry( 0, text );
        return -1;
    }
    else if( type == "information" )
    {
        KMessageBox::information( 0, text );
        return -1;
    }
    else if( type == "questionYesNo" )
        return KMessageBox::questionYesNo( 0, text );
    else if( type == "questionYesNoCancel" )
        return KMessageBox::questionYesNo( 0, text );
    else if( type == "warningYesNo" )
        return KMessageBox::warningYesNo( 0, text );
    else if( type == "warningContinueCancel" )
        return KMessageBox::warningContinueCancel( 0, text );
    else if( type == "warningYesNoCancel" )
        return KMessageBox::warningYesNoCancel( 0, text );

    debug( "alert type not found!" );
    return -1;
}

void
AmarokScript::AmarokScript::end()
{
    ScriptManager::instance()->stopScript( m_name );
}

bool
AmarokScript::AmarokScript::runScript( const QString& name ) const
{
    return ScriptManager::instance()->runScript( name, true );
}

bool
AmarokScript::AmarokScript::stopScript( const QString& name ) const
{
    return ScriptManager::instance()->stopScript( name );
}

QStringList
AmarokScript::AmarokScript::listRunningScripts() const
{
    return ScriptManager::instance()->listRunningScripts();
}
