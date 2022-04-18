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
#include <kio/job.h>

struct MDResourceData
{
    int type;
    QUrl url;
    QByteArray kiodata;
};

class MDDocument : public QTextDocument
{
        Q_OBJECT
    public:
        MDDocument(const QString &fileName);

        void slotMdCallback(const char* data, qlonglong datasize);

    protected:
        QVariant loadResource(int type, const QUrl &url) final;

    private Q_SLOTS:
        void slotKIOData(KIO::Job *kiojob, const QByteArray &data);
        void slotKIOResult(KJob *kiojob);

    private:
        QByteArray m_mddata;
        QMap<KIO::TransferJob*, MDResourceData> m_kiojobs;
};

#endif // MD_DOCUMENT_H
