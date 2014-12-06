/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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
 
#include "BrowserBreadcrumbItem.h"

#include "browsers/BrowserCategoryList.h"
#include "browsers/filebrowser/FileBrowser.h"
#include "core/support/Debug.h"
#include "widgets/BreadcrumbItemButton.h"

#include <KIcon>
#include <KLocale>

#include <QDir>
#include <QMenu>

BrowserBreadcrumbItem::BrowserBreadcrumbItem( BrowserCategory *category, QWidget *parent )
    : KHBox( parent )
    , m_menuButton( 0 )
{
    //figure out if we want to add a menu to this item. A menu allows you to select
    //any of the _sibling_ items. (yes, I know, this is different from how Dolphin
    //does it, but I find the Dolphin way amazingly unintuitive and I always get it
    //wrong when using it...)

    BrowserCategoryList * parentList = category->parentList();
    if( parentList )
    {
        m_menuButton = new BreadcrumbItemMenuButton( this );
        QMenu *menu = new QMenu( this ); //see QMenu docs: it's still a top-level widget.
                                         //parent is only for memory management.
        QMap<QString,BrowserCategory *> siblingMap = parentList->categories();

        QStringList siblingNames = siblingMap.keys();

        foreach( const QString &siblingName, siblingNames )
        {
            //no point in adding ourselves to this menu
            if ( siblingName == category->name() )
                continue;

            BrowserCategory *siblingCategory = siblingMap.value( siblingName );

            QAction *action = menu->addAction( siblingCategory->icon(), siblingCategory->prettyName() );
            connect( action, SIGNAL(triggered()), siblingMap.value( siblingName ), SLOT(activate()) );
        }

        m_menuButton->setMenu( menu );
    }

    m_mainButton = new BreadcrumbItemButton( category->icon(), category->prettyName(), this );

    if( category->prettyName().isEmpty() )
    {
        // root item
        m_mainButton->setToolTip( i18n( "Media Sources Home" ) );
        m_mainButton->setIcon( KIcon( "user-home" ) );
    }

    connect( m_mainButton, SIGNAL(sizePolicyChanged()), this, SLOT(updateSizePolicy()) );

    //if this is a list, make cliking on this item cause us
    //to navigate to its home.
    BrowserCategoryList *list = qobject_cast<BrowserCategoryList*>( category );
    if ( list )
    {
        connect( m_mainButton, SIGNAL(clicked(bool)), list, SLOT(home()) );
    }
    else  
    {
        connect( m_mainButton, SIGNAL(clicked(bool)), category, SLOT(reActivate()) );
    }

    adjustSize();
    m_nominalWidth = width();

    hide();

    updateSizePolicy();
}

BrowserBreadcrumbItem::BrowserBreadcrumbItem( const QString &name, const QString &callback,
        const BreadcrumbSiblingList &childItems, FileBrowser *handler, QWidget *parent )
    : KHBox( parent )
    , m_menuButton( 0 )
    , m_callback( callback )
{
    if ( !childItems.isEmpty() )
    {
        m_menuButton = new BreadcrumbItemMenuButton( this );
        QMenu *menu = new QMenu( this );

        int i = 0;
        foreach( const BreadcrumbSibling &sibling, childItems )
        {
            QString visibleName = sibling.name;
            visibleName.replace( '&', "&&" ); // prevent bug 244817
            QAction *action = menu->addAction( sibling.icon, visibleName );
            action->setProperty( "callback", sibling.callback );

            // the current action should be bolded
            if( sibling.name == name )
            {
                QFont font = action->font();
                font.setBold( true );
                action->setFont( font );
            }
            connect( action, SIGNAL(triggered()), this, SLOT(activateSibling()) );
            i++;
        }
        m_menuButton->setMenu( menu );
    }

    m_mainButton = new BreadcrumbItemButton( name, this );
    connect( m_mainButton, SIGNAL(sizePolicyChanged()), this, SLOT(updateSizePolicy()) );
    connect( m_mainButton, SIGNAL(clicked(bool)), this, SLOT(activate()) );
    connect( this, SIGNAL(activated(QString)), handler, SLOT(addItemActivated(QString)) );

    adjustSize();
    m_nominalWidth = width();

    hide();
    updateSizePolicy(); 
}

BrowserBreadcrumbItem::~BrowserBreadcrumbItem()
{
}

void
BrowserBreadcrumbItem::setActive( bool active )
{
    m_mainButton->setActive( active );
}

QSizePolicy BrowserBreadcrumbItem::sizePolicy() const
{
    return m_mainButton->sizePolicy();
}

void BrowserBreadcrumbItem::updateSizePolicy()
{
    setSizePolicy( m_mainButton->sizePolicy() );
}

void BrowserBreadcrumbItem::activate()
{
    emit activated( m_callback );
}

void BrowserBreadcrumbItem::activateSibling()
{
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action )
        emit activated( action->property( "callback" ).toString() );
}

int BrowserBreadcrumbItem::nominalWidth() const
{
    return m_nominalWidth;
}
