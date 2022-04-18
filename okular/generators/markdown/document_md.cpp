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

    QTextCodec *codec = QTextCodec::codecForUtfText(m_mddata);
    setHtml(codec->toUnicode(m_mddata));
}

QVariant MDDocument::loadResource(int type, const QUrl &url)
{
    kDebug() << "Resource" << type << url;
    const QString urlstring = url.toString();
    if (type == QTextDocument::ImageResource) {
        foreach (const MDResourceData &mdresource, m_kiojobs.values()) {
            if (mdresource.url == url) {
                return QVariant();
            }
        }

        KIO::TransferJob *kiojob = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
        MDResourceData resourcedata;
        resourcedata.type = type;
        resourcedata.url = url;
        m_kiojobs.insert(kiojob, resourcedata);
        connect(kiojob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(slotKIOData(KIO::Job*,QByteArray)));
        connect(kiojob, SIGNAL(result(KJob*)), SLOT(slotKIOResult(KJob*)));
        return QVariant();
    }
    return QTextDocument::loadResource(type, url);
}

void MDDocument::slotMdCallback(const char* data, qlonglong datasize)
{
    m_mddata.append(data, datasize);
}

void MDDocument::slotKIOData(KIO::Job *kiojob, const QByteArray &data)
{
    KIO::TransferJob *transferjob = static_cast<KIO::TransferJob*>(kiojob);
    MDResourceData &mdresource = m_kiojobs[transferjob];
    mdresource.kiodata.append(data);
}

void MDDocument::slotKIOResult(KJob *kiojob)
{
    KIO::TransferJob *transferjob = static_cast<KIO::TransferJob*>(kiojob);
    if (kiojob->error() != 0) {
        kDebug() << "Could not fetch resource";
    } else {
        kDebug() << "Resource fetched";
    }
    const MDResourceData mdresource = m_kiojobs.take(transferjob);
    QImage mdimage;
    if (mdimage.loadFromData(mdresource.kiodata.constData(), mdresource.kiodata.size())) {
        mdimage = mdimage.scaledToHeight(size().height(), Qt::SmoothTransformation);
        mdimage = mdimage.scaledToWidth(size().width(), Qt::SmoothTransformation);
        QVariant mdvariant;
        mdvariant.setValue(mdimage);
        QTextDocument::addResource(mdresource.type, mdresource.url, mdvariant);
    } else {
        kDebug() << "Could not load resource";
    }
}

#include "moc_document_md.cpp"
