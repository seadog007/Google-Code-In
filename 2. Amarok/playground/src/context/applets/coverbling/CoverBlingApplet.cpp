/****************************************************************************************
 * Copyright (c) 2010 Emmanuel Wagner <manu.wagner@sfr.fr>                              *
 * Copyright (c) 2010 Mark Kretschmann <kretschmann@kde.org>                            *
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

#define DEBUG_PREFIX "CoverBlingApplet"

#include "CoverBlingApplet.h"

#include "EngineController.h"
#include "context/ContextView.h"
#include "context/widgets/RatingWidget.h"
#include "context/widgets/TextScrollingWidget.h"
#include "core/collections/Collection.h"
#include "core/meta/Meta.h"
#include "core/meta/Statistics.h"
#include "core/support/Amarok.h"
#include "core/support/Debug.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "playlist/PlaylistModelStack.h"
#include "SearchBarTextItem.h"
#include "playlist/PlaylistController.h"

#include <KAction>
#include <KColorScheme>
#include <KConfigDialog>
#include <KGlobalSettings>
#include <Plasma/BusyWidget>
#include <Plasma/IconWidget>
#include <Plasma/Theme>

#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsWidget>
#include <QLabel>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <KStandardDirs>
#include <QDesktopWidget>

CoverBlingApplet::CoverBlingApplet( QObject* parent, const QVariantList& args )
    : Context::Applet( parent, args )
{
    DEBUG_BLOCK

    setHasConfigurationInterface( true );
}

void
CoverBlingApplet::init()
{
	// Call the base implementation.
    Context::Applet::init();
    setBackgroundHints( Plasma::Applet::NoBackground );

    m_fullsize = false;

	KConfigGroup config = Amarok::config( "CoverBling Applet" );
    m_coversize = config.readEntry( "CoverSize", 200 );
    int reflectioneffect = config.readEntry( "ReflectionEffect", 1 );
    if ( reflectioneffect == 0 )
        m_reflectionEffect = PictureFlow::NoReflection;
    else if ( reflectioneffect == 1 )
        m_reflectionEffect = PictureFlow::PlainReflection;
    else if ( reflectioneffect == 2 )
        m_reflectionEffect = PictureFlow::BlurredReflection;
    m_autojump = config.readEntry( "AutoJump", false );
    m_animatejump = config.readEntry( "AnimateJump", true );
    m_layout = new QGraphicsProxyWidget( this );
	m_openGL = false;
	//bool setting_opengl = config.readEntry( "OpenGL", false );
	//if (QGLFormat::hasOpenGL() && setting_opengl) m_openGL = true;
	
 	m_pictureflow = new PhotoBrowser(0,m_openGL);
    m_layout->setWidget( m_pictureflow );
	
    m_pictureflow->setRenderHints( QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform );

    m_pictureflow->show();

    Collections::Collection *coll = CollectionManager::instance()->primaryCollection();
    Collections::QueryMaker *qm = coll->queryMaker();
    qm->setAutoDelete( true );
    qm->setQueryType( Collections::QueryMaker::Album );
    qm->orderBy( Meta::valArtist );

    connect( qm, SIGNAL(newResultReady(Meta::AlbumList)),
             this, SLOT(slotAlbumQueryResult(Meta::AlbumList)) );
	qm->run();

    m_label = new QGraphicsSimpleTextItem( this );
    m_label->setBrush( QBrush( Qt::white ) );
    QFont labelFont;
    QFont bigFont( labelFont );
    bigFont.setPointSize( bigFont.pointSize() + 4 );
    m_label->setFont( labelFont );

    m_ratingWidget = new RatingWidget( this );
    m_ratingWidget->setRating( 0 );
    m_ratingWidget->setEnabled( false );

    // Construct icon widgets
    m_blingtofirst = new Plasma::IconWidget( this );
    m_blingtofirst->setIcon( KStandardDirs::locate( "data", "amarok/images/blingtofirst.png" ) );
    m_blingtofirst->setMaximumSize( 16.0, 16.0 );
    m_blingtofirst->setToolTip( i18n( "Jump to First" ) );

    m_blingtolast = new Plasma::IconWidget( this );
    m_blingtolast->setIcon( KStandardDirs::locate( "data", "amarok/images/blingtolast.png" ) );
    m_blingtolast->setMaximumSize( 16.0, 16.0 );
    m_blingtolast->setToolTip( i18n( "Jump to Last" ) );

    m_blingfastback = new Plasma::IconWidget( this );
    m_blingfastback->setIcon( KStandardDirs::locate( "data", "amarok/images/blingfastback.png" ) );
    m_blingfastback->setMaximumSize( 16.0, 16.0 );
    m_blingfastback->setToolTip( i18n( "Fast Backward" ) );

    m_blingfastforward = new Plasma::IconWidget( this );
    m_blingfastforward->setIcon( KStandardDirs::locate( "data", "amarok/images/blingfastforward.png" ) );
    m_blingfastforward->setMaximumSize( 16.0, 16.0 );
    m_blingfastforward->setToolTip( i18n( "Fast Forward" ) );

    m_fullscreen = new Plasma::IconWidget( this );
    m_fullscreen->setIcon( KStandardDirs::locate( "data", "amarok/images/blingfullscreen.png" ) );
    m_fullscreen->setMaximumSize( 16.0, 16.0 );
    m_fullscreen->setToolTip( i18n( "Maximize/Minimize" ) );

    m_jumptoplaying = new Plasma::IconWidget( this );
    m_jumptoplaying->setIcon( KStandardDirs::locate( "data", "amarok/images/blingjumptoplaying.png" ) );
    m_jumptoplaying->setMaximumSize( 16.0, 16.0 );
    m_jumptoplaying->setToolTip( i18n( "Jump to Current" ) );

    m_albumsearch = new Plasma::IconWidget( this );
    m_albumsearch->setIcon( KStandardDirs::locate( "data", "amarok/images/blingsearchalbum.png" ) );
    m_albumsearch->setMaximumSize( 18.0, 18.0 );
    m_albumsearch->setToolTip( i18n( "Toggle Album/Artist search" ) );
    m_editsearch = new SearchBarTextItem( this );
    m_editsearch->setTextInteractionFlags( Qt::TextEditorInteraction );
    labelFont.setItalic( true );
    m_editsearch->setFont( labelFont );
    m_editsearch->setDefaultTextColor( Qt::white );
    m_album_or_artist = true;
    displaySearchName();

    m_initrandompos = config.readEntry( "RandomPos", false );

    if ( m_autojump )
    {
      EngineController *engine = The::engineController();
      connect( engine, SIGNAL(trackPlaying(Meta::TrackPtr)),
	      this, SLOT(jumpToPlaying()) );
    }
}

CoverBlingApplet::~CoverBlingApplet()
{
    delete m_ratingWidget;
    delete m_label;
    delete m_layout;
    delete m_blingfastback;
    delete m_blingfastforward;
    delete m_blingtofirst;
    delete m_blingtolast;
    delete m_jumptoplaying;
    delete m_albumsearch;
    delete m_editsearch;
}

void CoverBlingApplet::slotAlbumQueryResult( Meta::AlbumList albums ) //SLOT
{
    DEBUG_BLOCK
    m_pictureflow->fillAlbums( albums );

    connect( m_pictureflow, SIGNAL(centerIndexChanged(int)), this, SLOT(slideChanged(int)) );
    connect( m_pictureflow, SIGNAL(doubleClicked(int)), this, SLOT(slotDoubleClicked(int)) );
    connect( m_blingtofirst, SIGNAL(clicked()), this, SLOT(skipToFirst()) );
    connect( m_blingtolast, SIGNAL(clicked()), this, SLOT(skipToLast()) );
    connect( m_blingfastback, SIGNAL(clicked()), m_pictureflow, SLOT(fastBackward()) );
    connect( m_blingfastforward, SIGNAL(clicked()), m_pictureflow, SLOT(fastForward()) );
    connect( m_fullscreen, SIGNAL(clicked()), this, SLOT(toggleFullscreen()) );
    connect( m_jumptoplaying, SIGNAL(clicked()), this, SLOT(jumpToPlaying()) );
    connect( m_albumsearch, SIGNAL(clicked()), this, SLOT(switchSearchIcon()) );
    connect( m_editsearch, SIGNAL(editionValidated(QString)), this, SLOT(albumSearch(QString)) );
    if (m_initrandompos)
    {
		int nbAlbums = m_pictureflow->slideCount() -1;
		int initial_pos = rand() % nbAlbums;
        if ( m_animatejump )
        {
	    m_pictureflow->skipToSlide( initial_pos - 10 );
            m_pictureflow->showSlide( initial_pos );
        }
        else
           m_pictureflow->skipToSlide( initial_pos );
        slideChanged( initial_pos );
	}
	constraintsEvent();
}

void CoverBlingApplet::slideChanged( int islideindex )
{
    Meta::AlbumPtr album = m_pictureflow->album( islideindex );
    if ( album )
    {
        Meta::ArtistPtr artist = album->albumArtist();
        QString label = album->prettyName();
        if ( artist ) label += " - " + artist->prettyName();
        m_label->setText( label );

        //center the label
        m_label->setPos( ( size().width() - m_label->boundingRect().width() ) / 2, m_label->y() );
 
        m_label->show();
        int nbtracks = 0;
        int rating = 0;

        foreach( Meta::TrackPtr track, album->tracks() )
        {
            nbtracks++;
            if ( track )
                rating += track->statistics()->rating();
        }

        if ( nbtracks )
            rating = rating / nbtracks;

        m_ratingWidget->setRating( rating );
    }
}

void CoverBlingApplet::slotDoubleClicked( int islideindex )
{
    Meta::AlbumPtr album = m_pictureflow->album( islideindex );
    if ( album )
        The::playlistController()->insertOptioned( album->tracks(), Playlist::OnDoubleClickOnSelectedItems );
}

void CoverBlingApplet::constraintsEvent( Plasma::Constraints constraints )
{
    Q_UNUSED( constraints )

    prepareGeometryChange();
    const int vertical_size = boundingRect().height();
    const int horizontal_size = boundingRect().width();
    QSize slideSize( vertical_size / 2, vertical_size / 2 );

    if ( !m_fullsize )
    {
        slideSize.setWidth( m_coversize );
        slideSize.setHeight( m_coversize );
    }
	setMinimumHeight( 2 * m_coversize ); 
    m_pictureflow->setSlideSize( slideSize );
    m_pictureflow->setReflectionEffect( m_reflectionEffect );
    m_pictureflow->setAnimationTime( 10 );
    m_ratingWidget->setSpacing( 2 );
    m_ratingWidget->setPos( horizontal_size / 2 - 40, vertical_size - 30 );
    m_label ->setPos( horizontal_size / 2 - 40, vertical_size - 50 );

    m_blingtofirst->setPos( 20, vertical_size - 30 );
    m_blingtolast->setPos( horizontal_size - 30, vertical_size - 30 );
    m_blingfastback->setPos( 50, vertical_size - 30 );
    m_blingfastforward->setPos( horizontal_size - 60, vertical_size - 30 );
    m_fullscreen->setPos( horizontal_size - 30, 30 );
    m_jumptoplaying->setPos( horizontal_size - 60, 30 );
    m_albumsearch->setPos( 20, 30 );
    m_editsearch->setPos( 38, 28 );
    m_pictureflow->resize( horizontal_size, vertical_size );

    m_label->setPos( ( size().width() - m_label->boundingRect().width() ) / 2, m_label->y() );
}

void CoverBlingApplet::toggleFullscreen()
{
    if ( m_fullsize )
    {
        resize( -1, -1 );
    }
    else
    {
        //QDesktopWidget* desktop = QApplication::desktop();
        //if (desktop)
        {
            //    QRect rect = desktop->screenGeometry();
            //    resize(rect.width(),rect.height());
            resize( -1, 2 * m_coversize );
        }
    }
    updateConstraints();
    //constraintsEvent();
    m_fullsize = !m_fullsize;
}

void CoverBlingApplet::createConfigurationInterface( KConfigDialog *parent )
{
    KConfigGroup configuration = config();
    QWidget * const settings = new QWidget;
    ui_Settings.setupUi( settings );

    if ( m_reflectionEffect == PictureFlow::NoReflection )
        ui_Settings.reflectionEffectCombo->setCurrentIndex( 0 );
    else if ( m_reflectionEffect == PictureFlow::PlainReflection )
        ui_Settings.reflectionEffectCombo->setCurrentIndex( 1 );
    else if ( m_reflectionEffect == PictureFlow::BlurredReflection )
        ui_Settings.reflectionEffectCombo->setCurrentIndex( 2 );
    if ( m_coversize )
        ui_Settings.coversizeSpin->setValue( m_coversize );
    ui_Settings.autoJumpChk->setChecked( m_autojump );
    ui_Settings.animJumpChk->setChecked( m_animatejump );
    ui_Settings.randomPosChk->setChecked( m_initrandompos );
	//if (m_openGL) ui_Settings.renderingCombo->setCurrentIndex(1);
	//else ui_Settings.renderingCombo->setCurrentIndex(0);
    parent->addPage( settings, i18n( "Coverbling Settings" ), "preferences-system" );
    connect( parent, SIGNAL(accepted()), this, SLOT(saveSettings()) );
}

void CoverBlingApplet::saveSettings()
{
    m_coversize = ui_Settings.coversizeSpin->value();
    if ( ui_Settings.reflectionEffectCombo->currentIndex() == 0 )
        m_reflectionEffect = PictureFlow::NoReflection;
    else if ( ui_Settings.reflectionEffectCombo->currentIndex() == 1 )
        m_reflectionEffect = PictureFlow::PlainReflection;
    else if ( ui_Settings.reflectionEffectCombo->currentIndex() == 2 )
        m_reflectionEffect = PictureFlow::BlurredReflection;
	//if (ui_Settings.renderingCombo->currentIndex()==1)
		 //m_openGL = true;
	//else
		 //m_openGL = false;
    m_autojump = ui_Settings.autoJumpChk->isChecked();
    m_animatejump = ui_Settings.animJumpChk->isChecked();
    m_initrandompos = ui_Settings.randomPosChk->isChecked();
    KConfigGroup config = Amarok::config( "CoverBling Applet" );
    config.writeEntry( "CoverSize", m_coversize );
    config.writeEntry( "ReflectionEffect", ( int ) m_reflectionEffect );
    config.writeEntry( "AutoJump", m_autojump );
    config.writeEntry( "AnimateJump", m_animatejump );
    config.writeEntry( "RandomPos", m_initrandompos );
	//config.writeEntry( "OpenGL", (int) m_openGL );

    constraintsEvent();
}

void CoverBlingApplet::jumpToPlaying()
{
    Meta::TrackPtr track = The::engineController()->currentTrack();

    if ( !track )
        return;
    Meta::AlbumPtr album = track->album();
    if ( !album )
        return;
    int center = m_pictureflow->centerIndex();
    if ( m_pictureflow->album( center ) == album )
        return;
    int nbslides = m_pictureflow->slideCount();
    bool found = false;
    int index = 0;
    if ( nbslides > 0 )
    {
        for ( int i = 0; i < nbslides;i++ )
        {
            Meta::AlbumPtr current_album = m_pictureflow->album( i );
            if ( current_album == album )
            {
                index = i;
                found = true;
                break;
            }
        }
        if ( found )
        {
            if ( m_animatejump )
            {
                if ( center - index > 10 || index - center > 10 )
                {
                    if ( index > center )
                        m_pictureflow->skipToSlide( index - 10 );
                    else
                        m_pictureflow->skipToSlide( index + 10 );
                }
                m_pictureflow->showSlide( index );
            }
            else
                m_pictureflow->skipToSlide( index );
            slideChanged( index );
        }
    }
}
void CoverBlingApplet::skipToFirst()
{
    m_pictureflow->skipToSlide( 0 );
    slideChanged( 0 );
}

void CoverBlingApplet::skipToLast()
{
    int nbslides = m_pictureflow->slideCount();
    m_pictureflow->skipToSlide( nbslides - 1 );
    slideChanged( nbslides - 1 );
}
void CoverBlingApplet::albumSearch( QString ialbumName )
{
    QString album_name = ialbumName.remove( QChar( ' ' ) );
    int center = m_pictureflow->centerIndex();
    int nbslides = m_pictureflow->slideCount();
    bool found = false;
    int index = 0;
    if ( nbslides > 0 )
    {
        for ( int i = 0; i < nbslides;i++ )
        {
            Meta::AlbumPtr current_album = m_pictureflow->album( i );
            if ( !current_album ) continue;
            QString current_name = "";
            if ( m_album_or_artist )
            {
                current_name = current_album->prettyName();
            }
            else
            {
                Meta::ArtistPtr artist = current_album->albumArtist();
                if ( artist ) current_name = artist->prettyName();
            }
            current_name = current_name.remove( QChar( ' ' ) );
            if ( current_name.contains( album_name, Qt::CaseInsensitive ) )
            {
                index = i;
                found = true;
                break;
            }
        }
        if ( found )
        {
            if ( m_animatejump )
            {
                if ( center - index > 10 || index - center > 10 )
                {
                    if ( index > center )
                        m_pictureflow->skipToSlide( index - 10 );
                    else
                        m_pictureflow->skipToSlide( index + 10 );
                }
                m_pictureflow->showSlide( index );
            }
            else
                m_pictureflow->skipToSlide( index );
            slideChanged( index );
        }
    }
	displaySearchName();
}

void CoverBlingApplet::switchSearchIcon()
{
    m_album_or_artist = !m_album_or_artist;
    if ( m_album_or_artist )
        m_albumsearch->setIcon( KStandardDirs::locate( "data", "amarok/images/blingsearchalbum.png" ) );
    else
        m_albumsearch->setIcon( KStandardDirs::locate( "data", "amarok/images/blingsearchartist.png" ) );
    displaySearchName();
}
void CoverBlingApplet::displaySearchName()
{
	QString album_search_str = i18n( "Search for album..." );
	QString artist_search_str = i18n( "Search for artist..." );
	if ( m_album_or_artist)
		m_editsearch->setPlainText( album_search_str );
	else
		m_editsearch->setPlainText( artist_search_str );
}
#include "CoverBlingApplet.moc"

