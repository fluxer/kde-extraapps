// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// fontMap.cpp
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

#include <config.h>

#ifdef HAVE_FREETYPE

#include "fontMap.h"
#include "kvs_debug.h"

#include <QFile>
#include <QProcess>
#include <QTextStream>

//#define DEBUG_FONTMAP


fontMap::fontMap()
{
  // Read the map file of ps2pk which will provide us with a
  // dictionary "TeX Font names" <-> "Name of font files, Font Names
  // and Encodings" (example: the font "Times-Roman" is called
  // "ptmr8y" in the DVI file, but the Type1 font file name is
  // "utmr8a.pfb". We use the map file of "ps2pk" because that progam
  // has, like kdvi (and unlike dvips), no built-in fonts.

  // Finding ps2pk.map is not easy. In teTeX < 3.0, the kpsewhich
  // program REQUIRES the option "--format=dvips config". In teTeX =
  // 3.0, the option "--format=map" MUST be used. Since there is no
  // way to give both options at the same time, there is seemingly no
  // other way than to try both options one after another. We use the
  // teTeX 3.0 format first.
  QProcess kpsewhich;
  kpsewhich.start("kpsewhich",
                  QStringList() << "--format=map" << "ps2pk.map",
                  QIODevice::ReadOnly|QIODevice::Text);

  if (!kpsewhich.waitForStarted()) {
    kError(kvs::dvi) << "fontMap::fontMap(): kpsewhich could not be started." << endl;
    return;
  }

  // We wait here while the external program runs concurrently.
  kpsewhich.waitForFinished(-1);

  QString map_fileName = QString(kpsewhich.readAll()).trimmed();
  if (map_fileName.isEmpty()) {
    // Map file not found? Then we try the teTeX < 3.0 way of finding
    // the file.
    kpsewhich.start("kpsewhich",
                    QStringList() << "--format=dvips config" << "ps2pk.map",
                    QIODevice::ReadOnly|QIODevice::Text);
    if (!kpsewhich.waitForStarted()) {
      kError(kvs::dvi) << "fontMap::fontMap(): kpsewhich could not be started." << endl;
      return;
    }

    kpsewhich.waitForFinished(-1);

    map_fileName = QString(kpsewhich.readAll()).trimmed();
    // If both versions fail, then there is nothing left to do.
    if (map_fileName.isEmpty()) {
      kError(kvs::dvi) << "fontMap::fontMap(): The file 'ps2pk.map' could not be found by kpsewhich." << endl;
      return;
    }
  }

  QFile file( map_fileName );
  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream stream( &file );
    QString line;
    while ( !stream.atEnd() ) {
      line = stream.readLine().simplified();
      if (line.isEmpty() || (line.at(0) == '%'))
        continue;

      QString TeXName  = line.section(' ', 0, 0);
      QString FullName = line.section(' ', 1, 1);
      QString fontFileName = line.section('<', -1).trimmed().section(' ', 0, 0);
      QString encodingName = line.section('<', -2, -2).trimmed().section(' ', 0, 0);
      // It seems that sometimes the encoding is prepended by the
      // letter '[', which we ignore
      if ((!encodingName.isEmpty()) && (encodingName[0] == '['))
        encodingName = encodingName.mid(1);

      double slant = 0.0;
      int i = line.indexOf("SlantFont");
      if (i >= 0) {
        bool ok;
        slant = line.left(i).section(' ', -1, -1 ,QString::SectionSkipEmpty).toDouble(&ok);
        if (ok == false)
          slant = 0.0;
      }

      fontMapEntry &entry = fontMapEntries[TeXName];

      entry.slant        = slant;
      entry.fontFileName = fontFileName;
      entry.fullFontName = FullName;
      if (encodingName.endsWith(".enc"))
        entry.fontEncoding = encodingName;
      else
        entry.fontEncoding.clear();
    }
    file.close();
  } else
    kError(kvs::dvi) << QString("fontMap::fontMap(): The file '%1' could not be opened.").arg(map_fileName) << endl;

#ifdef DEBUG_FONTMAP
  kDebug(kvs::dvi) << "FontMap file parsed. Results:";
  QMap<QString, fontMapEntry>::Iterator it;
  for ( it = fontMapEntries.begin(); it != fontMapEntries.end(); ++it )
    kDebug(kvs::dvi) << "TeXName: " << it.key()
                  << ", FontFileName=" << it.data().fontFileName
                  << ", FullName=" << it.data().fullFontName
                  << ", Encoding=" << it.data().fontEncoding
                  << "." << endl;;
#endif
}


const QString &fontMap::findFileName(const QString &TeXName)
{
  QMap<QString, fontMapEntry>::Iterator it = fontMapEntries.find(TeXName);

  if (it != fontMapEntries.end())
    return it.value().fontFileName;

  static const QString nullstring;
  return nullstring;
}


const QString &fontMap::findFontName(const QString &TeXName)
{
  QMap<QString, fontMapEntry>::Iterator it = fontMapEntries.find(TeXName);

  if (it != fontMapEntries.end())
    return it.value().fullFontName;

  static const QString nullstring;
  return nullstring;
}


const QString &fontMap::findEncoding(const QString &TeXName)
{
  QMap<QString, fontMapEntry>::Iterator it = fontMapEntries.find(TeXName);

  if (it != fontMapEntries.end())
    return it.value().fontEncoding;

  static const QString nullstring;
  return nullstring;
}


double fontMap::findSlant(const QString &TeXName)
{
  QMap<QString, fontMapEntry>::Iterator it = fontMapEntries.find(TeXName);

  if (it != fontMapEntries.end())
    return it.value().slant;
  else
    return 0.0;
}

#endif // HAVE_FREETYPE
