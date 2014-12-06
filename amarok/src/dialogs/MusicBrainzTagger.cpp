/****************************************************************************************
 * Copyright (c) 2010 Sergey Ivanov <123kash@gmail.com>                                 *
 * Copyright (c) 2013 Alberto Villa <avilla@FreeBSD.org>                                *
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

#define DEBUG_PREFIX "MusicBrainzTagDialog"

#include "MusicBrainzTagger.h"

#include "core/meta/Meta.h"
#include "core/meta/support/MetaConstants.h"
#include "core/meta/support/MetaUtility.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "musicbrainz/MusicBrainzFinder.h"
#include "musicbrainz/MusicBrainzTagsModel.h"
#include "musicbrainz/MusicBrainzTagsModelDelegate.h"
#ifdef HAVE_LIBOFA
#include "musicbrainz/MusicDNSFinder.h"
#endif
#include "ui_MusicBrainzTagger.h"

#include <KIcon>

#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QToolButton>

MusicBrainzTagger::MusicBrainzTagger( const Meta::TrackList &tracks,
                                      QWidget *parent )
    : KDialog( parent )
    , ui( new Ui::MusicBrainzTagger() )
{
    DEBUG_BLOCK
    foreach( Meta::TrackPtr track, tracks )
    {
        if( !track->playableUrl().toLocalFile().isEmpty() )
            m_tracks << track;
    }
    ui->setupUi( mainWidget() );
    restoreDialogSize( Amarok::config( "MusicBrainzTagDialog" ) );

    init();
    search();
}

MusicBrainzTagger::~MusicBrainzTagger()
{
    KConfigGroup group = Amarok::config( "MusicBrainzTagDialog" );
    saveDialogSize( group );
    delete ui;
}

void
MusicBrainzTagger::init()
{
    DEBUG_BLOCK
    setButtons( KDialog::None );
    setAttribute( Qt::WA_DeleteOnClose );
    setMinimumSize( 550, 300 );

    m_resultsModel = new MusicBrainzTagsModel( this );
    m_resultsModelDelegate = new MusicBrainzTagsModelDelegate( this );
    m_resultsProxyModel = new QSortFilterProxyModel( this );

    m_resultsProxyModel->setSourceModel( m_resultsModel );
    m_resultsProxyModel->setSortRole( MusicBrainzTagsModel::SortRole );
    m_resultsProxyModel->setDynamicSortFilter( true );

    ui->resultsView->setModel( m_resultsProxyModel );
    ui->resultsView->setItemDelegate( m_resultsModelDelegate );
    // The column is not important, they all have the same data.
    ui->resultsView->sortByColumn( 0, Qt::AscendingOrder );

    if( m_tracks.count() > 1 )
    {
        QToolBar *toolBar = new QToolBar( this );
        toolBar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

        QAction *lastAction = toolBar->addAction( KIcon( "tools-wizard" ), i18n( "Choose Best Matches" ), m_resultsModel, SLOT(chooseBestMatches()) );
        lastAction->setToolTip( i18n( "Use the top result for each undecided track. Alternatively, you can click on <b>Choose Best Matches from This Album</b> in the context menu of a good suggestion; it may give even better results because it prevents mixing different album releases together." ) );
        lastAction = toolBar->addAction( KIcon( "edit-clear" ), i18n( "Clear Choices" ), m_resultsModel, SLOT(clearChoices()) );
        lastAction->setToolTip( i18n( "Clear all choices, even manually made ones." ) );

        toolBar->addSeparator();

        QToolButton *lastButton = new QToolButton( toolBar );
        lastAction = new QAction( i18n( "Collapse Chosen" ), lastButton );
        connect( lastAction, SIGNAL(triggered()),
                 ui->resultsView, SLOT(collapseChosen()) );
        lastButton->setDefaultAction( lastAction );
        lastAction = new QAction( i18n( "Collapse All" ), lastButton );
        connect( lastAction, SIGNAL(triggered()),
                 ui->resultsView, SLOT(collapseAll()) );
        lastButton->addAction( lastAction );
        toolBar->addWidget( lastButton );

        lastButton = new QToolButton( toolBar );
        lastAction = new QAction( i18n( "Expand Unchosen" ), lastButton );
        connect( lastAction, SIGNAL(triggered()),
                 ui->resultsView, SLOT(expandUnchosen()) );
        lastButton->setDefaultAction( lastAction );
        lastAction = new QAction( i18n( "Expand All" ), lastButton );
        connect( lastAction, SIGNAL(triggered()),
                 ui->resultsView, SLOT(expandAll()) );
        lastButton->addAction( lastAction );
        toolBar->addWidget( lastButton );

        ui->verticalLayout->insertWidget( 0, toolBar );
    }

    ui->progressBar->hide();

    mb_finder = new MusicBrainzFinder( this );
#ifdef HAVE_LIBOFA
    mdns_finder = new MusicDNSFinder( this );
    connect( mdns_finder, SIGNAL(trackFound(Meta::TrackPtr,QString)),
             mb_finder, SLOT(lookUpByPUID(Meta::TrackPtr,QString)) );
    connect( mdns_finder, SIGNAL(progressStep()), SLOT(progressStep()) );
    connect( mdns_finder, SIGNAL(done()), this, SLOT(mdnsSearchDone()) );
#endif
    connect( mb_finder, SIGNAL(done()), SLOT(searchDone()) );
    connect( mb_finder, SIGNAL(trackFound(Meta::TrackPtr,QVariantMap)),
             m_resultsModel, SLOT(addTrack(Meta::TrackPtr,QVariantMap)) );
    connect( mb_finder, SIGNAL(progressStep()), SLOT(progressStep()) );
    connect( ui->pushButton_saveAndClose, SIGNAL(clicked(bool)), SLOT(saveAndExit()) );
    connect( ui->pushButton_cancel, SIGNAL(clicked(bool)), SLOT(reject()) );
}

void
MusicBrainzTagger::search()
{
    int barSize = m_tracks.count();
    mb_finder->run( m_tracks );
#ifdef HAVE_LIBOFA
    barSize *= 2;
    mdns_searchDone = false;
    mdns_finder->run( m_tracks );
#endif
    ui->progressBar->setRange( 0, barSize );
    ui->progressBar->setValue( 0 );
    ui->horizontalSpacer->changeSize( 0, 0, QSizePolicy::Ignored );
    ui->progressBar->show();
}

void
MusicBrainzTagger::saveAndExit()
{
    QMap<Meta::TrackPtr, QVariantMap> result = m_resultsModel->chosenItems();

    if( !result.isEmpty() )
        emit sendResult( result );

    accept();
}

void
MusicBrainzTagger::searchDone()
{
    DEBUG_BLOCK
#ifdef HAVE_LIBOFA
    if( !mdns_searchDone )
        return;
#endif
    ui->horizontalSpacer->changeSize( 0, 0, QSizePolicy::Expanding );
    ui->progressBar->hide();
    ui->resultsView->expandAll();
    ui->resultsView->header()->resizeSections( QHeaderView::ResizeToContents );
}

#ifdef HAVE_LIBOFA
void
MusicBrainzTagger::mdnsSearchDone()
{
    DEBUG_BLOCK
    mdns_searchDone = true;
    if( !mb_finder->isRunning() )
        searchDone();
}
#endif

void
MusicBrainzTagger::progressStep()
{
    ui->progressBar->setValue( ui->progressBar->value() + 1 );
}

#include "MusicBrainzTagger.moc"
