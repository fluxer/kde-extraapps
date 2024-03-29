/***************************************************************************
 *   Copyright (C) 2007 by Albert Astals Cid <aacid@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_GENERATOR_GHOSTVIEW_H_
#define _OKULAR_GENERATOR_GHOSTVIEW_H_

#include <core/generator.h>
#include <interfaces/configinterface.h>

#include <libspectre/spectre.h>

class GSGenerator : public Okular::Generator, public Okular::ConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(Okular::ConfigInterface)

    public:
        /** constructor **/
        GSGenerator(QObject *parent, const QVariantList &args);
        ~GSGenerator();

        /** virtual methods to reimplement **/
        // load a document and fill up the pagesVector
        bool loadDocument(const QString &fileName, QVector<Okular::Page*> &pagesVector);

        // Document description and Table of contents
        const Okular::DocumentInfo *generateDocumentInfo();

        QVariant metaData(const QString &key, const QVariant &option) const;

        // export as PDF
        Okular::ExportFormat::List exportFormats() const;
        bool exportTo(const QString &fileName, const Okular::ExportFormat &format);

        bool reparseConfig();
        void addPages(KConfigDialog* dlg);

    protected:
        QImage image(Okular::PixmapRequest *req);
        bool doCloseDocument();

    private:
        Okular::Rotation orientation(SpectreOrientation orientation) const;

        // backendish stuff
        SpectreDocument *m_internalDocument;
        SpectreRenderContext *m_renderContext;
        Okular::DocumentInfo *m_docInfo;

        bool cache_AAtext;
        bool cache_AAgfx;
};

#endif
