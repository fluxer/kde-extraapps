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

#include <KMediaPlayer>

#include <QtCore/QObject>

#include <QImage>

class SnapshotTaker : public QObject
{
    Q_OBJECT

    public:
        SnapshotTaker( const QString &url, QObject *parent = 0 );
        ~SnapshotTaker();

    Q_SIGNALS:
        void finished( const QImage &image );

    private Q_SLOTS:
        void stateChanged(bool paused);

    private:
        KMediaPlayer *m_player;
};

#endif
