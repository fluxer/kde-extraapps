/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include "generator_md.h"
#include "converter_md.h"

#include <kaboutdata.h>
#include <KConfigDialog>

static KAboutData createAboutData()
{
    KAboutData aboutData(
         "okular_md",
         "okular_md",
         ki18n("Markdown Backend"),
         "0.1",
         ki18n("Markdown backend."),
         KAboutData::License_GPL,
         ki18n("© 2022 Ivailo Monev")
    );
    aboutData.addAuthor(ki18n("Ivailo Monev"), KLocalizedString(), "xakepa10@gmail.com");
    return aboutData;
}

OKULAR_EXPORT_PLUGIN( MDGenerator, createAboutData() )

MDGenerator::MDGenerator(QObject *parent, const QVariantList &args)
    : Okular::TextDocumentGenerator(new MDConverter(), "okular_md_generator_settings", parent, args)
{
}

void MDGenerator::addPages(KConfigDialog* dlg)
{
    Okular::TextDocumentSettingsWidget *widget = new Okular::TextDocumentSettingsWidget();

    dlg->addPage(widget, generalSettings(), i18n("Markdown"), "text-plain", i18n("Markdown Backend Configuration"));
}
