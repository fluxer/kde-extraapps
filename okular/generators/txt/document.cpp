/***************************************************************************
 *   Copyright (C) 2013 by Azat Khuzhin <a3at.mail@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <QFile>
#include <QDataStream>
#include <QTextCodec>

#include <kencodingprober.h>
#include <kdebug.h>

#include "document.h"

using namespace Txt;

Document::Document( const QString &fileName )
{
#ifdef TXT_DEBUG
    kDebug() << "Opening file" << fileName;
#endif

    QFile plainFile( fileName );
    if ( !plainFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        kDebug() << "Can't open file" << plainFile.fileName();
        return;
    }

    const QByteArray buffer = plainFile.readAll();
    setPlainText( toUnicode(buffer) );
}

Document::~Document()
{
}

QString Document::toUnicode( const QByteArray &array )
{
    QByteArray encoding;
    KEncodingProber prober(KEncodingProber::Universal);
    int charsFeeded = 0;
    int chunkSize = 3000; // ~= number of symbols in page.

    // Try to detect encoding.
    while ( encoding.isEmpty() && charsFeeded < array.size() )
    {
        prober.feed( array.mid( charsFeeded, chunkSize ) );
        charsFeeded += chunkSize;

        if (prober.confidence() >= 0.5)
        {
            encoding = prober.encoding();
            break;
        }
    }

    if ( encoding.isEmpty() )
    {
        return QString();
    }

    kDebug() << "Detected" << prober.encoding() << "encoding"
             << "based on" << charsFeeded << "chars";
    return QTextCodec::codecForName( encoding )->toUnicode( array );
}
