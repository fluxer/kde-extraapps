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

#include "pscreator.h"
#include "pscreatorsettings.h"
#include "ui_pscreatorform.h"

#include <QFile>
#include <QImage>
#include <klocale.h>
#include <kdebug.h>
#include <kdemacros.h>

#include <libspectre/spectre.h>

// NOTE: the stride may or may not be width * 4
static int widthForData(const int spectrelength, const int spectrewidth)
{
    if (spectrelength == (spectrewidth * 4)) {
        return spectrewidth;
    }
    return (spectrelength / 4);
}

class PSCreatorFormWidget : public QWidget, public Ui::PSCreatorForm
{
    Q_OBJECT
public:
    PSCreatorFormWidget() { setupUi(this); }
};


extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new PSCreator();
    }
}

PSCreator::PSCreator()
{
}

bool PSCreator::create(const QString &path, int width, int height, QImage &img)
{
    SpectreDocument *spectredocument = spectre_document_new();
    if (!spectredocument) {
        kWarning() << "Could not create PS document";
        return false;
    }
    const QByteArray pathbytes = QFile::encodeName(path);
    spectre_document_load(spectredocument, pathbytes.constData());
    const SpectreStatus spectrestatus = spectre_document_status(spectredocument);
    if (spectrestatus != SPECTRE_STATUS_SUCCESS) {
        kWarning() << "Could not open PS document" << pathbytes;
        spectre_document_free(spectredocument);
        return false;
    }

    SpectrePage *spectrepage = spectre_document_get_page(spectredocument, 0);
    if (!spectrepage) {
        kWarning() << "Could not get PS page";
        spectre_document_free(spectredocument);
        return false;
    }

    SpectreRenderContext* spectrerender = spectre_render_context_new();
    if (!spectrerender) {
        kWarning() << "Could not create PS render context";
        spectre_page_free(spectrepage);
        spectre_document_free(spectredocument);
        return false;
    }

    PSCreatorSettings* pscreatorsettings = PSCreatorSettings::self();
    pscreatorsettings->readConfig();
    const int spectregraphicaa = (pscreatorsettings->graphic_antialiasing() ? 4 : 1);
    const int spectretextaa = (pscreatorsettings->text_antialiasing() ? 2 : 1);
    spectre_render_context_set_antialias_bits(spectrerender, spectregraphicaa, spectretextaa);

    uchar *spectredata = nullptr;
    int spectrelength = 0;
    spectre_page_render(spectrepage, spectrerender, &spectredata, &spectrelength);
    if (!spectredata || !spectrelength) {
        kWarning() << "Could not render PS page";
        spectre_page_free(spectrepage);
        spectre_render_context_free(spectrerender);
        spectre_document_free(spectredocument);
        return false;
    }

    int spectrewidth = 0;
    int spectreheight = 0;
    spectre_page_get_size(spectrepage, &spectrewidth, &spectreheight);
    QImage tmp(
        spectredata,
        widthForData(spectrelength, spectrewidth), spectreheight,
        QImage::Format_RGB32
    );
    // NOTE: manually scaling because spectre_render_context_set_page_size() ignores aspect ratio
    // when scaling
    QSize pagesize(spectrewidth, spectreheight);
    pagesize.scale(width, height, Qt::KeepAspectRatio);
    img = tmp.scaled(pagesize);
    ::free(spectredata);

    spectre_page_free(spectrepage);
    spectre_render_context_free(spectrerender);
    spectre_document_free(spectredocument);
    return true;
}

ThumbCreator::Flags PSCreator::flags() const
{
    return ThumbCreator::Flags(ThumbCreator::DrawFrame | ThumbCreator::BlendIcon);
}

QWidget *PSCreator::createConfigurationWidget()
{
    PSCreatorSettings* pscreatorsettings = PSCreatorSettings::self();
    PSCreatorFormWidget* pscreatorformwidget = new PSCreatorFormWidget();
    pscreatorformwidget->graphicAntialiasingCheckBox->setChecked(pscreatorsettings->graphic_antialiasing());
    pscreatorformwidget->textAntialiasingCheckBox->setChecked(pscreatorsettings->text_antialiasing());
    return pscreatorformwidget;
}

void PSCreator::writeConfiguration(const QWidget *configurationWidget)
{
    const PSCreatorFormWidget *pscreatorformwidget = qobject_cast<const PSCreatorFormWidget*>(configurationWidget);
    Q_ASSERT(pscreatorformwidget);
    PSCreatorSettings* pscreatorsettings = PSCreatorSettings::self();
    pscreatorsettings->setGraphic_antialiasing(pscreatorformwidget->graphicAntialiasingCheckBox->isChecked());
    pscreatorsettings->setText_antialiasing(pscreatorformwidget->textAntialiasingCheckBox->isChecked());
    pscreatorsettings->writeConfig();
}

#include "pscreator.moc"
