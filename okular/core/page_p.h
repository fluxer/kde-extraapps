/***************************************************************************
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_PAGE_PRIVATE_H_
#define _OKULAR_PAGE_PRIVATE_H_

// qt/kde includes
#include <qlist.h>
#include <qmap.h>
#include <qtransform.h>
#include <qstring.h>
#include <qdom.h>
#include <qcolor.h>
#include <qpixmap.h>

// local includes
#include "global.h"
#include "area.h"

namespace Okular {

class Action;
class Annotation;
class DocumentObserver;
class DocumentPrivate;
class FormField;
class HighlightAreaRect;
class Page;
class PageSize;
class PageTransition;
class RotationJob;
class TextPage;

enum PageItem
{
    None = 0,
    AnnotationPageItems = 0x01,
    FormFieldPageItems = 0x02,
    AllPageItems = 0xff,

    /* If set along with AnnotationPageItems, tells saveLocalContents to save
     * the original annotations (if any) instead of the modified ones */
    OriginalAnnotationPageItems = 0x100
};
Q_DECLARE_FLAGS(PageItems, PageItem)

class PagePrivate
{
    public:
        PagePrivate( Page *page, uint n, double w, double h, Rotation o );
        ~PagePrivate();

        void imageRotationDone( RotationJob * job );
        QTransform rotationMatrix() const;

        /**
         * Loads the local contents (e.g. annotations) of the page.
         */
        void restoreLocalContents( const QDomNode & pageNode );

        /**
         * Saves the local contents (e.g. annotations) of the page.
         */
        void saveLocalContents( QDomNode & parentNode, QDomDocument & document, PageItems what = AllPageItems ) const;

        /**
         * Rotates the image and object rects of the page to the given @p orientation.
         */
        void rotateAt( Rotation orientation );

        /**
         * Changes the size of the page to the given @p size.
         *
         * The @p size is meant to be referred to the page not rotated.
         */
        void changeSize( const PageSize &size );

        /**
         * Sets the @p color and @p areas of text selections.
         */
        void setTextSelections( RegularAreaRect *areas, const QColor & color );

        /**
         * Sets the @p color and @p area of the highlight for the observer with
         * the given @p id.
         */
        void setHighlight( int id, RegularAreaRect *area, const QColor & color );

        /**
         * Deletes all highlight objects for the observer with the given @p id.
         */
        void deleteHighlights( int id = -1 );

        /**
         * Deletes all text selection objects of the page.
         */
        void deleteTextSelections();

        class PixmapObject
        {
            public:
                QPixmap *m_pixmap;
                Rotation m_rotation;
        };
        QMap< DocumentObserver*, PixmapObject > m_pixmaps;

        Page *m_page;
        int m_number;
        Rotation m_orientation;
        double m_width, m_height;
        DocumentPrivate *m_doc;
        NormalizedRect m_boundingBox;
        Rotation m_rotation;

        TextPage * m_text;
        PageTransition * m_transition;
        HighlightAreaRect *m_textSelections;
        QList< FormField * > formfields;
        Action * m_openingAction;
        Action * m_closingAction;
        double m_duration;
        QString m_label;

        bool m_isBoundingBoxKnown : 1;
        QDomDocument restoredLocalAnnotationList; // <annotationList>...</annotationList>
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Okular::PageItems)

#endif
