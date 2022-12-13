/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "pdfcreator.h"
#include "pdfcreatorsettings.h"
#include "ui_pdfcreatorform.h"

#include <QFile>
#include <QX11Info>
#include <QImage>
#include <klocale.h>
#include <kdebug.h>
#include <kdemacros.h>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

class PDFCreatorFormWidget : public QWidget, public Ui::PDFCreatorForm
{
    Q_OBJECT
public:
    PDFCreatorFormWidget() { setupUi(this); }
};


extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new PDFCreator();
    }
}

PDFCreator::PDFCreator()
{
}

bool PDFCreator::create(const QString &path, int, int, QImage &img)
{
    const QByteArray pathbytes = QFile::encodeName(path);
    poppler::document *popplerdocument = poppler::document::load_from_file(std::string(pathbytes.constData(), pathbytes.size()));
    if (!popplerdocument) {
        kWarning() << "Could not open" << path;
        return false;
    }

    if (popplerdocument->pages() < 1) {
        kWarning() << "Document has no pages" << path;
        delete popplerdocument;
        return false;
    }

    poppler::page *popplerpage = popplerdocument->create_page(0);
    if (!popplerpage) {
        kWarning() << "Could not create document page";
        delete popplerdocument;
        return false;
    }

    PDFCreatorSettings* pdfcreatorsettings = PDFCreatorSettings::self();
    pdfcreatorsettings->readConfig();
    poppler::page_renderer popplerrenderer;
    popplerrenderer.set_render_hint(poppler::page_renderer::antialiasing, pdfcreatorsettings->antialiasing());
    popplerrenderer.set_render_hint(poppler::page_renderer::text_antialiasing, pdfcreatorsettings->text_antialiasing());
    popplerrenderer.set_render_hint(poppler::page_renderer::text_hinting, pdfcreatorsettings->text_hinting());
    popplerrenderer.set_image_format(poppler::image::format_argb32);

    const poppler::image popplerimage = popplerrenderer.render_page(
        popplerpage,
        QX11Info::appDpiX(), QX11Info::appDpiY(),
        -1, -1, -1, -1,
        poppler::rotate_0
    );
    if (!popplerimage.is_valid()) {
        kWarning() << "Could not render page";
        delete popplerpage;
        delete popplerdocument;
        return false;
    }
    img = QImage(
        reinterpret_cast<const uchar*>(popplerimage.const_data()),
        popplerimage.width(), popplerimage.height(),
        QImage::Format_ARGB32
    ).copy();

    delete popplerpage;
    delete popplerdocument;
    return !img.isNull();
}

ThumbCreator::Flags PDFCreator::flags() const
{
    return ThumbCreator::Flags(ThumbCreator::DrawFrame | ThumbCreator::BlendIcon);
}

QWidget *PDFCreator::createConfigurationWidget()
{
    PDFCreatorSettings* pdfcreatorsettings = PDFCreatorSettings::self();
    PDFCreatorFormWidget* pdfcreatorformwidget = new PDFCreatorFormWidget();
    pdfcreatorformwidget->antialiasingCheckBox->setChecked(pdfcreatorsettings->antialiasing());
    pdfcreatorformwidget->textAntialiasingCheckBox->setChecked(pdfcreatorsettings->text_antialiasing());
    pdfcreatorformwidget->textHintingCheckBox->setChecked(pdfcreatorsettings->text_hinting());
    return pdfcreatorformwidget;
}

void PDFCreator::writeConfiguration(const QWidget *configurationWidget)
{
    const PDFCreatorFormWidget *pdfcreatorformwidget = qobject_cast<const PDFCreatorFormWidget*>(configurationWidget);
    Q_ASSERT(pdfcreatorformwidget);
    PDFCreatorSettings* pdfcreatorsettings = PDFCreatorSettings::self();
    pdfcreatorsettings->setAntialiasing(pdfcreatorformwidget->antialiasingCheckBox->isChecked());
    pdfcreatorsettings->setText_antialiasing(pdfcreatorformwidget->textAntialiasingCheckBox->isChecked());
    pdfcreatorsettings->setText_hinting(pdfcreatorformwidget->textHintingCheckBox->isChecked());
    pdfcreatorsettings->writeConfig();
}

#include "pdfcreator.moc"
