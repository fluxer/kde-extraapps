/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef MD_CONVERTER_H
#define MD_CONVERTER_H

#include <core/textdocumentgenerator.h>
#include <core/document.h>

class MDConverter : public Okular::TextDocumentConverter
{
public:
    MDConverter();

    virtual QTextDocument *convert(const QString &fileName);
};

#endif // MD_CONVERTER_H
