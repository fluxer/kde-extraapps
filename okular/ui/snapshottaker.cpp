/***************************************************************************
 *   Copyright (C) 2012 by Tobias Koening <tokoe@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "snapshottaker.h"

#include <QtGui/QImage>

#include <KUrl>

SnapshotTaker::SnapshotTaker( const QString &url, QObject *parent )
    : QObject( parent )
    , m_player( new KMediaPlayer )
{
    m_player->load( url );
    m_player->hide();

    connect(m_player, SIGNAL(paused(bool)),
            this, SLOT(stateChanged(bool)));

    m_player->play();
}

SnapshotTaker::~SnapshotTaker()
{
    m_player->stop();
    delete m_player;
}

void SnapshotTaker::stateChanged(bool paused)
{
    if (paused) {
        QPixmap pixmap = QPixmap::grabWidget(m_player);
        if (!pixmap.isNull())
            emit finished( pixmap.toImage() );

        m_player->stop();
        deleteLater();
    }
}
