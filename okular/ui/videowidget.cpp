/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "videowidget.h"

// qt/kde includes
#include <qaction.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qstackedlayout.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>

#include <kicon.h>
#include <klocale.h>

#include <phonon/mediaobject.h>
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>

#include "core/annotations.h"
#include "core/area.h"
#include "core/document.h"
#include "core/movie.h"
#include "snapshottaker.h"

static QAction* createToolBarButtonWithWidgetPopup( QToolBar* toolBar, QWidget *widget, const QIcon &icon )
{
    QToolButton *button = new QToolButton( toolBar );
    QAction *action = toolBar->addWidget( button );
    button->setAutoRaise( true );
    button->setIcon( icon );
    button->setPopupMode( QToolButton::InstantPopup );
    QMenu *menu = new QMenu( button );
    button->setMenu( menu );
    QWidgetAction *widgetAction = new QWidgetAction( menu );
    QWidget *dummy = new QWidget( menu );
    widgetAction->setDefaultWidget( dummy );
    QVBoxLayout *dummyLayout = new QVBoxLayout( dummy );
    dummyLayout->setMargin( 5 );
    dummyLayout->addWidget( widget );
    menu->addAction( widgetAction );
    return action;
}

/* Private storage. */
class VideoWidget::Private
{
public:
    Private( Okular::Movie *m, Okular::Document *doc, VideoWidget *qq )
        : q( qq ), movie( m ), document( doc ), player( 0 ), loaded( false )
    {
    }

    ~Private()
    {
        if ( player )
            player->stop();
    }

    enum PlayPauseMode { PlayMode, PauseMode };

    void load();
    void setupPlayPauseAction( PlayPauseMode mode );
    void setPosterImage( const QImage& );
    void takeSnapshot();
    void videoStopped();
    void stateChanged(Phonon::State, Phonon::State);

    // slots
    void finished();
    void playOrPause();

    VideoWidget *q;
    Okular::Movie *movie;
    Okular::Document *document;
    Okular::NormalizedRect geom;
    Phonon::VideoPlayer *player;
    Phonon::SeekSlider *seekSlider;
    QToolBar *controlBar;
    QAction *playPauseAction;
    QAction *stopAction;
    QAction *seekSliderAction;
    QAction *seekSliderMenuAction;
    QStackedLayout *pageLayout;
    QLabel *posterImagePage;
    bool loaded : 1;
};

void VideoWidget::Private::load()
{
    if ( loaded )
        return;

    loaded = true;

    QString url = movie->url();
    KUrl newurl;
    if ( QDir::isRelativePath( url ) )
    {
        newurl = document->currentDocument();
        newurl.setFileName( url );
    }
    else
    {
        newurl = url;
    }
    if ( newurl.isLocalFile() )
        player->load( newurl.toLocalFile() );
    else
        player->load( newurl );

    connect( player->mediaObject(), SIGNAL( stateChanged( Phonon::State, Phonon::State ) ),
             q, SLOT( stateChanged( Phonon::State, Phonon::State ) ) );

    seekSlider->setEnabled( true );
}

void VideoWidget::Private::setupPlayPauseAction( PlayPauseMode mode )
{
    if ( mode == PlayMode )
    {
        playPauseAction->setIcon( KIcon( "media-playback-start" ) );
        playPauseAction->setText( i18nc( "start the movie playback", "Play" ) );
    }
    else if ( mode == PauseMode )
    {
        playPauseAction->setIcon( KIcon( "media-playback-pause" ) );
        playPauseAction->setText( i18nc( "pause the movie playback", "Pause" ) );
    }
}

void VideoWidget::Private::takeSnapshot()
{
    const QString url = movie->url();
    KUrl newurl;
    if ( QDir::isRelativePath( url ) )
    {
        newurl = document->currentDocument();
        newurl.setFileName( url );
    }
    else
    {
        newurl = url;
    }

    SnapshotTaker *taker = 0;
    if ( newurl.isLocalFile() )
        taker = new SnapshotTaker( newurl.toLocalFile(), q );
    else
        taker = new SnapshotTaker( newurl.url(), q );

    q->connect( taker, SIGNAL( finished( const QImage& ) ), q, SLOT( setPosterImage( const QImage& ) ) );
}

void VideoWidget::Private::videoStopped()
{
    if ( movie->showPosterImage() )
        pageLayout->setCurrentIndex( 1 );
    else
        q->hide();
}

void VideoWidget::Private::finished()
{
    switch ( movie->playMode() )
    {
        case Okular::Movie::PlayOnce:
        case Okular::Movie::PlayOpen:
            // playback has ended, nothing to do
            stopAction->setEnabled( false );
            setupPlayPauseAction( PlayMode );
            if ( movie->playMode() == Okular::Movie::PlayOnce )
                controlBar->setVisible( false );
            videoStopped();
            break;
        case Okular::Movie::PlayRepeat:
            // repeat the playback
            player->play();
            break;
        case Okular::Movie::PlayPalindrome:
            // FIXME we should play backward, but we cannot
            player->play();
            break;
    }
}

void VideoWidget::Private::playOrPause()
{
    if ( player->isPlaying() )
    {
        player->pause();
        setupPlayPauseAction( PlayMode );
    }
    else
    {
        q->play();
    }
}

void VideoWidget::Private::setPosterImage( const QImage &image )
{
    if ( !image.isNull() )
    {
        // cache the snapshot image
        movie->setPosterImage( image );
    }

    posterImagePage->setPixmap( QPixmap::fromImage( image ) );
}

void VideoWidget::Private::stateChanged( Phonon::State newState, Phonon::State )
{
    if ( newState == Phonon::PlayingState )
        pageLayout->setCurrentIndex( 0 );
}

VideoWidget::VideoWidget( const Okular::Annotation *annotation, Okular::Movie *movie, Okular::Document *document, QWidget *parent )
    : QWidget( parent ), d( new Private( movie, document, this ) )
{
    // do not propagate the mouse events to the parent widget;
    // they should be tied to this widget, not spread around...
    setAttribute( Qt::WA_NoMousePropagation );

    // Setup player page
    QWidget *playerPage = new QWidget;

    QVBoxLayout *mainlay = new QVBoxLayout( playerPage );
    mainlay->setMargin( 0 );
    mainlay->setSpacing( 0 );

    d->player = new Phonon::VideoPlayer( Phonon::NoCategory, playerPage );
    d->player->installEventFilter( playerPage );
    mainlay->addWidget( d->player );

    d->controlBar = new QToolBar( playerPage );
    d->controlBar->setIconSize( QSize( 16, 16 ) );
    d->controlBar->setAutoFillBackground( true );
    mainlay->addWidget( d->controlBar );

    d->playPauseAction = new QAction( d->controlBar );
    d->controlBar->addAction( d->playPauseAction );
    d->setupPlayPauseAction( Private::PlayMode );
    d->stopAction = d->controlBar->addAction(
        KIcon( "media-playback-stop" ),
        i18nc( "stop the movie playback", "Stop" ),
        this, SLOT(stop()) );
    d->stopAction->setEnabled( false );
    d->controlBar->addSeparator();
    d->seekSlider = new Phonon::SeekSlider( d->player->mediaObject(), d->controlBar );
    d->seekSliderAction = d->controlBar->addWidget( d->seekSlider );
    d->seekSlider->setEnabled( false );

    Phonon::SeekSlider *verticalSeekSlider = new Phonon::SeekSlider( d->player->mediaObject(), 0 );
    verticalSeekSlider->setMaximumHeight( 100 );
    d->seekSliderMenuAction = createToolBarButtonWithWidgetPopup(
        d->controlBar, verticalSeekSlider, KIcon( "player-time" ) );
    d->seekSliderMenuAction->setVisible( false );

    d->controlBar->setVisible( movie->showControls() );

    connect( d->player, SIGNAL(finished()), this, SLOT(finished()) );
    connect( d->playPauseAction, SIGNAL(triggered()), this, SLOT(playOrPause()) );

    d->geom = annotation->transformedBoundingRectangle();

    // Setup poster image page
    d->posterImagePage = new QLabel;
    d->posterImagePage->setScaledContents( true );
    d->posterImagePage->installEventFilter( this );
    d->posterImagePage->setCursor( Qt::PointingHandCursor );

    d->pageLayout = new QStackedLayout( this );
    d->pageLayout->setMargin( 0 );
    d->pageLayout->setSpacing( 0 );
    d->pageLayout->addWidget( playerPage );
    d->pageLayout->addWidget( d->posterImagePage );


    if ( movie->showPosterImage() )
    {
        d->pageLayout->setCurrentIndex( 1 );

        const QImage posterImage = movie->posterImage();
        if ( posterImage.isNull() )
        {
            d->takeSnapshot();
        }
        else
        {
            d->setPosterImage( posterImage );
        }
    }
    else
    {
        d->pageLayout->setCurrentIndex( 0 );
    }
}

VideoWidget::~VideoWidget()
{
    delete d;
}

void VideoWidget::setNormGeometry( const Okular::NormalizedRect &rect )
{
    d->geom = rect;
}

Okular::NormalizedRect VideoWidget::normGeometry() const
{
    return d->geom;
}

bool VideoWidget::isPlaying() const
{
    return d->player->isPlaying();
}

void VideoWidget::pageInitialized()
{
    hide();
}

void VideoWidget::pageEntered()
{
    if ( d->movie->showPosterImage() )
    {
        d->pageLayout->setCurrentIndex( 1 );
        show();
    }

    if ( d->movie->autoPlay() )
    {
        show();
        QMetaObject::invokeMethod(this, "play", Qt::QueuedConnection);
    }
}

void VideoWidget::pageLeft()
{
    d->player->stop();
    d->videoStopped();

    hide();
}

void VideoWidget::play()
{
    d->controlBar->setVisible( d->movie->showControls() );
    d->load();
    d->player->play();
    d->stopAction->setEnabled( true );
    d->setupPlayPauseAction( Private::PauseMode );
}

void VideoWidget::stop()
{
    d->player->stop();
    d->stopAction->setEnabled( false );
    d->setupPlayPauseAction( Private::PlayMode );
}

void VideoWidget::pause()
{
    d->player->pause();
    d->setupPlayPauseAction( Private::PlayMode );
}

bool VideoWidget::eventFilter( QObject * object, QEvent * event )
{
    if ( object == d->player || object == d->posterImagePage )
    {
        switch ( event->type() )
        {
            case QEvent::MouseButtonPress:
            {
                QMouseEvent * me = static_cast< QMouseEvent * >( event );
                if ( me->button() == Qt::LeftButton )
                {
                    if ( !d->player->isPlaying() )
                    {
                        play();
                    }
                    event->accept();
                }
            }
            case QEvent::Wheel:
            {
                if ( object == d->posterImagePage )
                {
                    QWheelEvent * we = static_cast< QWheelEvent * >( event );

                    // forward wheel events to parent widget
                    QWheelEvent *copy = new QWheelEvent( we->pos(), we->globalPos(), we->delta(), we->buttons(), we->modifiers(), we->orientation() );
                    QCoreApplication::postEvent( parentWidget(), copy );
                }
                break;
            }
            default: ;
        }
    }

    return false;
}

bool VideoWidget::event( QEvent * event )
{
    switch ( event->type() )
    {
        case QEvent::ToolTip:
            // "eat" the help events (= tooltips), avoid parent widgets receiving them
            event->accept();
            return true;
            break;
        default: ;
    }

    return QWidget::event( event );
}

void VideoWidget::resizeEvent( QResizeEvent * event )
{
    const QSize &s = event->size();
    int usedSpace = d->seekSlider->geometry().left() + d->seekSlider->iconSize().width();
    // try to give the slider at least 30px of space
    if ( s.width() < ( usedSpace + 30 ) )
    {
        d->seekSliderAction->setVisible( false );
        d->seekSliderMenuAction->setVisible( true );
    }
    else
    {
        d->seekSliderAction->setVisible( true );
        d->seekSliderMenuAction->setVisible( false );
    }
}

#include "videowidget.moc"
