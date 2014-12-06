#/****************************************************************************************
 * Copyright (c) 2009 Simon Esneault <simon.esneault@gmail.com>                         *
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

#ifndef AMAROK_PHOTOS_ENGINE
#define AMAROK_PHOTOS_ENGINE

#include "context/DataEngine.h"
#include "core/meta/Observer.h"
#include "NetworkAccessManagerProxy.h"
#include "PhotosInfo.h"

#include <QXmlStreamReader>

using namespace Context;

 /**
   *   This class provide photos from flickr
   *
   */
class PhotosEngine : public DataEngine, public Meta::Observer
{
    Q_OBJECT
    Q_PROPERTY( int fetchSize READ fetchSize WRITE setFetchSize )
    Q_PROPERTY( QStringList keywords READ keywords WRITE setKeywords )

public:
    PhotosEngine( QObject* parent, const QList<QVariant>& args );
    virtual ~PhotosEngine();

    void init();

    int fetchSize() const;
    void setFetchSize( int size );

    QStringList keywords() const;
    void setKeywords( const QStringList &keywords );

    QStringList sources() const;

    // reimplemented from Meta::Observer
    using Observer::metadataChanged;
    void metadataChanged( Meta::TrackPtr track );

protected:
    //reimplement from Plasma::DataEngine
    bool sourceRequestEvent( const QString& name );

private slots:

    /**
     * This slots will handle Flickr result for this query :
     * API key is : 9c5a288116c34c17ecee37877397fe31
     * Secret is : cc25e5a9532ddc97
     * http://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=9c5a288116c34c17ecee37877397fe31&text=My+Bloody+Valentine
     * see here for details: http://www.flickr.com/services/api/
     */
    void resultFlickr( const KUrl &url, QByteArray data, NetworkAccessManagerProxy::Error e );

    void stopped();
    void trackChanged( Meta::TrackPtr track );

private:
    /**
     * Engine was updated, so we check if the songs is different, and if it is, we delete every and start
     * all the query/ fetching stuff
     */
    void update( bool force = false );

    PhotosInfo::List photosListFromXml( QXmlStreamReader &xml );

    // TODO implement a reload
    void reloadPhotos();

    int m_nbPhotos;

    QSet<KUrl> m_flickrUrls;
    QStringList m_sources;

    Meta::TrackPtr m_currentTrack;
    // Cache the artist of the current track so we can check against metadata
    // updates. We only want to update the photos if the artist change

    QString m_artist;
    QStringList m_keywords;
};

AMAROK_EXPORT_DATAENGINE( photos, PhotosEngine )

#endif
