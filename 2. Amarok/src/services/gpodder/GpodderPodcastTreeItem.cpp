/****************************************************************************************
 * Copyright (c) 2011 Stefan Derkits <stefan@derkits.at>                                *
 * Copyright (c) 2011 Christian Wagner <christian.wagner86@gmx.at>                      *
 * Copyright (c) 2011 Felix Winter <ixos01@gmail.com>                                   *
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

#include "GpodderPodcastTreeItem.h"

GpodderPodcastTreeItem::GpodderPodcastTreeItem( mygpo::PodcastPtr podcast, GpodderTreeItem *parent ) : GpodderTreeItem( parent ), m_podcast( podcast )
{
}

GpodderPodcastTreeItem::~GpodderPodcastTreeItem()
{
}

QVariant GpodderPodcastTreeItem::displayData() const
{
    return m_podcast->title();
}

mygpo::PodcastPtr GpodderPodcastTreeItem::podcast() const
{
    return m_podcast;
}
