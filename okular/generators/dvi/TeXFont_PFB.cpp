// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
// TeXFont_PFB.cpp
//
// Part of KDVI - A DVI previewer for the KDE desktop environment
//
// (C) 2003 Stefan Kebekus
// Distributed under the GPL

// This file is compiled only if the FreeType library is present on
// the system

#include <config.h>

#ifdef HAVE_FREETYPE

#include "TeXFont_PFB.h"
#include "fontpool.h"
#include "kvs_debug.h"

#include <klocale.h>

#include <QImage>

//#define DEBUG_PFB 1


TeXFont_PFB::TeXFont_PFB(TeXFontDefinition *parent, fontEncoding *enc, double slant)
  : TeXFont(parent), face(0)
{
#ifdef DEBUG_PFB
  if (enc != 0)
    kDebug(kvs::dvi) << "TeXFont_PFB::TeXFont_PFB( parent=" << parent << ", encoding=" << enc->encodingFullName << " )";
  else
    kDebug(kvs::dvi) << "TeXFont_PFB::TeXFont_PFB( parent=" << parent << ", encoding=0 )";
#endif

  fatalErrorInFontLoading = false;

  int error = FT_New_Face( parent->font_pool->FreeType_library, parent->filename.toLocal8Bit(), 0, &face );

  if ( error == FT_Err_Unknown_File_Format ) {
    errorMessage = i18n("The font file %1 could be opened and read, but its font format is unsupported.", parent->filename);
    kError(kvs::dvi) << errorMessage << endl;
    fatalErrorInFontLoading = true;
    return;
  } else
    if ( error ) {
      errorMessage = i18n("The font file %1 is broken, or it could not be opened or read.", parent->filename);
      kError(kvs::dvi) << errorMessage << endl;
      fatalErrorInFontLoading = true;
      return;
    }

  // Take care of slanting, and transform all characters in the font, if necessary.
  if (slant != 0.0) {
    // Construct a transformation matrix for vertical shear which will
    // be used to transform the characters.
    transformationMatrix.xx = 0x10000;
    transformationMatrix.xy = (FT_Fixed)(slant * 0x10000);
    transformationMatrix.yx = 0;
    transformationMatrix.yy = 0x10000;

    FT_Set_Transform( face, &transformationMatrix, 0);
  }

  if (face->family_name != 0)
    parent->fullFontName = face->family_name;

  // Finally, we need to set up the charMap array, which maps TeX
  // character codes to glyph indices in the font. (Remark: the
  // charMap, and the font encoding procedure is necessary, because
  // TeX is only able to address character codes 0-255 while
  // e.g. Type1 fonts may contain several thousands of characters)
  if (enc != 0) {
    parent->fullEncodingName = enc->encodingFullName.remove(QString::fromLatin1( "Encoding" ));
    parent->fullEncodingName = enc->encodingFullName.remove(QString::fromLatin1( "encoding" ));

    // An encoding vector is given for this font, i.e. an array of
    // character names (such as: 'parenleft' or 'dotlessj'). We use
    // the FreeType library function 'FT_Get_Name_Index()' to
    // associate glyph indices to those names.
#ifdef DEBUG_PFB
    kDebug(kvs::dvi) << "Trying to associate glyph indices to names from the encoding vector.";
#endif
    for(int i=0; i<256; i++) {
      charMap[i] = FT_Get_Name_Index( face, (FT_String *)(enc->glyphNameVector[i].toAscii().data()) );
#ifdef DEBUG_PFB
      kDebug(kvs::dvi) << i << ": " << enc->glyphNameVector[i] << ", GlyphIndex=" <<  charMap[i];
#endif
    }
  } else {
    // If there is no encoding vector available, we check if the font
    // itself contains a charmap that could be used. An admissible
    // charMap will be stored under platform_id=7 and encoding_id=2.
    FT_CharMap  found = 0;
    for (int n = 0; n<face->num_charmaps; n++ ) {
      FT_CharMap charmap = face->charmaps[n];
      if ( charmap->platform_id == 7 && charmap->encoding_id == 2 ) {
        found = charmap;
        break;
      }
    }

    if ((found != 0) && (FT_Set_Charmap( face, found ) == 0)) {
      // Feed the charMap array with the charmap data found in the
      // previous step.
#ifdef DEBUG_PFB
      kDebug(kvs::dvi) << "No encoding given: using charmap platform=7, encoding=2 that is contained in the font.";
#endif
      for(int i=0; i<256; i++)
        charMap[i] = FT_Get_Char_Index( face, i );
    } else {
      if ((found == 0) && (face->charmap != 0)) {
#ifdef DEBUG_PFB
        kDebug(kvs::dvi) << "No encoding given: using charmap platform=" << face->charmap->platform_id <<
          ", encoding=" << face->charmap->encoding_id << " that is contained in the font." << endl;
#endif
        for(int i=0; i<256; i++)
          charMap[i] = FT_Get_Char_Index( face, i );
      } else {
        // As a last resort, we use the identity map.
#ifdef DEBUG_PFB
        kDebug(kvs::dvi) << "No encoding given, no suitable charmaps found in the font: using identity charmap.";
#endif
        for(int i=0; i<256; i++)
          charMap[i] = i;
      }
    }
  }
}


TeXFont_PFB::~TeXFont_PFB()
{
  FT_Done_Face( face );
}


glyph* TeXFont_PFB::getGlyph(quint16 ch, bool generateCharacterPixmap, const QColor& color)
{
#ifdef DEBUG_PFB
  kDebug(kvs::dvi) << "TeXFont_PFB::getGlyph( ch=" << ch << ", '" << (char)(ch) << "', generateCharacterPixmap=" << generateCharacterPixmap << " )";
#endif

  // Paranoia checks
  if (ch >= TeXFontDefinition::max_num_of_chars_in_font) {
    kError(kvs::dvi) << "TeXFont_PFB::getGlyph(): Argument is too big." << endl;
    return glyphtable;
  }

  // This is the address of the glyph that will be returned.
  struct glyph *g = glyphtable+ch;


  if (fatalErrorInFontLoading == true)
    return g;

  if ((generateCharacterPixmap == true) && ((g->shrunkenCharacter.isNull()) || (color != g->color)) ) {
    int error;
    unsigned int res =  (unsigned int)(parent->displayResolution_in_dpi/parent->enlargement +0.5);
    g->color = color;

    // Character height in 1/64th of points (reminder: 1 pt = 1/72 inch)
    // Only approximate, may vary from file to file!!!! @@@@@

    long int characterSize_in_printers_points_by_64 = (long int)((64.0*72.0*parent->scaled_size_in_DVI_units*parent->font_pool->getCMperDVIunit())/2.54 + 0.5 );
    error = FT_Set_Char_Size(face, 0, characterSize_in_printers_points_by_64, res, res );
    if (error) {
      QString msg = i18n("FreeType reported an error when setting the character size for font file %1.", parent->filename);
      if (errorMessage.isEmpty())
        errorMessage = msg;
      kError(kvs::dvi) << msg << endl;
      g->shrunkenCharacter = QImage(1, 1, QImage::Format_RGB32);
      g->shrunkenCharacter.fill(qRgb(255, 255, 255));
      return g;
    }

    // load glyph image into the slot and erase the previous one
    if (parent->font_pool->getUseFontHints() == true)
      error = FT_Load_Glyph(face, charMap[ch], FT_LOAD_DEFAULT );
    else
      error = FT_Load_Glyph(face, charMap[ch], FT_LOAD_NO_HINTING );

    if (error) {
      QString msg = i18n("FreeType is unable to load glyph #%1 from font file %2.", ch, parent->filename);
      if (errorMessage.isEmpty())
        errorMessage = msg;
      kError(kvs::dvi) << msg << endl;
      g->shrunkenCharacter = QImage(1, 1, QImage::Format_RGB32);
      g->shrunkenCharacter.fill(qRgb(255, 255, 255));
      return g;
    }

    // convert to an anti-aliased bitmap
    error = FT_Render_Glyph( face->glyph, ft_render_mode_normal );
    if (error) {
      QString msg = i18n("FreeType is unable to render glyph #%1 from font file %2.", ch, parent->filename);
      if (errorMessage.isEmpty())
        errorMessage = msg;
      kError(kvs::dvi) << msg << endl;
      g->shrunkenCharacter = QImage(1, 1, QImage::Format_RGB32);
      g->shrunkenCharacter.fill(qRgb(255, 255, 255));
      return g;
    }

    FT_GlyphSlot slot = face->glyph;

    if ((slot->bitmap.width == 0) || (slot->bitmap.rows == 0)) {
      if (errorMessage.isEmpty())
        errorMessage = i18n("Glyph #%1 is empty.", ch);
      kError(kvs::dvi) << i18n("Glyph #%1 from font file %2 is empty.", ch, parent->filename) << endl;
      g->shrunkenCharacter = QImage(15, 15 , QImage::Format_RGB32);
      g->shrunkenCharacter.fill(qRgb(255, 0, 0));
      g->x2 = 0;
      g->y2 = 15;
    } else {
      QImage imgi(slot->bitmap.width, slot->bitmap.rows, QImage::Format_ARGB32);

      // Do QPixmaps fully support the alpha channel? If yes, we use
      // that. Otherwise, use other routines as a fallback
      if (parent->font_pool->QPixmapSupportsAlpha) {
        // If the alpha channel is properly supported, we set the
        // character glyph to a colored rectangle, and define the
        // character outline only using the alpha channel. That
        // ensures good quality rendering for overlapping characters.
        uchar *srcScanLine = slot->bitmap.buffer;
        for(int row=0; row<slot->bitmap.rows; row++) {
          uchar *destScanLine = imgi.scanLine(row);
          for(int col=0; col<slot->bitmap.width; col++) {
            destScanLine[4*col+0] = color.blue();
            destScanLine[4*col+1] = color.green();
            destScanLine[4*col+2] = color.red();
            destScanLine[4*col+3] = srcScanLine[col];
          }
          srcScanLine += slot->bitmap.pitch;
        }
      } else {
        // If the alpha channel is not supported... QT seems to turn
        // the alpha channel into a crude bitmap which is used to mask
        // the resulting QPixmap. In this case, we define the
        // character outline using the image data, and use the alpha
        // channel only to store "maximally opaque" or "completely
        // transparent" values. When characters are rendered,
        // overlapping characters are no longer correctly drawn, but
        // quality is still sufficient for most purposes. One notable
        // exception is output from the gftodvi program, which will be
        // partially unreadable.
        quint16 rInv = 0xFF - color.red();
        quint16 gInv = 0xFF - color.green();
        quint16 bInv = 0xFF - color.blue();

        for(quint16 y=0; y<slot->bitmap.rows; y++) {
          quint8 *srcScanLine = slot->bitmap.buffer + y*slot->bitmap.pitch;
          unsigned int *destScanLine = (unsigned int *)imgi.scanLine(y);
          for(quint16 col=0; col<slot->bitmap.width; col++) {
            quint16 data =  *srcScanLine;
            // The value stored in "data" now has the following meaning:
            // data = 0 -> white; data = 0xff -> use "color"
            *destScanLine = qRgba(0xFF - (rInv*data + 0x7F) / 0xFF,
                                  0xFF - (gInv*data + 0x7F) / 0xFF,
                                  0xFF - (bInv*data + 0x7F) / 0xFF,
                                  (data > 0x03) ? 0xff : 0x00);
            destScanLine++;
            srcScanLine++;
          }
        }
      }

      g->shrunkenCharacter = imgi;
      g->x2 = -slot->bitmap_left;
      g->y2 = slot->bitmap_top;
    }
  }

  // Load glyph width, if that hasn't been done yet.
  if (g->dvi_advance_in_units_of_design_size_by_2e20 == 0) {
    int error = FT_Load_Glyph(face, charMap[ch], FT_LOAD_NO_SCALE);
    if (error) {
      QString msg = i18n("FreeType is unable to load metric for glyph #%1 from font file %2.", ch, parent->filename);
      if (errorMessage.isEmpty())
        errorMessage = msg;
      kError(kvs::dvi) << msg << endl;
      g->dvi_advance_in_units_of_design_size_by_2e20 =  1;
    }
    g->dvi_advance_in_units_of_design_size_by_2e20 =  (qint32)(((qint64)(1<<20) * (qint64)face->glyph->metrics.horiAdvance) / (qint64)face->units_per_EM);
  }

  return g;
}

#endif // HAVE_FREETYPE
