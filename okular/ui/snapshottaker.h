/***************************************************************************
 *   Copyright (C) 2012 by Tobias Koening <tokoe@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef SNAPSHOTTAKER_H
#define SNAPSHOTTAKER_H

#include <KUrl>
#include <KIO/PreviewJob>

#include <QObject>
#include <QPixmap>

class SnapshotTaker : public QObject
{
    Q_OBJECT

    public:
        SnapshotTaker( const KUrl &url, QObject *parent, const QSize &size );
        ~SnapshotTaker();

    Q_SIGNALS:
        void finished( const QImage &image );

    private Q_SLOTS:
        void slotPreview(const KFileItem& item, const QPixmap &preview);
        void slotFailed(const KFileItem& item);

    private:
        KIO::PreviewJob *m_job;
        QStringList m_plugins;
};

#endif // SNAPSHOTTAKER_H
