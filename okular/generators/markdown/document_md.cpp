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
#ifdef MD_DEBUG
    kDebug() << "Opening file" << fileName;
#endif

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

MDDocument::~MDDocument()
{
}

void MDDocument::slotMdCallback(const char* data, qlonglong datasize)
{
    m_mddata.append(data, datasize);
}