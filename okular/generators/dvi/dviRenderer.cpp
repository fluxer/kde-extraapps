// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; c-brace-offset: 0; -*-
//
// Class: dviRenderer
//
// Class for rendering TeX DVI files.
// Part of KDVI- A previewer for TeX DVI files.
//
// (C) 2001-2005 Stefan Kebekus
// Distributed under the GPL
//

#include <config.h>

#include "dviRenderer.h"
#include "dviFile.h"
#include "dvisourcesplitter.h"
#include "hyperlink.h"
#include "kvs_debug.h"
#include "prebookmark.h"
#include "psgs.h"
//#include "renderedDviPagePixmap.h"
#include "dviPageInfo.h"

#include <math.h>
#include <QTime>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kvbox.h>

#include <QApplication>
#include <QCheckBox>
#include <QEventLoop>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProgressBar>
#include <QRegExp>

//#define DEBUG_DVIRENDERER



//------ now comes the dviRenderer class implementation ----------

dviRenderer::dviRenderer(bool useFontHinting)
  : dviFile(0),
    font_pool(useFontHinting),
    resolutionInDPI(0),
    embedPS_progress(0),
    embedPS_numOfProgressedFiles(0),
    shrinkfactor(3),
    source_href(0),
    HTML_href(0),
    editorCommand(""),
    PostScriptOutPutString(0),
    PS_interface(new ghostscript_interface),
    _postscript(true),
    line_boundary_encountered(false),
    word_boundary_encountered(false),
    current_page(0),
    penWidth_in_mInch(0),
    number_of_elements_in_path(0),
    currentlyDrawnPage(0),
    m_eventLoop(0),
    foreGroundPainter(0),
    fontpoolLocateFontsDone(false)
{
#ifdef DEBUG_DVIRENDERER
  //kDebug(kvs::dvi) << "dviRenderer( parent=" << par << " )";
#endif

  connect( &font_pool, SIGNAL( error(QString,int) ), this, SIGNAL( error(QString,int) ) );
  connect( &font_pool, SIGNAL( warning(QString,int) ), this, SIGNAL( warning(QString,int) ) );
  connect( PS_interface, SIGNAL( error(QString,int) ), this, SIGNAL( error(QString,int) ) );
}


dviRenderer::~dviRenderer()
{
#ifdef DEBUG_DVIRENDERER
  kDebug(kvs::dvi) << "~dviRenderer";
#endif

  QMutexLocker locker(&mutex);

  delete PS_interface;
  delete dviFile;
}

#if 0
void dviRenderer::setPrefs(bool flag_showPS, const QString &str_editorCommand, bool useFontHints )
{
  //QMutexLocker locker(&mutex);
  _postscript = flag_showPS;
  editorCommand = str_editorCommand;
  font_pool.setParameters( useFontHints );
}

#endif


//------ this function calls the dvi interpreter ----------


void dviRenderer::drawPage(RenderedDocumentPagePixmap* page)
{
#ifdef DEBUG_DVIRENDERER
  //kDebug(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called, page number " << page->pageNumber;
#endif

  // Paranoid safety checks
  if (page == 0) {
    kError(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called with argument == 0" << endl;
    return;
  }
  // Paranoid safety checks
  if (page->pageNumber == 0) {
    kError(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called for a documentPage with page number 0" << endl;
    return;
  }

  QMutexLocker locker(&mutex);


  if ( dviFile == 0 ) {
    kError(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called, but no dviFile class allocated." << endl;
    page->clear();
    return;
  }
  if (page->pageNumber > dviFile->total_pages) {
    kError(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called for a documentPage with page number " << page->pageNumber
                  << " but the current dviFile has only " << dviFile->total_pages << " pages." << endl;
    return;
  }
  if ( dviFile->dvi_Data() == 0 ) {
    kError(kvs::dvi) << "dviRenderer::drawPage(documentPage *) called, but no dviFile is loaded yet." << endl;
    page->clear();
    return;
  }

  /* locateFonts() is here just once (if it has not been executed
     not been executed yet), so that it is possible to easily intercept
     the cancel signal (because for example the user tries to open
     another document); it would not have been possible (or more
     complicated) in case it was still in the document loading section.
   */
  if ( !fontpoolLocateFontsDone ) {
    font_pool.locateFonts();
    fontpoolLocateFontsDone = true;
  }

  double resolution = page->resolution;

  if (resolution != resolutionInDPI)
    setResolution(resolution);
 
  currentlyDrawnPage     = page;
  shrinkfactor           = 1200/resolutionInDPI;
  current_page           = page->pageNumber-1;


  // Reset colors
  colorStack.clear();
  globalColor = Qt::black;

  int pageWidth = page->width;
  int pageHeight = page->height;
 
  QImage img(pageWidth, pageHeight, QImage::Format_RGB32);
  foreGroundPainter = new QPainter(&img);
  if (foreGroundPainter != 0) {
    errorMsg.clear();
    draw_page();
    delete foreGroundPainter;
    foreGroundPainter = 0;
  }
  else
  {
    kDebug(kvs::dvi) << "painter creation failed.";
  } 
  page->img = img;
//page->setImage(img);
 
  // Postprocess hyperlinks
  // Without that, based on the way TeX draws certain characters like german "Umlaute",
  // some hyperlinks would be broken into two overlapping parts, in the middle of a word.
  QVector<Hyperlink>::iterator i = page->hyperLinkList.begin();
  QVector<Hyperlink>::iterator j;
  while (i != page->hyperLinkList.end())
  {
    // Iterator j always points to the element after i.
    j = i;
    j++;

    if (j == page->hyperLinkList.end())
      break;

    Hyperlink& hi = *i;
    Hyperlink& hj = *j;

    bool merged = false;

    // Merge all hyperlinks that point to the same target, and have the same baseline.
    while (hi.linkText == hj.linkText && hi.baseline == hj.baseline)
    {
      merged = true;
      hi.box = hi.box.unite(hj.box);

      j++;
      if (j == page->hyperLinkList.end())
        break;

      hj = *j;
    }

    if (merged)
    {
      i = page->hyperLinkList.erase(++i, j);
    }
    else
    {
      i++;
    }
  }

  if (errorMsg.isEmpty() != true) {
    emit error(i18n("File corruption. %1", errorMsg), -1);
    errorMsg.clear();
    currentlyDrawnPage = 0;
    return;
  }
#if 0

  // Tell the user (once) if the DVI file contains source specials
  // ... we don't want our great feature to go unnoticed.
  RenderedDviPagePixmap* currentDVIPage = dynamic_cast<RenderedDviPagePixmap*>(currentlyDrawnPage);
  if (currentDVIPage)
  {
    if ((dviFile->sourceSpecialMarker == true) && (currentDVIPage->sourceHyperLinkList.size() > 0)) {
      dviFile->sourceSpecialMarker = false;
      // Show the dialog as soon as event processing is finished, and
      // the program is idle
      //FIXME
      //QTimer::singleShot( 0, this, SLOT(showThatSourceInformationIsPresent()) );
    }
  }
#endif
  currentlyDrawnPage = 0;
}


void dviRenderer::getText(RenderedDocumentPagePixmap* page)
{
  bool postscriptBackup = _postscript;
  // Disable postscript-specials temporarely to speed up text extraction.
  _postscript = false;

  drawPage(page);

  _postscript = postscriptBackup;
}

/*
void dviRenderer::showThatSourceInformationIsPresent()
{
  // In principle, we should use a KMessagebox here, but we want to
  // add a button "Explain in more detail..." which opens the
  // Helpcenter. Thus, we practically re-implement the KMessagebox
  // here. Most of the code is stolen from there.

  // Check if the 'Don't show again' feature was used
  KConfig *config = KGlobal::config();
  KConfigGroup saver(config, "Notification Messages");
  bool showMsg = config->readEntry( "KDVI-info_on_source_specials", true);

  if (showMsg) {
    KDialogBase dialog(i18n("KDVI: Information"), KDialogBase::Yes, KDialogBase::Yes, KDialogBase::Yes,
                       parentWidget, "information", true, true, KStandardGuiItem::ok());

    KVBox *topcontents = new KVBox (&dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint()*2);

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);

    lay->addStretch(1);
    QLabel *label1 = new QLabel( contents);
    label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
    lay->addWidget(label1);
    QLabel *label2 = new QLabel( i18n("<qt>This DVI file contains source file information. You may click into the text with the "
                                      "middle mouse button, and an editor will open the TeX-source file immediately.</qt>"),
                                 contents);
    label2->setMinimumSize(label2->sizeHint());
    lay->addWidget(label2);
    lay->addStretch(1);
    QSize extraSize = QSize(50,30);
    QCheckBox *checkbox = new QCheckBox(i18n("Do not show this message again"), topcontents);
    extraSize = QSize(50,0);
    dialog.setHelpLinkText(i18n("Explain in more detail..."));
    dialog.setHelp("inverse-search", "kdvi");
    dialog.enableLinkedHelp(true);
    dialog.setMainWidget(topcontents);
    dialog.enableButtonSeparator(false);
    dialog.incInitialSize( extraSize );
    dialog.exec();

    showMsg = !checkbox->isChecked();
    if (!showMsg) {
      KConfigGroup saver(config, "Notification Messages");
      config->writeEntry( "KDVI-info_on_source_specials", showMsg);
    }
    config->sync();
  }
}
*/

void dviRenderer::embedPostScript()
{
#ifdef DEBUG_DVIRENDERER
  kDebug(kvs::dvi) << "dviRenderer::embedPostScript()";
#endif

  if (!dviFile)
    return;

/*  embedPS_progress = new KProgressDialog(parentWidget,
                                         i18n("Embedding PostScript Files"), QString(), true); */
  if (!embedPS_progress)
    return;
  embedPS_progress->setAllowCancel(false);
  embedPS_progress->showCancelButton(false);
  embedPS_progress->setMinimumDuration(400);
  embedPS_progress->progressBar()->setMaximum(dviFile->numberOfExternalPSFiles);
  embedPS_progress->progressBar()->setValue(0);
  embedPS_numOfProgressedFiles = 0;

  quint16 currPageSav = current_page;
  errorMsg.clear();
  for(current_page=0; current_page < dviFile->total_pages; current_page++) {
    if (current_page < dviFile->total_pages) {
      command_pointer = dviFile->dvi_Data() + dviFile->page_offset[int(current_page)];
      end_pointer     = dviFile->dvi_Data() + dviFile->page_offset[int(current_page+1)];
    } else
      command_pointer = end_pointer = 0;

    memset((char *) &currinf.data, 0, sizeof(currinf.data));
    currinf.fonttable = &(dviFile->tn_table);
    currinf._virtual  = NULL;
    prescan(&dviRenderer::prescan_embedPS);
  }

  delete embedPS_progress;
  embedPS_progress = 0;

  if (!errorMsg.isEmpty()) {
    emit warning(i18n("Not all PostScript files could be embedded into your document. %1", errorMsg), -1);
    errorMsg.clear();
  } else {
    emit notice(i18n("All external PostScript files were embedded into your document."), -1);
  }


  // Prescan phase starts here
#ifdef PERFORMANCE_MEASUREMENT
  //kDebug(kvs::dvi) << "Time elapsed till prescan phase starts " << performanceTimer.elapsed() << "ms";
  //QTime preScanTimer;
  //preScanTimer.start();
#endif
  dviFile->numberOfExternalPSFiles = 0;
  prebookmarks.clear();
  for(current_page=0; current_page < dviFile->total_pages; current_page++) {
    PostScriptOutPutString = new QString();

    if (current_page < dviFile->total_pages) {
      command_pointer = dviFile->dvi_Data() + dviFile->page_offset[int(current_page)];
      end_pointer     = dviFile->dvi_Data() + dviFile->page_offset[int(current_page+1)];
    } else
      command_pointer = end_pointer = 0;

    memset((char *) &currinf.data, 0, sizeof(currinf.data));
    currinf.fonttable = &(dviFile->tn_table);
    currinf._virtual  = NULL;

    prescan(&dviRenderer::prescan_parseSpecials);

    if (!PostScriptOutPutString->isEmpty())
      PS_interface->setPostScript(current_page, *PostScriptOutPutString);
    delete PostScriptOutPutString;
  }
  PostScriptOutPutString = NULL;


#ifdef PERFORMANCE_MEASUREMENT
  //kDebug(kvs::dvi) << "Time required for prescan phase: " << preScanTimer.restart() << "ms";
#endif
  current_page = currPageSav;
  _isModified = true;
}


bool dviRenderer::isValidFile(const QString& filename) const
{
  QFile f(filename);
  if (!f.open(QIODevice::ReadOnly))
    return false;

  unsigned char test[4];
  if ( f.read( (char *)test,2)<2 || test[0] != 247 || test[1] != 2 )
    return false;

  int n = f.size();
  if ( n < 134 )        // Too short for a dvi file
    return false;
  f.seek( n-4 );

  unsigned char trailer[4] = { 0xdf,0xdf,0xdf,0xdf };

  if ( f.read( (char *)test, 4 )<4 || strncmp( (char *)test, (char *) trailer, 4 ) )
    return false;
  // We suppose now that the dvi file is complete and OK
  return true;
}

bool dviRenderer::setFile(const QString &fname, const KUrl &base)
{
#ifdef DEBUG_DVIRENDERER
  kDebug(kvs::dvi) << "dviRenderer::setFile( fname='" << fname << "' )"; //, ref='" << ref << "', sourceMarker=" << sourceMarker << " )";
#endif

  //QMutexLocker lock(&mutex);

  QFileInfo fi(fname);
  QString   filename = fi.absoluteFilePath();

  // If fname is the empty string, then this means: "close". Delete
  // the dvifile and the pixmap.
  if (fname.isEmpty()) {
    // Delete DVI file
    delete dviFile;
    dviFile = 0;
    return true;
  }


  // Make sure the file actually exists.
  if (!fi.exists() || fi.isDir()) {
    emit error(i18n("The specified file '%1' does not exist.", filename), -1);
    return false;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  dvifile *dviFile_new = new dvifile(filename, &font_pool);

  if ((dviFile == 0) || (dviFile->filename != filename))
    dviFile_new->sourceSpecialMarker = true;
  else
    dviFile_new->sourceSpecialMarker = false;

  if ((dviFile_new->dvi_Data() == NULL)||(dviFile_new->errorMsg.isEmpty() != true)) {
    QApplication::restoreOverrideCursor();
    if (dviFile_new->errorMsg.isEmpty() != true)
      emit error(i18n("File corruption. %1", dviFile_new->errorMsg), -1);
    delete dviFile_new;
    return false;
  }

  delete dviFile;
  dviFile = dviFile_new;
  numPages = dviFile->total_pages;
  _isModified = false;
  baseURL = base;

  font_pool.setExtraSearchPath( fi.absolutePath() );
  font_pool.setCMperDVIunit( dviFile->getCmPerDVIunit() );

  // Extract PostScript from the DVI file, and store the PostScript
  // specials in PostScriptDirectory, and the headers in the
  // PostScriptHeaderString.
  PS_interface->clear();

  // If the DVI file comes from a remote URL (e.g. downloaded from a
  // web server), we limit the PostScript files that can be accessed
  // by this file to the download directory, in order to limit the
  // possibilities of a denial of service attack.
  QString includePath;
  if (!baseURL.isLocalFile()) {
    includePath = filename;
    includePath.truncate(includePath.lastIndexOf('/'));
  }
  PS_interface->setIncludePath(includePath);

  // We will also generate a list of hyperlink-anchors and source-file
  // anchors in the document. So declare the existing lists empty.
  //anchorList.clear();
  sourceHyperLinkAnchors.clear();
  //bookmarks.clear();
  prebookmarks.clear();

  if (dviFile->page_offset.isEmpty() == true)
    return false;

  // We should pre-scan the document now (to extract embedded,
  // PostScript, Hyperlinks, ets).

  // PRESCAN STARTS HERE
#ifdef PERFORMANCE_MEASUREMENT
  //kDebug(kvs::dvi) << "Time elapsed till prescan phase starts " << performanceTimer.elapsed() << "ms";
  //QTime preScanTimer;
  //preScanTimer.start();
#endif
  dviFile->numberOfExternalPSFiles = 0;
  quint16 currPageSav = current_page;
  prebookmarks.clear();

  for(current_page=0; current_page < dviFile->total_pages; current_page++) {
    PostScriptOutPutString = new QString();

    if (current_page < dviFile->total_pages) {
      command_pointer = dviFile->dvi_Data() + dviFile->page_offset[int(current_page)];
      end_pointer     = dviFile->dvi_Data() + dviFile->page_offset[int(current_page+1)];
    } else
      command_pointer = end_pointer = 0;

    memset((char *) &currinf.data, 0, sizeof(currinf.data));
    currinf.fonttable = &(dviFile->tn_table);
    currinf._virtual  = NULL;
    prescan(&dviRenderer::prescan_parseSpecials);

    if (!PostScriptOutPutString->isEmpty())
      PS_interface->setPostScript(current_page, *PostScriptOutPutString);
    delete PostScriptOutPutString;
  }
  PostScriptOutPutString = NULL;

#if 0
  // Generate the list of bookmarks
  bookmarks.clear();
  Q3PtrStack<Bookmark> stack;
  stack.setAutoDelete (false);
  QVector<PreBookmark>::iterator it;
  for( it = prebookmarks.begin(); it != prebookmarks.end(); ++it ) {
    Bookmark *bmk = new Bookmark((*it).title, findAnchor((*it).anchorName));
    if (stack.isEmpty())
      bookmarks.append(bmk);
    else {
      stack.top()->subordinateBookmarks.append(bmk);
      stack.remove();
    }
    for(int i=0; i<(*it).noOfChildren; i++)
      stack.push(bmk);
  }
  prebookmarks.clear();
#endif

#ifdef PERFORMANCE_MEASUREMENT
  //kDebug(kvs::dvi) << "Time required for prescan phase: " << preScanTimer.restart() << "ms";
#endif
  current_page = currPageSav;
  // PRESCAN ENDS HERE

  pageSizes.resize(0);
  if (dviFile->suggestedPageSize != 0) {
    // Fill the vector pageSizes with total_pages identical entries
    pageSizes.fill(*(dviFile->suggestedPageSize), dviFile->total_pages);
  }
  QApplication::restoreOverrideCursor();
  return true;
}

Anchor dviRenderer::parseReference(const QString &reference)
{
  QMutexLocker locker(&mutex);

#ifdef DEBUG_DVIRENDERER
  kError(kvs::dvi) << "dviRenderer::parseReference( " << reference << " ) called" << endl;
#endif

  if (dviFile == 0)
    return Anchor();

  // case 1: The reference is a number, which we'll interpret as a
  // page number.
  bool ok;
  int page = reference.toInt ( &ok );
  if (ok == true) {
    if (page < 0)
      page = 0;
    if (page > dviFile->total_pages)
      page = dviFile->total_pages;

    return Anchor(page, Length() );
  }

  // case 2: The reference is of form "src:1111Filename", where "1111"
  // points to line number 1111 in the file "Filename". KDVI then
  // looks for source specials of the form "src:xxxxFilename", and
  // tries to find the special with the biggest xxxx
  if (reference.indexOf("src:", 0, Qt::CaseInsensitive) == 0) {

    // Extract the file name and the numeral part from the reference string
    DVI_SourceFileSplitter splitter(reference, dviFile->filename);
    quint32 refLineNumber = splitter.line();
    QString  refFileName   = splitter.filePath();

    if (sourceHyperLinkAnchors.isEmpty()) {
      emit warning(i18n("You have asked Okular to locate the place in the DVI file which corresponds to "
                        "line %1 in the TeX-file %2. It seems, however, that the DVI file "
                        "does not contain the necessary source file information. ",
                        refLineNumber, refFileName), -1);
      return Anchor();
    }

    // Go through the list of source file anchors, and find the anchor
    // whose line number is the biggest among those that are smaller
    // than the refLineNumber. That way, the position in the DVI file
    // which is highlighted is always a little further up than the
    // position in the editor, e.g. if the DVI file contains
    // positional information at the beginning of every paragraph,
    // KDVI jumps to the beginning of the paragraph that the cursor is
    // in, and never to the next paragraph. If source file anchors for
    // the refFileName can be found, but none of their line numbers is
    // smaller than the refLineNumber, the reason is most likely, that
    // the cursor in the editor stands somewhere in the preamble of
    // the LaTeX file. In that case, we jump to the beginning of the
    // document.
    bool anchorForRefFileFound = false; // Flag that is set if source file anchors for the refFileName could be found at all

    QVector<DVI_SourceFileAnchor>::iterator bestMatch = sourceHyperLinkAnchors.end();
    QVector<DVI_SourceFileAnchor>::iterator it;
    for( it = sourceHyperLinkAnchors.begin(); it != sourceHyperLinkAnchors.end(); ++it )
      if (refFileName.trimmed() == it->fileName.trimmed()
      || refFileName.trimmed() == it->fileName.trimmed() + ".tex"
      ) {
        anchorForRefFileFound = true;

        if ( (it->line <= refLineNumber) &&
             ( (bestMatch == sourceHyperLinkAnchors.end()) || (it->line > bestMatch->line) ) )
          bestMatch = it;
      }

    if (bestMatch != sourceHyperLinkAnchors.end())
      return Anchor(bestMatch->page, bestMatch->distance_from_top);
    else
      if (anchorForRefFileFound == false)
      {
        emit warning(i18n("Okular was not able to locate the place in the DVI file which corresponds to "
                          "line %1 in the TeX-file %2.", refLineNumber, refFileName), -1);
      }
      else
        return Anchor();
    return Anchor();
  }
  return Anchor();
}

void dviRenderer::setResolution(double resolution_in_DPI)
{
  // Ignore minute changes. The difference to the current value would
  // hardly be visible anyway. That saves a lot of re-painting,
  // e.g. when the user resizes the window, and a flickery mouse
  // changes the window size by 1 pixel all the time.
  if (fabs(resolutionInDPI-resolution_in_DPI) < 1)
    return;

  resolutionInDPI = resolution_in_DPI;

  // Pass the information on to the font pool.
  font_pool.setDisplayResolution( resolutionInDPI );
  shrinkfactor = 1200/resolutionInDPI;
  return;
}


void dviRenderer::handleSRCLink(const QString &linkText, const QPoint& point, DocumentWidget *win)
{
  Q_UNUSED( linkText );
  Q_UNUSED( point );
  Q_UNUSED( win );
#if 0
  KSharedPtr<DVISourceEditor> editor(new DVISourceEditor(*this, parentWidget, linkText, point, win));
  if (editor->started())
    editor_ = editor;
#endif
}


QString dviRenderer::PDFencodingToQString(const QString& _pdfstring)
{
  // This method locates special PDF characters in a string and
  // replaces them by UTF8. See Section 3.2.3 of the PDF reference
  // guide for information.
  QString pdfstring = _pdfstring;
  pdfstring = pdfstring.replace("\\n", "\n");
  pdfstring = pdfstring.replace("\\r", "\n");
  pdfstring = pdfstring.replace("\\t", "\t");
  pdfstring = pdfstring.replace("\\f", "\f");
  pdfstring = pdfstring.replace("\\(", "(");
  pdfstring = pdfstring.replace("\\)", ")");
  pdfstring = pdfstring.replace("\\\\", "\\");

  // Now replace octal character codes with the characters they encode
  int pos;
  QRegExp rx( "(\\\\)(\\d\\d\\d)" );  // matches "\xyz" where x,y,z are numbers
  while((pos = rx.indexIn( pdfstring )) != -1) {
    pdfstring = pdfstring.replace(pos, 4, QChar(rx.cap(2).toInt(0,8)));
  }
  rx.setPattern( "(\\\\)(\\d\\d)" );  // matches "\xy" where x,y are numbers
  while((pos = rx.indexIn( pdfstring )) != -1) {
    pdfstring = pdfstring.replace(pos, 3, QChar(rx.cap(2).toInt(0,8)));
  }
  rx.setPattern( "(\\\\)(\\d)" );  // matches "\x" where x is a number
  while((pos = rx.indexIn( pdfstring )) != -1) {
    pdfstring = pdfstring.replace(pos, 4, QChar(rx.cap(2).toInt(0,8)));
  }
  return pdfstring;
}


void dviRenderer::exportPDF()
{
/*
  KSharedPtr<DVIExport> exporter(new DVIExportToPDF(*this, parentWidget));
  if (exporter->started())
    all_exports_[exporter.data()] = exporter;
*/
}


void dviRenderer::exportPS(const QString& fname, const QStringList& options, QPrinter* printer, QPrinter::Orientation orientation)
{
  KSharedPtr<DVIExport> exporter(new DVIExportToPS(*this, fname, options, printer, font_pool.getUseFontHints(), orientation));
  if (exporter->started())
    all_exports_[exporter.data()] = exporter;
}

/*
void dviRenderer::editor_finished(const DVISourceEditor*)
{
  editor_.attach(0);
}
*/

void dviRenderer::export_finished(const DVIExport* key)
{
  typedef QMap<const DVIExport*, KSharedPtr<DVIExport> > ExportMap;
  ExportMap::iterator it = all_exports_.find(key);
  if (it != all_exports_.end())
    all_exports_.remove(key);
}

void dviRenderer::setEventLoop(QEventLoop *el) 
{
  if (el == NULL)
  {
     delete m_eventLoop;
     m_eventLoop = NULL; 
  }
  else
     m_eventLoop = el;
}

#include "dviRenderer.moc"
