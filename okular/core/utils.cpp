/***************************************************************************
 *   Copyright (C) 2006 by Luigi Toscano <luigi.toscano@tiscali.it>        *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "utils.h"
#include "utils_p.h"

#include <QtCore/QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QImage>
#include <QIODevice>

#ifdef Q_WS_X11
  #include <QX11Info>
#endif


using namespace Okular;

QRect Utils::rotateRect( const QRect & source, int width, int height, int orientation )
{
    QRect ret;

    // adapt the coordinates of the boxes to the rotation
    switch ( orientation )
    {
        case 1:
            ret = QRect( width - source.y() - source.height(), source.x(),
                         source.height(), source.width() );
            break;
        case 2:
            ret = QRect( width - source.x() - source.width(), height - source.y() - source.height(),
                         source.width(), source.height() );
            break;
        case 3:
            ret = QRect( source.y(), height - source.x() - source.width(),
                         source.height(), source.width() );
            break;
        case 0:  // no modifications
        default: // other cases
            ret = source;
    }

    return ret;
}

#if defined(Q_WS_X11)
double Utils::dpiX()
{
    return QX11Info::appDpiX();
}

double Utils::dpiY()
{
    return QX11Info::appDpiY();
}

double Utils::realDpiX()
{
    const QDesktopWidget* w = QApplication::desktop();
    if (w->width() > 0 && w->widthMM() > 0) {
        kDebug() << "Pix:" << w->width() << "MM:" << w->widthMM();
        return (double(w->width()) * 25.4) / double(w->widthMM());
    } else {
        return dpiX();
    }
}

double Utils::realDpiY()
{
    const QDesktopWidget* w = QApplication::desktop();
    if (w->height() > 0 && w->heightMM() > 0) {
        kDebug() << "Pix:" << w->height() << "MM:" << w->heightMM();
        return (double(w->height()) * 25.4) / double(w->heightMM());
    } else {
        return dpiY();
    }
}

QSizeF Utils::realDpi(QWidget* widgetOnScreen)
{
    if (widgetOnScreen)
    {
        // Firstly try to retrieve DPI via QWidget::x11Info()
        const QX11Info &info = widgetOnScreen->x11Info();
        QSizeF res = QSizeF(info.appDpiX(info.screen()), info.appDpiY(info.screen()));
        if (qAbs(res.width() - res.height()) / qMin(res.height(), res.width()) < 0.05) {
            return res;
        } else {
            kDebug() << "QX11Info calculation from widget returned a non square dpi." << res << ". Falling back";
        }
    }
    // Fallback if particular widget is invalid or calculation was not square
    QSizeF res = QSizeF(realDpiX(), realDpiY());
    if (qAbs(res.width() - res.height()) / qMin(res.height(), res.width()) < 0.05) {
        return res;
    } else {
        kDebug() << "QDesktopWidget calculation returned a non square dpi." << res << ". Falling back";
    }

    res = QSizeF(dpiX(), dpiY());
    if (qAbs(res.width() - res.height()) / qMin(res.height(), res.width()) < 0.05) {
        return res;
    } else {
        kDebug() << "QX11Info returned a non square dpi." << res << ". Falling back";
    }
    
    res = QSizeF(72, 72);
    return res;
}
#else

double Utils::dpiX()
{
    return QDesktopWidget().physicalDpiX();
}

double Utils::dpiY()
{
    return QDesktopWidget().physicalDpiY();
}

double Utils::realDpiX()
{
    return dpiX();
}

double Utils::realDpiY()
{
    return dpiY();
}

QSizeF Utils::realDpi(QWidget*)
{
    return QSizeF(realDpiX(), realDpiY());
}
#endif // Q_WS_X11

inline static bool isWhite( QRgb argb ) {
    return ( argb & 0xFFFFFF ) == 0xFFFFFF; // ignore alpha
}

NormalizedRect Utils::imageBoundingBox( const QImage * image )
{
    if ( !image )
        return NormalizedRect();

    int width = image->width();
    int height = image->height();
    int left, top, bottom, right, x, y;

#ifdef BBOX_DEBUG
    QTime time;
    time.start();
#endif

    // Scan pixels for top non-white
    for ( top = 0; top < height; ++top )
        for ( x = 0; x < width; ++x )
            if ( !isWhite( image->pixel( x, top ) ) )
                goto got_top;
    return NormalizedRect( 0, 0, 0, 0 ); // the image is blank
got_top:
    left = right = x;

    // Scan pixels for bottom non-white
    for ( bottom = height-1; bottom >= top; --bottom )
        for ( x = width-1; x >= 0; --x )
            if ( !isWhite( image->pixel( x, bottom ) ) )
                goto got_bottom;
    Q_ASSERT( 0 ); // image changed?!
got_bottom:
    if ( x < left )
        left = x;
    if ( x > right )
        right = x;

    // Scan for leftmost and rightmost (we already found some bounds on these):
    for ( y = top; y <= bottom && ( left > 0 || right < width-1 ); ++y )
    {
        for ( x = 0; x < left; ++x )
            if ( !isWhite( image->pixel( x, y ) ) )
                left = x;
        for ( x = width-1; x > right+1; --x )
            if ( !isWhite( image->pixel( x, y ) ) )
                right = x;
    }

    NormalizedRect bbox( QRect( left, top, ( right - left + 1), ( bottom - top + 1 ) ),
                         image->width(), image->height() );

#ifdef BBOX_DEBUG
    kDebug() << "Computed bounding box" << bbox << "in" << time.elapsed() << "ms";
#endif

    return bbox;
}

void Okular::copyQIODevice( QIODevice *from, QIODevice *to )
{
    QByteArray buffer( 65536, '\0' );
    qint64 read = 0;
    qint64 written = 0;
    while ( ( read = from->read( buffer.data(), buffer.size() ) ) > 0 )
    {
        written = to->write( buffer.constData(), read );
        if ( read != written )
            break;
    }
}

QTransform Okular::buildRotationMatrix(Rotation rotation)
{
    QTransform matrix;
    matrix.rotate( (int)rotation * 90 );

    switch ( rotation )
    {
        case Rotation90:
            matrix.translate( 0, -1 );
            break;
        case Rotation180:
            matrix.translate( -1, -1 );
            break;
        case Rotation270:
            matrix.translate( -1, 0 );
            break;
        default: ;
    }

    return matrix;
}

/* kate: replace-tabs on; indent-width 4; */
