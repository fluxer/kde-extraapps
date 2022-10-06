/***************************************************************************
 *   Copyright (C) 2006 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "document.h"

#include <QFile>

#include <klocale.h>
#include <karchive.h>

using namespace OOO;

Document::Document( const QString &fileName )
  : mFileName( fileName ), mManifest( 0 ), mAnyEncrypted( false )
{
}

bool Document::open( const QString &password )
{
  mContent.clear();
  mStyles.clear();

  KArchive zip( mFileName );
  if ( !zip.isReadable() ) {
    setError( i18n( "Document is not a valid ZIP archive" ) );
    return false;
  }

  if ( zip.entry( "META-INF/manifest.xml" ).isNull() ) {
    setError( i18n( "Invalid document structure (META-INF/manifest.xml is missing)" ) );
    return false;
  }

  mManifest = new Manifest( mFileName, zip.data( "META-INF/manifest.xml" ), password );

  // we should really get the file names from the manifest, but for now, we only care
  // if the manifest says the files are encrypted.

  if ( zip.entry( "content.xml" ).isNull() ) {
    setError( i18n( "Invalid document structure (content.xml is missing)" ) );
    return false;
  }

  if ( mManifest->testIfEncrypted( "content.xml" )  ) {
    mAnyEncrypted = true;
    mContent = mManifest->decryptFile( "content.xml", zip.data( "content.xml" ) );
  } else {
    mContent = zip.data( "content.xml" );
  }

  if ( !zip.entry( "styles.xml" ).isNull() ) {
    if ( mManifest->testIfEncrypted( "styles.xml" )  ) {
      mAnyEncrypted = true;
      mStyles = mManifest->decryptFile( "styles.xml", zip.data( "styles.xml" ) );
    } else {
      mStyles = zip.data( "styles.xml" );
    }
  }

  if ( !zip.entry( "meta.xml" ).isNull() ) {
    if ( mManifest->testIfEncrypted( "meta.xml" )  ) {
      mAnyEncrypted = true;
      mMeta = mManifest->decryptFile( "meta.xml", zip.data( "meta.xml" ) );
    } else {
      mMeta = zip.data( "meta.xml" );
    }
  }

  foreach (const KArchiveEntry &entry, zip.list()) {
    if ( entry.pathname.startsWith("Pictures/") ) {
      QString fullPath = QFile::decodeName( entry.pathname );
      if ( mManifest->testIfEncrypted( fullPath ) ) {
        mAnyEncrypted = true;
        mImages.insert( fullPath, mManifest->decryptFile( fullPath, zip.data( fullPath ) ) );
      } else {
        mImages.insert( fullPath, zip.data( fullPath ) );
      }
    }
  }

  return true;
}

Document::~Document()
{
  delete mManifest;
}

QString Document::lastErrorString() const
{
  return mErrorString;
}

QByteArray Document::content() const
{
  return mContent;
}

QByteArray Document::meta() const
{
  return mMeta;
}

QByteArray Document::styles() const
{
  return mStyles;
}

QMap<QString, QByteArray> Document::images() const
{
  return mImages;
}

bool Document::anyFileEncrypted() const
{
  return mAnyEncrypted;
}

void Document::setError( const QString &error )
{
  mErrorString = error;
}
