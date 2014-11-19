/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2007, 2009-2010 by Pino Toscano <pino@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_SEARCHLINEEDIT_H_
#define _OKULAR_SEARCHLINEEDIT_H_

#include "core/document.h"

#include <klineedit.h>

class QTimer;
class KPixmapSequenceWidget;

/**
 * @short A line edit for find-as-you-type search. Outputs to the Document.
 */
class SearchLineEdit : public KLineEdit
{
    Q_OBJECT
    public:
        SearchLineEdit( QWidget *parent, Okular::Document *document );

        void clearText();

        void setSearchCaseSensitivity( Qt::CaseSensitivity cs );
        void setSearchMinimumLength( int length );
        void setSearchType( Okular::Document::SearchType type );
        void setSearchId( int id );
        void setSearchColor( const QColor &color );
        void setSearchMoveViewport( bool move );
        void setSearchFromStart( bool fromStart );
        void resetSearch();

        bool isSearchRunning() const;

    signals:
        void searchStarted();
        void searchStopped();

    public slots:
        void restartSearch();
        void stopSearch();
        void findNext();
        void findPrev();

    private:
        void prepareLineEditForSearch();

        Okular::Document * m_document;
        QTimer * m_inputDelayTimer;
        int m_minLength;
        Qt::CaseSensitivity m_caseSensitivity;
        Okular::Document::SearchType m_searchType;
        int m_id;
        QColor m_color;
        bool m_moveViewport;
        bool m_changed;
        bool m_fromStart;
        bool m_searchRunning;

    private slots:
        void slotTextChanged( const QString & text );
        void slotReturnPressed( const QString &text );
        void startSearch();
        void searchFinished( int id, Okular::Document::SearchStatus endStatus );
};

class SearchLineWidget : public QWidget
{
    Q_OBJECT
    public:
        SearchLineWidget( QWidget *parent, Okular::Document *document );

        SearchLineEdit* lineEdit() const;

    private slots:
        void slotSearchStarted();
        void slotSearchStopped();
        void slotTimedout();

    private:
        SearchLineEdit *m_edit;
        KPixmapSequenceWidget* m_anim;
        QTimer *m_timer;
};

#endif
