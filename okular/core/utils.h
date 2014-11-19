/***************************************************************************
 *   Copyright (C) 2006 by Luigi Toscano <luigi.toscano@tiscali.it>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_UTILS_H_
#define _OKULAR_UTILS_H_

#include "okular_export.h"
#include "area.h"

class QRect;
class QImage;
class QWidget;

namespace Okular
{

/**
 * @short General utility functions.
 *
 * This class contains some static functions of general utility.
 */
class OKULAR_EXPORT Utils
{
  public:
    /**
     * Rotate the rect \p source in the area \p width x \p height with the
     * specified \p orientation .
     */
    static QRect rotateRect( const QRect & source, int width, int height, int orientation );

    /**
     * Return the horizontal DPI of the main display
     */
    static double dpiX();

    /**
     * Return the vertical DPI of the main display
     */
    static double dpiY();

    /**
     * Return the real horizontal DPI of the main display.
     *
     * On X11, it can indicate the real horizontal DPI value without any Xrdb
     * setting. Otherwise, returns the same as dpiX(),
     *
     * @since 0.9 (KDE 4.3)
     * @deprecated Can not work with multi-monitor configurations
     */
    static double realDpiX();

    /**
     * Return the real vertical DPI of the main display
     *
     * On X11, it can indicate the real horizontal DPI value without any Xrdb
     * setting. Otherwise, returns the same as dpiX(),
     *
     * @since 0.9 (KDE 4.3)
     * @deprecated Can not work with multi-monitor configurations
     */
    static double realDpiY();

    /**
     * Return the real DPI of the display containing given widget
     *
     * On X11, it can indicate the real horizontal DPI value without any Xrdb
     * setting. Otherwise, returns the same as realDpiX/Y(),
     *
     * @since 0.19 (KDE 4.13)
     */
    static QSizeF realDpi(QWidget* widgetOnScreen);

    /**
     * Compute the smallest rectangle that contains all non-white pixels in image),
     * in normalized [0,1] coordinates.
     *
     * @since 0.7 (KDE 4.1)
     */
    static NormalizedRect imageBoundingBox( const QImage* image );
};

}

#endif

/* kate: replace-tabs on; indent-width 4; */
