// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// fontEncoding.cpp
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

#include <config.h>

#ifdef HAVE_FREETYPE

#include "fontEncoding.h"
#include "kvs_debug.h"

#include <QFile>
#include <QProcess>
#include <QTextStream>

//#define DEBUG_FONTENC


fontEncoding::fontEncoding(const QString &encName)
{
#ifdef DEBUG_FONTENC
  kDebug(kvs::dvi) << "fontEncoding( " << encName << " )";
#endif

  _isValid = false;
  // Use kpsewhich to find the encoding file.
  QProcess kpsewhich;
  kpsewhich.setReadChannelMode(QProcess::MergedChannels);

  kpsewhich.start("kpsewhich",
                  QStringList() << encName,
                  QIODevice::ReadOnly|QIODevice::Text);

  if (!kpsewhich.waitForStarted()) {
    kError(kvs::dvi) << "fontEncoding::fontEncoding(...): kpsewhich could not be started." << endl;
    return;
  }

  // We wait here while the external program runs concurrently.
  kpsewhich.waitForFinished(-1);

  const QString encFileName = QString(kpsewhich.readAll()).trimmed();
  if (encFileName.isEmpty()) {
    kError(kvs::dvi) << QString("fontEncoding::fontEncoding(...): The file '%1' could not be found by kpsewhich.").arg(encName) << endl;
    return;
  }

#ifdef DEBUG_FONTENC
  kDebug(kvs::dvi) << "FileName of the encoding: " << encFileName;
#endif

  QFile file( encFileName );
  if ( file.open( QIODevice::ReadOnly ) ) {
    // Read the file (excluding comments) into the QString variable
    // 'fileContent'
    QTextStream stream( &file );
    QString fileContent;
    while ( !stream.atEnd() )
      fileContent += stream.readLine().section('%', 0, 0); // line of text excluding '\n' until first '%'-sign
    file.close();

    fileContent = fileContent.trimmed();

    // Find the name of the encoding
    encodingFullName = fileContent.section('[', 0, 0).simplified().mid(1);
#ifdef DEBUG_FONTENC
    kDebug(kvs::dvi) << "encodingFullName: " << encodingFullName;
#endif

    fileContent = fileContent.section('[', 1, 1).section(']',0,0).simplified();
    const QStringList glyphNameList = fileContent.split('/', QString::SkipEmptyParts);

    int i = 0;
    for ( QStringList::ConstIterator it = glyphNameList.constBegin(); (it != glyphNameList.constEnd())&&(i<256); ++it ) {
      glyphNameVector[i] = (*it).simplified();
#ifdef DEBUG_FONTENC
      kDebug(kvs::dvi) << i << ": " << glyphNameVector[i];
#endif
      i++;
    }
    for(; i<256; i++)
      glyphNameVector[i] = ".notdef";
  } else {
    kError(kvs::dvi) << QString("fontEncoding::fontEncoding(...): The file '%1' could not be opened.").arg(encFileName) << endl;
    return;
  }

  _isValid = true;
}

#endif // HAVE_FREETYPE
