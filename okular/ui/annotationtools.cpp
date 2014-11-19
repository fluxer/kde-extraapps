/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "annotationtools.h"

// qt / kde includes
#include <qcolor.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>

// local includes
#include "core/annotations.h"

AnnotatorEngine::AnnotatorEngine( const QDomElement & engineElement )
    : m_engineElement( engineElement ), m_creationCompleted( false ), m_item( 0 )
{
    // parse common engine attributes
    if ( engineElement.hasAttribute( "color" ) )
        m_engineColor = QColor( engineElement.attribute( "color" ) );

    // get the annotation element
    QDomElement annElement = m_engineElement.firstChild().toElement();
    if ( !annElement.isNull() && annElement.tagName() == "annotation" )
        m_annotElement = annElement;
}

void AnnotatorEngine::decodeEvent( const QMouseEvent * mouseEvent, EventType * eventType, Button * button )
{
    *eventType = AnnotatorEngine::Press;
    if ( mouseEvent->type() == QEvent::MouseMove )
        *eventType = AnnotatorEngine::Move;
    else if ( mouseEvent->type() == QEvent::MouseButtonRelease )
        *eventType = AnnotatorEngine::Release;

    *button = AnnotatorEngine::None;
    const Qt::MouseButtons buttonState = ( *eventType == AnnotatorEngine::Move ) ? mouseEvent->buttons() : mouseEvent->button();
    if ( buttonState == Qt::LeftButton )
        *button = AnnotatorEngine::Left;
    else if ( buttonState == Qt::RightButton )
        *button = AnnotatorEngine::Right;
}

void AnnotatorEngine::decodeEvent( const QTabletEvent * tabletEvent, EventType * eventType, Button * button )
{
    switch ( tabletEvent->type() )
    {
        case QEvent::TabletPress:
            // Tablet press event is equivalent to pressing the left mouse button
            *button = AnnotatorEngine::Left;
            *eventType = AnnotatorEngine::Press;
            break;
        case QEvent::TabletRelease:
            // Tablet release event is equivalent to releasing the left mouse button
            *button = AnnotatorEngine::Left;
            *eventType = AnnotatorEngine::Release;
            break;
        case QEvent::TabletMove:
            // Tablet events are only routed if the pen is down so
            // this is equivalent to the left mouse button being pressed
            *button = AnnotatorEngine::Left;
            *eventType = AnnotatorEngine::Move;
            break;
        default:
            Q_ASSERT(false);
            break;
    }
}

AnnotatorEngine::~AnnotatorEngine()
{
}

QCursor AnnotatorEngine::cursor() const
{
    return Qt::CrossCursor;
}

SmoothPath::SmoothPath( const QLinkedList<Okular::NormalizedPoint> &points, const QPen &pen )
    : points ( points ), pen ( pen )
{
}

/** SmoothPathEngine */
SmoothPathEngine::SmoothPathEngine( const QDomElement & engineElement )
    : AnnotatorEngine( engineElement )
{
    // parse engine specific attributes
}

QRect SmoothPathEngine::event( EventType type, Button button, double nX, double nY, double xScale, double yScale, const Okular::Page * /*page*/ )
{
    // only proceed if pressing left button
    if ( button != Left )
        return QRect();

    // start operation
    if ( type == Press && points.isEmpty() )
    {
        lastPoint.x = nX;
        lastPoint.y = nY;
        totalRect.left = totalRect.right = lastPoint.x;
        totalRect.top = totalRect.bottom = lastPoint.y;
        points.append( lastPoint );
    }
    // add a point to the path
    else if ( type == Move && points.count() > 0 )
    {
        //double dist = hypot( nX - points.last().x, nY - points.last().y );
        //if ( dist > 0.0001 )
        //{
            // append mouse position (as normalized point) to the list
            Okular::NormalizedPoint nextPoint = Okular::NormalizedPoint( nX, nY );
            points.append( nextPoint );
            // update total rect
            double dX = 2.0 / (double)xScale;
            double dY = 2.0 / (double)yScale;
            totalRect.left = qMin( totalRect.left, nX - dX );
            totalRect.top = qMin( totalRect.top, nY - dY );
            totalRect.right = qMax( nX + dX, totalRect.right );
            totalRect.bottom = qMax( nY + dY, totalRect.bottom );
            // paint the difference to previous full rect
            Okular::NormalizedRect incrementalRect;
            incrementalRect.left = qMin( nextPoint.x, lastPoint.x ) - dX;
            incrementalRect.right = qMax( nextPoint.x, lastPoint.x ) + dX;
            incrementalRect.top = qMin( nextPoint.y, lastPoint.y ) - dY;
            incrementalRect.bottom = qMax( nextPoint.y, lastPoint.y ) + dY;
            lastPoint = nextPoint;
            return incrementalRect.geometry( (int)xScale, (int)yScale );
        //}
    }
    // terminate process
    else if ( type == Release && points.count() > 0 )
    {
        if ( points.count() < 2 )
            points.clear();
        else
            m_creationCompleted = true;
        return totalRect.geometry( (int)xScale, (int)yScale );
    }
    return QRect();
}

void SmoothPathEngine::paint( QPainter * painter, double xScale, double yScale, const QRect & /*clipRect*/ )
{
    // use engine's color for painting
    const SmoothPath path( points, QPen(m_engineColor, 1) );

    // draw the path
    path.paint( painter, xScale, yScale );
}

void SmoothPath::paint( QPainter * painter, double xScale, double yScale ) const
{
    // draw SmoothPaths with at least 2 points
    if ( points.count() > 1 )
    {
        painter->setPen( pen );

        QLinkedList<Okular::NormalizedPoint>::const_iterator pIt = points.begin(), pEnd = points.end();
        Okular::NormalizedPoint pA = *pIt;
        ++pIt;
        for ( ; pIt != pEnd; ++pIt )
        {
            Okular::NormalizedPoint pB = *pIt;
            painter->drawLine( (int)(pA.x * (double)xScale), (int)(pA.y * (double)yScale),
                               (int)(pB.x * (double)xScale), (int)(pB.y * (double)yScale) );
            pA = pB;
        }
    }
}

QList< Okular::Annotation* > SmoothPathEngine::end()
{
    m_creationCompleted = false;

    // find out annotation's description node
    if ( m_annotElement.isNull() )
                return QList< Okular::Annotation* >();

    // find out annotation's type
    Okular::Annotation * ann = 0;
    QString typeString = m_annotElement.attribute( "type" );

    // create InkAnnotation from path
    if ( typeString == "Ink" )
    {
        Okular::InkAnnotation * ia = new Okular::InkAnnotation();
        ann = ia;
        if ( m_annotElement.hasAttribute( "width" ) )
            ann->style().setWidth( m_annotElement.attribute( "width" ).toDouble() );
        // fill points
        QList< QLinkedList<Okular::NormalizedPoint> > list = ia->inkPaths();
        list.append( points );
        ia->setInkPaths( list );
        // set boundaries
        ia->setBoundingRectangle( totalRect );
    }

    // safety check
    if ( !ann )
        return QList< Okular::Annotation* >();

    // set common attributes
    ann->style().setColor( m_annotElement.hasAttribute( "color" ) ?
        m_annotElement.attribute( "color" ) : m_engineColor );
    if ( m_annotElement.hasAttribute( "opacity" ) )
        ann->style().setOpacity( m_annotElement.attribute( "opacity", "1.0" ).toDouble() );

    // return annotation
    return QList< Okular::Annotation* >() << ann;
}

SmoothPath SmoothPathEngine::endSmoothPath()
{
    m_creationCompleted = false;

    double width = 1;
    if ( m_annotElement.hasAttribute( "width" ) )
        width = m_annotElement.attribute( "width" ).toDouble();

    QColor color( m_annotElement.hasAttribute( "color" ) ?
        m_annotElement.attribute( "color" ) : m_engineColor );

    return SmoothPath( points, QPen(color, width) );
}

/* kate: replace-tabs on; indent-width 4; */
