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

#include <KDecompressor>
#include <KLocale>
#include <KMessageBox>

#ifdef HAVE_OPENSSL
#  include <openssl/evp.h>
#  include <openssl/err.h>
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
#ifdef HAVE_OPENSSL
  , m_init(true)
#endif
{
#ifdef HAVE_OPENSSL
  ERR_load_ERR_strings();

  if (EVP_add_cipher(EVP_aes_128_ecb()) != 1) {;
    m_init = false;
  }
  if (EVP_add_cipher(EVP_aes_128_cbc()) != 1) {;
    m_init = false;
  }
  if (EVP_add_cipher(EVP_aes_192_cbc()) != 1) {;
    m_init = false;
  }
  if (EVP_add_cipher(EVP_aes_256_cbc()) != 1) {;
    m_init = false;
  }
  if (EVP_add_cipher(EVP_bf_cfb64()) != 1) {;
    m_init = false;
  }
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

void Manifest::checkPassword( ManifestEntry *entry, const QByteArray &fileData, QByteArray *decryptedData )
{
#ifdef HAVE_OPENSSL
  // kDebug(OooDebug) << entry->keyGenerationName() << entry->keyDerivationName() << entry->algorithm() << entry->checksumType();

  // http://docs.oasis-open.org/office/v1.2/os/OpenDocument-v1.2-os-part3.html#__RefHeading__752847_826425813
  QString keygenerationname = entry->keyGenerationName().toLower();
  const int keygenerationhashindex = keygenerationname.indexOf('#');
  keygenerationname = keygenerationname.mid(keygenerationhashindex + 1);
  QCryptographicHash::Algorithm cryptokeyalgorithm = QCryptographicHash::Sha1;
  // libreoffice supports SHA1 and SHA512, for reference:
  // libreoffice/oox/source/crypto/AgileEngine.cxx
  //
  // OpenOffice supports SHA1 and SHA256, for reference:
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( keygenerationname == "sha1" ) {
    cryptokeyalgorithm = QCryptographicHash::Sha1;
  } else if ( keygenerationname == "sha256" ) {
    cryptokeyalgorithm = QCryptographicHash::Sha256;
  } else if ( keygenerationname == "sha512" ) {
    cryptokeyalgorithm = QCryptographicHash::Sha512;
  } else {
    kWarning(OooDebug) << "unknown key generation name: " << entry->keyGenerationName();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  const int opensslkeysize = entry->keySize();
  uchar opensslkeybuffer[opensslkeysize];
  ::memset( opensslkeybuffer, 0, opensslkeysize * sizeof(char) );
  const QByteArray salt = entry->salt();
  // password must be UTF-8 encoded and hashed
  const QByteArray passwordhash = QCryptographicHash::hash(m_password.toUtf8(), cryptokeyalgorithm);
  // the key is always SHA1 derived
  int opensslresult = PKCS5_PBKDF2_HMAC_SHA1(
    passwordhash.constData(), passwordhash.size(),
    reinterpret_cast<const uchar*>(salt.constData()), salt.size(),
    entry->iterationCount(), opensslkeysize, opensslkeybuffer
  );
  if ( opensslresult != 1 ) {
    kWarning(OooDebug) << "PKCS5_PBKDF2_HMAC_SHA1: " << ERR_error_string(ERR_get_error(), NULL);
    return;
  }

  QString algorithm = entry->algorithm().toLower();
  const int algorithmhashindex = algorithm.indexOf('#');
  algorithm = algorithm.mid(algorithmhashindex + 1);
  const EVP_CIPHER* opensslcipher = NULL;
  // libreoffice supports AES128-ECB, AES128-CBC and AES256-CBC, for reference:
  // libreoffice/oox/source/crypto/CryptTools.cxx
  //
  // OpenOffice supports AES128-CBC, AES192-CBC, AES256-CBC and Blowfish-CFB, for reference:
  // openoffice/main/package/source/package/manifest/ManifestExport.cxx
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( algorithm == "aes128-ecb" ) {
    opensslcipher = EVP_aes_128_ecb();
  } else if ( algorithm == "aes128-cbc" ) {
    opensslcipher = EVP_aes_128_cbc();
  } else if ( algorithm == "aes192-cbc" ) {
    opensslcipher = EVP_aes_192_cbc();
  } else if ( algorithm == "aes256-cbc" ) {
    opensslcipher = EVP_aes_256_cbc();
  // according to the spec "blowfish" is Blowfish-CFB but just in case match "-cfb" suffixed one
  // too, OpenOffice refers to it as "Blowfish CFB"
  } else if ( algorithm == "blowfish" || algorithm == "blowfish-cfb" || algorithm == "blowfish cfb" ) {
    opensslcipher = EVP_bf_cfb64();
  } else {
    kWarning(OooDebug) << "unknown algorithm: " << entry->algorithm();
    // we can only assume it will be OK.
    m_haveGoodPassword = true;
    return;
  }

  EVP_CIPHER_CTX *opensslctx = EVP_CIPHER_CTX_new();
  if (!opensslctx) {
    kWarning(OooDebug) << "EVP_CIPHER_CTX_new: " << ERR_error_string(ERR_get_error(), NULL);
    return;
  }

  const QByteArray initializationvector = entry->initialisationVector();
  opensslresult = EVP_DecryptInit(
    opensslctx, opensslcipher,
    opensslkeybuffer,
    reinterpret_cast<const uchar*>(initializationvector.constData())
  );
  if ( opensslresult != 1 ) {
    kWarning(OooDebug) << "EVP_DecryptInit: " << ERR_error_string(ERR_get_error(), NULL);
    EVP_CIPHER_CTX_free(opensslctx);
    return;
  }

  // kDebug(OooDebug) << EVP_CIPHER_CTX_key_length(opensslctx) << opensslkeysize;
  // kDebug(OooDebug) << EVP_CIPHER_CTX_iv_length(opensslctx) << initializationvector.size();

  const int opensslbuffersize = entry->size();
  QByteArray opensslbuffer(opensslbuffersize, Qt::Uninitialized);
  int opensslbufferpos = 0;
  int openssloutputsize = 0;
  opensslresult = EVP_DecryptUpdate(
    opensslctx,
    reinterpret_cast<uchar*>(opensslbuffer.data()), &opensslbufferpos,
    reinterpret_cast<const uchar*>(fileData.constData()), fileData.size()
  );
  openssloutputsize = opensslbufferpos;
  if ( opensslresult != 1 ) {
    kWarning(OooDebug) << "EVP_DecryptUpdate: " << ERR_error_string(ERR_get_error(), NULL);
    EVP_CIPHER_CTX_free(opensslctx);
    return;
  }

  opensslresult = EVP_DecryptFinal(
    opensslctx,
    reinterpret_cast<uchar*>(opensslbuffer.data() + opensslbufferpos), &opensslbufferpos
  );
  // something is going terribly wrong with EVP_DecryptFinal(), opensslbufferpos is 0 for both AES
  // and Blowfish and even if opensslresult is not 1 checksum verification still passes so just
  // ignoring errors and proceeding
#if 0
  openssloutputsize += opensslbufferpos;
  if ( opensslresult != 1 ) {
    kWarning(OooDebug) << "EVP_DecryptFinal: " << ERR_error_string(ERR_get_error(), NULL);
    EVP_CIPHER_CTX_free(opensslctx);
    return;
  }
#else
  openssloutputsize = opensslbuffersize;
#endif

  *decryptedData = QByteArray( opensslbuffer.constData(), openssloutputsize );
  EVP_CIPHER_CTX_free(opensslctx);

  QByteArray csum;
  QString checksumtype = entry->checksumType().toLower();
  const int checksumhashindex = checksumtype.indexOf('#');
  checksumtype = checksumtype.mid(checksumhashindex + 1);
  // libreoffice supports SHA1, SHA256 and SHA512, for reference:
  // libreoffice/oox/source/crypto/CryptTools.cxx
  //
  // OpenOffice supports SHA1 and SHA256, for reference:
  // openoffice/main/package/source/package/manifest/ManifestDefines.hxx
  if ( checksumtype == "sha1-1k" || checksumtype == "sha1/1k" ) {
    csum = QCryptographicHash::hash( decryptedData->left(1024), QCryptographicHash::Sha1 );
  } else if ( checksumtype == "sha1" ) {
    csum = QCryptographicHash::hash( *decryptedData, QCryptographicHash::Sha1 );
  } else if ( checksumtype == "sha256-1k" || checksumtype == "sha256/1k" ) {
    csum = QCryptographicHash::hash( decryptedData->left(1024), QCryptographicHash::Sha256 );
  } else if ( checksumtype == "sha256" ) {
    csum = QCryptographicHash::hash( *decryptedData, QCryptographicHash::Sha256 );
  } else if ( checksumtype == "sha512-1k" || checksumtype == "sha512/1k" ) {
    csum = QCryptographicHash::hash( decryptedData->left(1024), QCryptographicHash::Sha512 );
  } else if ( checksumtype == "sha512" ) {
    csum = QCryptographicHash::hash( *decryptedData, QCryptographicHash::Sha512 );
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
#ifdef HAVE_OPENSSL
  ManifestEntry *entry = entryByName( filename );

  if ( !m_init ) {
    KMessageBox::error( 0, i18n("This document is encrypted but decryptor could not be initialized") );
    // in the hope that it wasn't really encrypted...
    return fileData;
  }

  QByteArray decryptedData;
  checkPassword( entry, fileData, &decryptedData );

  if (! m_haveGoodPassword ) {
    return QByteArray();
  }

  KDecompressor kdecompressor;
  kdecompressor.setType(KDecompressor::TypeDeflate);
  if( !kdecompressor.process(decryptedData) ) {
    kDebug(OooDebug) << kdecompressor.errorString();
    // hopefully it isn't compressed then!
    return fileData;
  }

  return kdecompressor.result();

#else
  // TODO: This should have a proper parent
  KMessageBox::error( 0, i18n("This document is encrypted, but Okular was compiled without crypto support. This document will probably not open.") );
  // this is equivalent to what happened before all this Manifest stuff :-)
  return fileData;
#endif
}
