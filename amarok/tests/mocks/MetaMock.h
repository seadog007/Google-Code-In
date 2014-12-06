/****************************************************************************************
 * Copyright (c) 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
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

#ifndef METAMOCK_H
#define METAMOCK_H

#include "core/meta/Meta.h"
#include "core/meta/Statistics.h"
#include "core/meta/support/MetaConstants.h"

#include <QDateTime>
#include <QVariantMap>

/**
  * This class provides simple mocks for meta classes.
  * it will look for the keys defined in meta/MetaConstants.h
  * in the given QVariantMap and return those values in the respective methods.
  */
class MetaMock : public Meta::Track, Meta::Statistics
{
public:
    MetaMock( const QVariantMap &data ) : Meta::Track(), m_data( data ), m_labels( Meta::LabelList() ) {}
    virtual ~MetaMock() {}

    Meta::AlbumPtr album() const { return m_album; }
    Meta::YearPtr year() const { return m_year; }
    Meta::GenrePtr genre() const { return m_genre; }
    Meta::ArtistPtr artist() const { return m_artist; }
    Meta::ComposerPtr composer() const { return m_composer; }

    QString name() const { return m_data.value( Meta::Field::TITLE ).toString(); }
    QString prettyName() const { return name(); }
    KUrl playableUrl() const { return m_data.value( Meta::Field::URL ).value<KUrl>(); }
    QString prettyUrl() const { return playableUrl().url(); }
    QString uidUrl() const { return m_data.value( Meta::Field::UNIQUEID ).toString(); }
    QString notPlayableReason() const { return QString( "dummy reason" ); }
    QString comment() const { return m_data.value( Meta::Field::COMMENT ).toString(); }
    qreal bpm() const { return m_data.value( Meta::Field::BPM ).toDouble(); }
    qint64 length() const { return m_data.value( Meta::Field::LENGTH ).toInt(); }
    int filesize() const { return m_data.value( Meta::Field::FILESIZE ).toInt(); }
    int sampleRate() const { return m_data.value( Meta::Field::SAMPLERATE ).toInt(); }
    int bitrate() const { return m_data.value( Meta::Field::BITRATE ).toInt(); }
    QDateTime createDate() const { return QDateTime(); }    //field missing
    int trackNumber() const { return m_data.value( Meta::Field::TRACKNUMBER ).toInt(); }
    int discNumber() const { return m_data.value( Meta::Field::DISCNUMBER ).toInt(); }
    QString type() const { return "Mock"; }

    Meta::LabelList labels() const { return m_labels; }

    virtual Meta::StatisticsPtr statistics() { return Meta::StatisticsPtr( this ); }

    // Meta::Statistics getters
    double score() const { return m_data.value( Meta::Field::SCORE ).toDouble(); }
    int rating() const { return m_data.value( Meta::Field::RATING ).toInt(); }
    QDateTime firstPlayed() const { return m_data.value( Meta::Field::FIRST_PLAYED ).toDateTime(); }
    QDateTime lastPlayed() const { return m_data.value( Meta::Field::LAST_PLAYED ).toDateTime(); }
    int playCount() const { return m_data.value( Meta::Field::PLAYCOUNT ).toInt(); }

    // Meta::Statistics setters
    void setScore( double newScore ) { m_data[Meta::Field::SCORE].setValue( newScore ); }
    void setRating( int newRating ) { m_data[Meta::Field::RATING].setValue( newRating ); }
    void setFirstPlayed( const QDateTime &date ) { m_data[Meta::Field::FIRST_PLAYED].setValue( date ); }
    void setLastPlayed( const QDateTime &date ) { m_data[Meta::Field::LAST_PLAYED].setValue( date ); }
    void setPlayCount( int newPlayCount ) { m_data[Meta::Field::PLAYCOUNT].setValue( newPlayCount ); }

public:
    QVariantMap m_data;
    Meta::ArtistPtr m_artist;
    Meta::AlbumPtr m_album;
    Meta::GenrePtr m_genre;
    Meta::YearPtr m_year;
    Meta::ComposerPtr m_composer;
    Meta::LabelList m_labels;
};

class MockYear : public Meta::Year
{
public:
    MockYear( const QString &name )
        : Meta::Year()
        , m_name( name ) {}

    QString name() const { return m_name; }
    QString prettyName() const { return m_name; }
    Meta::TrackList tracks() { return Meta::TrackList(); }

    QString m_name;
};

class MockGenre : public Meta::Genre
{
public:
    MockGenre( const QString &name )
        : Meta::Genre()
        , m_name( name ) {}

    QString name() const { return m_name; }
    QString prettyName() const { return m_name; }
    Meta::TrackList tracks() { return Meta::TrackList(); }

    QString m_name;
};

class MockComposer : public Meta::Composer
{
public:
    MockComposer( const QString &name )
        : Meta::Composer()
        , m_name( name ) {}

    QString name() const { return m_name; }
    QString prettyName() const { return m_name; }
    Meta::TrackList tracks() { return Meta::TrackList(); }

    QString m_name;
};

class MockArtist : public Meta::Artist
{
public:
    MockArtist( const QString &name )
        : Meta::Artist()
        , m_name( name ) {}

    QString name() const { return m_name; }
    QString prettyName() const { return m_name; }
    Meta::TrackList tracks() { return Meta::TrackList(); }
    Meta::AlbumList albums() { return Meta::AlbumList(); }

    QString m_name;
};

class MockAlbum : public Meta::Album
{
public:
    MockAlbum( const QString &name, const Meta::ArtistPtr &albumArtist = Meta::ArtistPtr() )
        : Meta::Album()
        , m_name( name )
        , m_albumArtist( albumArtist ) {}

    QString name() const { return m_name; }
    QString prettyName() const { return m_name; }
    Meta::TrackList tracks() { return Meta::TrackList(); }
    bool hasAlbumArtist() const { return ( m_albumArtist ) ? true : false; }
    Meta::ArtistPtr albumArtist() const { return m_albumArtist; }
    bool isCompilation() const { return !hasAlbumArtist(); }

    QString m_name;
    Meta::ArtistPtr m_albumArtist;
};

class MockLabel : public Meta::Label
{
    public:
        MockLabel( const QString &name )
            : Meta::Label()
            , m_name( name ) {}

    QString name() const { return m_name; }

    QString m_name;
};

#endif // METAMOCK_H
