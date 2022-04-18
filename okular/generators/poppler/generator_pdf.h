/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_GENERATOR_PDF_H_
#define _OKULAR_GENERATOR_PDF_H_

#include <core/document.h>
#include <core/generator.h>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

/**
 * @short A generator that builds contents from a PDF document.
 *
 */
class PDFGenerator : public Okular::Generator
{
    Q_OBJECT

    public:
        PDFGenerator(QObject *parent, const QVariantList &args);
        ~PDFGenerator();

        Okular::Document::OpenResult loadDocumentWithPassword(const QString &fileName,
            QVector<Okular::Page*> &pagesVector, const QString &password) final;

        const Okular::DocumentInfo* generateDocumentInfo() final;
        const Okular::DocumentSynopsis* generateDocumentSynopsis() final;

        void walletDataForFile(const QString &fileName, QString *walletName, QString *walletKey) const final;

    protected:
        QImage image(Okular::PixmapRequest *request) final;
        Okular::TextPage* textPage(Okular::Page *page) final;
        bool doCloseDocument() final;

    private:
        poppler::document *m_popplerdocument;
        QList<poppler::page*> m_popplerpages;
        Okular::DocumentInfo* m_documentinfo;
        Okular::DocumentSynopsis *m_documentsynopsis;
};

#endif // _OKULAR_GENERATOR_PDF_H_
