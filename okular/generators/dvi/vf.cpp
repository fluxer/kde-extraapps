// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <config.h>

#include "dvi.h"
#include "dviRenderer.h"
#include "kvs_debug.h"
#include "xdvi.h"

#include <klocale.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern void oops(const QString& message);

/***
 ***   VF font reading routines.
 ***   Public routine is read_index---because virtual characters are presumed
 ***   to be short, we read the whole virtual font in at once, instead of
 ***   faulting in characters as needed.
 ***/

#define        LONG_CHAR        242

/*
 *  These are parameters which determine whether macros are combined for
 *  storage allocation purposes.  Small macros ( <= VF_PARM_1 bytes) are
 *  combined into chunks of size VF_PARM_2.
 */

#ifndef        VF_PARM_1
#define        VF_PARM_1        20
#endif
#ifndef        VF_PARM_2
#define        VF_PARM_2        256
#endif

/*
 *  The main routine
 */

void TeXFontDefinition::read_VF_index()
{
#ifdef DEBUG_FONTS
  kDebug(kvs::dvi) << "font::read_VF_index()";
#endif
  FILE *VF_file = file;
  unsigned char        cmnd;
  // available space for macros
  unsigned char        *avail, *availend;

  flags      |= FONT_VIRTUAL;
  set_char_p  = &dviRenderer::set_vf_char;
#ifdef DEBUG_FONTS
  kDebug(kvs::dvi) << "TeXFontDefinition::read_VF_index: reading VF pixel file " << filename;
#endif
  // Read preamble.
  fseek(VF_file, (long) one(VF_file), 1);        /* skip comment */
  quint32 const file_checksum = four(VF_file);

  if (file_checksum && checksum && file_checksum != checksum)
    kError(kvs::dvi) << "Checksum mismatch dvi = " << checksum << "u, vf = " << file_checksum <<
      "u) in font file" << filename << endl;
  (void) four(VF_file);                /* skip design size */

  // Read the fonts.
  first_font = NULL;
  while ((cmnd = one(VF_file)) >= FNTDEF1 && cmnd <= FNTDEF4) {
    int   TeXnumber = num(VF_file, (int) cmnd - FNTDEF1 + 1);
    quint32 checksum  = four(VF_file);
    quint32 scale     = four(VF_file);
    quint32 design    = four(VF_file);
    Q_UNUSED(design);
    quint16 len       = one(VF_file) + one(VF_file); /* sequence point in the middle */
    char *fontname  = new char[len + 1];
    fread(fontname, sizeof(char), len, VF_file);
    fontname[len] = '\0';

#ifdef DEBUG_FONTS
    kDebug(kvs::dvi) << "Virtual font defines subfont \"" << fontname << "\" scale=" << scale << " design=" << design;
#endif

    // According to Knuth's documentation found in the web source code
    // of the "vftovp" program (which seems to be the standard
    // definition of virtual fonts), the "scale" is a fixed point
    // number which describes extra enlargement that the virtual font
    // imposes. One obtains the enlargement by dividing 2^20.
    double enlargement_factor = double(scale)/(1<<20) * enlargement;

    //    TeXFontDefinition *newfontp = font_pool->appendx(fontname, checksum, (quint32)(scaled_size_in_DVI_units*enlargement_factor), enlargement_factor);
    TeXFontDefinition *newfontp = font_pool->appendx(fontname, checksum, (quint32)((double(scale)/(1<<20))*scaled_size_in_DVI_units), enlargement_factor);

    // Insert font in dictionary and make sure the dictionary is big
    // enough.
    if (vf_table.capacity()-2 <= vf_table.count())
      // Not quite optimal. The size of the dictionary should be a
      // prime. I don't care.
      vf_table.reserve(vf_table.capacity()*2);
    vf_table.insert(TeXnumber, newfontp);

    if (first_font == NULL)
      first_font = newfontp;
  }

  // Prepare macro array.
  macrotable = new macro[max_num_of_chars_in_font];
  if (macrotable == 0) {
    kError(kvs::dvi) << "Could not allocate memory for a macro table.";
    exit(0);
  }

  // Read macros.
  avail = availend = NULL;
  for (; cmnd <= LONG_CHAR; cmnd = one(VF_file)) {
    macro *m;
    int len;
    unsigned long cc;
    long width;

    if (cmnd == LONG_CHAR) {        /* long form packet */
      len = four(VF_file);
      cc = four(VF_file);
      width = four(VF_file);
      if (cc >= 256) {
        kError(kvs::dvi) << "Virtual character" << cc << "in font"
                  << fontname << "ignored.";
        fseek(VF_file, (long) len, 1);
        continue;
      }
    } else {        /* short form packet */
      len = cmnd;
      cc = one(VF_file);
      width = num(VF_file, 3);
    }
    m = &macrotable[cc];

    m->dvi_advance_in_units_of_design_size_by_2e20 = width;
    if (len > 0) {
      if (len <= availend - avail) {
        m->pos = avail;
        avail += len;
      } else {
        m->free_me = true;
        if (len <= VF_PARM_1) {
          m->pos = avail = new unsigned char [VF_PARM_2];
          availend = avail + VF_PARM_2;
          avail += len;
        } else
          m->pos = new unsigned char[len];
      }
      fread((char *) m->pos, 1, len, VF_file);
      m->end = m->pos + len;
    }
  }
  if (cmnd != POST)
    oops(i18n("Wrong command byte found in VF macro list: %1", cmnd));

  fclose (VF_file);
  file = NULL;
}
