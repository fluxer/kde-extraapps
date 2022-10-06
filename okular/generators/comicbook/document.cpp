/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "document.h"

#include <QtCore/QScopedPointer>
#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtGui/QImageReader>

#include <klocale.h>
#include <kmimetype.h>
#include <karchive.h>

#include <sys/stat.h>
#include <memory>

#include <core/page.h>

#include "directory.h"
#include "qnatsort.h"

using namespace ComicBook;

static void imagesInArchive( const KArchive* archive, QStringList *entries )
{
    Q_FOREACH ( const KArchiveEntry &entry, archive->list() ) {
        if ( S_ISREG(entry.mode) ) {
            entries->append( QFile::decodeName(entry.pathname) );
        }
    }
}


Document::Document()
    : mDirectory( 0 ), mArchive( 0 )
{
}

Document::~Document()
{
}

bool Document::open( const QString &fileName )
{
    close();

    const KMimeType::Ptr mime = KMimeType::findByFileContent( fileName );

    // qDebug() << Q_FUNC_INFO << mime->name();
    /**
     * We have a 7-zip, zip or rar archive
     */
    if ( mime->is( "application/x-cb7" ) ||
        mime->is( "application/x-cbz" ) || mime->is( "application/zip" ) ||
        mime->is( "application/x-cbt" ) || mime->is( "application/x-gzip" ) ||
        mime->is( "application/x-tar" ) || mime->is( "application/x-bzip" ) ||
        mime->is( "application/x-cbr" ) || mime->is( "application/x-rar" )) {
        mArchive = new KArchive( fileName );

        if ( !processArchive() ) {
            return false;
        }
    } else if ( mime->is( "inode/directory" ) ) {
        mDirectory = new Directory();

        if ( !mDirectory->open( fileName ) ) {
            delete mDirectory;
            mDirectory = 0;

            return false;
        }

        mEntries = mDirectory->list();
    } else {
        mLastErrorString = i18n( "Unknown ComicBook format." );
        return false;
    }

    return true;
}

void Document::close()
{
    mLastErrorString.clear();

    if ( !( mArchive || mDirectory ) )
        return;

    delete mArchive;
    mArchive = 0;
    delete mDirectory;
    mDirectory = 0;
    mPageMap.clear();
    mEntries.clear();
}

bool Document::processArchive() {
    if ( !mArchive->isReadable() ) {
        delete mArchive;
        mArchive = 0;

        return false;
    }

    imagesInArchive( mArchive, &mEntries );

    return true;
}

void Document::pages( QVector<Okular::Page*> * pagesVector )
{
    qSort( mEntries.begin(), mEntries.end(), caseSensitiveNaturalOrderLessThen );
    QScopedPointer< QIODevice > dev;

    int count = 0;
    pagesVector->clear();
    pagesVector->resize( mEntries.size() );
    QImageReader reader;
    foreach(const QString &file, mEntries) {
        if ( mArchive ) {
            const KArchiveEntry entry = mArchive->entry( file );
            if ( !entry.isNull() ) {
                dev.reset( new QBuffer() );
                qobject_cast<QBuffer*>(dev.data())->setData( mArchive->data( file) ); 
            }
        } else if ( mDirectory ) {
            dev.reset( mDirectory->createDevice( file ) );
        }

        if ( ! dev.isNull() ) {
            reader.setDevice( dev.data() );
            if ( reader.canRead() )
            {
                QSize pageSize = reader.size();
                if ( !pageSize.isValid() ) {
                    const QImage i = reader.read();
                    if ( !i.isNull() )
                        pageSize = i.size();
                }
                if ( pageSize.isValid() ) {
                    pagesVector->replace( count, new Okular::Page( count, pageSize.width(), pageSize.height(), Okular::Rotation0 ) );
                    mPageMap.append(file);
                    count++;
                } else {
                    kDebug() << "Ignoring" << file << "doesn't seem to be an image even if QImageReader::canRead returned true";
                }
            }
        }
    }
    pagesVector->resize( count );
}

QStringList Document::pageTitles() const
{
    return QStringList();
}

QImage Document::pageImage( int page ) const
{
    if ( mArchive ) {
        const KArchiveEntry entry = mArchive->entry( mPageMap[ page ] );
        if ( !entry.isNull() )
            return QImage::fromData( mArchive->data( mPageMap[ page ] ) );
    } else if ( mDirectory ) {
        return QImage( mPageMap[ page ] );
    }

    return QImage();
}

QString Document::lastErrorString() const
{
    return mLastErrorString;
}

