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
 *
 * NOTE:
 *        xdvi is based on prior work as noted in the modification history, below.
 */

/*
 * DVI previewer for X.
 *
 * Eric Cooper, CMU, September 1985.
 *
 * Code derived from dvi-imagen.c.
 *
 * Modification history:
 * 1/1986        Modified for X.10        --Bob Scheifler, MIT LCS.
 * 7/1988        Modified for X.11        --Mark Eichin, MIT
 * 12/1988        Added 'R' option, toolkit, magnifying glass
 *                                        --Paul Vojta, UC Berkeley.
 * 2/1989        Added tpic support       --Jeffrey Lee, U of Toronto
 * 4/1989        Modified for System V    --Donald Richardson, Clarkson Univ.
 * 3/1990        Added VMS support        --Scott Allendorf, U of Iowa
 * 7/1990        Added reflection mode    --Michael Pak, Hebrew U of Jerusalem
 * 1/1992        Added greyscale code     --Till Brychcy, Techn. Univ. Muenchen
 *                                          and Lee Hetherington, MIT
 * 4/1994        Added DPS support, bounding box
 *                                        --Ricardo Telichevesky
 *                                          and Luis Miguel Silveira, MIT RLE.
 */

//#define DEBUG_RENDER 0

#include <config.h>

#include "dviRenderer.h"
#include "dvi.h"
#include "dviFile.h"
#include "hyperlink.h"
#include "kvs_debug.h"
#include "psgs.h"
//#include "renderedDviPagePixmap.h"
#include "TeXFont.h"
#include "textBox.h"
#include "xdvi.h"

#include <klocale.h>

#include <QPainter>



/** Routine to print characters.  */

void dviRenderer::set_char(unsigned int cmd, unsigned int ch)
{
#ifdef DEBUG_RENDER
  kDebug(kvs::dvi) << "set_char #" << ch;
#endif

  glyph *g;
  if (colorStack.isEmpty())
    g = ((TeXFont *)(currinf.fontp->font))->getGlyph(ch, true, globalColor);
  else
    g = ((TeXFont *)(currinf.fontp->font))->getGlyph(ch, true, colorStack.top());
  if (g == NULL)
    return;

  long dvi_h_sav = currinf.data.dvi_h;

  QImage pix = g->shrunkenCharacter;
  int x = ((int) ((currinf.data.dvi_h) / (shrinkfactor * 65536))) - g->x2;
  int y = currinf.data.pxl_v - g->y2;

  // Draw the character.
  foreGroundPainter->drawImage(x, y, pix);

  // Are we drawing text for a hyperlink? And are hyperlinks
  // enabled?
  if (HTML_href != NULL) {
    // Now set up a rectangle which is checked against every mouse
    // event.
    if (line_boundary_encountered == true) {
      // Set up hyperlink
      Hyperlink dhl;
      dhl.baseline = currinf.data.pxl_v;
      dhl.box.setRect(x, y, pix.width(), pix.height());
      dhl.linkText = *HTML_href;
      currentlyDrawnPage->hyperLinkList.push_back(dhl);
    } else {
      QRect dshunion = currentlyDrawnPage->hyperLinkList[currentlyDrawnPage->hyperLinkList.size()-1].box.unite(QRect(x, y, pix.width(), pix.height())) ;
      currentlyDrawnPage->hyperLinkList[currentlyDrawnPage->hyperLinkList.size()-1].box = dshunion;
    }
  }

  // Are we drawing text for a source hyperlink? And are source
  // hyperlinks enabled?
  // If we are printing source hyperlinks are irrelevant, otherwise we
  // actually got a pointer to a RenderedDviPagePixmap.
  RenderedDviPagePixmap* currentDVIPage = dynamic_cast<RenderedDviPagePixmap*>(currentlyDrawnPage);
  if (source_href != 0 && currentDVIPage) {
    // Now set up a rectangle which is checked against every mouse
    // event.
    if (line_boundary_encountered == true) {
      // Set up source hyperlinks
      Hyperlink dhl;
      dhl.baseline = currinf.data.pxl_v;
      dhl.box.setRect(x, y, pix.width(), pix.height());
      if (source_href != NULL)
        dhl.linkText = *source_href;
      else
        dhl.linkText = "";
      currentDVIPage->sourceHyperLinkList.push_back(dhl);
    } else {
      QRect dshunion = currentDVIPage->sourceHyperLinkList[currentDVIPage->sourceHyperLinkList.size()-1].box.unite(QRect(x, y, pix.width(), pix.height())) ;
      currentDVIPage->sourceHyperLinkList[currentDVIPage->sourceHyperLinkList.size()-1].box = dshunion;
    }
  }

  // Code for DVI -> text functions (e.g. marking of text, full text
  // search, etc.). Set up the currentlyDrawnPage->textBoxList.
  TextBox link;
  link.box.setRect(x, y, pix.width(), pix.height());
  link.text = "";
  currentlyDrawnPage->textBoxList.push_back(link);

  switch(ch) {
  case 0x0b:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "ff";
    break;
  case 0x0c:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "fi";
    break;
  case 0x0d:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "fl";
    break;
  case 0x0e:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "ffi";
    break;
  case 0x0f:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "ffl";
    break;

  case 0x7b:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '-';
    break;
  case 0x7c:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "---";
    break;
  case 0x7d:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "\"";
    break;
  case 0x7e:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '~';
    break;
  case 0x7f:
    currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += "@@"; // @@@ check!
    break;

  default:
    if ((ch >= 0x21) && (ch <= 0x7a))
      currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += QChar(ch);
    else
      currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '?';
    break;
  }


  if (cmd == PUT1)
    currinf.data.dvi_h = dvi_h_sav;
  else
    currinf.data.dvi_h += (int)(currinf.fontp->scaled_size_in_DVI_units * dviFile->getCmPerDVIunit() *
                                (1200.0 / 2.54)/16.0 * g->dvi_advance_in_units_of_design_size_by_2e20 + 0.5);

  word_boundary_encountered = false;
  line_boundary_encountered = false;
}

void dviRenderer::set_empty_char(unsigned int, unsigned int)
{
  return;
}

void dviRenderer::set_vf_char(unsigned int cmd, unsigned int ch)
{
#ifdef DEBUG_RENDER
  kDebug(kvs::dvi) << "dviRenderer::set_vf_char( cmd=" << cmd << ", ch=" << ch << " )";
#endif

  static unsigned char   c;
  macro *m = &currinf.fontp->macrotable[ch];
  if (m->pos == NULL) {
    kError(kvs::dvi) << "Character " << ch << " not defined in font " << currinf.fontp->fontname << endl;
    m->pos = m->end = &c;
    return;
  }

  long dvi_h_sav = currinf.data.dvi_h;

  struct drawinf oldinfo = currinf;
  currinf.data.w         = 0;
  currinf.data.x         = 0;
  currinf.data.y         = 0;
  currinf.data.z         = 0;

  currinf.fonttable         = &(currinf.fontp->vf_table);
  currinf._virtual          = currinf.fontp;
  quint8 *command_ptr_sav  = command_pointer;
  quint8 *end_ptr_sav      = end_pointer;
  command_pointer           = m->pos;
  end_pointer               = m->end;
  draw_part(currinf.fontp->scaled_size_in_DVI_units*(dviFile->getCmPerDVIunit() * 1200.0 / 2.54)/16.0, true);
  command_pointer           = command_ptr_sav;
  end_pointer               = end_ptr_sav;
  currinf = oldinfo;

  if (cmd == PUT1)
    currinf.data.dvi_h = dvi_h_sav;
  else
    currinf.data.dvi_h += (int)(currinf.fontp->scaled_size_in_DVI_units * dviFile->getCmPerDVIunit() *
                                (1200.0 / 2.54)/16.0 * m->dvi_advance_in_units_of_design_size_by_2e20 + 0.5);
}


void dviRenderer::set_no_char(unsigned int cmd, unsigned int ch)
{
#ifdef DEBUG_RENDER
  kDebug(kvs::dvi) << "dviRenderer::set_no_char( cmd=" << cmd << ", ch =" << ch << " )" ;
#endif

  if (currinf._virtual) {
    currinf.fontp = currinf._virtual->first_font;
    if (currinf.fontp != NULL) {
      currinf.set_char_p = currinf.fontp->set_char_p;
      (this->*currinf.set_char_p)(cmd, ch);
      return;
    }
  }

  errorMsg = i18n("The DVI code set a character of an unknown font.");
  return;
}


void dviRenderer::draw_part(double current_dimconv, bool is_vfmacro)
{
#ifdef DEBUG_RENDER
  kDebug(kvs::dvi) << "draw_part";
#endif

  qint32 RRtmp=0, WWtmp=0, XXtmp=0, YYtmp=0, ZZtmp=0;
  quint8 ch;

  currinf.fontp        = NULL;
  currinf.set_char_p   = &dviRenderer::set_no_char;

  int last_space_index = 0;
  bool space_encountered = false;
  bool after_space = false;
  for (;;) {
    space_encountered = false;
    ch = readUINT8();
    if (ch <= (unsigned char) (SETCHAR0 + 127)) {
      (this->*currinf.set_char_p)(ch, ch);
    } else
      if (FNTNUM0 <= ch && ch <= (unsigned char) (FNTNUM0 + 63)) {
        currinf.fontp = currinf.fonttable->value(ch - FNTNUM0);
        if (currinf.fontp == NULL) {
          errorMsg = i18n("The DVI code referred to font #%1, which was not previously defined.", ch - FNTNUM0);
          return;
        }
        currinf.set_char_p = currinf.fontp->set_char_p;
      } else {
        qint32 a, b;

        switch (ch) {
        case SET1:
        case PUT1:
          (this->*currinf.set_char_p)(ch, readUINT8());
          break;

        case SETRULE:
          if (is_vfmacro == false) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
          }
          /* Be careful, dvicopy outputs rules with height =
             0x80000000. We don't want any SIGFPE here. */
          a = readUINT32();
          b = readUINT32();
          b = ((long) (b *  current_dimconv));
          if (a > 0 && b > 0) {
            int h = ((int) ROUNDUP(((long) (a *  current_dimconv)), shrinkfactor * 65536));
            int w =  ((int) ROUNDUP(b, shrinkfactor * 65536));

            if (colorStack.isEmpty())
              foreGroundPainter->fillRect( ((int) ((currinf.data.dvi_h) / (shrinkfactor * 65536))),
                                         currinf.data.pxl_v - h + 1, w?w:1, h?h:1, globalColor );
            else
              foreGroundPainter->fillRect( ((int) ((currinf.data.dvi_h) / (shrinkfactor * 65536))),
                                        currinf.data.pxl_v - h + 1, w?w:1, h?h:1, colorStack.top() );
          }
          currinf.data.dvi_h += b;
          break;

        case PUTRULE:
          if (is_vfmacro == false) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
          }
          a = readUINT32();
          b = readUINT32();
          a = ((long) (a *  current_dimconv));
          b = ((long) (b *  current_dimconv));
          if (a > 0 && b > 0) {
            int h = ((int) ROUNDUP(a, shrinkfactor * 65536));
            int w = ((int) ROUNDUP(b, shrinkfactor * 65536));
            if (colorStack.isEmpty())
              foreGroundPainter->fillRect( ((int) ((currinf.data.dvi_h) / (shrinkfactor * 65536))),
                                         currinf.data.pxl_v - h + 1, w?w:1, h?h:1, globalColor );
            else
              foreGroundPainter->fillRect( ((int) ((currinf.data.dvi_h) / (shrinkfactor * 65536))),
                                        currinf.data.pxl_v - h + 1, w?w:1, h?h:1, colorStack.top() );
          }
          break;

        case NOP:
          break;

        case BOP:
          if (is_vfmacro == false) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
          }
          command_pointer += 11 * 4;
          currinf.data.dvi_h = 1200 << 16; // Reminder: DVI-coordinates start at (1",1") from top of page
          currinf.data.dvi_v = 1200;
          currinf.data.pxl_v = int(currinf.data.dvi_v/shrinkfactor);
          currinf.data.w = currinf.data.x = currinf.data.y = currinf.data.z = 0;
          break;

        case EOP:
          // Check if we are just at the end of a virtual font macro.
          if (is_vfmacro == false) {
            // This is really the end of a page, and not just the end
            // of a macro. Mark the end of the current word.
            word_boundary_encountered = true;
            line_boundary_encountered = true;
            // Sanity check for the dvi-file: The DVI-standard asserts
            // that at the end of a page, the stack should always be
            // empty.
            if (!stack.isEmpty()) {
              kDebug(kvs::dvi) << "DRAW: The stack was not empty when the EOP command was encountered.";
              errorMsg = i18n("The stack was not empty when the EOP command was encountered.");
              return;
            }
          }
          return;

        case PUSH:
          stack.push(currinf.data);
          break;

        case POP:
          if (stack.isEmpty()) {
            errorMsg = i18n("The stack was empty when a POP command was encountered.");
            return;
          } else
            currinf.data = stack.pop();
          word_boundary_encountered = true;
          line_boundary_encountered = true;
          break;

        case RIGHT1:
        case RIGHT2:
        case RIGHT3:
        case RIGHT4:
          RRtmp = readINT(ch - RIGHT1 + 1);

          // A horizontal motion in the range 4 * font space [f] < h <
          // font space [f] will be treated as a kern that is not
          // indicated in the printouts that DVItype produces between
          // brackets. We allow a larger space in the negative
          // direction than in the positive one, because TEX makes
          // comparatively large backspaces when it positions
          // accents. (comments stolen from the source of dvitype)
          if ((is_vfmacro == false) &&
              (currinf.fontp != 0) &&
              ((RRtmp >= currinf.fontp->scaled_size_in_DVI_units/6) || (RRtmp <= -4*(currinf.fontp->scaled_size_in_DVI_units/6))) &&
              (currentlyDrawnPage->textBoxList.size() > 0)) {
            //currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += ' ';
            space_encountered = true;
          }
          currinf.data.dvi_h += ((long) (RRtmp *  current_dimconv));
          break;

        case W1:
        case W2:
        case W3:
        case W4:
          WWtmp = readINT(ch - W0);
          currinf.data.w = ((long) (WWtmp *  current_dimconv));
        case W0:
          if ((is_vfmacro == false) &&
              (currinf.fontp != 0) &&
              ((WWtmp >= currinf.fontp->scaled_size_in_DVI_units/6) || (WWtmp <= -4*(currinf.fontp->scaled_size_in_DVI_units/6))) &&
              (currentlyDrawnPage->textBoxList.size() > 0) ) {
            //currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += ' ';
            space_encountered = true;
          }
          currinf.data.dvi_h += currinf.data.w;
          break;

        case X1:
        case X2:
        case X3:
        case X4:
          XXtmp = readINT(ch - X0);
          currinf.data.x = ((long) (XXtmp *  current_dimconv));
        case X0:
          if ((is_vfmacro == false)  &&
              (currinf.fontp != 0) &&
              ((XXtmp >= currinf.fontp->scaled_size_in_DVI_units/6) || (XXtmp <= -4*(currinf.fontp->scaled_size_in_DVI_units/6))) &&
              (currentlyDrawnPage->textBoxList.size() > 0)) {
            //currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += ' ';
            space_encountered = true;
          }
          currinf.data.dvi_h += currinf.data.x;
          break;

        case DOWN1:
        case DOWN2:
        case DOWN3:
        case DOWN4:
          {
            qint32 DDtmp = readINT(ch - DOWN1 + 1);
            if ((is_vfmacro == false) &&
                (currinf.fontp != 0) &&
                (abs(DDtmp) >= 5*(currinf.fontp->scaled_size_in_DVI_units/6)) &&
                (currentlyDrawnPage->textBoxList.size() > 0)) {
              word_boundary_encountered = true;
              line_boundary_encountered = true;
              space_encountered = true;
              if (abs(DDtmp) >= 10*(currinf.fontp->scaled_size_in_DVI_units/6))
                currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '\n';
            }
            currinf.data.dvi_v += ((long) (DDtmp *  current_dimconv))/65536;
            currinf.data.pxl_v  = int(currinf.data.dvi_v/shrinkfactor);
          }
          break;

        case Y1:
        case Y2:
        case Y3:
        case Y4:
          YYtmp = readINT(ch - Y0);
          currinf.data.y    = ((long) (YYtmp *  current_dimconv));
        case Y0:
          if ((is_vfmacro == false) &&
              (currinf.fontp != 0) &&
              (abs(YYtmp) >= 5*(currinf.fontp->scaled_size_in_DVI_units/6)) &&
              (currentlyDrawnPage->textBoxList.size() > 0)) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
            space_encountered = true;
            if (abs(YYtmp) >= 10*(currinf.fontp->scaled_size_in_DVI_units/6))
              currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '\n';
          }
          currinf.data.dvi_v += currinf.data.y/65536;
          currinf.data.pxl_v = int(currinf.data.dvi_v/shrinkfactor);
          break;

        case Z1:
        case Z2:
        case Z3:
        case Z4:
          ZZtmp = readINT(ch - Z0);
          currinf.data.z    = ((long) (ZZtmp *  current_dimconv));
        case Z0:
          if ((is_vfmacro == false) &&
              (currinf.fontp != 0) &&
              (abs(ZZtmp) >= 5*(currinf.fontp->scaled_size_in_DVI_units/6)) &&
              (currentlyDrawnPage->textBoxList.size() > 0)) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
            space_encountered = true;
            if (abs(ZZtmp) >= 10*(currinf.fontp->scaled_size_in_DVI_units/6))
              currentlyDrawnPage->textBoxList[currentlyDrawnPage->textBoxList.size()-1].text += '\n';
          }
          currinf.data.dvi_v += currinf.data.z/65536;
          currinf.data.pxl_v  = int(currinf.data.dvi_v/shrinkfactor);
          break;

        case FNT1:
        case FNT2:
        case FNT3:
          currinf.fontp = currinf.fonttable->value(readUINT(ch - FNT1 + 1));
          if (currinf.fontp == NULL) {
            errorMsg = i18n("The DVI code referred to a font which was not previously defined.");
            return;
          }
          currinf.set_char_p = currinf.fontp->set_char_p;
          break;

        case FNT4:
          currinf.fontp = currinf.fonttable->value(readINT(ch - FNT1 + 1));
          if (currinf.fontp == NULL) {
            errorMsg = i18n("The DVI code referred to a font which was not previously defined.");
            return;
          }
          currinf.set_char_p = currinf.fontp->set_char_p;
          break;

        case XXX1:
        case XXX2:
        case XXX3:
        case XXX4:
          if (is_vfmacro == false) {
            word_boundary_encountered = true;
            line_boundary_encountered = true;
            space_encountered = true;
          }
          a = readUINT(ch - XXX1 + 1);
          if (a > 0) {
            char        *cmd        = new char[a+1];
            strncpy(cmd, (char *)command_pointer, a);
            command_pointer += a;
            cmd[a] = '\0';
            applicationDoSpecial(cmd);
            delete [] cmd;
          }
          break;

        case FNTDEF1:
        case FNTDEF2:
        case FNTDEF3:
        case FNTDEF4:
          command_pointer += 12 + ch - FNTDEF1 + 1;
          {
            quint8 tempa = readUINT8();
            quint8 tempb = readUINT8();
            command_pointer += tempa + tempb;
          }
          break;

        case PRE:
        case POST:
        case POSTPOST:
          errorMsg = i18n("An illegal command was encountered.");
          return;
          break;

        default:
          errorMsg = i18n("The unknown op-code %1 was encountered.", ch);
          return;
        } /* end switch*/
      } /* end else (ch not a SETCHAR or FNTNUM) */

#ifdef DEBUG_RENDER
    if (currentlyDrawnPage->textBoxList.size() > 0)
      kDebug(kvs::dvi) << "Element:"
                       << currentlyDrawnPage->textBoxList.last().box
                       << currentlyDrawnPage->textBoxList.last().text
                       << " ? s:" << space_encountered
                       << " / nl:" << line_boundary_encountered
                       << " / w:" << word_boundary_encountered
                       << ", " << last_space_index << "/"
                       << currentlyDrawnPage->textBoxList.size();
#endif

    /* heuristic to properly detect newlines; a space is needed */
    if (after_space &&
        line_boundary_encountered && word_boundary_encountered) {
      if (currentlyDrawnPage->textBoxList.last().text.endsWith('\n'))
         currentlyDrawnPage->textBoxList.last().text.chop(1);
      currentlyDrawnPage->textBoxList.last().text += " \n";
      after_space = false;
    }

    /* a "space" has been found and there is some (new) character
       in the array */
    if (space_encountered &&
        (currentlyDrawnPage->textBoxList.size() > last_space_index)) {
      for (int lidx = last_space_index+1; lidx<currentlyDrawnPage->textBoxList.size(); ++lidx) {
        // merge two adjacent boxes which are part of the same word
        currentlyDrawnPage->textBoxList[lidx-1].box.setRight(currentlyDrawnPage->textBoxList[lidx].box.x());
      }
#ifdef DEBUG_RENDER
      QString lastword(currentlyDrawnPage->textBoxList[last_space_index].text);
      for (int lidx = last_space_index+1; lidx<currentlyDrawnPage->textBoxList.size(); ++lidx)
        lastword += currentlyDrawnPage->textBoxList[lidx].text;
      kDebug(kvs::dvi) << "space encountered: '" << lastword << "'";
#endif
      last_space_index = currentlyDrawnPage->textBoxList.size();
      after_space = true;
    }
  } /* end for */
}


void dviRenderer::draw_page()
{
  // Reset a couple of variables
  HTML_href         = 0;
  source_href       = 0;
  penWidth_in_mInch = 0.0;

  // Calling resize() here rather than clear() means that the memory
  // taken up by the vector is not freed. This is faster than
  // constantly allocating/freeing memory.
  currentlyDrawnPage->textBoxList.resize(0);

  RenderedDviPagePixmap* currentDVIPage = dynamic_cast<RenderedDviPagePixmap*>(currentlyDrawnPage);
  if (currentDVIPage)
  {
    currentDVIPage->sourceHyperLinkList.resize(0);
  }

#ifdef PERFORMANCE_MEASUREMENT
  // If this is the first time a page is drawn, take the time that is
  // elapsed till the kdvi_multipage was constructed, and print
  // it. Set the flag so that is message will not be printed again.
  if (performanceFlag == 0) {
    kDebug(kvs::dvi) << "Time elapsed till the first page is drawn: " << performanceTimer.restart() << "ms";
    performanceFlag = 1;
  }
#endif


#ifdef DEBUG_RENDER
  kDebug(kvs::dvi) <<"draw_page";
#endif

#if 0
  if (!accessibilityBackground)
  {
#endif
    foreGroundPainter->fillRect( foreGroundPainter->viewport(), PS_interface->getBackgroundColor(current_page) );
#if 0
  }
  else
  {
    // In accessiblity mode use the custom background color
    foreGroundPainter->fillRect( foreGroundPainter->viewport(), accessibilityBackgroundColor );
  }
#endif

  // Render the PostScript background, if there is one.
  if (_postscript)
  {
#if 0
    // In accessiblity mode use the custom background color
    if (accessibilityBackground)
    {
      // Flag permanent is set to false because otherwise we would not be able to restore
      // the original background color.
      PS_interface->setBackgroundColor(current_page, accessibilityBackgroundColor, false);
    }
    else
#endif
      PS_interface->restoreBackgroundColor(current_page);

    PS_interface->graphics(current_page, resolutionInDPI, dviFile->getMagnification(), foreGroundPainter);
  }

  // Now really write the text
  if (dviFile->page_offset.isEmpty() == true)
    return;
  if (current_page < dviFile->total_pages) {
    command_pointer = dviFile->dvi_Data() + dviFile->page_offset[int(current_page)];
    end_pointer     = dviFile->dvi_Data() + dviFile->page_offset[int(current_page+1)];
  } else
    command_pointer = end_pointer = 0;

  memset((char *) &currinf.data, 0, sizeof(currinf.data));
  currinf.fonttable      = &(dviFile->tn_table);
  currinf._virtual       = 0;

  double fontPixelPerDVIunit = dviFile->getCmPerDVIunit() * 1200.0/2.54;

  draw_part(65536.0*fontPixelPerDVIunit, false);
  if (HTML_href != 0) {
    delete HTML_href;
    HTML_href = 0;
  }
  if (source_href != 0) {
    delete source_href;
    source_href = 0;
  }
}
