/***************************************************************************
 *   Copyright (C) 2012 by Tobias Koening <tokoe@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "snapshottaker.h"

#include <KFileItem>

SnapshotTaker::SnapshotTaker( const KUrl &url, QObject *parent, const QSize &size )
    : QObject( parent )
    , m_job( nullptr )
{
    m_plugins.append(QString::fromLatin1("ffmpegthumbs"));

    m_job = KIO::filePreview(
        KFileItemList() << KFileItem(url, QString(), 0),
        size,
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
