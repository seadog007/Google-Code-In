/****************************************************************************************
 * Copyright (c) 2008 Daniel Jones <danielcjones@gmail.com>                             *
 * Copyright (c) 2009-2010 Leo Franchi <lfranchi@kde.org>                               *
 * Copyright (c) 2009 Mark Kretschmann <kretschmann@kde.org>                            *
 * Copyright (c) 2010-2011 Ralf Engels <ralf-engels@gmx.de>                             *
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

#include "DynamicCategory.h"
#include "DynamicView.h"

#include "amarokconfig.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "playlist/PlaylistActions.h"
#include "playlist/PlaylistModelStack.h"
#include "dynamic/DynamicModel.h"
#include "dynamic/BiasedPlaylist.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>

#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSpinBox>

#include <KHBox>
#include <KIcon>
#include <KStandardDirs>
#include <KSeparator>
#include <KToolBar>


PlaylistBrowserNS::DynamicCategory::DynamicCategory( QWidget* parent )
    : BrowserCategory( "dynamic category", parent )
{
    setPrettyName( i18n( "Dynamic Playlists" ) );
    setShortDescription( i18n( "Dynamically updating parameter based playlists" ) );
    setIcon( KIcon( "dynamic-amarok" ) );

    setLongDescription( i18n( "With a dynamic playlist, Amarok becomes your own personal dj, automatically selecting tracks for you, based on a number of parameters that you select." ) );

    setImagePath( KStandardDirs::locate( "data", "amarok/images/hover_info_dynamic_playlists.png" ) );

    // set background
    if( AmarokConfig::showBrowserBackgroundImage() )
        setBackgroundImage( imagePath() );

    bool enabled = AmarokConfig::dynamicMode();

    setContentsMargins( 0, 0, 0, 0 );

    KHBox* controls2Layout = new KHBox( this );

    QLabel *label;
    label = new QLabel( i18n( "Previous:" ), controls2Layout );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    m_previous = new QSpinBox( controls2Layout );
    m_previous->setMinimum( 0 );
    m_previous->setToolTip( i18n( "Number of previous tracks to remain in the playlist." ) );
    m_previous->setValue( AmarokConfig::previousTracks() );
    QObject::connect( m_previous, SIGNAL(valueChanged(int)), this, SLOT(setPreviousTracks(int)) );

    label = new QLabel( i18n( "Upcoming:" ), controls2Layout );
    // label->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    m_upcoming = new QSpinBox( controls2Layout );
    m_upcoming->setMinimum( 1 );
    m_upcoming->setToolTip( i18n( "Number of upcoming tracks to add to the playlist." ) );
    m_upcoming->setValue( AmarokConfig::upcomingTracks() );
    QObject::connect( m_upcoming, SIGNAL(valueChanged(int)), this, SLOT(setUpcomingTracks(int)) );


    QObject::connect( (const QObject*)Amarok::actionCollection()->action( "playlist_clear" ),  SIGNAL(triggered(bool)),  this, SLOT(playlistCleared()) );
    QObject::connect( (const QObject*)Amarok::actionCollection()->action( "disable_dynamic" ),  SIGNAL(triggered(bool)),  this, SLOT(playlistCleared()), Qt::DirectConnection );


    // -- the tool bar

    KHBox* presetLayout = new KHBox( this );
    KToolBar* presetToolbar = new KToolBar( presetLayout );
    presetToolbar->setIconSize( QSize( 22, 22 ) );

    presetToolbar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    presetToolbar->setMovable( false );
    presetToolbar->setFloatable( false );
    presetToolbar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );

    m_onOffButton = new QToolButton( presetToolbar );
    m_onOffButton->setText( i18nc( "Turn dynamic mode on", "On") );
    m_onOffButton->setCheckable( true );
    m_onOffButton->setIcon( KIcon( "dynamic-amarok" ) );
    m_onOffButton->setToolTip( i18n( "Turn dynamic mode on." ) );
    presetToolbar->addWidget( m_onOffButton );

    m_duplicateButton = new QToolButton( presetToolbar );
    m_duplicateButton->setText( i18n("Duplicates") );
    m_duplicateButton->setCheckable( true );
    m_duplicateButton->setChecked( allowDuplicates() );
    m_duplicateButton->setIcon( KIcon( "edit-copy" ) );
    m_duplicateButton->setToolTip( i18n( "Allow duplicate songs in result" ) );
    presetToolbar->addWidget( m_duplicateButton );

    m_addButton = new QToolButton( presetToolbar );
    m_addButton->setText( i18n("New") );
    m_addButton->setIcon( KIcon( "document-new" ) );
    m_addButton->setToolTip( i18n( "New playlist" ) );
    presetToolbar->addWidget( m_addButton );

    m_editButton = new QToolButton( presetToolbar );
    m_editButton->setText( i18n("Edit") );
    m_editButton->setIcon( KIcon( "document-properties-amarok" ) );
    m_editButton->setToolTip( i18n( "Edit the selected playlist or bias" ) );
    presetToolbar->addWidget( m_editButton );

    m_deleteButton = new QToolButton( presetToolbar );
    m_deleteButton->setText( i18n("Delete") );
    m_deleteButton->setEnabled( false );
    m_deleteButton->setIcon( KIcon( "edit-delete" ) );
    m_deleteButton->setToolTip( i18n( "Delete the selected playlist or bias") );
    presetToolbar->addWidget( m_deleteButton );

    m_repopulateButton = new QPushButton( presetLayout );
    m_repopulateButton->setText( i18n("Repopulate") );
    m_repopulateButton->setToolTip( i18n("Replace the upcoming tracks with fresh ones.") );
    m_repopulateButton->setIcon( KIcon( "view-refresh-amarok" ) );
    m_repopulateButton->setEnabled( enabled );
    // m_repopulateButton->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    QObject::connect( m_repopulateButton, SIGNAL(clicked(bool)), The::playlistActions(), SLOT(repopulateDynamicPlaylist()) );


    // -- the tree view

    m_tree = new DynamicView( this );
    connect( m_tree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(selectionChanged()) );

    connect( m_onOffButton, SIGNAL(toggled(bool)), The::playlistActions(), SLOT(enableDynamicMode(bool)) );
    connect( m_duplicateButton, SIGNAL(toggled(bool)), this, SLOT(setAllowDuplicates(bool)) );

    connect( m_addButton, SIGNAL(clicked(bool)), m_tree, SLOT(addPlaylist()) );
    connect( m_editButton, SIGNAL(clicked(bool)), m_tree, SLOT(editSelected()) );
    connect( m_deleteButton, SIGNAL(clicked(bool)), m_tree, SLOT(removeSelected()) );

    navigatorChanged();
    selectionChanged();

    connect( The::playlistActions(), SIGNAL(navigatorChanged()),
             this, SLOT(navigatorChanged()) );
}


PlaylistBrowserNS::DynamicCategory::~DynamicCategory()
{ }

void
PlaylistBrowserNS::DynamicCategory::navigatorChanged()
{
    m_onOffButton->setChecked( AmarokConfig::dynamicMode() );
    m_repopulateButton->setEnabled( AmarokConfig::dynamicMode() );
}

void
PlaylistBrowserNS::DynamicCategory::selectionChanged()
{
    DEBUG_BLOCK;

    QModelIndexList indexes = m_tree->selectionModel()->selectedIndexes();

    if( indexes.isEmpty() )
    {
        m_addButton->setEnabled( true );
        m_editButton->setEnabled( false );
        m_deleteButton->setEnabled( false );
        return;
    }

    QVariant v = m_tree->model()->data( indexes.first(), Dynamic::DynamicModel::PlaylistRole );
    if( v.isValid() )
    {
        m_addButton->setEnabled( true );
        m_editButton->setEnabled( true );
        m_deleteButton->setEnabled( true );
        return;
    }

    v = m_tree->model()->data( indexes.first(), Dynamic::DynamicModel::BiasRole );
    if( v.isValid() )
    {
        m_addButton->setEnabled( true );
        m_editButton->setEnabled( true );
        m_deleteButton->setEnabled( false ); // TODO
        return;
    }
}

bool
PlaylistBrowserNS::DynamicCategory::allowDuplicates() const
{
    return AmarokConfig::dynamicDuplicates();
}


void
PlaylistBrowserNS::DynamicCategory::playlistCleared() // SLOT
{
    The::playlistActions()->enableDynamicMode( false );
}

void
PlaylistBrowserNS::DynamicCategory::setUpcomingTracks( int n ) // SLOT
{
    if( n >= 1 )
        AmarokConfig::setUpcomingTracks( n );
}

void
PlaylistBrowserNS::DynamicCategory::setPreviousTracks( int n ) // SLOT
{
    if( n >= 0 )
        AmarokConfig::setPreviousTracks( n );
}

void
PlaylistBrowserNS::DynamicCategory::setAllowDuplicates( bool value ) // SLOT
{
    if( AmarokConfig::dynamicDuplicates() == value )
        return;

    AmarokConfig::setDynamicDuplicates( value );
    AmarokConfig::self()->writeConfig();

    m_duplicateButton->setChecked( value );
}


#include "DynamicCategory.moc"

