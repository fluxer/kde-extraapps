/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2006 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "minibar.h"

// qt / kde includes
#include <qapplication.h>
#include <qevent.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QtGui/QToolButton>
#include <qvalidator.h>
#include <qpainter.h>
#include <kicon.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <kacceleratormanager.h>
#include <qtoolbar.h>

// local includes
#include "core/document.h"
#include "core/page.h"

// [private widget] a flat qpushbutton that enlights on hover
class HoverButton : public QToolButton
{
    public:
        HoverButton( QWidget * parent );
};

MiniBarLogic::MiniBarLogic( QObject * parent, Okular::Document * document )
 : QObject(parent)
 , m_document( document )
{
}   

MiniBarLogic::~MiniBarLogic()
{
    m_document->removeObserver( this );
}

void MiniBarLogic::addMiniBar( MiniBar * miniBar )
{
    m_miniBars.insert( miniBar );
}

void MiniBarLogic::removeMiniBar( MiniBar * miniBar )
{
    m_miniBars.remove( miniBar );
}

Okular::Document *MiniBarLogic::document() const
{
    return m_document;
}

int MiniBarLogic::currentPage() const
{
    return m_document->currentPage();
}

void MiniBarLogic::notifySetup( const QVector< Okular::Page * > & pageVector, int setupFlags )
{
    // only process data when document changes
    if ( !( setupFlags & Okular::DocumentObserver::DocumentChanged ) )
        return;

    // if document is closed or has no pages, hide widget
    const int pages = pageVector.count();
    if ( pages < 1 )
    {
        foreach ( MiniBar *miniBar, m_miniBars )
        {
            miniBar->setEnabled( false );
        }
        return;
    }
    
    bool labelsDiffer = false;
    foreach(const Okular::Page * page, pageVector)
    {
        if (!page->label().isEmpty())
        {
            if (page->label().toInt() != (page->number() + 1))
            {
                labelsDiffer = true;
            }
        }
    }
    
    const QString pagesString = QString::number( pages );

    foreach ( MiniBar *miniBar, m_miniBars )
    {
        // resize width of widgets
        miniBar->resizeForPage( pages );
            
        // update child widgets
        miniBar->m_pageLabelEdit->setPageLabels( pageVector );
        miniBar->m_pageNumberEdit->setPagesNumber( pages );
        miniBar->m_pagesButton->setText( pagesString );
        miniBar->m_prevButton->setEnabled( false );
        miniBar->m_nextButton->setEnabled( false );
        miniBar->m_pageLabelEdit->setVisible( labelsDiffer );
        miniBar->m_pageNumberLabel->setVisible( labelsDiffer );
        miniBar->m_pageNumberEdit->setVisible( !labelsDiffer );

        miniBar->resize( miniBar->minimumSizeHint() );

        miniBar->setEnabled( true );
    }
}

void MiniBarLogic::notifyCurrentPageChanged( int previousPage, int currentPage )
{
    Q_UNUSED( previousPage )

    // get current page number
    const int pages = m_document->pages();

    // if the document is opened and page is changed
    if ( pages > 0 )
    {
        const QString pageNumber = QString::number( currentPage + 1 );
        const QString pageLabel = m_document->page( currentPage )->label();

        foreach ( MiniBar *miniBar, m_miniBars )
        {
            // update prev/next button state
            miniBar->m_prevButton->setEnabled( currentPage > 0 );
            miniBar->m_nextButton->setEnabled( currentPage < ( pages - 1 ) );
            // update text on widgets
            miniBar->m_pageNumberEdit->setText( pageNumber );
            miniBar->m_pageNumberLabel->setText( pageNumber );
            miniBar->m_pageLabelEdit->setText( pageLabel );
        }
    }
}

/** MiniBar **/

MiniBar::MiniBar( QWidget * parent, MiniBarLogic * miniBarLogic )
    : QWidget( parent )
    , m_miniBarLogic( miniBarLogic )
    , m_oldToobarParent( 0 )
{
    setObjectName( QLatin1String( "miniBar" ) );
    
    m_miniBarLogic->addMiniBar( this );

    QHBoxLayout * horLayout = new QHBoxLayout( this );

    horLayout->setMargin( 0 );
    horLayout->setSpacing( 3 );

    QSize buttonSize( KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium );
    // bottom: left prev_page button
    m_prevButton = new HoverButton( this );
    m_prevButton->setIcon( KIcon( layoutDirection() == Qt::RightToLeft ? "arrow-right" : "arrow-left" ) );
    m_prevButton->setIconSize( buttonSize );
    horLayout->addWidget( m_prevButton );
    // bottom: left lineEdit (current page box)
    m_pageNumberEdit = new PageNumberEdit( this );
    horLayout->addWidget( m_pageNumberEdit );
    m_pageNumberEdit->installEventFilter( this );
    // bottom: left labelWidget (current page label)
    m_pageLabelEdit = new PageLabelEdit( this );
    horLayout->addWidget(m_pageLabelEdit);
    m_pageLabelEdit->installEventFilter( this );
    // bottom: left labelWidget (current page label)
    m_pageNumberLabel = new QLabel( this );
    m_pageNumberLabel->setAlignment( Qt::AlignCenter );
    horLayout->addWidget(m_pageNumberLabel);
    // bottom: central 'of' label
    horLayout->addSpacing(5);
    horLayout->addWidget( new QLabel( i18nc( "Layouted like: '5 [pages] of 10'", "of" ), this ) );
    // bottom: right button
    m_pagesButton = new HoverButton( this );
    horLayout->addWidget( m_pagesButton );
    // bottom: right next_page button
    m_nextButton = new HoverButton( this );
    m_nextButton->setIcon( KIcon( layoutDirection() == Qt::RightToLeft ? "arrow-left" : "arrow-right" ) );
    m_nextButton->setIconSize( buttonSize );
    horLayout->addWidget( m_nextButton );

    QSizePolicy sp = sizePolicy();
    sp.setHorizontalPolicy( QSizePolicy::Fixed );
    sp.setVerticalPolicy( QSizePolicy::Fixed );
    setSizePolicy( sp );

    // resize width of widgets
    resizeForPage( 0 );

    // connect signals from child widgets to internal handlers / signals bouncers
    connect( m_pageNumberEdit, SIGNAL(returnPressed()), this, SLOT(slotChangePage()) );
    connect( m_pageLabelEdit, SIGNAL(pageNumberChosen(int)), this, SLOT(slotChangePage(int)) );
    connect( m_pagesButton, SIGNAL(clicked()), this, SIGNAL(gotoPage()) );
    connect( m_prevButton, SIGNAL(clicked()), this, SIGNAL(prevPage()) );
    connect( m_nextButton, SIGNAL(clicked()), this, SIGNAL(nextPage()) );

    resize( minimumSizeHint() );

    // widget starts disabled (will be enabled after opening a document)
    setEnabled( false );
}

MiniBar::~MiniBar()
{
    m_miniBarLogic->removeMiniBar( this );
}

void MiniBar::changeEvent( QEvent * event )
{
    if ( event->type() == QEvent::ParentChange )
    {
        QToolBar *tb = dynamic_cast<QToolBar*>( parent() );
        if ( tb != m_oldToobarParent )
        {
            if ( m_oldToobarParent )
            {
                disconnect( m_oldToobarParent, SIGNAL(iconSizeChanged(QSize)), this, SLOT(slotToolBarIconSizeChanged()));
            }
            m_oldToobarParent = tb;
            if ( tb )
            {
                connect( tb, SIGNAL(iconSizeChanged(QSize)), this, SLOT(slotToolBarIconSizeChanged()));
                slotToolBarIconSizeChanged();
            }
        }
    }
}

bool MiniBar::eventFilter( QObject *target, QEvent *event )
{
    if ( target == m_pageNumberEdit || target == m_pageLabelEdit )
    {
        if ( event->type() == QEvent::KeyPress )
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
            int key = keyEvent->key();
            if ( key == Qt::Key_PageUp || key == Qt::Key_PageDown || key == Qt::Key_Up || key == Qt::Key_Down )
            {
                emit forwardKeyPressEvent( keyEvent );
                return true;
            }
        }
    }
    return false;
}

void MiniBar::slotChangePage()
{
    // get text from the lineEdit
    const QString pageNumber = m_pageNumberEdit->text();

    // convert it to page number and go to that page
    bool ok;
    int number = pageNumber.toInt( &ok ) - 1;
    if ( ok && number >= 0 && number < (int)m_miniBarLogic->document()->pages() &&
         number != m_miniBarLogic->currentPage() )
    {
        slotChangePage( number );
    }
}

void MiniBar::slotChangePage( int pageNumber )
{
    m_miniBarLogic->document()->setViewportPage( pageNumber );
    m_pageNumberEdit->clearFocus();
    m_pageLabelEdit->clearFocus();
}

void MiniBar::slotEmitNextPage()
{
    // emit signal
    nextPage();
}

void MiniBar::slotEmitPrevPage()
{
    // emit signal
    prevPage();
}

void MiniBar::slotToolBarIconSizeChanged()
{
    const QSize buttonSize = m_oldToobarParent->iconSize();
    m_prevButton->setIconSize( buttonSize );
    m_nextButton->setIconSize( buttonSize );
}

void MiniBar::resizeForPage( int pages )
{
    int numberWidth = 10 + fontMetrics().width( QString::number( pages ) );
    m_pageNumberEdit->setMinimumWidth( numberWidth );
    m_pageNumberEdit->setMaximumWidth( 2 * numberWidth );
    m_pageLabelEdit->setMinimumWidth( numberWidth );
    m_pageLabelEdit->setMaximumWidth( 2 * numberWidth );
    m_pageNumberLabel->setMinimumWidth( numberWidth );
    m_pageNumberLabel->setMaximumWidth( 2 * numberWidth );
    m_pagesButton->setMinimumWidth( numberWidth );
    m_pagesButton->setMaximumWidth( 2 * numberWidth );
}



/** ProgressWidget **/

ProgressWidget::ProgressWidget( QWidget * parent, Okular::Document * document )
    : QWidget( parent ), m_document( document ),
    m_progressPercentage( -1 )
{
    setObjectName( QLatin1String( "progress" ) );
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setFixedHeight( 4 );
    setMouseTracking( true );
}

ProgressWidget::~ProgressWidget()
{
    m_document->removeObserver( this );
}

void ProgressWidget::notifyCurrentPageChanged( int previousPage, int currentPage )
{
    Q_UNUSED( previousPage )

    // get current page number
    int pages = m_document->pages();

    // if the document is opened and page is changed
    if ( pages > 0 )
    {
        // update percentage
        const float percentage = pages < 2 ? 1.0 : (float)currentPage / (float)(pages - 1);
        setProgress( percentage );
    }
}

void ProgressWidget::setProgress( float percentage )
{
    m_progressPercentage = percentage;
    update();
}

void ProgressWidget::slotGotoNormalizedPage( float index )
{
    // figure out page number and go to that page
    int number = (int)( index * (float)m_document->pages() );
    if ( number >= 0 && number < (int)m_document->pages() &&
         number != (int)m_document->currentPage() )
        m_document->setViewportPage( number );
}

void ProgressWidget::mouseMoveEvent( QMouseEvent * e )
{
    if ( ( QApplication::mouseButtons() & Qt::LeftButton ) && width() > 0 )
        slotGotoNormalizedPage( (float)( QApplication::isRightToLeft() ? width() - e->x() : e->x() ) / (float)width() );
}

void ProgressWidget::mousePressEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton && width() > 0 )
        slotGotoNormalizedPage( (float)( QApplication::isRightToLeft() ? width() - e->x() : e->x() ) / (float)width() );
}

void ProgressWidget::wheelEvent( QWheelEvent * e )
{
    if ( e->delta() > 0 )
        emit nextPage();
    else
        emit prevPage();
}

void ProgressWidget::paintEvent( QPaintEvent * e )
{
    QPainter p( this );

    if ( m_progressPercentage < 0.0 )
    {
        p.fillRect( rect(), palette().color( QPalette::Active, QPalette::HighlightedText ) );
        return;
    }

    // find out the 'fill' and the 'clear' rectangles
    int w = width(),
        h = height(),
        l = (int)( (float)w * m_progressPercentage );
    QRect cRect = ( QApplication::isRightToLeft() ? QRect( 0, 0, w - l, h ) : QRect( l, 0, w - l, h ) ).intersect( e->rect() );
    QRect fRect = ( QApplication::isRightToLeft() ? QRect( w - l, 0, l, h ) : QRect( 0, 0, l, h ) ).intersect( e->rect() );

    QPalette pal = palette();
    // paint clear rect
    if ( cRect.isValid() )
        p.fillRect( cRect, pal.color( QPalette::Active, QPalette::HighlightedText ) );
    // draw a frame-like outline
    //p.setPen( palette().active().mid() );
    //p.drawRect( 0,0, w, h );
    // paint fill rect
    if ( fRect.isValid() )
        p.fillRect( fRect, pal.color( QPalette::Active, QPalette::Highlight ) );
    if ( l && l != w  )
    {
        p.setPen( pal.color( QPalette::Active, QPalette::Highlight ).dark( 120 ) );
        int delta = QApplication::isRightToLeft() ? w - l : l;
        p.drawLine( delta, 0, delta, h );
    }
}

/** PageLabelEdit **/

PageLabelEdit::PageLabelEdit( MiniBar * parent )
    : PagesEdit( parent )
{
    setVisible( false );
    connect( this, SIGNAL(returnPressed()), this, SLOT(pageChosen()) );
}

void PageLabelEdit::setText( const QString & newText )
{
    m_lastLabel = newText;
    PagesEdit::setText( newText );
}

void PageLabelEdit::setPageLabels( const QVector<Okular::Page *> &pageVector )
{
    m_labelPageMap.clear();
    completionObject()->clear();
    foreach(const Okular::Page * page, pageVector)
    {
        if ( !page->label().isEmpty() )
        {
            m_labelPageMap.insert( page->label(), page->number() );
            bool ok;
            page->label().toInt( &ok );
            if ( !ok )
            {
                // Only add to the completion objects labels that are not numbers
                completionObject()->addItem( page->label() );
            }
        }
    }
}

void PageLabelEdit::pageChosen()
{
    const QString newInput = text();
    const int pageNumber = m_labelPageMap.value( newInput, -1 );
    if (pageNumber != -1)
    {
        emit pageNumberChosen( pageNumber );
    }
    else
    {
        setText( m_lastLabel );
    }
}

/** PageNumberEdit **/

PageNumberEdit::PageNumberEdit( MiniBar * miniBar )
    : PagesEdit( miniBar )
{
    // use an integer validator
    m_validator = new QIntValidator( 1, 1, this );
    setValidator( m_validator );
}

void PageNumberEdit::setPagesNumber( int pages )
{
    m_validator->setTop( pages );
}


/** PagesEdit **/

PagesEdit::PagesEdit( MiniBar * parent )
    : KLineEdit( parent ), m_miniBar( parent ), m_eatClick( false )
{

    // customize text properties
    setAlignment( Qt::AlignCenter );

    // send a focus out event
    QFocusEvent fe( QEvent::FocusOut );
    QApplication::sendEvent( this, &fe );

    connect( KGlobalSettings::self(), SIGNAL(appearanceChanged()), this, SLOT(updatePalette()) );
}

void PagesEdit::setText( const QString & newText )
{
    // call default handler if hasn't focus
    if ( !hasFocus() )
    {
        KLineEdit::setText( newText );
    }
    // else preserve existing selection
    else
    {
        // save selection and adapt it to the new text length
        int selectionLength = selectedText().length();
        const bool allSelected = ( selectionLength == text().length() );
        if ( allSelected )
        {
            KLineEdit::setText( newText );
            selectAll();
        }
        else
        {
            int newSelectionStart = newText.length() - text().length() + selectionStart();
            if ( newSelectionStart < 0 )
            {
                // the new text is shorter than the old one, and the front part, which is "cut off", is selected
                // shorten the selection accordingly
                selectionLength += newSelectionStart;
                newSelectionStart = 0;
            }
            KLineEdit::setText( newText );
            setSelection( newSelectionStart, selectionLength );
        }
    }
}

void PagesEdit::updatePalette()
{
    QPalette pal;

    if ( hasFocus() )
        pal.setColor( QPalette::Active, QPalette::Base, QApplication::palette().color( QPalette::Active, QPalette::Base ) );
    else
        pal.setColor( QPalette::Base, QApplication::palette().color( QPalette::Base ).dark( 102 ) );

    setPalette( pal );
}

void PagesEdit::focusInEvent( QFocusEvent * e )
{
    // select all text
    selectAll();
    if ( e->reason() == Qt::MouseFocusReason )
        m_eatClick = true;
    // change background color to the default 'edit' color
    updatePalette();
    // call default handler
    KLineEdit::focusInEvent( e );
}

void PagesEdit::focusOutEvent( QFocusEvent * e )
{
    // change background color to a dark tone
    updatePalette();
    // call default handler
    KLineEdit::focusOutEvent( e );
}

void PagesEdit::mousePressEvent( QMouseEvent * e )
{
    // if this click got the focus in, don't process the event
    if ( !m_eatClick )
        KLineEdit::mousePressEvent( e );
    m_eatClick = false;
}

void PagesEdit::wheelEvent( QWheelEvent * e )
{
    if ( e->delta() > 0 )
        m_miniBar->slotEmitNextPage();
    else
        m_miniBar->slotEmitPrevPage();
}


/** HoverButton **/

HoverButton::HoverButton( QWidget * parent )
    : QToolButton( parent )
{
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    KAcceleratorManager::setNoAccel( this );
}

#include "minibar.moc"

/* kate: replace-tabs on; indent-width 4; */
