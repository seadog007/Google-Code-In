/****************************************************************************************
 * Copyright (c) 2002 Mark Kretschmann <kretschmann@kde.org>                            *
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

#include "core/support/Amarok.h"

#include "core/meta/Meta.h"
#include "core/meta/support/MetaUtility.h"
#include "core/capabilities/SourceInfoCapability.h"
#include "core/playlists/PlaylistFormat.h"

#include <KCalendarSystem>
#include <KConfigGroup>
#include <KDirLister>
#include <KGlobalSettings>
#include <KIcon>
#include <KIconEffect>
#include <KStandardDirs>
#include <KUniqueApplication>

#include <QDateTime>
#include <QPixmapCache>
#include <QTextDocument>

QWeakPointer<KActionCollection> Amarok::actionCollectionObject;
QMutex Amarok::globalDirsMutex;

namespace Amarok
{

    QString verboseTimeSince( const QDateTime &datetime )
    {
        if( datetime.isNull() || !datetime.toTime_t() )
            return i18nc( "The amount of time since last played", "Never" );

        const QDateTime now = QDateTime::currentDateTime();
        const int datediff = datetime.daysTo( now );

        // HACK: Fix 203522. Arithmetic overflow?
        // Getting weird values from Plasma::DataEngine (LAST_PLAYED field).
        if( datediff < 0 )
            return i18nc( "When this track was last played", "Unknown" );

        if( datediff >= 6*7 /*six weeks*/ ) {  // return absolute month/year
            const KCalendarSystem *cal = KGlobal::locale()->calendar();
            const QDate date = datetime.date();
            return i18nc( "monthname year", "%1 %2", cal->monthName(date),
                          cal->formatDate( date, KLocale::Year, KLocale::LongNumber ) );
        }

        //TODO "last week" = maybe within 7 days, but prolly before last Sunday

        if( datediff >= 7 )  // return difference in weeks
            return i18np( "One week ago", "%1 weeks ago", (datediff+3)/7 );

        const int timediff = datetime.secsTo( now );

        if( timediff >= 24*60*60 /*24 hours*/ )  // return difference in days
            return datediff == 1 ?
                    i18n( "Yesterday" ) :
                    i18np( "One day ago", "%1 days ago", (timediff+12*60*60)/(24*60*60) );

        if( timediff >= 90*60 /*90 minutes*/ )  // return difference in hours
            return i18np( "One hour ago", "%1 hours ago", (timediff+30*60)/(60*60) );

        //TODO are we too specific here? Be more fuzzy? ie, use units of 5 minutes, or "Recently"

        if( timediff >= 0 )  // return difference in minutes
            return timediff/60 ?
                    i18np( "One minute ago", "%1 minutes ago", (timediff+30)/60 ) :
                    i18n( "Within the last minute" );

        return i18n( "The future" );
    }

    QString verboseTimeSince( uint time_t )
    {
        if( !time_t )
            return i18nc( "The amount of time since last played", "Never" );

        QDateTime dt;
        dt.setTime_t( time_t );
        return verboseTimeSince( dt );
    }

    QString conciseTimeSince( uint time_t )
    {
        if( !time_t )
            return i18nc( "The amount of time since last played", "0" );

        QDateTime datetime;
        datetime.setTime_t( time_t );

        const QDateTime now = QDateTime::currentDateTime();
        const int datediff = datetime.daysTo( now );

        if( datediff >= 6*7 /*six weeks*/ ) {  // return difference in months
            return i18nc( "number of months ago", "%1M", datediff/7/4 );
        }

        if( datediff >= 7 )  // return difference in weeks
            return i18nc( "w for weeks", "%1w", (datediff+3)/7 );

        if( datediff == -1 )
            return i18nc( "When this track was last played", "Tomorrow" );

        const int timediff = datetime.secsTo( now );

        if( timediff >= 24*60*60 /*24 hours*/ )  // return difference in days
            // xgettext: no-c-format
            return i18nc( "d for days", "%1d", (timediff+12*60*60)/(24*60*60) );

        if( timediff >= 90*60 /*90 minutes*/ )  // return difference in hours
            return i18nc( "h for hours", "%1h", (timediff+30*60)/(60*60) );

        //TODO are we too specific here? Be more fuzzy? ie, use units of 5 minutes, or "Recently"

        if( timediff >= 60 )  // return difference in minutes
            return QString("%1'").arg( ( timediff + 30 )/60 );
        if( timediff >= 0 )  // return difference in seconds
            return QString("%1\"").arg( ( timediff + 1 )/60 );

        return i18n( "0" );
    }

    void manipulateThe( QString &str, bool reverse )
    {
        if( reverse )
        {
            if( !str.startsWith( "the ", Qt::CaseInsensitive ) )
                return;

            QString begin = str.left( 3 );
            str = str.append( ", %1" ).arg( begin );
            str = str.mid( 4 );
            return;
        }

        if( !str.endsWith( ", the", Qt::CaseInsensitive ) )
            return;

        QString end = str.right( 3 );
        str = str.prepend( "%1 " ).arg( end );

        uint newLen = str.length() - end.length() - 2;

        str.truncate( newLen );
    }

    QString generatePlaylistName( const Meta::TrackList tracks )
    {
        QString datePart = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(),
                                                              KLocale::ShortDate, true );
        if( tracks.count() == 0 )
        {
            return i18nc( "A saved playlist with the current time (KLocale::Shortdate) added between \
                          the parentheses",
                          "Empty Playlist (%1)", datePart );
        }

        bool singleArtist = true;
        bool singleAlbum = true;

        Meta::ArtistPtr artist = tracks.first()->artist();
        Meta::AlbumPtr album = tracks.first()->album();

        QString artistPart;
        QString albumPart;

        foreach( const Meta::TrackPtr track, tracks )
        {
            if( artist != track->artist() )
                singleArtist = false;

            if( album != track->album() )
                singleAlbum = false;

            if ( !singleArtist && !singleAlbum )
                break;
        }

        if( ( !singleArtist && !singleAlbum ) ||
            ( !artist && !album ) )
            return i18nc( "A saved playlist with the current time (KLocale::Shortdate) added between \
                          the parentheses",
                          "Various Tracks (%1)", datePart );

        if( singleArtist )
        {
            if( artist )
                artistPart = artist->prettyName();
            else
                artistPart = i18n( "Unknown Artist(s)" );
        }
        else if( album && album->hasAlbumArtist() && singleAlbum )
        {
            artistPart = album->albumArtist()->prettyName();
        }
        else
        {
            artistPart = i18n( "Various Artists" );
        }

        if( singleAlbum )
        {
            if( album )
                albumPart = album->prettyName();
            else
                albumPart = i18n( "Unknown Album(s)" );
        }
        else
        {
            albumPart = i18n( "Various Albums" );
        }

        return i18nc( "A saved playlist titled <artist> - <album>", "%1 - %2",
                      artistPart, albumPart );
    }

   KActionCollection* actionCollection()  // TODO: constify?
    {
        if( !actionCollectionObject )
        {
            actionCollectionObject = new KActionCollection( kapp );
            actionCollectionObject.data()->setObjectName( "Amarok-KActionCollection" );
        }

        return actionCollectionObject.data();
    }

    KConfigGroup config( const QString &group )
    {
        //Slightly more useful config() that allows setting the group simultaneously
        return KGlobal::config()->group( group );
    }

    namespace ColorScheme
    {
        QColor Base;
        QColor Text;
        QColor Background;
        QColor Foreground;
        QColor AltBase;
    }

    OverrideCursor::OverrideCursor( Qt::CursorShape cursor )
    {
        QApplication::setOverrideCursor( cursor == Qt::WaitCursor ?
                                        Qt::WaitCursor :
                                        Qt::BusyCursor );
    }

    OverrideCursor::~OverrideCursor()
    {
        QApplication::restoreOverrideCursor();
    }

    QString saveLocation( const QString &directory )
    {
        globalDirsMutex.lock();
        QString result = KGlobal::dirs()->saveLocation( "data", QString("amarok/") + directory, true );
        globalDirsMutex.unlock();
        return result;
    }

    QString defaultPlaylistPath()
    {
        return Amarok::saveLocation() + QLatin1String("current.xspf");
    }

    QString cleanPath( const QString &path )
    {
        /* Unicode uses combining characters to form accented versions of other characters.
         * (Exception: Latin-1 table for compatibility with ASCII.)
         * Those can be found in the Unicode tables listed at:
         * http://en.wikipedia.org/w/index.php?title=Combining_character&oldid=255990982
         * Removing those characters removes accents. :)                                   */
        QString result = path;

        // German umlauts
        result.replace( QChar(0x00e4), "ae" ).replace( QChar(0x00c4), "Ae" );
        result.replace( QChar(0x00f6), "oe" ).replace( QChar(0x00d6), "Oe" );
        result.replace( QChar(0x00fc), "ue" ).replace( QChar(0x00dc), "Ue" );
        result.replace( QChar(0x00df), "ss" );

        // other special cases
        result.replace( QChar(0x00C6), "AE" );
        result.replace( QChar(0x00E6), "ae" );

        result.replace( QChar(0x00D8), "OE" );
        result.replace( QChar(0x00F8), "oe" );

        // normalize in a form where accents are separate characters
        result = result.normalized( QString::NormalizationForm_D );

        // remove accents from table "Combining Diacritical Marks"
        for( int i = 0x0300; i <= 0x036F; i++ )
        {
            result.remove( QChar( i ) );
        }

        return result;
    }

    QString asciiPath( const QString &path )
    {
        QString result = path;
        for( int i = 0; i < result.length(); i++ )
        {
            QChar c = result[ i ];
            if( c > QChar(0x7f) || c == QChar(0) )
            {
                c = '_';
            }
            result[ i ] = c;
        }
        return result;
    }

    QString vfatPath( const QString &path, PathSeparatorBehaviour behaviour )
    {
        QString s = path;

        if( behaviour == AutoBehaviour )
            behaviour = ( QDir::separator() == '/' ) ? UnixBehaviour : WindowsBehaviour;

        if( behaviour == UnixBehaviour ) // we are on *nix, \ is a valid character in file or directory names, NOT the dir separator
            s.replace( '\\', '_' );
        else
            s.replace( '/', '_' ); // on windows we have to replace / instead

        int start = 0;
#ifdef Q_OS_WIN
        // exclude the leading "C:/" from special character replecement in the loop below
        // bug 279560, bug 302251
        if( QDir::isAbsolutePath( s ) )
            start = 3;
#endif
        for( int i = start; i < s.length(); i++ )
        {
            QChar c = s[ i ];
            if( c < QChar(0x20) || c == QChar(0x7F) // 0x7F = 127 = DEL control character
                || c=='*' || c=='?' || c=='<' || c=='>'
                || c=='|' || c=='"' || c==':' )
                c = '_';
            else if( c == '[' )
                c = '(';
            else if ( c == ']' )
                c = ')';
            s[ i ] = c;
        }

        /* beware of reserved device names */
        uint len = s.length();
        if( len == 3 || (len > 3 && s[3] == '.') )
        {
            QString l = s.left(3).toLower();
            if( l=="aux" || l=="con" || l=="nul" || l=="prn" )
                s = '_' + s;
        }
        else if( len == 4 || (len > 4 && s[4] == '.') )
        {
            QString l = s.left(3).toLower();
            QString d = s.mid(3,1);
            if( (l=="com" || l=="lpt") &&
                    (d=="0" || d=="1" || d=="2" || d=="3" || d=="4" ||
                     d=="5" || d=="6" || d=="7" || d=="8" || d=="9") )
                s = '_' + s;
        }

        // "clock$" is only allowed WITH extension, according to:
        // http://en.wikipedia.org/w/index.php?title=Filename&oldid=303934888#Comparison_of_file_name_limitations
        if( QString::compare( s, "clock$", Qt::CaseInsensitive ) == 0 )
            s = '_' + s;

        /* max path length of Windows API */
        s = s.left(255);

        /* whitespace at the end of folder/file names or extensions are bad */
        len = s.length();
        if( s[len-1] == ' ' )
            s[len-1] = '_';

        int extensionIndex = s.lastIndexOf( '.' ); // correct trailing spaces in file name itself
        if( ( s.length() > 1 ) &&  ( extensionIndex > 0 ) )
            if( s.at( extensionIndex - 1 ) == ' ' )
                s[extensionIndex - 1] = '_';

        for( int i = 1; i < s.length(); i++ ) // correct trailing whitespace in folder names
        {
            if( ( s.at( i ) == QDir::separator() ) && ( s.at( i - 1 ) == ' ' ) )
                s[i - 1] = '_';
        }

        return s;
    }

    QPixmap semiTransparentLogo( int dim )
    {
        QPixmap logo;
        #define AMAROK_LOGO_CACHE_KEY QLatin1String("AmarokSemiTransparentLogo")+QString::number(dim)
        if( !QPixmapCache::find( AMAROK_LOGO_CACHE_KEY, &logo ) )
        {
            QImage amarokIcon = KIcon( QLatin1String("amarok") ).pixmap( dim, dim ).toImage();
            KIconEffect::toGray( amarokIcon, 1 );
            KIconEffect::semiTransparent( amarokIcon );
            logo = QPixmap::fromImage( amarokIcon );
            QPixmapCache::insert( AMAROK_LOGO_CACHE_KEY, logo );
        }
        #undef AMAROK_LOGO_CACHE_KEY
        return logo;
    }

} // End namespace Amarok
