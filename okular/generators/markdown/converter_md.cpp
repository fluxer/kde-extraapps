/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <QtGui/qtextobject.h>

#include "converter_md.h"
#include "document_md.h"

MDConverter::MDConverter()
{
}

QTextDocument* MDConverter::convert(const QString &fileName)
{
    MDDocument *textDocument = new MDDocument(fileName);

    textDocument->setPageSize(QSizeF(600, 800));

    QTextFrameFormat frameFormat;
    frameFormat.setMargin(20);

    QTextFrame *rootFrame = textDocument->rootFrame();
    rootFrame->setFrameFormat(frameFormat);

    emit addMetaData(Okular::DocumentInfo::MimeType, "text/markdown");

    return textDocument;
}
