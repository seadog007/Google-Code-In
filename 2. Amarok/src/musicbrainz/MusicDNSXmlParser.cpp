/****************************************************************************************
 * Copyright (c) 2010 Sergey Ivanov <123kash@gmail.com>                                 *
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

#define DEBUG_PREFIX "MusicDNSXmlParser"

#include "MusicDNSXmlParser.h"

#include "core/support/Debug.h"

MusicDNSXmlParser::MusicDNSXmlParser( QString &doc )
                    : ThreadWeaver::Job()
                    , m_doc( "musicdns" )
{
    m_doc.setContent( doc );
}

void
MusicDNSXmlParser::run()
{
    DEBUG_BLOCK
    QDomElement docElem = m_doc.documentElement();
    parseElement( docElem );
}

QStringList
MusicDNSXmlParser::puid()
{
    return ( m_puid.isEmpty() )?m_puid << "00000000-0000-0000-0000-000000000000":m_puid;
}

void
MusicDNSXmlParser::parseElement( const QDomElement &e )
{
    QString elementName = e.tagName();
    if( elementName == "track" )
        parseTrack( e );
    else
        parseChildren( e );
}

void
MusicDNSXmlParser::parseChildren( const QDomElement &e )
{
    QDomNode child = e.firstChild();
    while( !child.isNull() )
    {
        if( child.isElement() )
            parseElement( child.toElement() );
        child = child.nextSibling();
    }
}

void
MusicDNSXmlParser::parseTrack( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "puid-list" )
                parsePUIDList( dElement );
        }
        dNode = dNode.nextSibling();
    }
}

void
MusicDNSXmlParser::parsePUIDList( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "puid" )
                parsePUID( dElement );
        }
        dNode = dNode.nextSibling();
    }
}

void
MusicDNSXmlParser::parsePUID( const QDomElement &e )
{
    if( e.hasAttribute( "id" ) )
    {
        QString id = e.attribute( "id" );
        if( id.isEmpty() )
            return;
        m_puid << id;
    }
}
