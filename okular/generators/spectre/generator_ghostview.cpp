/***************************************************************************
 *   Copyright (C) 2007-2008 by Albert Astals Cid <aacid@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "generator_ghostview.h"

#include <qfile.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qpainter.h>
#include <kaboutdata.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kmimetype.h>

#include <core/document.h>
#include <core/page.h>
#include <core/utils.h>

#include "ui_gssettingswidget.h"
#include "gssettings.h"

#include <math.h>

static KAboutData createAboutData()
{
    KAboutData aboutData(
         "okular_ghostview",
         "okular_ghostview",
         ki18n("PS Backend"),
         "0.1.7",
         ki18n("A PostScript file renderer."),
         KAboutData::License_GPL,
         ki18n("© 2007-2008 Albert Astals Cid")
    );
    aboutData.addAuthor(ki18n("Albert Astals Cid"), KLocalizedString(), "aacid@kde.org");
    return aboutData;
}

OKULAR_EXPORT_PLUGIN(GSGenerator, createAboutData())

GSGenerator::GSGenerator(QObject *parent, const QVariantList &args)
    : Okular::Generator(parent, args),
    m_internalDocument(0),
    m_docInfo(0),
    m_renderContext(0)
{
    m_renderContext = spectre_render_context_new();
    
    setFeature(Generator::Threaded);
}

GSGenerator::~GSGenerator()
{
    spectre_render_context_free(m_renderContext);
}

#define SET_HINT(hintname, hintdefvalue, hintvar) \
{ \
    bool newhint = documentMetaData(hintname, hintdefvalue).toBool(); \
    if (newhint != cache_##hintvar) \
    { \
        cache_##hintvar = newhint; \
        changed = true; \
    } \
}

bool GSGenerator::reparseConfig()
{
    bool changed = false;
    if (m_internalDocument) {
        SET_HINT("GraphicsAntialias", true, AAgfx)
        SET_HINT("TextAntialias", true, AAtext)
    }
    return changed;
}
#undef SET_HINT

void GSGenerator::addPages(KConfigDialog *dlg)
{
    Ui_GSSettingsWidget gsw;
    QWidget* w = new QWidget(dlg);
    gsw.setupUi(w);
    dlg->addPage(w, GSSettings::self(), i18n("Ghostscript"), "okular-gv", i18n("Ghostscript Backend Configuration"));
}

Okular::ExportFormat::List GSGenerator::exportFormats() const
{
    Okular::ExportFormat::List result;
    result.append(
        Okular::ExportFormat(
            KIcon(QString::fromLatin1("application-pdf")),
            i18n("PDF Document"),
            KMimeType::mimeType(QString::fromLatin1("application/pdf"))
        )
    );
    return result;
}

bool GSGenerator::exportTo( const QString &fileName, const Okular::ExportFormat &format )
{
    if (!m_internalDocument) {
        return false;
    }
    if (format.mimeType()->name() == QLatin1String("application/pdf")) {
        const QByteArray filenamebytes = QFile::encodeName(fileName);
        spectre_document_save_to_pdf(m_internalDocument, filenamebytes.constData());
        const SpectreStatus loadStatus = spectre_document_status(m_internalDocument);
        if (loadStatus != SPECTRE_STATUS_SUCCESS) {
            kDebug() << "ERR:" << spectre_status_to_string(loadStatus);
            return false;
        }
        return true;
    }
    return false;
}

bool GSGenerator::loadDocument(const QString &fileName, QVector<Okular::Page*> &pagesVector)
{
    cache_AAtext = documentMetaData("TextAntialias", true).toBool();
    cache_AAgfx = documentMetaData("GraphicsAntialias", true).toBool();

    m_internalDocument = spectre_document_new();
    spectre_document_load(m_internalDocument, QFile::encodeName(fileName));
    const SpectreStatus loadStatus = spectre_document_status(m_internalDocument);
    if (loadStatus != SPECTRE_STATUS_SUCCESS) {
        kDebug() << "ERR:" << spectre_status_to_string(loadStatus);
        spectre_document_free(m_internalDocument);
        m_internalDocument = 0;
        return false;
    }
    pagesVector.resize(spectre_document_get_n_pages(m_internalDocument));
    kDebug() << "Page count:" << pagesVector.count();
    for (uint i = 0; i < spectre_document_get_n_pages(m_internalDocument); i++) {
        int width = 0;
        int height = 0;
        SpectreOrientation pageOrientation = SPECTRE_ORIENTATION_PORTRAIT;
        SpectrePage *page = spectre_document_get_page (m_internalDocument, i);
        if (spectre_document_status (m_internalDocument)) {
            kDebug() << "Error getting page" << i << spectre_status_to_string(spectre_document_status(m_internalDocument));
        } else {
            spectre_page_get_size(page, &width, &height);
            pageOrientation = spectre_page_get_orientation(page);
        }
        spectre_page_free(page);
        if (pageOrientation % 2 == 1) {
            qSwap(width, height);
        }
        pagesVector[i] = new Okular::Page(i, width, height, orientation(pageOrientation));
    }
    return pagesVector.count() > 0;
}

bool GSGenerator::doCloseDocument()
{
    spectre_document_free(m_internalDocument);
    m_internalDocument = 0;

    delete m_docInfo;
    m_docInfo = 0;

    return true;
}

QImage GSGenerator::image(Okular::PixmapRequest *req)
{
    kDebug() << "receiving" << *req;

    SpectrePage *page = spectre_document_get_page(m_internalDocument, req->pageNumber());

    const int platformFonts = GSSettings::platformFonts();
    int graphicsAA = 1;
    int textAA = 1;
    if (cache_AAgfx) {
        graphicsAA = 4;
    }
    if (cache_AAtext) {
        textAA = 4;
    }

    const int orientation = req->page()->orientation();
    double magnify = 0.0;
    if (req->page()->rotation() == Okular::Rotation90 || req->page()->rotation() == Okular::Rotation270) {
        magnify = qMax(
            (double)req->height() / req->page()->width(),
            (double)req->width() / req->page()->height()
        );
    } else {
        magnify = qMax(
            (double)req->width() / req->page()->width(),
            (double)req->height() / req->page()->height()
        );
    }

    spectre_render_context_set_scale(m_renderContext, magnify, magnify);
    spectre_render_context_set_use_platform_fonts(m_renderContext, platformFonts);
    spectre_render_context_set_antialias_bits(m_renderContext, graphicsAA, textAA);
    // Do not use spectre_render_context_set_rotation makes some files not render correctly, e.g. bug210499.ps
    // so we basically do the rendering without any rotation and then rotate to the orientation as needed
    // spectre_render_context_set_rotation(m_renderContext, orientation);

    unsigned char *data = NULL;
    int row_length = 0;
    int wantedWidth = req->width();
    int wantedHeight = req->height();

    if (orientation % 2) {
        qSwap(wantedWidth, wantedHeight);
    }

    spectre_page_render(page, m_renderContext, &data, &row_length);

    // Katie needs the missing alpha of QImage::Format_RGB32 to be 0xff
    if (data && data[3] != 0xff) {
        for (int i = 3; i < row_length * wantedHeight; i += 4) {
            data[i] = 0xff;
        }
    }

    QImage img;
    if (row_length == wantedWidth * 4) {
        img = QImage(data, wantedWidth, wantedHeight, QImage::Format_RGB32);
    } else {
        // In case this ends up beign very slow we can try with some memmove
        QImage aux(data, row_length / 4, wantedHeight, QImage::Format_RGB32);
        img = QImage(aux.copy(0, 0, wantedWidth, wantedHeight));
    }

    switch (orientation) {
        case Okular::Rotation0: {
            img = img.copy();
            break;
        }
        case Okular::Rotation90: {
            QTransform m;
            m.rotate(90);
            img = img.transformed(m);
            break;
        }
        case Okular::Rotation180: {
            QTransform m;
            m.rotate(180);
            img = img.transformed(m);
            break;
        }
        case Okular::Rotation270: {
            QTransform m;
            m.rotate(270);
            img = img.transformed(m);
            break;
        }
    }

    ::free(data);

    if (img.width() != req->width() || img.height() != req->height()) {
        kWarning().nospace() << "Generated image does not match wanted size: "
            << "[" << img.width() << "x" << img.height() << "] vs requested "
            << "[" << req->width() << "x" << req->height() << "]";
        img = img.scaled(wantedWidth, wantedHeight);
    }

    spectre_page_free(page);

    return img;
}

const Okular::DocumentInfo* GSGenerator::generateDocumentInfo()
{
    if (!m_docInfo) {
        m_docInfo = new Okular::DocumentInfo();

        m_docInfo->set(Okular::DocumentInfo::Title, spectre_document_get_title(m_internalDocument));
        m_docInfo->set(Okular::DocumentInfo::Author, spectre_document_get_for(m_internalDocument));
        m_docInfo->set(Okular::DocumentInfo::Creator, spectre_document_get_creator(m_internalDocument));
        m_docInfo->set(Okular::DocumentInfo::CreationDate, spectre_document_get_creation_date(m_internalDocument));
        m_docInfo->set("dscversion", spectre_document_get_format(m_internalDocument), i18n("Document version"));
        const int languageLevel = spectre_document_get_language_level(m_internalDocument);
        if (languageLevel > 0) {
            m_docInfo->set("langlevel", QString::number(languageLevel), i18n("Language Level"));
        }
        if (spectre_document_is_eps(m_internalDocument)) {
            m_docInfo->set(Okular::DocumentInfo::MimeType, "image/x-eps");
        } else {
            m_docInfo->set(Okular::DocumentInfo::MimeType, "application/postscript");
        }
        m_docInfo->set(Okular::DocumentInfo::Pages, QString::number(spectre_document_get_n_pages(m_internalDocument)));
    }
    return m_docInfo;
}

Okular::Rotation GSGenerator::orientation(SpectreOrientation pageOrientation) const
{
    switch (pageOrientation) {
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
    if (key == "DocumentTitle") {
        const char *title = spectre_document_get_title(m_internalDocument);
        if (title) {
            return QString::fromAscii(title);
        }
    }
    return QVariant();
}

#include "moc_generator_ghostview.cpp"
