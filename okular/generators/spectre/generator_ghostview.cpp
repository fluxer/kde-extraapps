/***************************************************************************
 *   Copyright (C) 2007-2008 by Albert Astals Cid <aacid@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "generator_ghostview.h"

#include <math.h>

#include <qfile.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <QtGui/QPrinter>

#include <kaboutdata.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>

#include <core/document.h>
#include <core/page.h>
#include <core/fileprinter.h>
#include <core/utils.h>

#include "ui_gssettingswidget.h"
#include "gssettings.h"

#include "rendererthread.h"

static KAboutData createAboutData()
{
    KAboutData aboutData(
         "okular_ghostview",
         "okular_ghostview",
         ki18n( "PS Backend" ),
         "0.1.7",
         ki18n( "A PostScript file renderer." ),
         KAboutData::License_GPL,
         ki18n( "© 2007-2008 Albert Astals Cid" ),
         ki18n( "Based on the Spectre library." )
    );
    aboutData.addAuthor( ki18n( "Albert Astals Cid" ), KLocalizedString(), "aacid@kde.org" );
    return aboutData;
}

OKULAR_EXPORT_PLUGIN(GSGenerator, createAboutData())

GSGenerator::GSGenerator( QObject *parent, const QVariantList &args ) :
    Okular::Generator( parent, args ),
    m_internalDocument(0),
    m_docInfo(0),
    m_request(0)
{
    setFeature( PrintPostscript );
    setFeature( PrintToFile );

    GSRendererThread *renderer = GSRendererThread::getCreateRenderer();
    if (!renderer->isRunning()) renderer->start();
    connect(renderer, SIGNAL(imageDone(QImage*,Okular::PixmapRequest*)),
                      SLOT(slotImageGenerated(QImage*,Okular::PixmapRequest*)),
                      Qt::QueuedConnection);
}

GSGenerator::~GSGenerator()
{
}

bool GSGenerator::reparseConfig()
{
    bool changed = false;
    if (m_internalDocument)
    {
#define SET_HINT(hintname, hintdefvalue, hintvar) \
{ \
    bool newhint = documentMetaData(hintname, hintdefvalue).toBool(); \
    if (newhint != cache_##hintvar) \
    { \
        cache_##hintvar = newhint; \
        changed = true; \
    } \
}
    SET_HINT("GraphicsAntialias", true, AAgfx)
    SET_HINT("TextAntialias", true, AAtext)
#undef SET_HINT
    }
    return changed;
}

void GSGenerator::addPages( KConfigDialog *dlg )
{
    Ui_GSSettingsWidget gsw;
    QWidget* w = new QWidget(dlg);
    gsw.setupUi(w);
    dlg->addPage(w, GSSettings::self(), i18n("Ghostscript"), "okular-gv", i18n("Ghostscript Backend Configuration") );
}

bool GSGenerator::print( QPrinter& printer )
{
    bool result = false;

    // Create tempfile to write to
    KTemporaryFile tf;
    tf.setSuffix( ".ps" );

    // Get list of pages to print
    QList<int> pageList = Okular::FilePrinter::pageList( printer,
                                               spectre_document_get_n_pages( m_internalDocument ),
                                               document()->currentPage() + 1,
                                               document()->bookmarkedPageList() );

    // Default to Postscript export, but if printing to PDF use that instead
    SpectreExporterFormat exportFormat = SPECTRE_EXPORTER_FORMAT_PS;
    if ( printer.outputFileName().right(3) == "pdf" )
    {
        exportFormat = SPECTRE_EXPORTER_FORMAT_PDF;
        tf.setSuffix(".pdf");
    }

    if ( !tf.open() )
        return false;

    SpectreExporter *exporter = spectre_exporter_new( m_internalDocument, exportFormat );
    SpectreStatus exportStatus = spectre_exporter_begin( exporter, tf.fileName().toAscii() );

    int i = 0;
    while ( i < pageList.count() && exportStatus == SPECTRE_STATUS_SUCCESS )
    {
        exportStatus = spectre_exporter_do_page( exporter, pageList.at( i ) - 1 );
        i++;
    }

    SpectreStatus endStatus = SPECTRE_STATUS_EXPORTER_ERROR;
    if (exportStatus == SPECTRE_STATUS_SUCCESS)
        endStatus = spectre_exporter_end( exporter );

    spectre_exporter_free( exporter );

    const QString fileName = tf.fileName();
    tf.close();

    if ( exportStatus == SPECTRE_STATUS_SUCCESS && endStatus == SPECTRE_STATUS_SUCCESS )
    {
        tf.setAutoRemove( false );
        int ret = Okular::FilePrinter::printFile( printer, fileName, document()->orientation(),
                                                  Okular::FilePrinter::SystemDeletesFiles,
                                                  Okular::FilePrinter::ApplicationSelectsPages,
                                                  document()->bookmarkedPageRange() );
        if ( ret >= 0 ) result = true;
    }

    return result;
}

bool GSGenerator::loadDocument( const QString & fileName, QVector< Okular::Page * > & pagesVector )
{
    cache_AAtext = documentMetaData("TextAntialias", true).toBool();
    cache_AAgfx = documentMetaData("GraphicsAntialias", true).toBool();

    m_internalDocument = spectre_document_new();
    spectre_document_load(m_internalDocument, QFile::encodeName(fileName));
    const SpectreStatus loadStatus = spectre_document_status(m_internalDocument);
    if (loadStatus != SPECTRE_STATUS_SUCCESS)
    {
        kDebug(4711) << "ERR:" << spectre_status_to_string(loadStatus);
        spectre_document_free(m_internalDocument);
        m_internalDocument = 0;
        return false;
    }
    pagesVector.resize( spectre_document_get_n_pages(m_internalDocument) );
    kDebug(4711) << "Page count:" << pagesVector.count();
    return loadPages(pagesVector);
}

bool GSGenerator::doCloseDocument()
{
    spectre_document_free(m_internalDocument);
    m_internalDocument = 0;

    delete m_docInfo;
    m_docInfo = 0;

    return true;
}

void GSGenerator::slotImageGenerated(QImage *img, Okular::PixmapRequest *request)
{
    // This can happen as GSInterpreterCMD is a singleton and on creation signals all the slots
    // of all the generators attached to it
    if (request != m_request) return;

    if ( !request->page()->isBoundingBoxKnown() )
        updatePageBoundingBox( request->page()->number(), Okular::Utils::imageBoundingBox( img ) );

    m_request = 0;
    QPixmap *pix = new QPixmap(QPixmap::fromImage(*img));
    delete img;
    request->page()->setPixmap( request->observer(), pix );
    signalPixmapRequestDone( request );
}

bool GSGenerator::loadPages( QVector< Okular::Page * > & pagesVector )
{
    for (uint i = 0; i < spectre_document_get_n_pages(m_internalDocument); i++)
    {
        SpectrePage     *page;
        int              width = 0, height = 0;
        SpectreOrientation pageOrientation = SPECTRE_ORIENTATION_PORTRAIT;
        page = spectre_document_get_page (m_internalDocument, i);
        if (spectre_document_status (m_internalDocument)) {
            kDebug(4711) << "Error getting page" << i << spectre_status_to_string(spectre_document_status(m_internalDocument));
        } else {
            spectre_page_get_size(page, &width, &height);
            pageOrientation = spectre_page_get_orientation(page);
        }
        spectre_page_free(page);
        if (pageOrientation % 2 == 1) qSwap(width, height);
        pagesVector[i] = new Okular::Page(i, width, height, orientation(pageOrientation));
    }
    return pagesVector.count() > 0;
}

void GSGenerator::generatePixmap( Okular::PixmapRequest * req )
{
    kDebug(4711) << "receiving" << *req;

    SpectrePage *page = spectre_document_get_page(m_internalDocument, req->pageNumber());

    GSRendererThread *renderer = GSRendererThread::getCreateRenderer();

    GSRendererThreadRequest gsreq(this);
    gsreq.spectrePage = page;
    gsreq.platformFonts = GSSettings::platformFonts();
    int graphicsAA = 1;
    int textAA = 1;
    if (cache_AAgfx) graphicsAA = 4;
    if (cache_AAtext) textAA = 4;
    gsreq.textAAbits = textAA;
    gsreq.graphicsAAbits = graphicsAA;

    gsreq.orientation = req->page()->orientation();
    if (req->page()->rotation() == Okular::Rotation90 ||
        req->page()->rotation() == Okular::Rotation270)
    {
        gsreq.magnify = qMax( (double)req->height() / req->page()->width(),
                              (double)req->width() / req->page()->height() );
    }
    else
    {
        gsreq.magnify = qMax( (double)req->width() / req->page()->width(),
                              (double)req->height() / req->page()->height() );
    }
    gsreq.request = req;
    m_request = req;
    renderer->addRequest(gsreq);
}

bool GSGenerator::canGeneratePixmap() const
{
    return !m_request;
}

const Okular::DocumentInfo * GSGenerator::generateDocumentInfo()
{
    if (!m_docInfo)
    {
        m_docInfo = new Okular::DocumentInfo();

        m_docInfo->set( Okular::DocumentInfo::Title, spectre_document_get_title(m_internalDocument) );
        m_docInfo->set( Okular::DocumentInfo::Author, spectre_document_get_for(m_internalDocument) );
        m_docInfo->set( Okular::DocumentInfo::Creator, spectre_document_get_creator(m_internalDocument) );
        m_docInfo->set( Okular::DocumentInfo::CreationDate, spectre_document_get_creation_date(m_internalDocument) );
        m_docInfo->set( "dscversion", spectre_document_get_format(m_internalDocument), i18n("Document version") );

        int languageLevel = spectre_document_get_language_level(m_internalDocument);
        if (languageLevel > 0) m_docInfo->set( "langlevel", QString::number(languageLevel), i18n("Language Level") );
        if (spectre_document_is_eps(m_internalDocument))
            m_docInfo->set( Okular::DocumentInfo::MimeType, "image/x-eps" );
        else
            m_docInfo->set( Okular::DocumentInfo::MimeType, "application/postscript" );

        m_docInfo->set( Okular::DocumentInfo::Pages, QString::number(spectre_document_get_n_pages(m_internalDocument)) );
    }
    return m_docInfo;
}

Okular::Rotation GSGenerator::orientation(SpectreOrientation pageOrientation) const
{
    switch (pageOrientation)
    {
        case SPECTRE_ORIENTATION_PORTRAIT:
            return Okular::Rotation0;
        case SPECTRE_ORIENTATION_LANDSCAPE:
            return Okular::Rotation90;
        case SPECTRE_ORIENTATION_REVERSE_PORTRAIT:
            return Okular::Rotation180;
        case SPECTRE_ORIENTATION_REVERSE_LANDSCAPE:
            return Okular::Rotation270;
    }
// get rid of warnings, should never happen
    return Okular::Rotation0;
}

QVariant GSGenerator::metaData(const QString &key, const QVariant &option) const
{
    Q_UNUSED(option)
    if (key == "DocumentTitle")
    {
        const char *title = spectre_document_get_title(m_internalDocument);
        if (title)
            return QString::fromAscii(title);
    }
    return QVariant();
}

#include "generator_ghostview.moc"
