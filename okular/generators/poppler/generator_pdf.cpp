/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "generator_pdf.h"

#include <qimage.h>
#include <qdatetime.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdatetime.h>
#include <kdebug.h>
#include <kglobal.h>

#include <core/page.h>
#include <core/utils.h>

#include <poppler/cpp/poppler-toc.h>
#include <poppler/cpp/poppler-rectangle.h>
#include <poppler/cpp/poppler-page-renderer.h>

static QString okularString(const poppler::ustring &popplerstring)
{
    const poppler::byte_array popplerbytes = popplerstring.to_utf8();
    return QString::fromUtf8(popplerbytes.data(), popplerbytes.size());
}

static QString okularTime(const poppler::time_type &popplertime)
{
    const KDateTime kdatetime(QDateTime::fromTime_t(popplertime));
    return KGlobal::locale()->formatDateTime(kdatetime, KLocale::FancyLongDate);
}

static Okular::Rotation okularRotation(const poppler::page::orientation_enum popplerorientation)
{
    switch (popplerorientation) {
        case poppler::page::portrait: {
            return Okular::Rotation0;
        }
        case poppler::page::landscape: {
            return Okular::Rotation90;
        }
        case poppler::page::upside_down: {
            return Okular::Rotation180;
        }
        case poppler::page::seascape: {
            return Okular::Rotation270;
        }
    }
    kWarning() << "Unknown orientation";
    return Okular::Rotation0;
}

static poppler::rotation_enum popplerRotation(const Okular::Rotation okularorientation)
{
    switch (okularorientation) {
        case Okular::Rotation0: {
            return poppler::rotate_0;
        }
        case Okular::Rotation90: {
            return poppler::rotate_90;
        }
        case Okular::Rotation180: {
            return poppler::rotate_180;
        }
        case Okular::Rotation270: {
            return poppler::rotate_270;
        }
    }
    kWarning() << "Unknown orientation";
    return poppler::rotate_0;
}

static KAboutData createAboutData()
{
    KAboutData aboutData(
         "okular_poppler",
         "okular_poppler",
         ki18n("PDF Backend"),
         "1.0.0",
         ki18n("A PDF file renderer"),
         KAboutData::License_GPL,
         ki18n("© 2022 Ivailo Monev")
    );
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");
    return aboutData;
}

OKULAR_EXPORT_PLUGIN(PDFGenerator, createAboutData())

PDFGenerator::PDFGenerator(QObject *parent, const QVariantList &args)
    : Generator(parent, args),
    m_popplerdocument(nullptr),
    m_documentinfo(nullptr),
    m_documentsynopsis(nullptr)
{
}

PDFGenerator::~PDFGenerator()
{
}

Okular::Document::OpenResult PDFGenerator::loadDocumentWithPassword(const QString &fileName,
    QVector<Okular::Page*> &pagesVector, const QString &password)
{
    pagesVector.clear();

    const QByteArray filebytes = fileName.toLocal8Bit();
    m_popplerdocument = poppler::document::load_from_file(std::string(filebytes.constData(), filebytes.size()));
    if (!m_popplerdocument) {
        return Okular::Document::OpenError;
    }
    if (m_popplerdocument->is_locked() && password.isEmpty()) {
        delete m_popplerdocument;
        m_popplerdocument = nullptr;
        return Okular::Document::OpenNeedsPassword;
    }
    if (!password.isEmpty()) {
        // NOTE: owner password is security bypass
        if (!m_popplerdocument->unlock(std::string(), password.toStdString())) {
            delete m_popplerdocument;
            m_popplerdocument = nullptr;
            // or Okular::Document::OpenError?
            return Okular::Document::OpenNeedsPassword;
        }
    }

    for (int i = 0; i < m_popplerdocument->pages(); i++) {
        poppler::page *popplerpage = m_popplerdocument->create_page(i);
        const poppler::rectf popplerbbox = popplerpage->page_rect();
        Okular::Page* okularpage = new Okular::Page(
            i,
            popplerbbox.width(), popplerbbox.height(),
            okularRotation(popplerpage->orientation())
        );
        okularpage->setLabel(okularString(popplerpage->label()));
        okularpage->setBoundingBox(
            Okular::NormalizedRect(
                popplerbbox.left(), popplerbbox.top(),
                popplerbbox.right(), popplerbbox.bottom()
            )
        );
        pagesVector.append(okularpage);
        m_popplerpages.append(popplerpage);
    }

    return Okular::Document::OpenSuccess;
}

const Okular::DocumentInfo* PDFGenerator::generateDocumentInfo()
{
    if (m_documentinfo) {
        return m_documentinfo;
    }

    m_documentinfo = new Okular::DocumentInfo();
    m_documentinfo->set(Okular::DocumentInfo::MimeType, "application/pdf");
    m_documentinfo->set(Okular::DocumentInfo::Pages, QString::number(m_popplerpages.size()));
    m_documentinfo->set(Okular::DocumentInfo::Title, okularString(m_popplerdocument->get_title()));
    m_documentinfo->set(Okular::DocumentInfo::Author, okularString(m_popplerdocument->get_author()));
    m_documentinfo->set(Okular::DocumentInfo::Subject, okularString(m_popplerdocument->get_subject()));
    m_documentinfo->set(Okular::DocumentInfo::Creator, okularString(m_popplerdocument->get_creator()));
    m_documentinfo->set(Okular::DocumentInfo::Producer, okularString(m_popplerdocument->get_producer()));
    m_documentinfo->set(Okular::DocumentInfo::Keywords, okularString(m_popplerdocument->get_keywords()));
    m_documentinfo->set(Okular::DocumentInfo::CreationDate, okularTime(m_popplerdocument->get_creation_date()));
    m_documentinfo->set(Okular::DocumentInfo::ModificationDate, okularTime(m_popplerdocument->get_modification_date()));
    
    return m_documentinfo;
}

const Okular::DocumentSynopsis* PDFGenerator::generateDocumentSynopsis()
{
    if (m_documentsynopsis) {
        return m_documentsynopsis;
    }

    // TODO: implement when Poppler ToC API has something to work with
#if 0
    poppler::toc* popplertoc = m_popplerdocument->create_toc();
    if (popplertoc) {
        poppler::toc_item* popplertocroot = popplertoc->root();
        if (popplertocroot) {
            m_documentsynopsis = new Okular::DocumentSynopsis();
            foreach (const poppler::toc_item* it, popplertocroot->children()) {
                QDomElement tocelement = m_documentsynopsis->createElement(okularString(it->title()));
                m_documentsynopsis->appendChild(tocelement);
            }
        }
        delete popplertoc;
    }
#endif

    return m_documentsynopsis;
}

void PDFGenerator::walletDataForFile(const QString &fileName, QString *walletName, QString *walletKey) const
{
    *walletKey = fileName + "/pdf";
    *walletName = "okular_poppler_generator";
}

QImage PDFGenerator::image(Okular::PixmapRequest *request)
{
    Okular::Page* okularpage = request->page();
    const int pageindex = okularpage->number();
    if (pageindex < 0 || (pageindex + 1) > m_popplerpages.size()) {
        kWarning() << "Page index out of range";
        return QImage();
    }

    const poppler::page* popplerpage = m_popplerpages.at(pageindex);
    // TODO: figure out why it's not working
#if 0
    if (!okularpage->hasTextPage()) {
        Okular::TextPage* okulartextpage = new Okular::TextPage();
        const std::vector<poppler::text_box> popplertextboxes = popplerpage->text_list();
        for (int i = 0; i < popplertextboxes.size(); i++) {
            const poppler::rectf popplertextbbox = popplertextboxes.at(i).bbox();
            Okular::NormalizedRect* okularrect = new Okular::NormalizedRect(
                popplertextbbox.left(), popplertextbbox.top(),
                popplertextbbox.right(), popplertextbbox.bottom()
            );
            okulartextpage->append(
                okularString(popplertextboxes.at(i).text()),
                okularrect
            );
            qDebug() << Q_FUNC_INFO << okularpage->boundingBox() << *okularrect;
        }
        okularpage->setTextPage(okulartextpage);
    }
#endif

    poppler::page_renderer popplerrenderer;
    const bool okularantialias = documentMetaData("GraphicsAntialias", QVariant(true)).toBool();
    popplerrenderer.set_render_hint(poppler::page_renderer::antialiasing, okularantialias);
    const bool okulartextantialias = documentMetaData("TextAntialias", QVariant(true)).toBool();
    popplerrenderer.set_render_hint(poppler::page_renderer::text_antialiasing, okulartextantialias);
    const bool okulartexthinting = documentMetaData("TextHinting", QVariant(true)).toBool();
    popplerrenderer.set_render_hint(poppler::page_renderer::text_hinting, okulartexthinting);
    const QColor okularpapercolor = qvariant_cast<QColor>(documentMetaData("PaperColor", QVariant()));
    if (okularpapercolor.isValid()) {
        popplerrenderer.set_paper_color(okularpapercolor.rgba());
    }
    popplerrenderer.set_image_format(poppler::image::format_argb32);
    poppler::image popplerimage = popplerrenderer.render_page(
        popplerpage,
        120.0, 120.0,
        -1, -1, -1, -1,
        popplerRotation(okularpage->orientation())
    );
    if (!popplerimage.is_valid()) {
        kWarning() << "Page rendering failed";
        return QImage();
    }
    QImage qimage(
        reinterpret_cast<const uchar*>(popplerimage.const_data()),
        popplerimage.width(), popplerimage.height(),
        QImage::Format_ARGB32
    );
    // NOTE: do not update bounding box or Okular will be stuck in infinite loop on rotation
    return qimage.copy().scaled(
        request->width(), request->height(),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation
    );
}

bool PDFGenerator::doCloseDocument()
{
    qDeleteAll(m_popplerpages);
    m_popplerpages.clear();
    delete m_popplerdocument;
    m_popplerdocument = nullptr;
    delete m_documentinfo;
    m_documentinfo = nullptr;
    delete m_documentsynopsis;
    m_documentsynopsis = nullptr;
    return true;
}

#include "moc_generator_pdf.cpp"
