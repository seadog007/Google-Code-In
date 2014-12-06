/****************************************************************************************
 * Copyright (c) 2009 Téo Mrnjavac <teo@kde.org>                                        *
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

#include "OcsPersonItem.h"

#include "core/support/Debug.h"
#include "libattica-ocsclient/provider.h"
#include "libattica-ocsclient/providerinitjob.h"
#include "libattica-ocsclient/personjob.h"

#include <KAction>
#include <KRun>
#include <KStandardDirs>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyleOption>

OcsPersonItem::OcsPersonItem( const KAboutPerson &person, const QString ocsUsername, PersonStatus status, QWidget *parent )
    : QWidget( parent )
    , m_status( status )
    , m_state( Offline )
{
    m_person = &person;
    m_ocsUsername = ocsUsername;

    setupUi( this );
    init();

    m_avatar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    //TODO: Add favorite artists!

}

void
OcsPersonItem::init()
{
    m_textLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    m_textLabel->setOpenExternalLinks( true );
    m_textLabel->setContentsMargins( 5, 0, 0, 2 );
    m_verticalLayout->setSpacing( 0 );

    m_vertLine->hide();
    m_initialSpacer->changeSize( 0, 40, QSizePolicy::Fixed, QSizePolicy::Fixed );
    layout()->invalidate();

    m_aboutText.append( "<b>" + m_person->name() + "</b>" );
    if( !m_person->task().isEmpty() )
        m_aboutText.append( "<br/>" + m_person->task() );

    m_iconsBar = new KToolBar( this, false, false );
    m_snBar = new KToolBar( this, false, false );

    m_iconsBar->setIconSize( QSize( 22, 22 ) );
    m_iconsBar->setContentsMargins( 0, 0, 0, 0 );
    m_iconsBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    if( m_status == Author )
    {
        QHBoxLayout *iconsLayout = new QHBoxLayout( this );
        iconsLayout->setMargin( 0 );
        iconsLayout->setSpacing( 0 );
        m_verticalLayout->insertLayout( m_verticalLayout->count() - 1, iconsLayout );
        iconsLayout->addWidget( m_iconsBar );
        iconsLayout->addWidget( m_snBar );
        iconsLayout->addStretch( 0 );

        m_snBar->setIconSize( QSize( 16, 16 ) );
        m_snBar->setContentsMargins( 0, 0, 0, 0 );
        m_snBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    }
    else
    {
        layout()->addWidget( m_iconsBar );
        m_snBar->hide();
    }

    if( !m_person->emailAddress().isEmpty() )
    {
        KAction *email = new KAction( KIcon( "internet-mail" ), i18n("Email contributor"), this );
        email->setToolTip( m_person->emailAddress() );
        email->setData( QString( "mailto:" + m_person->emailAddress() ) );
        m_iconsBar->addAction( email );
    }

    if( !m_person->webAddress().isEmpty() )
    {
        KAction *homepage = new KAction( KIcon( "applications-internet" ), i18n("Visit contributor's homepage"), this );
        homepage->setToolTip( m_person->webAddress() );
        homepage->setData( m_person->webAddress() );
        m_iconsBar->addAction( homepage );
    }

    connect( m_iconsBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(launchUrl(QAction*)) );
    connect( m_snBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(launchUrl(QAction*)) );
    m_textLabel->setText( m_aboutText );
}

OcsPersonItem::~OcsPersonItem()
{}

QString
OcsPersonItem::name()
{
    return m_person->name();
}

void
OcsPersonItem::launchUrl( QAction *action ) //SLOT
{
    KUrl url = KUrl( action->data().toString() );
    KRun::runUrl( url, "text/html", 0, false );
}

void
OcsPersonItem::switchToOcs( const AmarokAttica::Provider &provider )
{
    if( m_state == Online )
        return;
    m_avatar->setFixedWidth( 56 );
    m_vertLine->show();
    m_initialSpacer->changeSize( 5, 40, QSizePolicy::Fixed, QSizePolicy::Fixed );
    layout()->invalidate();

    if( !m_ocsUsername.isEmpty() )
    {
        AmarokAttica::PersonJob *personJob;
        if( m_ocsUsername == QString( "%%category%%" ) )   //TODO: handle grouping
            return;

        personJob = provider.requestPerson( m_ocsUsername );
        connect( personJob, SIGNAL(result(KJob*)), this, SLOT(onJobFinished(KJob*)) );
        emit ocsFetchStarted();
        m_state = Online;
    }
}

void
OcsPersonItem::onJobFinished( KJob *job )
{
    AmarokAttica::PersonJob *personJob = qobject_cast< AmarokAttica::PersonJob * >( job );
    if( personJob->error() == 0 )
    {
        fillOcsData( personJob->person() );
    }
    emit ocsFetchResult( personJob->error() );
}

void
OcsPersonItem::fillOcsData( const AmarokAttica::Person &ocsPerson )
{
    if( !( ocsPerson.avatar().isNull() ) )
    {
        m_avatar->setFixedSize( 56, 56 );
        m_avatar->setFrameShape( QFrame::StyledPanel ); //this is a FramedLabel, otherwise oxygen wouldn't paint the frame
        m_avatar->setPixmap( ocsPerson.avatar() );
        m_avatar->setAlignment( Qt::AlignCenter );
    }

    if( !ocsPerson.country().isEmpty() )
    {
        m_aboutText.append( "<br/>" );
        if( !ocsPerson.city().isEmpty() )
            m_aboutText.append( i18nc( "A person's location: City, Country", "%1, %2", ocsPerson.city(), ocsPerson.country() ) );
        else
            m_aboutText.append( ocsPerson.country() );
    }

    if( m_status == Author )
    {
        if( !ocsPerson.extendedAttribute( "ircchannels" ).isEmpty() )
        {
            QString channelsString = ocsPerson.extendedAttribute( "ircchannels" );
            //We extract the channel names from the string provided by OCS:
            QRegExp channelrx = QRegExp( "#+[\\w\\.\\-\\/!()+]+([\\w\\-\\/!()+]?)", Qt::CaseInsensitive );
            QStringList channels;
            int pos = 0;
            while( ( pos = channelrx.indexIn( channelsString, pos ) ) != -1 )
            {
                channels << channelrx.cap( 0 );
                pos += channelrx.matchedLength();
            }

            m_aboutText.append( "<br/>" + i18n("IRC channels: ") );
            QString link;
            foreach( const QString &channel, channels )
            {
                const QString channelName = QString( channel ).remove( '#' );
                link = QString( "irc://irc.freenode.org/%1" ).arg( channelName );
                m_aboutText.append( QString( "<a href=\"%1\">%2</a>" ).arg( link, channel ) + "  " );
            }
        }
        if( !ocsPerson.extendedAttribute( "favouritemusic" ).isEmpty() )
        {
            QStringList artists = ocsPerson.extendedAttribute( "favouritemusic" ).split( ", " );
            //TODO: make them clickable
            m_aboutText.append( "<br/>" + i18n( "Favorite music: " ) + artists.join( ", " ) );
        }
    }

    KAction *visitProfile = new KAction( KIcon( QPixmap( KStandardDirs::locate( "data",
            "amarok/images/opendesktop-22.png" ) ) ), i18n( "Visit %1's openDesktop.org profile", ocsPerson.firstName() ), this );

    visitProfile->setToolTip( i18n( "Visit %1's profile on openDesktop.org", ocsPerson.firstName() ) );

    visitProfile->setData( ocsPerson.extendedAttribute( "profilepage" ) );
    m_iconsBar->addAction( visitProfile );

    if( m_status == Author )
    {
        QList< QPair< QString, QString > > ocsHomepages;
        ocsHomepages.append( QPair< QString, QString >( ocsPerson.extendedAttribute( "homepagetype" ), ocsPerson.homepage() ) );

        debug() << "USER HOMEPAGE DATA STARTS HERE";
        debug() << ocsHomepages.last().first << " :: " << ocsHomepages.last().second;

        for( int i = 2; i <= 10; i++ )  //OCS supports 10 total homepages as of 2/oct/2009
        {
            QString type = ocsPerson.extendedAttribute( QString( "homepagetype%1" ).arg( i ) );
            ocsHomepages.append( QPair< QString, QString >( ( type == "&nbsp;" ) ? "" : type,
                                                            ocsPerson.extendedAttribute( QString( "homepage%1" ).arg( i ) ) ) );
            debug() << ocsHomepages.last().first << " :: " << ocsHomepages.last().second;
        }

        bool fillHomepageFromOcs = m_person->webAddress().isEmpty();    //We check if the person already has a homepage in KAboutPerson.

        for( QList< QPair< QString, QString > >::const_iterator entry = ocsHomepages.constBegin();
             entry != ocsHomepages.constEnd(); ++entry )
        {
            QString type = (*entry).first;
            QString url = (*entry).second;
            KIcon icon;
            QString text;

            if( type == "Blog" )
            {
                icon = KIcon( "kblogger" );
                text = i18n( "Visit contributor's blog" );
            }
            else if( type == "delicious" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-delicious.png" ) ) );
                text = i18n( "Visit contributor's del.icio.us profile" );
            }
            else if( type == "Digg" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-digg.png" ) ) );
                text = i18n( "Visit contributor's Digg profile" );
            }
            else if( type == "Facebook" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-facebook.png" ) ) );
                text = i18n( "Visit contributor's Facebook profile" );
            }
            else if( type == "Homepage" || type == "other" || ( type.isEmpty() && !url.isEmpty() ) )
            {
                if( fillHomepageFromOcs )
                {
                    KAction *homepage = new KAction( KIcon( "applications-internet" ), i18n("Visit contributor's homepage"), this );
                    homepage->setToolTip( url );
                    homepage->setData( url );
                    m_iconsBar->addAction( homepage );
                    fillHomepageFromOcs = false;
                    continue;
                }
                if( type == "other" && url.contains( "last.fm/" ) )     //HACK: assign a last.fm icon if the URL contains last.fm
                {
                    icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-lastfm.png" ) ) );
                    text = i18n( "Visit contributor's Last.fm profile" );
                }
                else
                    continue;
            }
            else if( type == "LinkedIn" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-linkedin.png" ) ) );
                text = i18n( "Visit contributor's LinkedIn profile" );
            }
            else if( type == "MySpace" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-myspace.png" ) ) );
                text = i18n( "Visit contributor's MySpace homepage" );
            }
            else if( type == "Reddit" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-reddit.png" ) ) );
                text = i18n( "Visit contributor's Reddit profile" );
            }
            else if( type == "YouTube" )
            {
                icon = KIcon( "dragonplayer" ); //FIXME: icon
                text = i18n( "Visit contributor's YouTube profile" );
            }
            else if( type == "Twitter" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-twitter.png" ) ) );
                text = i18n( "Visit contributor's Twitter feed" );
            }
            else if( type == "Wikipedia" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-wikipedia.png" ) ) );
                text = i18n( "Visit contributor's Wikipedia profile" );
            }
            else if( type == "Xing" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-xing.png" ) ) );
                text = i18n( "Visit contributor's Xing profile" );
            }
            else if( type == "identi.ca" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-identica.png" ) ) );
                text = i18n( "Visit contributor's identi.ca feed" );
            }
            else if( type == "libre.fm" )
            {
                icon = KIcon( "juk" );  //FIXME: icon
                text = i18n( "Visit contributor's libre.fm profile" );
            }
            else if( type == "StackOverflow" )
            {
                icon = KIcon( QPixmap( KStandardDirs::locate( "data", "amarok/images/emblem-stackoverflow.png" ) ) );
                text = i18n( "Visit contributor's StackOverflow profile" );
            }
            else
                break;
            KAction *action = new KAction( icon, text, this );
            action->setToolTip( url );
            action->setData( url );
            m_snBar->addAction( action );
        }

        debug() << "END USER HOMEPAGE DATA";
    }

    m_textLabel->setText( m_aboutText );
}

