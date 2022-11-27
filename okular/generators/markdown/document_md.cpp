/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <QFile>
#include <QDataStream>
#include <QTextCodec>

#include <kdebug.h>
#include <KIO/NetAccess>

#include "document_md.h"
#include "md4c-html.h"

void okular_md_callback(const MD_CHAR* mddata, MD_SIZE mdsize, void* mdptr)
{
    MDDocument* document = static_cast<MDDocument*>(mdptr);
    document->slotMdCallback(mddata, mdsize);
}

MDDocument::MDDocument(const QString &fileName)
{
    kDebug() << "Opening file" << fileName;

    QFile mdfile(fileName);
    if (!mdfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kDebug() << "Can't open file" << mdfile.fileName();
        return;
    }
    const QByteArray buffer = mdfile.readAll();

    const int mdresult = md_html(buffer.constData(), buffer.size(), okular_md_callback, this, MD_FLAG_TABLES, 0);
    if (mdresult != 0) {
        kWarning() << "Could not convert file to HTML";
        return;
    }

    QTextCodec *codec = QTextCodec::codecForHtml(m_mddata);
    setHtml(codec->toUnicode(m_mddata));
}

QVariant MDDocument::loadResource(int type, const QUrl &url)
{
    if (type == QTextDocument::ImageResource) {
        if (m_resources.contains(url)) {
            kDebug() << "Resource cached" << url;
            return m_resources.value(url);
        }

        kDebug() << "Fetching resource" << url;
        m_resourcedata.clear();
        KIO::TransferJob *kiojob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
        kiojob->setAutoDelete(false);
        connect(kiojob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(slotKIOData(KIO::Job*,QByteArray)));
        connect(kiojob, SIGNAL(result(KJob*)), SLOT(slotKIOResult(KJob*)));
        const bool kioresult = KIO::NetAccess::synchronousRun(kiojob, nullptr);
        if (kioresult) {
            QImage resourceimage;
            if (resourceimage.loadFromData(m_resourcedata.constData(), m_resourcedata.size())) {
                resourceimage = resourceimage.scaled(pageSize().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QVariant resourcevariant;
                resourcevariant.setValue(resourceimage);
                m_resources.insert(url, resourcevariant);
                kDebug() << "Resource loaded" << url;
                QTextDocument::addResource(type, url, resourcevariant);
                return resourcevariant;
            } else {
                kWarning() << "Could not load resource" << url;
            }
        }
        return QTextDocument::loadResource(type, url);
    }
    return QTextDocument::loadResource(type, url);
}

void MDDocument::slotMdCallback(const char* data, qlonglong datasize)
{
    m_mddata.append(data, datasize);
}

void MDDocument::slotKIOData(KIO::Job *kiojob, const QByteArray &data)
{
    m_resourcedata.append(data);
}

void MDDocument::slotKIOResult(KJob *kiojob)
{
    KIO::TransferJob *transferjob = qobject_cast<KIO::TransferJob*>(kiojob);
    if (transferjob->error() != 0) {
        kWarning() << "Could not fetch resource";
    } else {
        kDebug() << "Resource fetched";
    }
    transferjob->deleteLater();
}

#include "moc_document_md.cpp"
