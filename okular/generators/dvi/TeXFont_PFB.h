// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// TeXFont_PFB.h
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

// This file is compiled only if the FreeType library is present on
// the system

#ifndef _TEXFONT_PFB_H
#define _TEXFONT_PFB_H

#include "TeXFont.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class fontEncoding;
class glyph;


class TeXFont_PFB : public TeXFont {
 public:
  TeXFont_PFB(TeXFontDefinition *parent, fontEncoding *enc=0, double slant=0.0 );
  ~TeXFont_PFB();

  glyph* getGlyph(quint16 character, bool generateCharacterPixmap=false, const QColor& color=Qt::black);

 private:
  FT_Face       face;
  bool          fatalErrorInFontLoading;
  quint16      charMap[256];

  // This matrix is used internally to describes the slant, if
  // nonzero. Otherwise, this is undefined.
  FT_Matrix     transformationMatrix;
};

#endif
