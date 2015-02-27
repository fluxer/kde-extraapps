/***************************************************************************
 *   Copyright (C) 2004-2006 by Albert Astals Cid <aacid@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "thumbnaillist.h"

// qt/kde includes
#include <qevent.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qsizepolicy.h>
#include <klocale.h>
#include <kurl.h>
#include <kaction.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <kglobalsettings.h>

// local includes
#include "pagepainter.h"
#include "core/area.h"
#include "core/bookmarkmanager.h"
#include "core/document.h"
#include "core/generator.h"
#include "core/page.h"
#include "settings.h"
#include "priorities.h"

class ThumbnailWidget;

class ThumbnailListPrivate : public QWidget
{
    public:
        ThumbnailListPrivate( ThumbnailList *qq, Okular::Document *document );
        ~ThumbnailListPrivate();

        enum ChangePageDirection
        {
            Null,
            Left,
            Right,
            Up,
            Down
        };

        ThumbnailList *q;
        Okular::Document *m_document;
        ThumbnailWidget *m_selected;
        QTimer *m_delayTimer;
        QPixmap *m_bookmarkOverlay;
        QVector<ThumbnailWidget *> m_thumbnails;
        QList<ThumbnailWidget *> m_visibleThumbnails;
        int m_vectorIndex;
        // Grabbing variables
        QPoint m_mouseGrabPos;
        ThumbnailWidget *m_mouseGrabItem;
        int m_pageCurrentlyGrabbed;

        // resize thumbnails to fit the width
        void viewportResizeEvent( QResizeEvent * );
        // called by ThumbnailWidgets to get the overlay bookmark pixmap
        const QPixmap * getBookmarkOverlay() const;
        // called by ThumbnailWidgets to send (forward) the mouse move signals
        ChangePageDirection forwardTrack( const QPoint &, const QSize & );

        ThumbnailWidget* itemFor( const QPoint & p ) const;
        void delayedRequestVisiblePixmaps( int delayMs = 0 );

        // SLOTS:
        // make requests for generating pixmaps for visible thumbnails
        void slotRequestVisiblePixmaps( int newContentsY = -1 );
        // delay timeout: resize overlays and requests pixmaps
        void slotDelayTimeout();
        ThumbnailWidget* getPageByNumber( int page ) const;
        int getNewPageOffset( int n, ThumbnailListPrivate::ChangePageDirection dir ) const;
        ThumbnailWidget *getThumbnailbyOffset( int current, int offset ) const;

    protected:
        void mousePressEvent( QMouseEvent * e );
        void mouseReleaseEvent( QMouseEvent * e );
        void mouseMoveEvent( QMouseEvent * e );
        void wheelEvent( QWheelEvent * e );
        void contextMenuEvent( QContextMenuEvent * e );
        void paintEvent( QPaintEvent * e );
};


// ThumbnailWidget represents a single thumbnail in the ThumbnailList
class ThumbnailWidget
{
    public:
        ThumbnailWidget( ThumbnailListPrivate * parent, const Okular::Page * page );

        // set internal parameters to fit the page in the given width
        void resizeFitWidth( int width );
        // set thumbnail's selected state
        void setSelected( bool selected );
        // set the visible rect of the current page
        void setVisibleRect( const Okular::NormalizedRect & rect );

        // query methods
        int heightHint() const { return m_pixmapHeight + m_labelHeight + m_margin; }
        int pixmapWidth() const { return m_pixmapWidth; }
        int pixmapHeight() const { return m_pixmapHeight; }
        int pageNumber() const { return m_page->number(); }
        const Okular::Page * page() const { return m_page; }
        QRect visibleRect() const { return m_visibleRect.geometry( m_pixmapWidth, m_pixmapHeight ); }

        void paint( QPainter &p, const QRect &clipRect );

        static int margin() { return m_margin; }

        // simulating QWidget
        QRect rect() const { return m_rect; }
        int height() const { return m_rect.height(); }
        int width() const { return m_rect.width(); }
        QPoint pos() const { return m_rect.topLeft(); }
        void move( int x, int y ) { m_rect.setTopLeft( QPoint( x, y ) ); }
        void update() { m_parent->update( m_rect ); }
        void update( const QRect & rect ) { m_parent->update( rect.translated( m_rect.topLeft() ) ); }

    private:
        // the margin around the widget
        static int const m_margin = 16;

        ThumbnailListPrivate * m_parent;
        const Okular::Page * m_page;
        bool m_selected;
        int m_pixmapWidth, m_pixmapHeight;
        int m_labelHeight, m_labelNumber;
        Okular::NormalizedRect m_visibleRect;
        QRect m_rect;
};


ThumbnailListPrivate::ThumbnailListPrivate( ThumbnailList *qq, Okular::Document *document )
    : QWidget(), q( qq ), m_document( document ), m_selected( 0 ),
    m_delayTimer( 0 ), m_bookmarkOverlay( 0 ), m_vectorIndex( 0 )
{
    setMouseTracking( true );
    m_mouseGrabItem = 0;
}


ThumbnailWidget* ThumbnailListPrivate::getPageByNumber( int page ) const
{
    QVector< ThumbnailWidget * >::const_iterator tIt = m_thumbnails.constBegin(), tEnd = m_thumbnails.constEnd();
    for ( ; tIt != tEnd; ++tIt )
    {
        if ( (*tIt)->pageNumber() == page )
            return (*tIt);
    }
    return 0;
}

ThumbnailListPrivate::~ThumbnailListPrivate()
{
}

ThumbnailWidget* ThumbnailListPrivate::itemFor( const QPoint & p ) const
{
    QVector< ThumbnailWidget * >::const_iterator tIt = m_thumbnails.constBegin(), tEnd = m_thumbnails.constEnd();
    for ( ; tIt != tEnd; ++tIt )
    {
        if ( (*tIt)->rect().contains( p ) )
            return (*tIt);
    }
    return 0;
}

void ThumbnailListPrivate::paintEvent( QPaintEvent * e )
{
    QPainter painter( this );
    QVector<ThumbnailWidget *>::const_iterator tIt = m_thumbnails.constBegin(), tEnd = m_thumbnails.constEnd();
    for ( ; tIt != tEnd; ++tIt )
    {
        QRect rect = e->rect().intersected( (*tIt)->rect() );
        if ( !rect.isNull() )
        {
            rect.translate( -(*tIt)->pos() );
            painter.save();
            painter.translate( (*tIt)->pos() );
            (*tIt)->paint( painter, rect );
            painter.restore();
        }
    }
}


/** ThumbnailList implementation **/

ThumbnailList::ThumbnailList( QWidget *parent, Okular::Document *document )
    : QScrollArea( parent ), d( new ThumbnailListPrivate( this, document ) )
{
    setObjectName( QLatin1String( "okular::Thumbnails" ) );
    // set scrollbars
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    verticalScrollBar()->setEnabled( false );

    setAttribute( Qt::WA_StaticContents );

    viewport()->setBackgroundRole( QPalette::Base );

    setWidget( d );
    // widget setup: can be focused by mouse click (not wheel nor tab)
    widget()->setFocusPolicy( Qt::ClickFocus );
    widget()->show();
    widget()->setBackgroundRole( QPalette::Base );

    connect( verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotRequestVisiblePixmaps(int)) );
}

ThumbnailList::~ThumbnailList()
{
    d->m_document->removeObserver( this );
    delete d->m_bookmarkOverlay;
}

//BEGIN DocumentObserver inherited methods
void ThumbnailList::notifySetup( const QVector< Okular::Page * > & pages, int setupFlags )
{
    // if there was a widget selected, save its pagenumber to restore
    // its selection (if available in the new set of pages)
    int prevPage = -1;
    if ( !( setupFlags & Okular::DocumentObserver::DocumentChanged ) && d->m_selected )
    {
        prevPage = d->m_selected->page()->number();
    } else 
        prevPage = d->m_document->viewport().pageNumber;

    // delete all the Thumbnails
    QVector<ThumbnailWidget *>::const_iterator tIt = d->m_thumbnails.constBegin(), tEnd = d->m_thumbnails.constEnd();
    for ( ; tIt != tEnd; ++tIt )
        delete *tIt;
    d->m_thumbnails.clear();
    d->m_visibleThumbnails.clear();
    d->m_selected = 0;
    d->m_mouseGrabItem = 0;

    if ( pages.count() < 1 )
    {
        widget()->resize( 0, 0 );
        return;
    }

    // show pages containing hilighted text or bookmarked ones
    //RESTORE THIS int flags = Okular::Settings::filterBookmarks() ? Okular::Page::Bookmark : Okular::Page::Highlight;

    // if no page matches filter rule, then display all pages
    QVector< Okular::Page * >::const_iterator pIt = pages.constBegin(), pEnd = pages.constEnd();
    bool skipCheck = true;
    for ( ; pIt != pEnd ; ++pIt )
        //if ( (*pIt)->attributes() & flags )
        if ( (*pIt)->hasHighlights( SW_SEARCH_ID ) )
            skipCheck = false;

    // generate Thumbnails for the given set of pages
    const int width = viewport()->width();
    int height = 0;
    int centerHeight = 0;
    for ( pIt = pages.constBegin(); pIt != pEnd ; ++pIt )
        //if ( skipCheck || (*pIt)->attributes() & flags )
        if ( skipCheck || (*pIt)->hasHighlights( SW_SEARCH_ID ) )
        {
            ThumbnailWidget * t = new ThumbnailWidget( d, *pIt );
            t->move(0, height);
            // add to the internal queue
            d->m_thumbnails.push_back( t );
            // update total height (asking widget its own height)
            t->resizeFitWidth( width );
            // restoring the previous selected page, if any
            if ( (*pIt)->number() < prevPage )
            {
                centerHeight = height + t->height() + KDialog::spacingHint()/2;
            }
            if ( (*pIt)->number() == prevPage )
            {
                d->m_selected = t;
                d->m_selected->setSelected( true );
                centerHeight = height + t->height() / 2;
            }
            height += t->height() + KDialog::spacingHint();
        }

    // update scrollview's contents size (sets scrollbars limits)
    height -= KDialog::spacingHint();
    widget()->resize( width, height );

    // enable scrollbar when there's something to scroll
    verticalScrollBar()->setEnabled( viewport()->height() < height );
    verticalScrollBar()->setValue(centerHeight - viewport()->height() / 2);

    // request for thumbnail generation
    d->delayedRequestVisiblePixmaps( 200 );
}

void ThumbnailList::notifyCurrentPageChanged( int previousPage, int currentPage )
{
    Q_UNUSED( previousPage )

    // skip notifies for the current page (already selected)
    if ( d->m_selected && d->m_selected->pageNumber() == currentPage )
        return;

    // deselect previous thumbnail
    if ( d->m_selected )
        d->m_selected->setSelected( false );
    d->m_selected = 0;

    // select the page with viewport and ensure it's centered in the view
    d->m_vectorIndex = 0;
    QVector<ThumbnailWidget *>::const_iterator tIt = d->m_thumbnails.constBegin(), tEnd = d->m_thumbnails.constEnd();
    for ( ; tIt != tEnd; ++tIt )
    {
        if ( (*tIt)->pageNumber() == currentPage )
        {
            d->m_selected = *tIt;
            d->m_selected->setSelected( true );
            if ( Okular::Settings::syncThumbnailsViewport() )
            {
                int yOffset = qMax( viewport()->height() / 4, d->m_selected->height() / 2 );
                ensureVisible( 0, d->m_selected->pos().y() + d->m_selected->height()/2, 0, yOffset );
            }
            break;
        }
        d->m_vectorIndex++;
    }
}

void ThumbnailList::notifyPageChanged( int pageNumber, int changedFlags )
{
    static int interestingFlags = DocumentObserver::Pixmap | DocumentObserver::Bookmark | DocumentObserver::Highlights | DocumentObserver::Annotations;
    // only handle change notifications we are interested in
    if ( !( changedFlags & interestingFlags ) )
        return;

    // iterate over visible items: if page(pageNumber) is one of them, repaint it
    QList<ThumbnailWidget *>::const_iterator vIt = d->m_visibleThumbnails.constBegin(), vEnd = d->m_visibleThumbnails.constEnd();
    for ( ; vIt != vEnd; ++vIt )
        if ( (*vIt)->pageNumber() == pageNumber )
        {
            (*vIt)->update();
            break;
        }
}

void ThumbnailList::notifyContentsCleared( int changedFlags )
{
    // if pixmaps were cleared, re-ask them
    if ( changedFlags & DocumentObserver::Pixmap )
        d->slotRequestVisiblePixmaps();
}

void ThumbnailList::notifyVisibleRectsChanged()
{
    bool found = false;
    const QVector<Okular::VisiblePageRect *> & visibleRects = d->m_document->visiblePageRects();
    QVector<ThumbnailWidget *>::const_iterator tIt = d->m_thumbnails.constBegin(), tEnd = d->m_thumbnails.constEnd();
    QVector<Okular::VisiblePageRect *>::const_iterator vEnd = visibleRects.end();
    for ( ; tIt != tEnd; ++tIt )
    {
        found = false;
        QVector<Okular::VisiblePageRect *>::const_iterator vIt = visibleRects.begin();
        for ( ; ( vIt != vEnd ) && !found; ++vIt )
        {
            if ( (*tIt)->pageNumber() == (*vIt)->pageNumber )
            {
                (*tIt)->setVisibleRect( (*vIt)->rect );
                found = true;
            }
        }
        if ( !found )
        {
            (*tIt)->setVisibleRect( Okular::NormalizedRect() );
        }
    }
}

bool ThumbnailList::canUnloadPixmap( int pageNumber ) const
{
    // if the thubnail 'pageNumber' is one of the visible ones, forbid unloading
    QList<ThumbnailWidget *>::const_iterator vIt = d->m_visibleThumbnails.constBegin(), vEnd = d->m_visibleThumbnails.constEnd();
    for ( ; vIt != vEnd; ++vIt )
        if ( (*vIt)->pageNumber() == pageNumber )
            return false;
    // if hidden permit unloading
    return true;
}
//END DocumentObserver inherited methods 


void ThumbnailList::updateWidgets()
{
    // Update all visible widgets
    QList<ThumbnailWidget *>::const_iterator vIt = d->m_visibleThumbnails.constBegin(), vEnd = d->m_visibleThumbnails.constEnd();
    for ( ; vIt != vEnd; ++vIt )
    {
        ThumbnailWidget * t = *vIt;
        t->update();
    }
}

int ThumbnailListPrivate::getNewPageOffset(int n, ThumbnailListPrivate::ChangePageDirection dir) const
{
    int reason = 1;
    int facingFirst = 0; //facingFirstCentered cornercase
    if ( Okular::Settings::viewMode() == Okular::Settings::EnumViewMode::Facing )
        reason = 2;
    else if (Okular::Settings::viewMode() == Okular::Settings::EnumViewMode::FacingFirstCentered )
    {
        facingFirst = 1;
        reason = 2;
    }
    else if ( Okular::Settings::viewMode() == Okular::Settings::EnumViewMode::Summary )
        reason = 3;
    if ( dir == ThumbnailListPrivate::Up )
    {
        if ( facingFirst && n == 1 )
            return -1;
        return -reason;
    }
    if ( dir == ThumbnailListPrivate::Down )
        return reason;
    if ( dir == ThumbnailListPrivate::Left && reason > 1 && (n + facingFirst)  % reason )
        return -1;
    if ( dir == ThumbnailListPrivate::Right && reason > 1 && (n + 1 + facingFirst) % reason )
        return 1;
    return 0;
}

ThumbnailWidget *ThumbnailListPrivate::getThumbnailbyOffset(int current, int offset) const
{
    QVector<ThumbnailWidget *>::const_iterator it = m_thumbnails.begin();
    QVector<ThumbnailWidget *>::const_iterator itE = m_thumbnails.end();
    int idx = 0;
    while ( it != itE )
    {
        if ( (*it)->pageNumber() == current )
            break;
        ++idx;
        ++it;
    }
    if ( it == itE )
        return 0;
    idx += offset;
    if ( idx < 0 || idx >= m_thumbnails.size() )
        return 0;
    return m_thumbnails[idx];
}

ThumbnailListPrivate::ChangePageDirection ThumbnailListPrivate::forwardTrack(const QPoint &point, const QSize &r )
{
    Okular::DocumentViewport vp = m_document->viewport();
    const double deltaX = (double)point.x() / r.width(),
                 deltaY = (double)point.y() / r.height();
    vp.rePos.normalizedX -= deltaX;    
    vp.rePos.normalizedY -= deltaY;
    if ( vp.rePos.normalizedY > 1.0 )
        return ThumbnailListPrivate::Down;
    if ( vp.rePos.normalizedY < 0.0 )
        return ThumbnailListPrivate::Up;                
    if ( vp.rePos.normalizedX > 1.0 )
        return ThumbnailListPrivate::Right;
    if ( vp.rePos.normalizedX < 0.0 )
        return ThumbnailListPrivate::Left;
    vp.rePos.enabled = true;
    m_document->setViewport( vp );
    return ThumbnailListPrivate::Null;
}

const QPixmap * ThumbnailListPrivate::getBookmarkOverlay() const
{
    return m_bookmarkOverlay;
}

void ThumbnailList::slotFilterBookmarks( bool filterOn )
{
    // save state
    Okular::Settings::setFilterBookmarks( filterOn );
    Okular::Settings::self()->writeConfig();
    // ask for the 'notifySetup' with a little trick (on reinsertion the
    // document sends the list again)
    d->m_document->removeObserver( this );
    d->m_document->addObserver( this );
}


//BEGIN widget events 
void ThumbnailList::keyPressEvent( QKeyEvent * keyEvent )
{
    if ( d->m_thumbnails.count() < 1 )
        return keyEvent->ignore();

    int nextPage = -1;
    if ( keyEvent->key() == Qt::Key_Up )
    {
        if ( !d->m_selected )
            nextPage = 0;
        else if ( d->m_vectorIndex > 0 )
            nextPage = d->m_thumbnails[ d->m_vectorIndex - 1 ]->pageNumber();
    }
    else if ( keyEvent->key() == Qt::Key_Down )
    {
        if ( !d->m_selected )
            nextPage = 0;
        else if ( d->m_vectorIndex < (int)d->m_thumbnails.count() - 1 )
            nextPage = d->m_thumbnails[ d->m_vectorIndex + 1 ]->pageNumber();
    }
    else if ( keyEvent->key() == Qt::Key_PageUp )
        verticalScrollBar()->triggerAction( QScrollBar::SliderPageStepSub );
    else if ( keyEvent->key() == Qt::Key_PageDown )
        verticalScrollBar()->triggerAction( QScrollBar::SliderPageStepAdd );
    else if ( keyEvent->key() == Qt::Key_Home )
        nextPage = d->m_thumbnails[ 0 ]->pageNumber();
    else if ( keyEvent->key() == Qt::Key_End )
        nextPage = d->m_thumbnails[ d->m_thumbnails.count() - 1 ]->pageNumber();

    if ( nextPage == -1 )
        return keyEvent->ignore();

    keyEvent->accept();
    if ( d->m_selected )
        d->m_selected->setSelected( false );
    d->m_selected = 0;
    d->m_document->setViewportPage( nextPage );
}

bool ThumbnailList::viewportEvent( QEvent * e )
{
    switch ( e->type() )
    {
        case QEvent::Resize:
        {
            d->viewportResizeEvent( (QResizeEvent*)e );
            break;
        }
        default:
            ;
    }
    return QScrollArea::viewportEvent( e );
}

void ThumbnailListPrivate::viewportResizeEvent( QResizeEvent * e )
{
    if ( m_thumbnails.count() < 1 || width() < 1 )
        return;

    // if width changed resize all the Thumbnails, reposition them to the
    // right place and recalculate the contents area
    if ( e->size().width() != e->oldSize().width() )
    {
        // runs the timer avoiding a thumbnail regeneration by 'contentsMoving'
        delayedRequestVisiblePixmaps( 2000 );

        // resize and reposition items
        const int newWidth = q->viewport()->width();
        int newHeight = 0;
        QVector<ThumbnailWidget *>::const_iterator tIt = m_thumbnails.constBegin(), tEnd = m_thumbnails.constEnd();
        for ( ; tIt != tEnd; ++tIt )
        {
            ThumbnailWidget *t = *tIt;
            t->move(0, newHeight);
            t->resizeFitWidth( newWidth );
            newHeight += t->height() + KDialog::spacingHint();
        }

        // update scrollview's contents size (sets scrollbars limits)
        newHeight -= KDialog::spacingHint();
        const int oldHeight = q->widget()->height();
        const int oldYCenter = q->verticalScrollBar()->value() + q->viewport()->height() / 2;
        q->widget()->resize( newWidth, newHeight );

        // enable scrollbar when there's something to scroll
        q->verticalScrollBar()->setEnabled( q->viewport()->height() < newHeight );

        // ensure that what was visibile before remains visible now
        q->ensureVisible( 0, int( (qreal)oldYCenter * q->widget()->height() / oldHeight ), 0, q->viewport()->height() / 2 );
    }
    else if ( e->size().height() <= e->oldSize().height() )
        return;

    // invalidate the bookmark overlay
    if ( m_bookmarkOverlay )
    {
        delete m_bookmarkOverlay;
        m_bookmarkOverlay = 0;
    }

    // update Thumbnails since width has changed or height has increased
    delayedRequestVisiblePixmaps( 500 );
}
//END widget events

//BEGIN internal SLOTS 
void ThumbnailListPrivate::slotRequestVisiblePixmaps( int /*newContentsY*/ )
{
    // if an update is already scheduled or the widget is hidden, don't proceed
    if ( ( m_delayTimer && m_delayTimer->isActive() ) || q->isHidden() )
        return;

    // scroll from the top to the last visible thumbnail
    m_visibleThumbnails.clear();
    QLinkedList< Okular::PixmapRequest * > requestedPixmaps;
    QVector<ThumbnailWidget *>::const_iterator tIt = m_thumbnails.constBegin(), tEnd = m_thumbnails.constEnd();
    const QRect viewportRect = q->viewport()->rect().translated( q->horizontalScrollBar()->value(), q->verticalScrollBar()->value() );
    for ( ; tIt != tEnd; ++tIt )
    {
        ThumbnailWidget * t = *tIt;
        const QRect thumbRect = t->rect();
        if ( !thumbRect.intersects( viewportRect ) )
          continue;
        // add ThumbnailWidget to visible list
        m_visibleThumbnails.push_back( t );
        // if pixmap not present add it to requests
        if ( !t->page()->hasPixmap( q, t->pixmapWidth(), t->pixmapHeight() ) )
        {
            Okular::PixmapRequest * p = new Okular::PixmapRequest( q, t->pageNumber(), t->pixmapWidth(), t->pixmapHeight(), THUMBNAILS_PRIO, Okular::PixmapRequest::Asynchronous );
            requestedPixmaps.push_back( p );
        }
    }

    // actually request pixmaps
    if ( !requestedPixmaps.isEmpty() )
        m_document->requestPixmaps( requestedPixmaps );
}

void ThumbnailListPrivate::slotDelayTimeout()
{
    // resize the bookmark overlay
    delete m_bookmarkOverlay;
    const int expectedWidth = q->viewport()->width() / 4;
    if ( expectedWidth > 10 )
        m_bookmarkOverlay = new QPixmap( DesktopIcon( "bookmarks", expectedWidth ) );
    else
        m_bookmarkOverlay = 0;

    // request pixmaps
    slotRequestVisiblePixmaps();
}
//END internal SLOTS

void ThumbnailListPrivate::delayedRequestVisiblePixmaps( int delayMs )
{
    if ( !m_delayTimer )
    {
        m_delayTimer = new QTimer( q );
        m_delayTimer->setSingleShot( true );
        connect( m_delayTimer, SIGNAL(timeout()), q, SLOT(slotDelayTimeout()) );
    }
    m_delayTimer->start( delayMs );
}


/** ThumbnailWidget implementation **/

ThumbnailWidget::ThumbnailWidget( ThumbnailListPrivate * parent, const Okular::Page * kp )
    : m_parent( parent ), m_page( kp ),
    m_selected( false ), m_pixmapWidth( 10 ), m_pixmapHeight( 10 )
{
    m_labelNumber = m_page->number() + 1;
    m_labelHeight = QFontMetrics( m_parent->font() ).height();

}

void ThumbnailWidget::resizeFitWidth( int width )
{
    m_pixmapWidth = width - m_margin;
    m_pixmapHeight = qRound( m_page->ratio() * (double)m_pixmapWidth );
    m_rect.setSize( QSize( width, heightHint() ) );
}

void ThumbnailWidget::setSelected( bool selected )
{
    // update selected state
    if ( m_selected != selected )
    {
        m_selected = selected;
        update();
    }
}

void ThumbnailWidget::setVisibleRect( const Okular::NormalizedRect & rect )
{
    if ( rect == m_visibleRect )
       return;

    m_visibleRect = rect;
    update();
}

void ThumbnailListPrivate::mousePressEvent( QMouseEvent * e )
{
    ThumbnailWidget* item = itemFor( e->pos() );
    if ( !item ) // mouse on the spacing between items
        return e->ignore();

    const QRect r = item->visibleRect();
    const int margin = ThumbnailWidget::margin();
    const QPoint p = e->pos() - item->pos();

    if ( e->button() != Qt::RightButton && r.contains( p - QPoint( margin / 2, margin / 2 ) ) )
    {
        m_mouseGrabPos.setX( 0 );
        m_mouseGrabPos.setY( 0 );
        m_mouseGrabItem = item;
        m_pageCurrentlyGrabbed = item->pageNumber();
        m_mouseGrabItem = item;
    }
    else
    {
        m_mouseGrabPos.setX( 0 );
        m_mouseGrabPos.setY( 0 );
        m_mouseGrabItem = 0;
    }
}

void ThumbnailListPrivate::mouseReleaseEvent( QMouseEvent * e )
{
    ThumbnailWidget* item = itemFor( e->pos() );
    m_mouseGrabItem = item;
    if ( !item ) // mouse on the spacing between items
        return e->ignore();

    QRect r = item->visibleRect();
    const int margin = ThumbnailWidget::margin();
    const QPoint p = e->pos() - item->pos();

    if ( r.contains( p - QPoint( margin / 2, margin / 2 ) ) )
    {
        setCursor( Qt::OpenHandCursor );
    }
    else
    {
        setCursor( Qt::ArrowCursor );
        if ( m_mouseGrabPos.isNull() )
        {
            if ( m_document->viewport().pageNumber != item->pageNumber() )
            {
                m_document->setViewportPage( item->pageNumber() );
                r = item->visibleRect();
                Okular::DocumentViewport vp = Okular::DocumentViewport( item->pageNumber() );
                vp.rePos.normalizedX = 0.5;
                vp.rePos.normalizedY = (double) r.height() / 2.0  / (double) item->pixmapHeight();
                vp.rePos.pos = Okular::DocumentViewport::Center;
                vp.rePos.enabled = true;
                m_document->setViewport( vp );
            }
        }
    }
    m_mouseGrabPos.setX( 0 );
    m_mouseGrabPos.setY( 0 );
}

void ThumbnailListPrivate::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->buttons() == Qt::NoButton )
        return e->ignore();
    // no item under the mouse or previously selected
    if ( !m_mouseGrabItem )
        return e->ignore();
    const QRect r = m_mouseGrabItem->rect();
    if ( !m_mouseGrabPos.isNull() )
    {
        const QPoint mousePos = e->pos();
        const QPoint delta = m_mouseGrabPos - mousePos;
        m_mouseGrabPos = e->pos();
        // don't handle the mouse move, forward it to the thumbnail list
        ThumbnailListPrivate::ChangePageDirection direction;
        if (( direction = forwardTrack( delta, r.size() ))
            != ThumbnailListPrivate::Null )
        {
            // Changing the selected page
            const int offset = getNewPageOffset( m_pageCurrentlyGrabbed, direction );
            const ThumbnailWidget *newThumb = getThumbnailbyOffset( m_pageCurrentlyGrabbed, offset );
            if ( !newThumb )
                return;
            int newPageOn = newThumb->pageNumber();
            if ( newPageOn == m_pageCurrentlyGrabbed || newPageOn < 0 || 
                 newPageOn >= (int)m_document->pages() )
            {
                return;
            }
            Okular::DocumentViewport vp = m_document->viewport();
            const float origNormalX = vp.rePos.normalizedX;
            const float origNormalY = vp.rePos.normalizedY;
            vp = Okular::DocumentViewport( newPageOn );
            vp.rePos.normalizedX = origNormalX;
            vp.rePos.normalizedY = origNormalY;
            if ( direction == ThumbnailListPrivate::Up )
            {
                vp.rePos.normalizedY = 1.0;
                if ( Okular::Settings::viewMode() == 
                    Okular::Settings::EnumViewMode::FacingFirstCentered 
                    && !newPageOn)
                {
                    if ( m_pageCurrentlyGrabbed == 1 )
                        vp.rePos.normalizedX = origNormalX - 0.5;
                    else
                        vp.rePos.normalizedX = origNormalX + 0.5;
                    if ( vp.rePos.normalizedX < 0.0 )
                        vp.rePos.normalizedX = 0.0;
                    else if ( vp.rePos.normalizedX > 1.0 )
                        vp.rePos.normalizedX = 1.0;
                }
            }
            else if ( direction == ThumbnailListPrivate::Down )
            {
                vp.rePos.normalizedY = 0.0;
                if ( Okular::Settings::viewMode() == 
                     Okular::Settings::EnumViewMode::FacingFirstCentered 
                     && !m_pageCurrentlyGrabbed)
                {
                    if ( origNormalX < 0.5 )
                    {
                        vp = Okular::DocumentViewport( --newPageOn );
                        vp.rePos.normalizedX = origNormalX + 0.5;
                    }
                    else
                        vp.rePos.normalizedX = origNormalX - 0.5;
                    if ( vp.rePos.normalizedX < 0.0 )
                        vp.rePos.normalizedX = 0.0;
                    else if ( vp.rePos.normalizedX > 1.0 )
                        vp.rePos.normalizedX = 1.0;
                }
            }
            else if ( Okular::Settings::viewMode() != Okular::Settings::EnumViewMode::Single )
            {
                if ( direction == ThumbnailListPrivate::Left )
                    vp.rePos.normalizedX = 1.0;
                else
                    vp.rePos.normalizedX = 0.0;
            }
            vp.rePos.pos = Okular::DocumentViewport::Center;
            vp.rePos.enabled = true;
            m_document->setViewport( vp );
            m_mouseGrabPos.setX( 0 );
            m_mouseGrabPos.setY( 0 );
            m_pageCurrentlyGrabbed = newPageOn;
            m_mouseGrabItem = getPageByNumber( m_pageCurrentlyGrabbed );
        }
        // wrap mouse from top to bottom
        const QRect mouseContainer = KGlobalSettings::desktopGeometry( this );
        QPoint currentMousePos = QCursor::pos();
        if ( currentMousePos.y() <= mouseContainer.top() + 4 )
        {
            currentMousePos.setY( mouseContainer.bottom() - 5 );
            QCursor::setPos( currentMousePos );
            m_mouseGrabPos.setX( 0 );
            m_mouseGrabPos.setY( 0 );
        }
        // wrap mouse from bottom to top
        else if ( currentMousePos.y() >= mouseContainer.bottom() - 4 )
        {
            currentMousePos.setY( mouseContainer.top() + 5 );
            QCursor::setPos( currentMousePos );
            m_mouseGrabPos.setX( 0 );
            m_mouseGrabPos.setY( 0 );
        }
    }
    else
    {
        setCursor( Qt::ClosedHandCursor );
        m_mouseGrabPos = e->pos();
    }
}

void ThumbnailListPrivate::wheelEvent( QWheelEvent * e )
{
    const ThumbnailWidget* item = itemFor( e->pos() );
    if ( !item ) // wheeling on the spacing between items
        return e->ignore();

    const QRect r = item->visibleRect();
    const int margin = ThumbnailWidget::margin();

    if ( r.contains( e->pos() - QPoint( margin / 2, margin / 2 ) ) && e->orientation() == Qt::Vertical && e->modifiers() == Qt::ControlModifier )
    {
        m_document->setZoom( e->delta() );
    }
    else
    {
        e->ignore();
    }
}

void ThumbnailListPrivate::contextMenuEvent( QContextMenuEvent * e )
{
    const ThumbnailWidget* item = itemFor( e->pos() );
    if ( item )
    {
        emit q->rightClick( item->page(), e->globalPos() );
    }
}

void ThumbnailWidget::paint( QPainter &p, const QRect &_clipRect )
{
    const int width = m_pixmapWidth + m_margin;
    QRect clipRect = _clipRect;
    const QPalette pal = m_parent->palette();

    // draw the bottom label + highlight mark
    const QColor fillColor = m_selected ? pal.color( QPalette::Active, QPalette::Highlight ) : pal.color( QPalette::Active, QPalette::Base );
    p.fillRect( clipRect, fillColor );
    p.setPen( m_selected ? pal.color( QPalette::Active, QPalette::HighlightedText ) : pal.color( QPalette::Active, QPalette::Text ) );
    p.drawText( 0, m_pixmapHeight + (m_margin - 3), width, m_labelHeight, Qt::AlignCenter, QString::number( m_labelNumber ) );

    // draw page outline and pixmap
    if ( clipRect.top() < m_pixmapHeight + m_margin )
    {
        // if page is bookmarked draw a colored border
        const bool isBookmarked = m_parent->m_document->bookmarkManager()->isBookmarked( pageNumber() );
        // draw the inner rect
        p.setPen( isBookmarked ? QColor( 0xFF8000 ) : Qt::black );
        p.drawRect( m_margin/2 - 1, m_margin/2 - 1, m_pixmapWidth + 1, m_pixmapHeight + 1 );
        // draw the clear rect
        p.setPen( isBookmarked ? QColor( 0x804000 ) : pal.color( QPalette::Active, QPalette::Base ) );
        // draw the bottom and right shadow edges
        if ( !isBookmarked )
        {
            int left, right, bottom, top;
            left = m_margin/2 + 1;
            right = m_margin/2 + m_pixmapWidth + 1;
            bottom = m_pixmapHeight + m_margin/2 + 1;
            top = m_margin/2 + 1;
            p.setPen( Qt::gray );
            p.drawLine( left, bottom, right, bottom );
            p.drawLine( right, top, right, bottom );
        }

        // draw the page using the shared PagePainter class
        p.translate( m_margin/2, m_margin/2 );
        clipRect.translate( -m_margin/2, -m_margin/2 );
        clipRect = clipRect.intersect( QRect( 0, 0, m_pixmapWidth, m_pixmapHeight ) );
        if ( clipRect.isValid() )
        {
            int flags = PagePainter::Accessibility | PagePainter::Highlights |
                        PagePainter::Annotations;
            PagePainter::paintPageOnPainter( &p, m_page, m_parent->q, flags, m_pixmapWidth, m_pixmapHeight, clipRect );
        }

        if ( !m_visibleRect.isNull() )
        {
            p.save();
            p.setPen( QColor( 255, 255, 0, 200 ) );
            p.setBrush( QColor( 0, 0, 0, 100 ) );
            p.drawRect( m_visibleRect.geometry( m_pixmapWidth, m_pixmapHeight ).adjusted( 0, 0, -1, -1 ) );
            p.restore();
        }

        // draw the bookmark overlay on the top-right corner
        const QPixmap * bookmarkPixmap = m_parent->getBookmarkOverlay();
        if ( isBookmarked && bookmarkPixmap )
        {
            int pixW = bookmarkPixmap->width(),
                pixH = bookmarkPixmap->height();
            clipRect = clipRect.intersect( QRect( m_pixmapWidth - pixW, 0, pixW, pixH ) );
            if ( clipRect.isValid() )
                p.drawPixmap( m_pixmapWidth - pixW, -pixH/8, *bookmarkPixmap );
        }
    }
}


/** ThumbnailsController implementation **/

#define FILTERB_ID  1

ThumbnailController::ThumbnailController( QWidget * parent, ThumbnailList * list )
    : QToolBar( parent )
{
    setObjectName( QLatin1String( "ThumbsControlBar" ) );
    // change toolbar appearance
    setIconSize( QSize( 16, 16 ) );
    setMovable( false );
    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy( QSizePolicy::Minimum );
    setSizePolicy( sp );

    // insert a togglebutton [show only bookmarked pages]
    //insertSeparator();
    QAction * showBoomarkOnlyAction = addAction(
        KIcon( "bookmarks" ), i18n( "Show bookmarked pages only" ) );
    showBoomarkOnlyAction->setCheckable( true );
    connect( showBoomarkOnlyAction, SIGNAL(toggled(bool)), list, SLOT(slotFilterBookmarks(bool)) );
    showBoomarkOnlyAction->setChecked( Okular::Settings::filterBookmarks() );
    //insertLineSeparator();
}


#include "moc_thumbnaillist.cpp"

/* kate: replace-tabs on; indent-width 4; */
