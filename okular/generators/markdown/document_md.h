/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef MD_DOCUMENT_H
#define MD_DOCUMENT_H

#include <QtGui/QTextDocument>

class MDDocument : public QTextDocument
{
    public:
        MDDocument(const QString &fileName);

        void slotMdCallback(const char* data, qlonglong datasize);

    private:
        QByteArray m_mddata;
};

#endif // MD_DOCUMENT_H