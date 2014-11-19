/***************************************************************************
 *   Copyright (C) 2013 by Azat Khuzhin <a3at.mail@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef TXT_CONVERTER_H
#define TXT_CONVERTER_H

#include <core/textdocumentgenerator.h>
#include <core/document.h>

namespace Txt
{
    class Converter : public Okular::TextDocumentConverter
    {
    public:
        Converter();
        ~Converter();

        virtual QTextDocument *convert( const QString &fileName );
    };
}

#endif
