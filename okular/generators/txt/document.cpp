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
    QTextCodec *codec = QTextCodec::codecForUtfText(buffer);
    setPlainText( codec->toUnicode( buffer ) );
}

Document::~Document()
{
}
