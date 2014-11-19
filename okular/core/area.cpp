/***************************************************************************
 *   Copyright (C) 2004-05 by Enrico Ros <eros.kde@email.it>               *
 *   Copyright (C) 2005 by Piotr Szymanski <niedakh@gmail.com>             *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "area.h"

#include <QtCore/QRect>
#include <QtGui/QPolygonF>
#include <kdebug.h>

#include <math.h>

#include "action.h"
#include "annotations.h"
#include "annotations_p.h"
#include "debug_p.h"
#include "sourcereference.h"

using namespace Okular;

/** class NormalizedPoint **/
NormalizedPoint::NormalizedPoint()
    : x( 0.0 ), y( 0.0 ) {}

NormalizedPoint::NormalizedPoint( double dX, double dY )
    : x( dX ), y( dY ) {}

NormalizedPoint::NormalizedPoint( int iX, int iY, int xScale, int yScale )
    : x( (double)iX / (double)xScale ), y( (double)iY / (double)yScale ) {}

NormalizedPoint& NormalizedPoint::operator=( const NormalizedPoint & p )
{
    x = p.x;
    y = p.y;
    return *this;
}

void NormalizedPoint::transform( const QTransform &matrix )
{
    qreal tmp_x = (qreal)x;
    qreal tmp_y = (qreal)y;
    matrix.map( tmp_x, tmp_y, &tmp_x, &tmp_y );
    x = tmp_x;
    y = tmp_y;
}

double NormalizedPoint::distanceSqr( double x, double y, double xScale, double yScale ) const
{
    return pow( (this->x - x) * xScale, 2 ) + pow( (this->y - y) * yScale, 2 );
}

/**
 * Returns a vector from the given points @p a and @p b
 * @internal
 */
NormalizedPoint operator-( const NormalizedPoint& a, const NormalizedPoint& b )
{
    return NormalizedPoint( a.x - b.x, a.y - b.y );
}

/**
 * @brief Calculates distance of the point @p x @p y @p xScale @p yScale to the line segment from @p start to @p end
 */
double NormalizedPoint::distanceSqr( double x, double y, double xScale, double yScale, const NormalizedPoint& start, const NormalizedPoint& end )
{
    NormalizedPoint point( x, y );
    double thisDistance;
    NormalizedPoint lineSegment( end - start );
    const double lengthSqr = pow( lineSegment.x, 2 ) + pow( lineSegment.y, 2 );

    //if the length of the current segment is null, we can just
    //measure the distance to either end point
    if ( lengthSqr == 0.0 )
    {
        thisDistance = end.distanceSqr( x, y, xScale, yScale );
    }
    else
    {
        //vector from the start point of the current line segment to the measurement point
        NormalizedPoint a = point - start;
        //vector from the same start point to the end point of the current line segment
        NormalizedPoint b = end - start;

        //we're using a * b (dot product) := |a| * |b| * cos(phi) and the knowledge
        //that cos(phi) is adjacent side / hypotenuse (hypotenuse = |b|)
        //therefore, t becomes the length of the vector that represents the projection of
        //the point p onto the current line segment
        //(hint: if this is still unclear, draw it!)
        float t = (a.x * b.x + a.y * b.y) / lengthSqr;

        if ( t < 0 )
        {
            //projection falls outside the line segment on the side of "start"
            thisDistance = point.distanceSqr( start.x, start.y, xScale, yScale );
        }
        else if ( t > 1 )
        {
            //projection falls outside the line segment on the side of the current point
            thisDistance = point.distanceSqr( end.x, end.y, xScale, yScale );
        }
        else
        {
            //projection is within [start, *i];
            //determine the length of the perpendicular distance from the projection to the actual point
            NormalizedPoint direction = end - start;
            NormalizedPoint projection = start - NormalizedPoint( -t * direction.x, -t * direction.y );
            thisDistance = projection.distanceSqr( x, y, xScale, yScale );
        }
    }
    return thisDistance;
}

QDebug operator<<( QDebug str, const Okular::NormalizedPoint& p )
{
    str.nospace() << "NormPt(" << p.x << "," << p.y << ")";
    return str.space();
}

/** class NormalizedRect **/

NormalizedRect::NormalizedRect()
    : left( 0.0 ), top( 0.0 ), right( 0.0 ), bottom( 0.0 ) {}

NormalizedRect::NormalizedRect( double l, double t, double r, double b )
    // note: check for swapping coords?
    : left( l ), top( t ), right( r ), bottom( b ) {}

NormalizedRect::NormalizedRect( const QRect & r, double xScale, double yScale )
    : left( (double)r.left() / xScale ), top( (double)r.top() / yScale ),
    right( (double)r.right() / xScale ), bottom( (double)r.bottom() / yScale ) {}

NormalizedRect::NormalizedRect( const NormalizedRect & rect )
    : left( rect.left ), top( rect.top ), right( rect.right ), bottom( rect.bottom ) {}

NormalizedRect NormalizedRect::fromQRectF( const QRectF &rect )
{
    QRectF nrect = rect.normalized();
    NormalizedRect ret;
    ret.left = nrect.left();
    ret.top = nrect.top();
    ret.right = nrect.right();
    ret.bottom = nrect.bottom();
    return ret;
}

bool NormalizedRect::isNull() const
{
    return left == 0 && top== 0 && right == 0 && bottom == 0;
}

bool NormalizedRect::contains( double x, double y ) const
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

bool NormalizedRect::intersects( const NormalizedRect & r ) const
{
    return (r.left <= right) && (r.right >= left) && (r.top <= bottom) && (r.bottom >= top);
}

bool NormalizedRect::intersects( const NormalizedRect * r ) const
{
    return (r->left <= right) && (r->right >= left) && (r->top <= bottom) && (r->bottom >= top);
}

bool NormalizedRect::intersects( double l, double t, double r, double b ) const
{
    return (l <= right) && (r >= left) && (t <= bottom) && (b >= top);
}

NormalizedRect NormalizedRect::operator| (const NormalizedRect & r) const
{
	NormalizedRect ret;
 // todo !       
	ret.left=qMin(left,r.left);
        ret.top=qMin(top,r.top);
        ret.bottom=qMax(bottom,r.bottom);
        ret.right=qMax(right,r.right);
	return ret;
}

NormalizedRect& NormalizedRect::operator|= (const NormalizedRect & r)
{
    left = qMin( left, r.left );
    top = qMin( top, r.top );
    bottom = qMax( bottom, r.bottom );
    right = qMax( right, r.right );
    return *this;
}

NormalizedRect NormalizedRect::operator&( const NormalizedRect & r ) const
{
    if ( isNull() || r.isNull() )
        return NormalizedRect();

    NormalizedRect ret;
    ret.left = qMax( left, r.left );
    ret.top = qMax( top, r.top );
    ret.bottom = qMin( bottom, r.bottom );
    ret.right = qMin( right, r.right );
    return ret;
}

NormalizedRect & NormalizedRect::operator=( const NormalizedRect & r )
{
    left = r.left;
    right = r.right;
    top = r.top;
    bottom = r.bottom;
    return *this;
}

bool NormalizedRect::operator==( const NormalizedRect & r ) const
{
    return ( isNull() && r.isNull() ) ||
       ( fabs( left - r.left ) < 1e-4 &&
         fabs( right - r.right ) < 1e-4 &&
         fabs( top - r.top ) < 1e-4 &&
         fabs( bottom - r.bottom ) < 1e-4 );
}

NormalizedPoint NormalizedRect::center() const
{
    return NormalizedPoint((left+right)/2.0, (top+bottom)/2.0);
}

/*
QDebug operator << (QDebug str , const NormalizedRect &r)
{
    str << "[" <<r.left() << "," << r.top() << "] x "<< "[" <<r.right() << "," << r.bottom() << "]";
    return str;
}*/

QRect NormalizedRect::geometry( int xScale, int yScale ) const
{
    int l = (int)( left * xScale ),
        t = (int)( top * yScale ),
        r = (int)( right * xScale ),
        b = (int)( bottom * yScale );

    return QRect( l, t, r - l + 1, b - t + 1 );
}

QRect NormalizedRect::roundedGeometry( int xScale, int yScale ) const
{
    int l = (int)( left * xScale + 0.5 ),
        t = (int)( top * yScale + 0.5 ),
        r = (int)( right * xScale + 0.5 ),
        b = (int)( bottom * yScale + 0.5 );

    return QRect( l, t, r - l + 1, b - t + 1 );
}

void NormalizedRect::transform( const QTransform &matrix )
{
    QRectF rect( left, top, right - left, bottom - top );
    rect = matrix.mapRect( rect );

    left = rect.left();
    top = rect.top();
    right = rect.right();
    bottom = rect.bottom();
}

QDebug operator<<( QDebug str, const Okular::NormalizedRect& r )
{
    str.nospace() << "NormRect(" << r.left << "," << r.top << " x " << ( r.right - r.left ) << "+" << ( r.bottom - r.top ) << ")";
    return str.space();
}

RegularAreaRect::RegularAreaRect()
    : RegularArea< NormalizedRect, QRect >(), d( 0 )
{
}

RegularAreaRect::RegularAreaRect( const RegularAreaRect& rar )
    : RegularArea< NormalizedRect, QRect >( rar ), d( 0 )
{
}

RegularAreaRect::~RegularAreaRect()
{
}

RegularAreaRect& RegularAreaRect::operator=( const RegularAreaRect& rar )
{
    RegularArea< NormalizedRect, QRect >::operator=( rar );
    return *this;
}


HighlightAreaRect::HighlightAreaRect( const RegularAreaRect *area )
    : RegularAreaRect(), s_id( -1 )
{
    if ( area )
    {
        RegularAreaRect::ConstIterator it = area->begin();
        RegularAreaRect::ConstIterator itEnd = area->end();
        for ( ; it != itEnd; ++it )
        {
            append( NormalizedRect( *it ) );
        }
    }
}

/** class ObjectRect **/

ObjectRect::ObjectRect( double l, double t, double r, double b, bool ellipse, ObjectType type, void * pnt )
    : m_objectType( type ), m_object( pnt )
{
    // assign coordinates swapping them if negative width or height
    QRectF rect( r > l ? l : r, b > t ? t : b, fabs( r - l ), fabs( b - t ) );
    if ( ellipse )
        m_path.addEllipse( rect );
    else
        m_path.addRect( rect );

    m_transformedPath = m_path;
}

ObjectRect::ObjectRect( const NormalizedRect& x, bool ellipse, ObjectType type, void * pnt )
    : m_objectType( type ), m_object( pnt )
{
    QRectF rect( x.left, x.top, fabs( x.right - x.left ), fabs( x.bottom - x.top ) );
    if ( ellipse )
        m_path.addEllipse( rect );
    else
        m_path.addRect( rect );

    m_transformedPath = m_path;
}

ObjectRect::ObjectRect( const QPolygonF &poly, ObjectType type, void * pnt )
    : m_objectType( type ), m_object( pnt )
{
    m_path.addPolygon( poly );

    m_transformedPath = m_path;
}

ObjectRect::ObjectType ObjectRect::objectType() const
{
    return m_objectType;
}

const void * ObjectRect::object() const
{
    return m_object;
}

const QPainterPath &ObjectRect::region() const
{
    return m_transformedPath;
}

QRect ObjectRect::boundingRect( double xScale, double yScale ) const
{
    const QRectF &br = m_transformedPath.boundingRect();

    return QRect( (int)( br.left() * xScale ), (int)( br.top() * yScale ),
                  (int)( br.width() * xScale ), (int)( br.height() * yScale ) );
}

bool ObjectRect::contains( double x, double y, double, double ) const
{
    return m_transformedPath.contains( QPointF( x, y ) );
}

void ObjectRect::transform( const QTransform &matrix )
{
    m_transformedPath = matrix.map( m_path );
}

double ObjectRect::distanceSqr( double x, double y, double xScale, double yScale ) const
{
    switch ( m_objectType )
    {
        case Action:
        case Image:
        {
            const QRectF& rect( m_transformedPath.boundingRect() );
            return NormalizedRect( rect.x(), rect.y(), rect.right(), rect.bottom() ).distanceSqr( x, y, xScale, yScale );
        }
        case OAnnotation:
        {
            return static_cast<Annotation*>(m_object)->d_func()->distanceSqr( x, y, xScale, yScale );
        }
        case SourceRef:
        {
            const SourceRefObjectRect * sr = static_cast< const SourceRefObjectRect * >( this );
            const NormalizedPoint& point = sr->m_point;
            if ( point.x == -1.0 )
            {
                return pow( ( y - point.y ) * yScale, 2 );
            }
            else if ( point.y == -1.0 )
            {
                return pow( ( x - point.x ) * xScale, 2 );
            }
            else
            {
                return pow( ( x - point.x ) * xScale, 2 ) + pow( ( y - point.y ) * yScale, 2 );
            }
        }
    }
    return 0.0;
}

ObjectRect::~ObjectRect()
{
    if ( !m_object )
        return;

    if ( m_objectType == Action )
        delete static_cast<Okular::Action*>( m_object );
    else if ( m_objectType == SourceRef )
        delete static_cast<Okular::SourceReference*>( m_object );
    else
        kDebug(OkularDebug).nospace() << "Object deletion not implemented for type '" << m_objectType << "'.";
}

/** class AnnotationObjectRect **/

AnnotationObjectRect::AnnotationObjectRect( Annotation * annotation )
    : ObjectRect( QPolygonF(), OAnnotation, annotation ), m_annotation( annotation )
{
}

Annotation *AnnotationObjectRect::annotation() const
{
    return m_annotation;
}

QRect AnnotationObjectRect::boundingRect( double xScale, double yScale ) const
{
    const QRect annotRect = AnnotationUtils::annotationGeometry( m_annotation, xScale, yScale );
    const QPoint center = annotRect.center();

    // Make sure that the rectangle has a minimum size, so that it's possible
    // to click on it
    const int minSize = 14;
    const QRect minRect( center.x()-minSize/2, center.y()-minSize/2, minSize, minSize );

    return annotRect | minRect;
}

bool AnnotationObjectRect::contains( double x, double y, double xScale, double yScale ) const
{
    return boundingRect( xScale, yScale ).contains( (int)( x * xScale ), (int)( y * yScale ), false );
}

AnnotationObjectRect::~AnnotationObjectRect()
{
    // the annotation pointer is kept elsewehere (in Page, most probably),
    // so just release its pointer
    m_object = 0;
}

void AnnotationObjectRect::transform( const QTransform &matrix )
{
    m_annotation->d_func()->annotationTransform( matrix );
}

/** class SourceRefObjectRect **/

SourceRefObjectRect::SourceRefObjectRect( const NormalizedPoint& point, void * srcRef )
    : ObjectRect( point.x, point.y, .0, .0, false, SourceRef, srcRef ), m_point( point )
{
    const double x = m_point.x < 0.0 ? 0.5 : m_point.x;
    const double y = m_point.y < 0.0 ? 0.5 : m_point.y;
    const QRectF rect( x - 2, y - 2, 5, 5 );
    m_path.addRect( rect );

    m_transformedPath = m_path;
}

QRect SourceRefObjectRect::boundingRect( double xScale, double yScale ) const
{
    const double x = m_point.x < 0.0 ? 0.5 : m_point.x;
    const double y = m_point.y < 0.0 ? 0.5 : m_point.y;

    return QRect( x * xScale, y * yScale, 1, 1 );
}

bool SourceRefObjectRect::contains( double x, double y, double xScale, double yScale ) const
{
    return distanceSqr( x, y, xScale, yScale ) < ( pow( 7.0 / xScale, 2 ) + pow( 7.0 / yScale, 2 ) );
}
