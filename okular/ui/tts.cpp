/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "tts.h"

#include <qset.h>

#include <klocale.h>
#include <kspeech.h>

/* Private storage. */
class OkularTTS::Private
{
public:
    Private( OkularTTS *qq )
        : q( qq ), kspeech( 0 )
    {
    }

    void setupIface();
    void teardownIface();

    OkularTTS *q;
    KSpeech* kspeech;
    QSet< int > jobs;
};

void OkularTTS::Private::setupIface()
{
    if ( kspeech )
        return;

    if ( !KSpeech::isSupported() )
    {
        emit q->errorMessage( i18n( "Text-to-Speech not supported" ) );
        return;
    }

    kspeech = new KSpeech(q);
    kspeech->setSpeechID(QString::fromLatin1("okular"));
    connect( kspeech, SIGNAL(jobStateChanged(int,int)),
             q, SLOT(slotJobStateChanged(int,int)) );
}

void OkularTTS::Private::teardownIface()
{
    delete kspeech;
    kspeech = 0;
}


OkularTTS::OkularTTS( QObject *parent )
    : QObject( parent ), d( new Private( this ) )
{
}

OkularTTS::~OkularTTS()
{
    d->teardownIface();

    delete d;
}

void OkularTTS::say( const QString &text )
{
    if ( text.isEmpty() )
        return;

    d->setupIface();
    if ( d->kspeech )
    {
        const int jobId = d->kspeech->say( text );
        if ( jobId > 0 )
        {
            d->jobs.insert( jobId );
            emit hasSpeechs( true );
        }
    }
}

void OkularTTS::stopAllSpeechs()
{
    if ( !d->kspeech )
        return;

    d->kspeech->removeAllJobs();
}

void OkularTTS::slotJobStateChanged( int jobNum, int state )
{
    if ( !d->kspeech )
        return;

    switch ( state )
    {
        case KSpeech::JobCanceled: {
            d->jobs.remove( jobNum );
            emit hasSpeechs( !d->jobs.isEmpty() );
            break;
        }
        default: {
            break;
        }
    }
}

#include "moc_tts.cpp"
