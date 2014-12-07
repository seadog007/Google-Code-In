/****************************************************************************************
 * Copyright (c) 2010 Sergey Ivanov <123kash@gmail.com>                                 *
 * Copyright (c) 2013 Alberto Villa <avilla@FreeBSD.org>                                *
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

#define DEBUG_PREFIX "MusicBrainzXmlParser"

#include "MusicBrainzXmlParser.h"

#include "core/meta/support/MetaConstants.h"
#include "core/support/Debug.h"
#include "MusicBrainzMeta.h"

#include <QStringList>
#include <QVariantList>

MusicBrainzXmlParser::MusicBrainzXmlParser( QString &doc )
    : ThreadWeaver::Job()
    , m_doc( "musicbrainz" )
    , m_type( 0 )
{
    m_doc.setContent( doc );
}

void
MusicBrainzXmlParser::run()
{
    DEBUG_BLOCK
    QDomElement docElem = m_doc.documentElement();
    parseElement( docElem );
}

int
MusicBrainzXmlParser::type()
{
    return m_type;
}

void
MusicBrainzXmlParser::parseElement( const QDomElement &e )
{
    QString elementName = e.tagName();
    if( elementName == "recording-list" )
    {
        m_type = TrackList;
        parseRecordingList( e );
    }
    else if( elementName == "release-group" )
    {
        m_type = ReleaseGroup;
        parseReleaseGroup( e );
    }
    else
        parseChildren( e );
}

void
MusicBrainzXmlParser::parseChildren( const QDomElement &e )
{
    QDomNode child = e.firstChild();
    while( !child.isNull() )
    {
        if( child.isElement() )
            parseElement( child.toElement() );
        child = child.nextSibling();
    }
}

QStringList
MusicBrainzXmlParser::parseRecordingList( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QStringList list;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "recording" )
                list << parseRecording( dElement );
        }
        dNode = dNode.nextSibling();
    }
    return list;
}

QString
MusicBrainzXmlParser::parseRecording( const QDomElement &e )
{
    QString id;
    QVariantMap track;

    if( e.hasAttribute( "id" ) )
        id = e.attribute( "id" );
    if( id.isEmpty() )
        return id;

    if( tracks.contains( id ) )
        track = tracks.value( id );
    else
        track.insert( MusicBrainz::TRACKID, id );
    if( track.isEmpty() )
        return id;

    if( e.hasAttribute( "ext:score" ) )
        track.insert( Meta::Field::SCORE, e.attribute( "ext:score" ).toInt() );

    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            if( elementName == "title" )
                track.insert( Meta::Field::TITLE, dElement.text() );
            else if( elementName == "length" )
            {
                int length = dElement.text().toInt();
                if( length > 0 )
                    track.insert( Meta::Field::LENGTH, length );
            }
            else if( elementName == "artist-credit" )
            {
                QStringList idList = parseArtist( dElement );
                if( !idList.isEmpty() )
                {
                    QString artist;
                    QVariantMap artistInfo;
                    foreach( const QString &id, idList )
                    {
                        if( artists.contains( id ) )
                        {
                            artistInfo.insert( id, artists.value( id ) );
                            artist += artists.value( id );
                        }
                        else
                            // If it's not among IDs, it's a joinphrase attribute.
                            artist += id;
                    }
                    if( !artistInfo.isEmpty() )
                    {
                        track.insert( MusicBrainz::ARTISTID, artistInfo );
                        track.insert( Meta::Field::ARTIST, artist );
                    }
                }
            }
            else if( elementName == "release-list" )
            {
                m_currentTrackInfo.clear();
                track.insert( MusicBrainz::RELEASELIST, parseReleaseList( dElement ) );
                track.insert( MusicBrainz::TRACKINFO, m_currentTrackInfo );
            }
        }
        dNode = dNode.nextSibling();
    }

    tracks.insert( id, track );
    return id;
}

QStringList
MusicBrainzXmlParser::parseReleaseList( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QStringList list;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "release" )
                list << parseRelease( dElement );
        }
        dNode = dNode.nextSibling();
    }
    list.removeDuplicates();
    return list;
}

QString
MusicBrainzXmlParser::parseRelease( const QDomElement &e )
{
    QString id;
    QVariantMap release;

    if( e.hasAttribute( "id" ) )
        id = e.attribute( "id" );
    if( id.isEmpty() )
        return id;

    if( releases.contains( id ) )
        release = releases.value( id );
    else
        release.insert( MusicBrainz::RELEASEID, id );
    if( release.isEmpty() )
        return id;

    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            if( elementName == "title" )
                /*
                 * Avoid checking for "(disc N)" string as it's not a safe way to detect
                 * disc number.
                 */
                release.insert( Meta::Field::TITLE, dElement.text() );
            else if( elementName == "medium-list" )
            {
                QVariantMap info = parseMediumList( dElement );
                QVariantList trackCountList = info.values( MusicBrainz::TRACKCOUNT );
                int trackCount = 0;
                foreach( const QVariant &count, trackCountList )
                {
                    trackCount += count.toInt();
                    if( count.toInt() > 0 )
                        release.insert( MusicBrainz::TRACKCOUNT, count.toInt() );
                }
                if( info.contains( Meta::Field::DISCNUMBER ) )
                {
                    int discNumber = info.value( Meta::Field::DISCNUMBER ).toInt();
                    if( discNumber < 1 || ( discNumber == 1 &&
                        ( trackCount <= 0 || trackCountList.size() != 2 ) ) )
                        info.remove( Meta::Field::DISCNUMBER );
                }
                QVariantList trackInfoList = m_currentTrackInfo.value( id ).toList();
                trackInfoList.append( info );
                m_currentTrackInfo.insert( id, trackInfoList );
            }
            else if( elementName == "release-group" )
                release.insert( MusicBrainz::RELEASEGROUPID, parseReleaseGroup( dElement ) );
        }
        dNode = dNode.nextSibling();
    }

    releases.insert( id, release );
    return id;
}

QVariantMap
MusicBrainzXmlParser::parseMediumList( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;
    QVariantMap info;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            if( elementName == "track-count" )
                info.insert( MusicBrainz::TRACKCOUNT, dElement.text().toInt() );
            else if( elementName == "medium" )
                info.unite( parseMedium( dElement ) );
        }
        dNode = dNode.nextSibling();
    }
    return info;
}

QVariantMap
MusicBrainzXmlParser::parseMedium( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;
    QVariantMap info;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            if( elementName == "position" )
            {
                int discNumber = dElement.text().toInt();
                if( discNumber > 0 )
                    info.insert( Meta::Field::DISCNUMBER, discNumber );
            }
            else if( elementName == "track-list" )
            {
                if( dElement.hasAttribute( "count" ) )
                    info.insert( MusicBrainz::TRACKCOUNT,
                                 -1 * dElement.attribute( "count" ).toInt() );
                info.unite( parseTrackList( dElement ) );
            }
        }
        dNode = dNode.nextSibling();
    }
    return info;
}

QVariantMap
MusicBrainzXmlParser::parseTrackList( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QVariantMap info;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "track" )
                info = parseTrack( dElement );
        }
        dNode = dNode.nextSibling();
    }
    return info;
}

QVariantMap
MusicBrainzXmlParser::parseTrack( const QDomElement &e )
{
    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;
    QVariantMap info;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            /*
             * Ignore any <artist-credit /> tag because per track-artists are used
             * inconsistently (especially with classical). Composer tag should be used to
             * get more information. Should the tag differ from the main (<recording />'s)
             * one only by language, "joinphrase" attribute, etc., we better use the main
             * one (as confirmed by MusicBrainz developers).
             */
            if( elementName == "title" )
                info.insert( Meta::Field::TITLE, dElement.text() );
            else if( elementName == "length" )
            {
                int length = dElement.text().toInt();
                if( length > 0 )
                    info.insert( Meta::Field::LENGTH, length );
            }
            else if( elementName == "number" )
            {
                int number = dElement.text().toInt();
                if( number > 0 )
                    info.insert( Meta::Field::TRACKNUMBER, number );
            }
        }
        dNode = dNode.nextSibling();
    }
    return info;
}

QString
MusicBrainzXmlParser::parseReleaseGroup( const QDomElement &e )
{
    QString id;
    QVariantMap releaseGroup;

    if( e.hasAttribute( "id" ) )
        id = e.attribute( "id" );
    if( id.isEmpty() )
        return id;

    if( releaseGroups.contains( id ) )
        releaseGroup = releaseGroups.value( id );
    else
        releaseGroup.insert( MusicBrainz::RELEASEGROUPID, id );
    if( releaseGroup.isEmpty() )
        return id;

    if( m_type != ReleaseGroup )
        return id;

    QDomNode dNode = e.firstChild();
    QDomElement dElement;
    QString elementName;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();
            elementName = dElement.tagName();

            if( elementName == "artist-credit" )
            {
                QStringList idList = parseArtist( dElement );
                if( !idList.isEmpty() )
                {
                    QString artist;
                    QVariantMap artistInfo;
                    foreach( const QString &id, idList )
                    {
                        if( artists.contains( id ) )
                        {
                            artistInfo.insert( id, artists.value( id ) );
                            artist += artists.value( id );
                        }
                        else
                            // If it's not among IDs, it's a joinphrase attribute.
                            artist += id;
                    }
                    if( !artistInfo.isEmpty() )
                    {
                        releaseGroup.insert( MusicBrainz::ARTISTID, artistInfo );
                        releaseGroup.insert( Meta::Field::ARTIST, artist );
                    }
                }
            }
            else if( elementName == "first-release-date" )
            {
                int year = 0;
                QRegExp yearMatcher( "^(\\d{4}).*$" );
                if( yearMatcher.exactMatch( dElement.text() ) )
                    year = yearMatcher.cap( 1 ).toInt();
                if( year > 0 )
                    releaseGroup.insert( Meta::Field::YEAR, year );
            }
        }
        dNode = dNode.nextSibling();
    }

    releaseGroups.insert( id, releaseGroup );
    return id;
}

QStringList
MusicBrainzXmlParser::parseArtist( const QDomElement &e )
{
    QDomNode dNode = e.firstChild(), dNode2, dNode3;
    QDomElement dElement, dElement2, dElement3;
    QStringList idList;
    QString id;

    while( !dNode.isNull() )
    {
        if( dNode.isElement() )
        {
            dElement = dNode.toElement();

            if( dElement.tagName() == "name-credit" )
            {
                /*
                 * <name-credit /> can have a <name /> tag which overwrites the
                 * <artist />'s one. It's set per track or per release, so it's better to
                 * ignore it to avoid having the same artist twice, maybe spelled
                 * differently, which is bad for library organization. The <name /> tag
                 * under <artist /> is global, instead, so let's use it.
                 */
                dNode2 = dNode.firstChild();
                while( !dNode2.isNull() )
                {
                    if( dNode2.isElement() )
                    {
                        dElement2 = dNode2.toElement();

                        if( dElement2.tagName() == "artist" )
                        {
                            dNode3 = dNode2.firstChild();
                            while( !dNode3.isNull() )
                            {
                                if( dNode3.isElement() )
                                {
                                    dElement3 = dNode3.toElement();

                                    if( dElement3.tagName() == "name" )
                                    {
                                        if( dElement2.hasAttribute( "id" ) )
                                            id = dElement2.attribute( "id" );
                                        if( id.isEmpty() )
                                            return QStringList();
                                        artists.insert( id, dElement3.text() );
                                        idList.append( id );
                                        if( dElement.hasAttribute( "joinphrase" ) )
                                            idList.append( dElement.attribute( "joinphrase" ) );
                                    }
                                }
                                dNode3 = dNode3.nextSibling();
                            }
                        }
                    }
                    dNode2 = dNode2.nextSibling();
                }
            }
        }
        dNode = dNode.nextSibling();
    }

    return idList;
}
