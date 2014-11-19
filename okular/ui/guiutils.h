/***************************************************************************
 *   Copyright (C) 2006-2007 by Pino Toscano <pino@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef OKULAR_GUIUTILS_H
#define OKULAR_GUIUTILS_H

#include <QtCore/QString>

class QColor;
class QImage;
class QPixmap;
class QSize;
class QWidget;
class KIconLoader;

namespace Okular {
class Annotation;
class EmbeddedFile;
class Movie;
class ScreenAnnotation;
}

namespace GuiUtils
{
    /**
     * Returns the translated string with the type of the given @p annotation.
     */
    QString captionForAnnotation( const Okular::Annotation * annotation );
    QString authorForAnnotation( const Okular::Annotation * annotation );

    QString contentsHtml( const Okular::Annotation * annotation );

    QString prettyToolTip( const Okular::Annotation * annotation );

    QPixmap loadStamp( const QString& name, const QSize& size, int iconSize = 0 );

    void addIconLoader( KIconLoader * loader );
    void removeIconLoader( KIconLoader * loader );
    KIconLoader* iconLoader();

    void saveEmbeddedFile( Okular::EmbeddedFile *ef, QWidget *parent );

    /**
     * Returns the movie object that is referenced by a rendition action of the passed screen @p annotation
     * or @c 0 if the screen annotation has no rendition action set or the rendition action does not contain
     * a media rendition.
     */
    Okular::Movie* renditionMovieFromScreenAnnotation( const Okular::ScreenAnnotation * annotation );

    // colorize a gray image to the given color
    void colorizeImage( QImage & image, const QColor & color, unsigned int alpha = 255 );
}


#endif
