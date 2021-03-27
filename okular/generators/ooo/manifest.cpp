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
  m_size = size.toInt();
}

QString ManifestEntry::fileName() const
{
  return m_fileName;
}

QString ManifestEntry::mimeType() const
{
  return m_mimeType;
}

int ManifestEntry::size() const
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

void ManifestEntry::setKeyGenerationName( const QString &keyGenerationName )
{
  m_keyGenerationName = keyGenerationName;
}

QString ManifestEntry::keyGenerationName() const
{
  return m_keyGenerationName;
}

void ManifestEntry::setKeySize( const QString &keySize )
{
  m_keySize = keySize.toInt();
}

int ManifestEntry::keySize() const
{
  return m_keySize;
}

//---------------------------------------------------------------------

Manifest::Manifest( const QString &odfFileName, const QByteArray &manifestData, const QString &password )
  : m_odfFileName( odfFileName ), m_haveGoodPassword( false ), m_password( password )
#ifdef HAVE_GCRPYT
  , m_init(false)
#endif
{
#ifdef HAVE_GCRPYT
  m_init = gcry_check_version("1.5.0");
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
        currentEntry->setKeySize( kdfAttributes.value("manifest:key-size").toString() );
      } else if ( xml.name().toString() == "start-key-generation" ) {
        if (currentEntry == 0) {
          kWarning(OooDebug) << "Got start-key-generation without valid file-entry at line" << xml.lineNumber();
          continue;
        }
        QXmlStreamAttributes kdfAttributes = xml.attributes();
        currentEntry->setKeyGenerationName( kdfAttributes.value("manifest:start-key-generation-name").toString() );
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

// libgcrypt alternative of QCryptographicHash, produces same results
static QByteArray gcrypthash(const QByteArray &data, const int algorithm)
{
    const int algorithmlength = gcry_md_get_algo_dlen( algorithm );
    gcry_md_hd_t context;
    gcry_md_open( &context, algorithm, 0 );
    gcry_md_write( context, data.constData(), data.size() );
    unsigned char *md = gcry_md_read( context, algorithm );
    QByteArray result( reinterpret_cast<char*>(md), algorithmlength);
    gcry_md_close( context );
    return result;
}

void Manifest::checkPassword( ManifestEntry *entry, const QByteArray &fileData, QByteArray *decryptedData )
{
#ifdef HAVE_GCRPYT
  // kDebug(OooDebug) << entry->keyGenerationName() << entry->algorithm() << entry->checksumType();

  // http://docs.oasis-open.org/office/v1.2/os/OpenDocument-v1.2-os-part3.html#__RefHeading__752847_826425813
  QString keygenerationname = entry->keyGenerationName().toLower();
  const int keygenerationhashindex = keygenerationname.indexOf('#');
  keygenerationname = keygenerationname.mid(keygenerationhashindex + 1);
  int gcryptkeyalgorithm = GCRY_MD_NONE;
  // libreoffice suports SHA1 and SHA512, for reference:
  // libreoffice/oox/source/crypto/AgileEngine.cxx
  //
  // OpenOffice suports SHA1 and SHA256, for reference:
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( keygenerationname == "sha1" ) {
    gcryptkeyalgorithm = GCRY_MD_SHA1;
  } else if ( keygenerationname == "sha256" ) {
    gcryptkeyalgorithm = GCRY_MD_SHA256;
  } else if ( keygenerationname == "sha512" ) {
    gcryptkeyalgorithm = GCRY_MD_SHA512;
  } else {
    kWarning(OooDebug) << "unknown key generation name: " << entry->keyGenerationName();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  const int keysize = entry->keySize();
  char keybuff[keysize];
  ::memset( keybuff, 0, keysize * sizeof(char) );
  const QByteArray salt = entry->salt();
  // password must be UTF-8 encoded and hashed
  const QByteArray utf8pass = gcrypthash(m_password.toUtf8(), gcryptkeyalgorithm);
  // the key is always SHA1 derived
  gpg_error_t gcrypterror = gcry_kdf_derive(utf8pass.constData(), utf8pass.size(),
                                            GCRY_KDF_PBKDF2, GCRY_MD_SHA1,
                                            salt.constData(), salt.size(),
                                            entry->iterationCount(), keysize, keybuff);
  if ( gcrypterror != 0 ) {
    kWarning(OooDebug) << "gcry_kdf_derive: " << gcry_strerror(gcrypterror);
    return;
  }

  QString algorithm = entry->algorithm().toLower();
  const int algorithmhashindex = algorithm.indexOf('#');
  algorithm = algorithm.mid(algorithmhashindex + 1);
  int gcryptalgorithm = GCRY_CIPHER_NONE;
  int gcryptmode = GCRY_CIPHER_MODE_NONE;
  // libreoffice suports AES128-ECB, AES128-CBC and AES256-CBC, for reference:
  // libreoffice/oox/source/crypto/CryptTools.cxx
  //
  // OpenOffice supports AES128-CBC, AES192-CBC, AES256-CBC and Blowfish-CFB, for reference:
  // openoffice/main/package/source/package/manifest/ManifestExport.cxx
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( algorithm == "aes128-ecb" ) {
    gcryptalgorithm = GCRY_CIPHER_AES128;
    gcryptmode = GCRY_CIPHER_MODE_ECB;
  } else if ( algorithm == "aes128-cbc" ) {
    gcryptalgorithm = GCRY_CIPHER_AES128;
    gcryptmode = GCRY_CIPHER_MODE_CBC;
  } else if ( algorithm == "aes192-cbc" ) {
    gcryptalgorithm = GCRY_CIPHER_AES192;
    gcryptmode = GCRY_CIPHER_MODE_CBC;
  } else if ( algorithm == "aes256-cbc" ) {
    gcryptalgorithm = GCRY_CIPHER_AES256;
    gcryptmode = GCRY_CIPHER_MODE_CBC;
  // according to the spec "blowfish" is Blowfish-CFB but just in case match "-cfb" suffixed one
  // too, OpenOffice refers to it as "Blowfish CFB"
  } else if ( algorithm == "blowfish" || algorithm == "blowfish-cfb" || algorithm == "blowfish cfb" ) {
    gcryptalgorithm = GCRY_CIPHER_BLOWFISH;
    gcryptmode = GCRY_CIPHER_MODE_CFB;
  } else {
    kWarning(OooDebug) << "unknown algorithm: " << entry->algorithm();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  const QByteArray initializationvector = entry->initialisationVector();
  gcry_cipher_hd_t dec;
  gcrypterror = gcry_cipher_open( &dec, gcryptalgorithm, gcryptmode, 0 );
  if ( gcrypterror != 0 ) {
    kWarning(OooDebug) << "gcry_cipher_open: " << gcry_strerror(gcrypterror);
    return;
  }

  gcrypterror = gcry_cipher_setkey( dec, keybuff, keysize );
  if ( gcrypterror != 0 ) {
    kWarning(OooDebug) << "gcry_cipher_setkey: " << gcry_strerror(gcrypterror);
    gcry_cipher_close( dec );
    return;
  }

  gcrypterror = gcry_cipher_setiv( dec, initializationvector.constData(), initializationvector.size() );
  if ( gcrypterror != 0 ) {
    kWarning(OooDebug) << "gcry_cipher_setiv: " << gcry_strerror(gcrypterror);
    gcry_cipher_close( dec );
    return;
  }

  const int decbufflen = entry->size();
  char decbuff[decbufflen];
  ::memset( decbuff, 0, decbufflen * sizeof(char) );
  gcrypterror = gcry_cipher_decrypt( dec, decbuff, decbufflen, fileData.constData(), fileData.size() );
  if ( gcrypterror != 0 ) {
    kWarning(OooDebug) << "gcry_cipher_decrypt: " << gcry_strerror(gcrypterror);
    gcry_cipher_close( dec );
    return;
  }
  gcry_cipher_close( dec );

  *decryptedData = QByteArray( decbuff, decbufflen );

  QByteArray csum;
  QString checksumtype = entry->checksumType().toLower();
  const int checksumhashindex = checksumtype.indexOf('#');
  checksumtype = checksumtype.mid(checksumhashindex + 1);
  // libreoffice suports SHA1, SHA256 and SHA512, for reference:
  // libreoffice/oox/source/crypto/CryptTools.cxx
  //
  // OpenOffice suports SHA1 and SHA256, for reference:
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( checksumtype == "sha1-1k" || checksumtype == "sha1/1k" ) {
    csum = gcrypthash( decryptedData->left(1024), GCRY_MD_SHA1 );
  } else if ( checksumtype == "sha1" ) {
    csum = gcrypthash( *decryptedData, GCRY_MD_SHA1 );
  } else if ( checksumtype == "sha256-1k" || checksumtype == "sha256/1k" ) {
    csum = gcrypthash( decryptedData->left(1024), GCRY_MD_SHA256 );
  } else if ( checksumtype == "sha256" ) {
    csum = gcrypthash( *decryptedData, GCRY_MD_SHA256 );
  } else if ( checksumtype == "sha512-1k" || checksumtype == "sha512/1k" ) {
    csum = gcrypthash( decryptedData->left(1024), GCRY_MD_SHA512 );
  } else if ( checksumtype == "sha512" ) {
    csum = gcrypthash( *decryptedData, GCRY_MD_SHA512 );
  } else {
    kWarning(OooDebug) << "unknown checksum type: " << entry->checksumType();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  // kDebug(OooDebug) << entry->checksum().toHex() << csum.toHex();

  m_haveGoodPassword = (entry->checksum() == csum);
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
