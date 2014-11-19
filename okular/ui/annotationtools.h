/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_ANNOTATIONTOOLS_H_
#define _OKULAR_ANNOTATIONTOOLS_H_

#include <qdom.h>
#include <qlinkedlist.h>
#include <qpen.h>
#include <qrect.h>

#include "core/area.h"

class QMouseEvent;
class QTabletEvent;
class QPainter;
class PageViewItem;
namespace Okular {
class Annotation;
class Page;
}

/**
 * @short Engine: filter events to distill Annotations.
 */
class AnnotatorEngine
{
    public:
        AnnotatorEngine( const QDomElement & engineElement );
        virtual ~AnnotatorEngine();

        // enum definitions
        enum EventType { Press, Move, Release };
        enum Button { None, Left, Right };

        // perform operations
        virtual QRect event( EventType type, Button button, double nX, double nY, double xScale, double yScale, const Okular::Page * page ) = 0;
        virtual void paint( QPainter * painter, double xScale, double yScale, const QRect & clipRect ) = 0;
        virtual QList< Okular::Annotation* > end() = 0;

        // query creation state
        //PageViewItem * editingItem() const { return m_lockedItem; }
        bool creationCompleted() const { return m_creationCompleted; }

        void setItem( PageViewItem * item ) { m_item = item; }

        static void decodeEvent( const QMouseEvent * mouseEvent, EventType * eventType, Button * button );
        static void decodeEvent( const QTabletEvent * tabletEvent, EventType * eventType, Button * button );

        virtual QCursor cursor() const;

    protected:
        PageViewItem * item() { return m_item; }

        // common engine attributes (the element includes annotation desc)
        QDomElement m_engineElement;
        QDomElement m_annotElement;
        QColor m_engineColor;
        // other vars (remove this!)
        bool m_creationCompleted;

    private:
        PageViewItem * m_item;
};

class SmoothPath
{
    public:
        SmoothPath( const QLinkedList<Okular::NormalizedPoint> &points, const QPen &pen );
        void paint( QPainter * painter, double xScale, double yScale ) const;

    private:
        const QLinkedList<Okular::NormalizedPoint> points;
        const QPen pen;
};

/** @short SmoothPathEngine */
class SmoothPathEngine
    : public AnnotatorEngine
{
    public:
        SmoothPathEngine( const QDomElement & engineElement );

        QRect event( EventType type, Button button, double nX, double nY, double xScale, double yScale, const Okular::Page * /*page*/ );

        void paint( QPainter * painter, double xScale, double yScale, const QRect & /*clipRect*/ );

        // These are two alternative ways to get the resulting path. Don't call them both!
        QList< Okular::Annotation* > end();
        SmoothPath endSmoothPath();

    private:
        // data
        QLinkedList<Okular::NormalizedPoint> points;
        Okular::NormalizedRect totalRect;
        Okular::NormalizedPoint lastPoint;
};

#endif

/* kate: replace-tabs on; indent-width 4; */
