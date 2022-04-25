/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "generator_pdf.h"

#include <qdatetime.h>
#include <qpainter.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdatetime.h>
#include <kdebug.h>
#include <kglobal.h>

#include <core/page.h>
#include <core/pagetransition.h>
#include <core/fileprinter.h>
#include <core/utils.h>

#include <poppler/cpp/poppler-toc.h>
#include <poppler/cpp/poppler-rectangle.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-embedded-file.h>
#include <poppler/cpp/poppler-page-transition.h>

static QByteArray okularBytes(const poppler::byte_array &popplerbytes)
{
    return QByteArray(popplerbytes.data(), popplerbytes.size());
}

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

static QDateTime okularDateTime(const poppler::time_type &popplertime)
{
    return QDateTime::fromTime_t(popplertime);
}

static Okular::FontInfo okularFontInfo(const poppler::font_info &popplerfont)
{
    Okular::FontInfo okularfont;
    okularfont.setName(QString::fromStdString(popplerfont.name()));
    okularfont.setFile(QString::fromStdString(popplerfont.file()));
    if (popplerfont.is_embedded()) {
        okularfont.setEmbedType(Okular::FontInfo::FullyEmbedded);
    } else if (popplerfont.is_subset()) {
        okularfont.setEmbedType(Okular::FontInfo::EmbeddedSubset);
    } else {
        okularfont.setEmbedType(Okular::FontInfo::NotEmbedded);
    }
    switch (popplerfont.type()) {
        case poppler::font_info::unknown: {
            okularfont.setType(Okular::FontInfo::Unknown);
            break;
        }
        case poppler::font_info::type1: {
            okularfont.setType(Okular::FontInfo::Type1);
            break;
        }
        case poppler::font_info::type1c: {
            okularfont.setType(Okular::FontInfo::Type1C);
            break;
        }
        case poppler::font_info::type1c_ot: {
            okularfont.setType(Okular::FontInfo::Type1COT);
            break;
        }
        case poppler::font_info::type3: {
            okularfont.setType(Okular::FontInfo::Type3);
            break;
        }
        case poppler::font_info::truetype: {
            okularfont.setType(Okular::FontInfo::TrueType);
            break;
        }
        case poppler::font_info::truetype_ot: {
            okularfont.setType(Okular::FontInfo::TrueTypeOT);
            break;
        }
        case poppler::font_info::cid_type0: {
            okularfont.setType(Okular::FontInfo::CIDType0);
            break;
        }
        case poppler::font_info::cid_type0c: {
            okularfont.setType(Okular::FontInfo::CIDType0C);
            break;
        }
        case poppler::font_info::cid_type0c_ot: {
            okularfont.setType(Okular::FontInfo::CIDType0COT);
            break;
        }
        case poppler::font_info::cid_truetype: {
            okularfont.setType(Okular::FontInfo::CIDTrueType);
            break;
        }
        case poppler::font_info::cid_truetype_ot: {
            okularfont.setType(Okular::FontInfo::CIDTrueTypeOT);
            break;
        }
        default: {
            kWarning() << "Unknown font info type" << popplerfont.type();
            okularfont.setType(Okular::FontInfo::Unknown);
            break;
        }
    }
    return okularfont;
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

class PDFEmbeddedFile : public Okular::EmbeddedFile
{
public:
    PDFEmbeddedFile();

    QString name() const final;
    QString description() const final;
    QByteArray data() const final;
    int size() const final;
    QDateTime modificationDate() const final;
    QDateTime creationDate() const final;

private:
    friend PDFGenerator;

    QString m_name;
    QString m_description;
    QByteArray m_data;
    int m_size;
    QDateTime m_modificationdate;
    QDateTime m_creationdate;
};

PDFEmbeddedFile::PDFEmbeddedFile()
    : m_size(0)
{
}

QString PDFEmbeddedFile::name() const
{
    return m_name;
}

QString PDFEmbeddedFile::description() const
{
    return m_description;
}

QByteArray PDFEmbeddedFile::data() const
{
    return m_data;
}

int PDFEmbeddedFile::size() const
{
    return m_size;
}

QDateTime PDFEmbeddedFile::modificationDate() const
{
    return m_modificationdate;
}

QDateTime PDFEmbeddedFile::creationDate() const
{
    return m_creationdate;
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
    setFeature(Generator::TextExtraction);
    setFeature(Generator::FontInfo);
    setFeature(Generator::PrintNative);
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
        okularpage->setDuration(popplerpage->duration());
        okularpage->setBoundingBox(
            Okular::NormalizedRect(
                popplerbbox.left(), popplerbbox.top(),
                popplerbbox.right(), popplerbbox.bottom()
            )
        );
        const poppler::page_transition *popplerpagetransition = popplerpage->transition();
        if (popplerpagetransition) {
            Okular::PageTransition* okulartransition = new Okular::PageTransition();
            okulartransition->setDuration(popplerpagetransition->duration());
            okulartransition->setAngle(popplerpagetransition->angle());
            okulartransition->setScale(popplerpagetransition->scale());
            okulartransition->setIsRectangular(popplerpagetransition->is_rectangular());
            switch (popplerpagetransition->type()) {
                case poppler::page_transition::replace: {
                    okulartransition->setType(Okular::PageTransition::Replace);
                    break;
                }
                case poppler::page_transition::split: {
                    okulartransition->setType(Okular::PageTransition::Split);
                    break;
                }
                case poppler::page_transition::blinds: {
                    okulartransition->setType(Okular::PageTransition::Blinds);
                    break;
                }
                case poppler::page_transition::box: {
                    okulartransition->setType(Okular::PageTransition::Box);
                    break;
                }
                case poppler::page_transition::wipe: {
                    okulartransition->setType(Okular::PageTransition::Wipe);
                    break;
                }
                case poppler::page_transition::dissolve: {
                    okulartransition->setType(Okular::PageTransition::Dissolve);
                    break;
                }
                case poppler::page_transition::glitter: {
                    okulartransition->setType(Okular::PageTransition::Glitter);
                    break;
                }
                case poppler::page_transition::fly: {
                    okulartransition->setType(Okular::PageTransition::Fly);
                    break;
                }
                case poppler::page_transition::push: {
                    okulartransition->setType(Okular::PageTransition::Push);
                    break;
                }
                case poppler::page_transition::cover: {
                    okulartransition->setType(Okular::PageTransition::Cover);
                    break;
                }
                case poppler::page_transition::uncover: {
                    okulartransition->setType(Okular::PageTransition::Uncover);
                    break;
                }
                case poppler::page_transition::fade: {
                    okulartransition->setType(Okular::PageTransition::Fade);
                    break;
                }
                default: {
                    kWarning() << "Unknown page transition type" << popplerpagetransition->type();
                    break;
                }
            }
            switch (popplerpagetransition->alignment()) {
                case poppler::page_transition::horizontal: {
                    okulartransition->setAlignment(Okular::PageTransition::Horizontal);
                    break;
                }
                case poppler::page_transition::vertical: {
                    okulartransition->setAlignment(Okular::PageTransition::Vertical);
                    break;
                }
                default: {
                    kWarning() << "Unknown page transition alignment" << popplerpagetransition->alignment();
                    break;
                }
            }
            switch (popplerpagetransition->direction()) {
                case poppler::page_transition::inward: {
                    okulartransition->setDirection(Okular::PageTransition::Inward);
                    break;
                }
                case poppler::page_transition::outward: {
                    okulartransition->setDirection(Okular::PageTransition::Outward);
                    break;
                }
                default: {
                    kWarning() << "Unknown page transition direction" << popplerpagetransition->direction();
                    break;
                }
            }
            okularpage->setTransition(okulartransition);
        }
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

Okular::FontInfo::List PDFGenerator::fontsForPage(int pageindex)
{
    Okular::FontInfo::List result;
    // font iterators created for page index include fonts for other pages too so do not bother to
    // emit results for requests other than the "all pages fonts" request
    if (pageindex == -1) {
        poppler::font_iterator* popplerfontiter = m_popplerdocument->create_font_iterator();
        while (popplerfontiter->has_next()) {
            const std::vector<poppler::font_info> popplerfontinfolist = popplerfontiter->next();
            // qDebug() << Q_FUNC_INFO << pageindex << popplerfontiter->current_page() << popplerfontinfolist.size();
            for (int i = 0; i < popplerfontinfolist.size(); i++) {
                result.append(okularFontInfo(popplerfontinfolist.at(i)));
            }
        }
        delete popplerfontiter;
    }
    return result;
}

const QList<Okular::EmbeddedFile*>* PDFGenerator::embeddedFiles() const
{
    if (!m_popplerdocument->has_embedded_files()) {
        return nullptr;
    }

    QList<Okular::EmbeddedFile*>* okularembeddedfiles = new QList<Okular::EmbeddedFile*>();
    std::vector<poppler::embedded_file*> popplerembeddedfiles = m_popplerdocument->embedded_files();
    okularembeddedfiles->reserve(popplerembeddedfiles.size());
    for (int i = 0; i < popplerembeddedfiles.size(); i++) {
        const poppler::embedded_file* popplerembeddedfile = popplerembeddedfiles.at(i);
        if (!popplerembeddedfile->is_valid()) {
            kWarning() << "Invalid embedded file";
            continue;
        }

        PDFEmbeddedFile* pdfembeddedfile = new PDFEmbeddedFile();
        pdfembeddedfile->m_name = QString::fromStdString(popplerembeddedfile->name());
        pdfembeddedfile->m_description = okularString(popplerembeddedfile->description());
        pdfembeddedfile->m_data = okularBytes(popplerembeddedfile->data());
        pdfembeddedfile->m_size = popplerembeddedfile->size();
        pdfembeddedfile->m_modificationdate = okularDateTime(popplerembeddedfile->modification_date());
        pdfembeddedfile->m_creationdate = okularDateTime(popplerembeddedfile->creation_date());
        // qDebug() << Q_FUNC_INFO << pdfembeddedfile->m_name;
        okularembeddedfiles->append(pdfembeddedfile);
    }
    return okularembeddedfiles;
}

bool PDFGenerator::print(QPrinter &printer)
{
    Okular::Rotation okularorientation = Okular::Rotation0;
    switch (printer.orientation()) {
        case QPrinter::Portrait: {
            okularorientation = Okular::Rotation0;
            break;
        }
        case QPrinter::Landscape: {
            okularorientation = Okular::Rotation90;
            break;
        }
        default: {
            kWarning() << "Unknown printer orientation" << printer.orientation();
            break;
        }
    }

    QPainter qpainter(&printer);
    const QList<int> okularpageslist = Okular::FilePrinter::pageList(
        printer, document()->pages(),
        document()->currentPage() + 1,
        document()->bookmarkedPageList()
    );
    for (int i = 0; i < okularpageslist.size(); i++) {
        // qDebug() << Q_FUNC_INFO << i << (okularpageslist.at(i) - 1);
        QImage qimage = pageImage(okularpageslist.at(i) - 1, okularorientation);
        qimage = qimage.scaled(
            printer.width(), printer.height(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation
        );
        if (i != 0) {
            printer.newPage();
        }
        qpainter.drawImage(0, 0, qimage);
    }

    return true;
}

void PDFGenerator::walletDataForFile(const QString &fileName, QString *walletName, QString *walletKey) const
{
    *walletKey = fileName + "/pdf";
    *walletName = "okular_poppler_generator";
}

QImage PDFGenerator::image(Okular::PixmapRequest *request)
{
    Okular::Page* okularpage = request->page();
    QImage qimage = pageImage(okularpage->number(), okularpage->orientation());
    // NOTE: do not update bounding box or Okular will be stuck in infinite loop on rotation
    return qimage.scaled(
        request->width(), request->height(),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation
    );
}

Okular::TextPage* PDFGenerator::textPage(Okular::Page *page)
{
    const int pageindex = page->number();
    if (pageindex < 0 || (pageindex + 1) > m_popplerpages.size()) {
        kWarning() << "Page index out of range" << pageindex;
        return nullptr;
    }

    const poppler::page* popplerpage = m_popplerpages.at(pageindex);

    Okular::TextPage* okulartextpage = new Okular::TextPage();

    const poppler::rectf popplerbbox = popplerpage->page_rect();
    const std::vector<poppler::text_box> popplertextboxes = popplerpage->text_list();
    for (int i = 0; i < popplertextboxes.size(); i++) {
        const poppler::rectf popplertextbbox = popplertextboxes.at(i).bbox();
        Okular::NormalizedRect* okularrect = new Okular::NormalizedRect(
            popplertextbbox.left() / popplerbbox.width(), popplertextbbox.top() / popplerbbox.height(),
            popplertextbbox.right() / popplerbbox.width(), popplertextbbox.bottom() / popplerbbox.height()
        );
        okulartextpage->append(
            okularString(popplertextboxes.at(i).text()),
            okularrect
        );
        // qDebug() << Q_FUNC_INFO << page->boundingBox() << *okularrect;
    }

    return okulartextpage;
}

QImage PDFGenerator::pageImage(const int pageindex, const Okular::Rotation okularorientation)
{
    if (pageindex < 0 || (pageindex + 1) > m_popplerpages.size()) {
        kWarning() << "Page index out of range" << pageindex;
        return QImage();
    }

    const poppler::page* popplerpage = m_popplerpages.at(pageindex);

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
    const QSizeF okulardpi = dpi();
    // qDebug() << Q_FUNC_INFO << okulardpi;
    const poppler::image popplerimage = popplerrenderer.render_page(
        popplerpage,
        // double the DPI since the image is always scaled
        okulardpi.width() * 2, okulardpi.height() * 2,
        -1, -1, -1, -1,
        popplerRotation(okularorientation)
    );
    if (!popplerimage.is_valid()) {
        kWarning() << "Page rendering failed";
        emit error(i18n("Page rendering failed"), -1);
        return QImage();
    }
    return QImage(
        reinterpret_cast<const uchar*>(popplerimage.const_data()),
        popplerimage.width(), popplerimage.height(),
        QImage::Format_ARGB32
    ).copy();
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
