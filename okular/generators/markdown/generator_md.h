/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#ifndef MD_GENERATOR_H
#define MD_GENERATOR_H


#include <core/textdocumentgenerator.h>

class MDGenerator : public Okular::TextDocumentGenerator
{
    public:
        MDGenerator( QObject *parent, const QVariantList &args );

        // [INHERITED] reparse configuration
        void addPages( KConfigDialog* dlg );
};

#endif // MD_GENERATOR_H
