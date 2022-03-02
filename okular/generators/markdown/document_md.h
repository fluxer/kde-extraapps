/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include <QtGui/QTextDocument>

class MDDocument : public QTextDocument
{
    public:
        MDDocument( const QString &fileName );
        ~MDDocument();

        void slotMdCallback(const char* data, qlonglong datasize);

    private:
        QByteArray m_mddata;
};
