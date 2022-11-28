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
#include <kmediawidget.h>

#include "core/annotations.h"
#include "core/area.h"
#include "core/document.h"
#include "core/movie.h"
#include "snapshottaker.h"

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
            player->player()->stop();
    }

    void load();
    void setPosterImage( const QImage& );
    void takeSnapshot();
    void videoStopped();
    void stateChanged(bool paused);

    // slots
    void finished();

    VideoWidget *q;
    Okular::Movie *movie;
    Okular::Document *document;
    Okular::NormalizedRect geom;
    KMediaWidget *player;
    QStackedLayout *pageLayout;
    QLabel *posterImagePage;
    bool loaded;
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
    player->open( newurl.prettyUrl() );

    connect( player->player(), SIGNAL( paused( bool ) ),
             q, SLOT( stateChanged( bool ) ) );
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
    taker = new SnapshotTaker( newurl, q, q->size());

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
            videoStopped();
            break;
        case Okular::Movie::PlayRepeat:
            // repeat the playback
            player->player()->play();
            break;
        case Okular::Movie::PlayPalindrome:
            // FIXME we should play backward, but we cannot
            player->player()->play();
            break;
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

void VideoWidget::Private::stateChanged( bool paused )
{
    if ( !paused )
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

    d->player = new KMediaWidget( playerPage );
    d->player->player()->setPlayerID("okulartpart_video");
    mainlay->addWidget( d->player );

    connect( d->player->player(), SIGNAL(finished()), this, SLOT(finished()) );
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
    return d->player->player()->isPlaying();
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
    d->player->player()->stop();
    d->videoStopped();

    hide();
}

void VideoWidget::play()
{
    d->load();
    d->player->player()->play();
}

void VideoWidget::stop()
{
    d->player->player()->stop();
}

void VideoWidget::pause()
{
    d->player->player()->pause();
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

#include "moc_videowidget.cpp"
