/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "searchwidget.h"

// qt/kde includes
#include <qlayout.h>
#include <qmenu.h>
#include <qaction.h>
#include <qsizepolicy.h>
#include <qtoolbutton.h>
#include <kicon.h>
#include <klocale.h>

// local includes
#include "searchlineedit.h"

SearchWidget::SearchWidget( QWidget * parent, Okular::Document * document )
    : QWidget( parent )
{
    setObjectName( QLatin1String( "iSearchBar" ) );

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy( QSizePolicy::Minimum );
    setSizePolicy( sp );

    QHBoxLayout * mainlay = new QHBoxLayout( this );
    mainlay->setMargin( 0 );
    mainlay->setSpacing( 3 );

    // 2. text line
    m_lineEdit = new SearchLineEdit( this, document );
    m_lineEdit->setClearButtonShown( true );
    m_lineEdit->setToolTip(i18n( "Enter at least 3 letters to filter pages" ));
    m_lineEdit->setSearchCaseSensitivity( Qt::CaseInsensitive );
    m_lineEdit->setSearchMinimumLength( 3 );
    m_lineEdit->setSearchType( Okular::Document::GoogleAll );
    m_lineEdit->setSearchId( SW_SEARCH_ID );
    m_lineEdit->setSearchColor( qRgb( 0, 183, 255 ) );
    mainlay->addWidget( m_lineEdit );

    // 3.1. create the popup menu for changing filtering features
    m_menu = new QMenu( this );
    m_caseSensitiveAction = m_menu->addAction( i18n("Case Sensitive") );
    m_menu->addSeparator();
    m_matchPhraseAction = m_menu->addAction( i18n("Match Phrase") );
    m_marchAllWordsAction = m_menu->addAction( i18n("Match All Words") );
    m_marchAnyWordsAction = m_menu->addAction( i18n("Match Any Word") );

    m_caseSensitiveAction->setCheckable( true );
    QActionGroup *actgrp = new QActionGroup( this );
    m_matchPhraseAction->setCheckable( true );
    m_matchPhraseAction->setActionGroup( actgrp );
    m_marchAllWordsAction->setCheckable( true );
    m_marchAllWordsAction->setActionGroup( actgrp );
    m_marchAnyWordsAction->setCheckable( true );
    m_marchAnyWordsAction->setActionGroup( actgrp );

    m_marchAllWordsAction->setChecked( true );
    connect( m_menu, SIGNAL(triggered(QAction*)), SLOT(slotMenuChaged(QAction*)) );

    // 3.2. create the toolbar button that spawns the popup menu
    QToolButton *optionsMenuAction =  new QToolButton( this );
    mainlay->addWidget( optionsMenuAction );
    optionsMenuAction->setAutoRaise( true );
    optionsMenuAction->setIcon( KIcon( "view-filter" ) );
    optionsMenuAction->setToolTip( i18n( "Filter Options" ) );
    optionsMenuAction->setPopupMode( QToolButton::InstantPopup );
    optionsMenuAction->setMenu( m_menu );
}

void SearchWidget::clearText()
{
    m_lineEdit->clear();
}

void SearchWidget::slotMenuChaged( QAction * act )
{
    // update internal variables and checked state
    if ( act == m_caseSensitiveAction )
    {
        m_lineEdit->setSearchCaseSensitivity( m_caseSensitiveAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive );
    }
    else if ( act == m_matchPhraseAction )
    {
        m_lineEdit->setSearchType( Okular::Document::AllDocument );
    }
    else if ( act == m_marchAllWordsAction )
    {
        m_lineEdit->setSearchType( Okular::Document::GoogleAll );
    }
    else if ( act == m_marchAnyWordsAction )
    {
        m_lineEdit->setSearchType( Okular::Document::GoogleAny );
    }
    else
        return;

    // update search
    m_lineEdit->restartSearch();
}

#include "searchwidget.moc"
