/****************************************************************************************
 * Copyright (c) 2008 Dan Meltzer <parallelgrapefruit@gmail.com>                        *
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

#include "SimilarArtistsAction.h"

#include "core/meta/Meta.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "playlist/PlaylistController.h"

#include <KLocale>
#include <KIcon>

SimilarArtistsAction::SimilarArtistsAction( QObject *parent )
    : GlobalCollectionArtistAction( i18n( "Play Similar Artists from Last.fm" ), parent )
{
    connect( this, SIGNAL(triggered(bool)), SLOT(slotTriggered()) );

    setIcon( KIcon("view-services-lastfm-amarok") );
    setProperty( "popupdropper_svg_id", "lastfm" );
}

void SimilarArtistsAction::slotTriggered()
{
    const QString url = "lastfm://artist/" + artist()->prettyName() + "/similarartists";
    Meta::TrackPtr lastfmtrack = CollectionManager::instance()->trackForUrl( KUrl( url ) );
    The::playlistController()->insertOptioned( lastfmtrack, Playlist::OnPlayMediaAction );
}

#include "SimilarArtistsAction.moc"
