/****************************************************************************************
 * Copyright (c) 2010 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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


#ifndef SERVICECAPABILITIES_H
#define SERVICECAPABILITIES_H


#include "amarok_export.h"
#include "amarokurls/AmarokUrl.h"

#include "core/capabilities/BookmarkThisCapability.h"
#include "core/capabilities/ActionsCapability.h"
#include "core/capabilities/FindInSourceCapability.h"
#include "core/capabilities/SourceInfoCapability.h"


class BookmarkThisProvider;
class ActionsProvider;
class SourceInfoProvider;

namespace Meta
{
    class ServiceTrack;
}


/**
A service specific implementation of the BookmarkThisCapability

    @author Nikolaj Hald Nielsen <nhn@kde.org>
*/
class AMAROK_EXPORT ServiceBookmarkThisCapability : public Capabilities::BookmarkThisCapability {
public:
    ServiceBookmarkThisCapability( BookmarkThisProvider * provider );

    ~ServiceBookmarkThisCapability();

    virtual bool isBookmarkable();
    virtual QString browserName();
    virtual QString collectionName();
    virtual bool simpleFiltering();
    virtual QAction * bookmarkAction() const;

private:

    BookmarkThisProvider * m_provider;
};


class AMAROK_EXPORT ServiceActionsCapability : public Capabilities::ActionsCapability
{
    Q_OBJECT

    public:
        ServiceActionsCapability( ActionsProvider * actionsProvider  );
        virtual ~ServiceActionsCapability();
        virtual QList< QAction * > actions() const;

    private:
        ActionsProvider * m_actionsProvider;
};



class AMAROK_EXPORT ServiceSourceInfoCapability : public Capabilities::SourceInfoCapability
{
public:
    ServiceSourceInfoCapability( SourceInfoProvider * sourceInfoProvider );

    ~ServiceSourceInfoCapability();

    QString sourceName();
    QString sourceDescription();
    QPixmap emblem();
    QString scalableEmblem();

private:
    SourceInfoProvider * m_sourceInfoProvider;

};



class AMAROK_EXPORT ServiceFindInSourceCapability : public Capabilities::FindInSourceCapability
{
    Q_OBJECT
    public:
        ServiceFindInSourceCapability( Meta::ServiceTrack *track );
        virtual void findInSource( QFlags<TargetTag> tag );

    private:
        Meta::ServiceTrack * m_track;
};



#endif // SERVICECAPABILITIES_H
