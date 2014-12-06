/****************************************************************************************
 * Copyright (c) 2010 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#include "ArtistHelper.h"

#include <KLocalizedString>

#include <QStringList>

QString
ArtistHelper::bestGuessAlbumArtist( const QString &albumArtist, const QString &trackArtist,
                                    const QString &genre, const QString &composer)
{
    QString best( albumArtist );

    // - for classical tracks it's the composer
    if( best.isEmpty() &&
        (genre.compare( i18nc( "The genre name for classical music", "Classical" ), Qt::CaseInsensitive ) == 0 ||
         genre.compare( QLatin1String( "Classical" ), Qt::CaseInsensitive ) == 0 ) )
        best = ArtistHelper::realTrackArtist( composer );

    // - for "normal" tracks it's the track artist
    if( best.isEmpty() )
        best = ArtistHelper::realTrackArtist( trackArtist );

    // - "Various Artists" is the same as no artist
    if( best.compare( i18n( "Various Artists" ), Qt::CaseInsensitive ) == 0 ||
        best.compare( QLatin1String( "Various Artists" ), Qt::CaseInsensitive ) == 0 )
        best.clear();

    return best;
}

QString
ArtistHelper::realTrackArtist( const QString &trackArtistTag )
{
    bool featuring = false;
    QStringList trackArtists;
    if( trackArtistTag.contains( "featuring" ) )
    {
        featuring = true;
        trackArtists = trackArtistTag.split( "featuring" );
    }
    else if( trackArtistTag.contains( "feat." ) )
    {
        featuring = true;
        trackArtists = trackArtistTag.split( "feat." );
    }
    else if( trackArtistTag.contains( "ft." ) )
    {
        featuring = true;
        trackArtists = trackArtistTag.split( "ft." );
    }
    else if( trackArtistTag.contains( "f." ) )
    {
        featuring = true;
        trackArtists = trackArtistTag.split( "f." );
    }

    //this needs to be improved

    if( featuring )
    {
        //always use the first artist
        QString tmp = trackArtists[0].simplified();
        //artists are written as "A (feat. B)" or "A [feat. B]" as well
        if( tmp.endsWith(" (") || tmp.endsWith( " [" ) )
            tmp = tmp.left( tmp.length() -2 ).simplified(); //remove last two characters

        if( tmp.isEmpty() )
            return trackArtistTag; //huh?
        else
        {
            return tmp;
        }
    }
    else
    {
        return trackArtistTag;
    }
}
