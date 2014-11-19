/***************************************************************************
 *   Copyright (C) 2006 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "pageitemdelegate.h"

// qt/kde includes
#include <qapplication.h>
#include <QItemDelegate>
#include <QModelIndex>
#include <QTextDocument>
#include <qvariant.h>

// local includes
#include "settings.h"

#define PAGEITEMDELEGATE_INTERNALMARGIN 3

class PageItemDelegate::Private
{
    public:
        Private()
        {}

        QModelIndex index;
};

PageItemDelegate::PageItemDelegate( QObject * parent )
    : QItemDelegate( parent ), d( new Private )
{
}

PageItemDelegate::~PageItemDelegate()
{
    delete d;
}

void PageItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    d->index = index;
    QItemDelegate::paint( painter, option, index );
}

void PageItemDelegate::drawDisplay( QPainter *painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text ) const
{
    QVariant pageVariant = d->index.data( PageRole );
    QVariant labelVariant = d->index.data( PageLabelRole );
    if ( ( labelVariant.type() != QVariant::String && !pageVariant.canConvert( QVariant::String ) ) || !Okular::Settings::tocPageColumn() )
    {
        QItemDelegate::drawDisplay( painter, option, rect, text );
        return;
    }
    QString label = labelVariant.toString();
    QString page = label.isEmpty() ? pageVariant.toString() : label;
    QTextDocument document;
    document.setPlainText( page );
    document.setDefaultFont( option.font );
    int margindelta = QApplication::style()->pixelMetric( QStyle::PM_FocusFrameHMargin ) + 1;
    int pageRectWidth = (int)document.size().width();
    QRect newRect( rect );
    QRect pageRect( rect );
    pageRect.setWidth( pageRectWidth + 2 * margindelta );
    newRect.setWidth( newRect.width() - pageRectWidth - PAGEITEMDELEGATE_INTERNALMARGIN );
    if ( option.direction == Qt::RightToLeft )
        newRect.translate( pageRectWidth + PAGEITEMDELEGATE_INTERNALMARGIN, 0 );
    else
        pageRect.translate( newRect.width() + PAGEITEMDELEGATE_INTERNALMARGIN - 2 * margindelta, 0 );
    QItemDelegate::drawDisplay( painter, option, newRect, text );
    QStyleOptionViewItemV2 newoption( option );
    newoption.displayAlignment = ( option.displayAlignment & ~Qt::AlignHorizontal_Mask ) | Qt::AlignRight;
    QItemDelegate::drawDisplay( painter, newoption, pageRect, page );
}

#include "pageitemdelegate.moc"
