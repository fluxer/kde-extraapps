/***************************************************************************
 *   Copyright (C) 2022 by Ivailo Monev <xakepa10@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "snapshottaker.h"

#include <KFileItem>
#include <KDebug>

SnapshotTaker::SnapshotTaker( const KUrl &url, QObject *parent, const QSize &size )
    : QObject( parent )
    , m_job( nullptr )
{
    m_plugins.append(QString::fromLatin1("ffmpegthumbs"));

    // qDebug() << Q_FUNC_INFO << size;
    // the video widget size is not set properly yet, doing it on QEvent::Show will force a new
    // snapshot to be taken after page change so just assuming 4x the size will be big enough
    const QSize biggersize = (size * 4);

    m_job = KIO::filePreview(
        KFileItemList() << KFileItem(url, QString(), 0),
        biggersize,
        &m_plugins
    );
    m_job->setScaleType(KIO::PreviewJob::Scaled);
    connect(
        m_job, SIGNAL(gotPreview(KFileItem,QPixmap)),
        this, SLOT(slotPreview(KFileItem,QPixmap))
    );
    connect(
        m_job, SIGNAL(failed(KFileItem)),
        this, SLOT(slotFailed(KFileItem))
    );
    m_job->start();
}

SnapshotTaker::~SnapshotTaker()
{
}

void SnapshotTaker::slotPreview(const KFileItem& item, const QPixmap &preview)
{
    Q_UNUSED(item);
    if (!preview.isNull()) {
        emit finished( preview.toImage() );
    }
    deleteLater();
}

void SnapshotTaker::slotFailed(const KFileItem& item)
{
    Q_UNUSED(item);
    deleteLater();
}
