/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *   Copyright (C) 2012 by Guillermo A. Amaral B. <gamaral@kde.org>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "movie.h"

// qt/kde includes
#include <qdir.h>
#include <qimage.h>
#include <qstring.h>
#include <qtemporaryfile.h>

#include <kdebug.h>

#include "debug_p.h"

using namespace Okular;

class Movie::Private
{
    public:
        Private( const QString &url )
            : m_url( url ),
              m_rotation( Rotation0 ),
              m_playMode( PlayOnce ),
              m_tmp( 0 ),
              m_showControls( false ),
              m_autoPlay( false ),
              m_showPosterImage( false )
        {
        }

        QString m_url;
        QSize m_aspect;
        Rotation m_rotation;
        PlayMode m_playMode;
        QTemporaryFile *m_tmp;
        QImage m_posterImage;
        bool m_showControls : 1;
        bool m_autoPlay : 1;
        bool m_showPosterImage : 1;
};

Movie::Movie( const QString& fileName )
    : d( new Private( fileName ) )
{
}

Movie::Movie( const QString& fileName, const QByteArray &data )
    : d( new Private( fileName ) )
{
    /* Store movie data as temporary file.
     *
     * Originally loaded movie data directly using a QBuffer, but sadly phonon
     * fails to play on a few of my test systems (I think it's the Phonon
     * GStreamer backend). Storing the data in a temporary file works fine
     * though, not to mention, it releases much needed memory. (gamaral)
     */
    d->m_tmp = new QTemporaryFile( QString( "%1/okrXXXXXX" ).arg( QDir::tempPath() ) );
    if ( d->m_tmp->open() ) {
       d->m_tmp->write( data );
       d->m_tmp->flush();
    }
    else kDebug(OkularDebug) << "Failed to create temporary file for video data.";
}

Movie::~Movie()
{
    delete d->m_tmp;
    delete d;
}

QString Movie::url() const
{
    if (d->m_tmp)
        return d->m_tmp->fileName();
    else
        return d->m_url;
}

void Movie::setSize( const QSize &aspect )
{
    d->m_aspect = aspect;
}

QSize Movie::size() const
{
    return d->m_aspect;
}

void Movie::setRotation( Rotation rotation )
{
    d->m_rotation = rotation;
}

Rotation Movie::rotation() const
{
    return d->m_rotation;
}

void Movie::setShowControls( bool show )
{
    d->m_showControls = show;
}

bool Movie::showControls() const
{
    return d->m_showControls;
}

void Movie::setPlayMode( Movie::PlayMode mode )
{
    d->m_playMode = mode;
}

Movie::PlayMode Movie::playMode() const
{
    return d->m_playMode;
}

void Movie::setAutoPlay( bool autoPlay )
{
    d->m_autoPlay = autoPlay;
}

bool Movie::autoPlay() const
{
    return d->m_autoPlay;
}

void Movie::setShowPosterImage( bool show )
{
    d->m_showPosterImage = show;
}

bool Movie::showPosterImage() const
{
    return d->m_showPosterImage;
}

void Movie::setPosterImage( const QImage &image )
{
    d->m_posterImage = image;
}

QImage Movie::posterImage() const
{
    return d->m_posterImage;
}
