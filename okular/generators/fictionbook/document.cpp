/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "document.h"

#include <QtCore/QFile>

#include <klocale.h>
#include <karchive.h>

using namespace FictionBook;

Document::Document( const QString &fileName )
    : mFileName( fileName )
{
}

bool Document::open()
{
    if ( mFileName.endsWith( ".fb" ) || mFileName.endsWith( ".fb2" ) ) {
        QFile file( mFileName );
        if ( !file.open( QIODevice::ReadOnly ) ) {
            setError( i18n( "Unable to open document: %1", file.errorString() ) );
            return false;
        }

        QString errorMsg;
        if ( !mDocument.setContent( &file, true, &errorMsg ) ) {
            setError( i18n( "Invalid XML document: %1", errorMsg ) );
            return false;
        }
        return true;
    }

    KArchive zip( mFileName );
    if ( !zip.isReadable() ) {
        setError( i18n( "Document is not a valid ZIP archive" ) );
        return false;
    }

    const QList<KArchiveEntry> entries = zip.list();

    QString documentFile;
    for ( int i = 0; i < entries.count(); ++i ) {
        if ( entries[ i ].pathname.endsWith( ".fb2" ) ) {
            documentFile = QFile::decodeName(entries[ i ].pathname);
            break;
        }
    }

    if ( documentFile.isEmpty() ) {
        setError( i18n( "No content found in the document" ) );
        return false;
    }

    QString errorMsg;
    if ( !mDocument.setContent( zip.data( documentFile ), true, &errorMsg ) ) {
        setError( i18n( "Invalid XML document: %1", errorMsg ) );
        return false;
    }
    return true;
}

QDomDocument Document::content() const
{
    return mDocument;
}

QString Document::lastErrorString() const
{
    return mErrorString;
}

void Document::setError( const QString &error )
{
    mErrorString = error;
}
