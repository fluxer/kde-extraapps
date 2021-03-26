/***************************************************************************
 *   Copyright (C) 2007, 2009 by Brad Hards <bradh@frogmouth.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "manifest.h"
#include "debug.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <qxmlstream.h>

#include <KFilterDev>
#include <KLocale>
#include <KMessageBox>

#ifdef HAVE_GCRPYT
#include <gcrypt.h>
#endif


using namespace OOO;

//---------------------------------------------------------------------

ManifestEntry::ManifestEntry( const QString &fileName ) :
  m_fileName( fileName )
{
}

void ManifestEntry::setMimeType( const QString &mimeType )
{
  m_mimeType = mimeType;
}

void ManifestEntry::setSize( const QString &size )
{
  m_size = size;
}

QString ManifestEntry::fileName() const
{
  return m_fileName;
}

QString ManifestEntry::mimeType() const
{
  return m_mimeType;
}

QString ManifestEntry::size() const
{
  return m_size;
}

void ManifestEntry::setChecksumType( const QString &checksumType )
{
  m_checksumType = checksumType;
}

QString ManifestEntry::checksumType() const
{
  return m_checksumType;
}

void ManifestEntry::setChecksum( const QString &checksum )
{
  m_checksum = QByteArray::fromBase64( checksum.toAscii() );
}

QByteArray ManifestEntry::checksum() const
{
  return m_checksum;
}

void ManifestEntry::setAlgorithm( const QString &algorithm )
{
  m_algorithm = algorithm;
}

QString ManifestEntry::algorithm() const
{
  return m_algorithm;
}

void ManifestEntry::setInitialisationVector( const QString &initialisationVector )
{
  m_initialisationVector = QByteArray::fromBase64( initialisationVector.toAscii() );
}

QByteArray ManifestEntry::initialisationVector() const
{
  return m_initialisationVector;
}

void ManifestEntry::setKeyDerivationName( const QString &keyDerivationName )
{
  m_keyDerivationName = keyDerivationName;
}

QString ManifestEntry::keyDerivationName() const
{
  return m_keyDerivationName;
}

void ManifestEntry::setIterationCount( const QString &iterationCount )
{
  m_iterationCount = iterationCount.toInt();
}

int ManifestEntry::iterationCount() const
{
  return m_iterationCount;
}

void ManifestEntry::setSalt( const QString &salt )
{
  m_salt = QByteArray::fromBase64( salt.toAscii() );
}

QByteArray ManifestEntry::salt() const
{
  return m_salt;
}

//---------------------------------------------------------------------

Manifest::Manifest( const QString &odfFileName, const QByteArray &manifestData, const QString &password )
  : m_odfFileName( odfFileName ), m_haveGoodPassword( false ), m_password( password )
#ifdef HAVE_GCRPYT
  , m_init(false)
#endif
{
#ifdef HAVE_GCRPYT
  // the minimum that supports SHA256
  m_init = gcry_check_version("1.1.8");
#endif
  // I don't know why the parser barfs on this.
  QByteArray manifestCopy = manifestData;
  manifestCopy.replace(QByteArray("DOCTYPE manifest:manifest"), QByteArray("DOCTYPE manifest"));

  QXmlStreamReader xml( manifestCopy );

  ManifestEntry *currentEntry = 0;
  while ( ! xml.atEnd() ) {
    xml.readNext();
    if ( (xml.tokenType() == QXmlStreamReader::NoToken) ||
	 (xml.tokenType() == QXmlStreamReader::Invalid) ||
	 (xml.tokenType() == QXmlStreamReader::StartDocument) ||
	 (xml.tokenType() == QXmlStreamReader::EndDocument) ||
	 (xml.tokenType() == QXmlStreamReader::DTD) || 
	 (xml.tokenType() == QXmlStreamReader::Characters) ) {
      continue;
    }
    if (xml.tokenType() == QXmlStreamReader::StartElement) {
      if ( xml.name().toString() == "manifest" ) {
	continue;
      } else if ( xml.name().toString() == "file-entry" ) {
	QXmlStreamAttributes attributes = xml.attributes();
	if (currentEntry != 0) {
	  kWarning(OooDebug) << "Got new StartElement for new file-entry, but haven't finished the last one yet!";
	  kWarning(OooDebug) << "processing" << currentEntry->fileName() << ", got" << attributes.value("manifest:full-path").toString();
	}
	currentEntry = new ManifestEntry( attributes.value("manifest:full-path").toString() );
	currentEntry->setMimeType( attributes.value("manifest:media-type").toString() );
	currentEntry->setSize( attributes.value("manifest:size").toString() );
      } else if ( xml.name().toString() == "encryption-data" ) {
	if (currentEntry == 0) {
	  kWarning(OooDebug) << "Got encryption-data without valid file-entry at line" << xml.lineNumber();
	  continue;
	}
	QXmlStreamAttributes encryptionAttributes = xml.attributes();
	currentEntry->setChecksumType( encryptionAttributes.value("manifest:checksum-type").toString() );
	currentEntry->setChecksum( encryptionAttributes.value("manifest:checksum").toString() );
      } else if ( xml.name().toString() == "algorithm" ) {
	if (currentEntry == 0) {
	  kWarning(OooDebug) << "Got algorithm without valid file-entry at line" << xml.lineNumber();
	  continue;
	}
	QXmlStreamAttributes algorithmAttributes = xml.attributes();
	currentEntry->setAlgorithm( algorithmAttributes.value("manifest:algorithm-name").toString() );
	currentEntry->setInitialisationVector( algorithmAttributes.value("manifest:initialisation-vector").toString() );
      } else if ( xml.name().toString() == "key-derivation" ) {
	if (currentEntry == 0) {
	  kWarning(OooDebug) << "Got key-derivation without valid file-entry at line" << xml.lineNumber();
	  continue;
	}
	QXmlStreamAttributes kdfAttributes = xml.attributes();
	currentEntry->setKeyDerivationName( kdfAttributes.value("manifest:key-derivation-name").toString() );
	currentEntry->setIterationCount( kdfAttributes.value("manifest:iteration-count").toString() );
	currentEntry->setSalt( kdfAttributes.value("manifest:salt").toString() );
      } else {
	// handle other StartDocument types here 
	kWarning(OooDebug) << "Unexpected start document type: " << xml.name().toString();
      }
    } else if ( xml.tokenType() == QXmlStreamReader::EndElement ) {
      if ( xml.name().toString() == "manifest" ) {
	continue;
      } else if ( xml.name().toString() == "file-entry") {
	if (currentEntry == 0) {
	  kWarning(OooDebug) << "Got EndElement for file-entry without valid StartElement at line" << xml.lineNumber();
	  continue;
	}
	// we're finished processing that file entry
	if ( mEntries.contains( currentEntry->fileName() ) ) {
	  kWarning(OooDebug) << "Can't insert entry because of duplicate name:" << currentEntry->fileName();
	  delete currentEntry;
	} else {
	  mEntries.insert( currentEntry->fileName(), currentEntry);
	}
        currentEntry = 0;
      }
    }
  }
  if (xml.hasError()) {
    kWarning(OooDebug) << "error: " << xml.errorString() << xml.lineNumber() << xml.columnNumber();
  }
}

Manifest::~Manifest()
{
  qDeleteAll( mEntries );
}

ManifestEntry* Manifest::entryByName( const QString &filename )
{
  return mEntries.value( filename, 0 );
}

bool Manifest::testIfEncrypted( const QString &filename )
{
  ManifestEntry *entry = entryByName( filename );

  if (entry) {
    return ( entry->salt().length() > 0 );
  }

  return false;
}

void Manifest::checkPassword( ManifestEntry *entry, const QByteArray &fileData, QByteArray *decryptedData )
{
#ifdef HAVE_GCRPYT
  // http://docs.oasis-open.org/office/v1.2/os/OpenDocument-v1.2-os-part3.html#__RefHeading__752847_826425813
  QString checksumtype = entry->checksumType().toLower();
  const int hashindex = checksumtype.indexOf('#');
  checksumtype = checksumtype.mid(hashindex + 1);

  const QByteArray bytepass = m_password.toLocal8Bit();
  const QByteArray salt = entry->salt();
  int algorithmlength = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
  QByteArray keyhash;
  if ( checksumtype == "sha1" || checksumtype == "sha1-1k" ) {
    keyhash.resize(algorithmlength);
    gcry_kdf_derive(bytepass.constData(), bytepass.size(),
                    GCRY_KDF_PBKDF2, GCRY_MD_SHA1,
                    salt.constData(), salt.size(),
                    entry->iterationCount(), algorithmlength, keyhash.data());
  } else if ( checksumtype == "sha256" || checksumtype == "sha256-1k" ) {
    algorithmlength = gcry_md_get_algo_dlen( GCRY_MD_SHA256 );
    keyhash.resize(algorithmlength);
    gcry_kdf_derive(bytepass.constData(), bytepass.size(),
                    GCRY_KDF_PBKDF2, GCRY_MD_SHA256,
                    salt.constData(), salt.size(),
                    entry->iterationCount(), algorithmlength, keyhash.data());
  } else {
    kWarning(OooDebug) << "unknown checksum type: " << entry->checksumType();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  const QByteArray initializationvector = entry->initialisationVector();
  gcry_cipher_hd_t dec;
  gcry_cipher_open( &dec, GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_MODE_CFB, 0 );
  gcry_cipher_setkey( dec, keyhash.constData(), keyhash.size() );
  gcry_cipher_setiv( dec, initializationvector.constData(), initializationvector.size() );

  // buffer size must be multiple of the hashing algorithm block size
  unsigned int decbufflen = fileData.size() * algorithmlength;
  unsigned char decbuff[decbufflen];
  ::memset(decbuff, 0, decbufflen * sizeof(unsigned char));
  gcry_cipher_decrypt( dec, decbuff, decbufflen, fileData.constData(), fileData.size() );
  gcry_cipher_final( dec );
  gcry_cipher_close( dec );

  *decryptedData = QByteArray( reinterpret_cast<char*>(decbuff), decbufflen );

  QByteArray csum;
  if ( checksumtype == "sha1-1k" ) {
    csum = QCryptographicHash::hash( decryptedData->left(1024), QCryptographicHash::Sha1 );
  } else if ( checksumtype == "sha1" ) {
    csum = QCryptographicHash::hash( *decryptedData, QCryptographicHash::Sha1 );
  } else if ( checksumtype == "sha256-1k") {
    csum = QCryptographicHash::hash( decryptedData->left(1024), QCryptographicHash::Sha256 );
  } else if ( checksumtype == "sha256") {
    csum = QCryptographicHash::hash( *decryptedData, QCryptographicHash::Sha256 );
  }

  // qDebug() << Q_FUNC_INFO << entry->checksum().toHex() << csum.toHex();

  if ( entry->checksum() == csum ) {
    m_haveGoodPassword = true;
  } else {
    m_haveGoodPassword = false;
  }
#else
  m_haveGoodPassword = false;
#endif
}

QByteArray Manifest::decryptFile( const QString &filename, const QByteArray &fileData )
{
#ifdef HAVE_GCRPYT
  ManifestEntry *entry = entryByName( filename );

  if ( !m_init ) {
    KMessageBox::error( 0, i18n("This document is encrypted but decryptor could not be initialized") );
    // in the hope that it wasn't really encrypted...
    return QByteArray( fileData );
  }

  QByteArray decryptedData;
  checkPassword( entry, fileData, &decryptedData );

  if (! m_haveGoodPassword ) {
    return QByteArray();
  }

  QIODevice *decompresserDevice = KFilterDev::device( new QBuffer( &decryptedData, 0 ), "application/x-gzip", true );
  if( !decompresserDevice ) {
    kDebug(OooDebug) << "Couldn't create decompressor";
    // hopefully it isn't compressed then!
    return QByteArray( fileData );
  }

  static_cast<KFilterDev*>( decompresserDevice )->setSkipHeaders( );

  decompresserDevice->open( QIODevice::ReadOnly );

  return decompresserDevice->readAll();

#else
  // TODO: This should have a proper parent
  KMessageBox::error( 0, i18n("This document is encrypted, but Okular was compiled without crypto support. This document will probably not open.") );
  // this is equivalent to what happened before all this Manifest stuff :-)
  return QByteArray( fileData );
#endif
}
