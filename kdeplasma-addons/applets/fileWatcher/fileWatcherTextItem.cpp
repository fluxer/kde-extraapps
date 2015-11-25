/***************************************************************************
 *   Copyright (C) 2008 by Davide Bettio <davide.bettio@kdemail.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "fileWatcherTextItem.h"

#include <QtCore/qrect.h>

FileWatcherTextItem::FileWatcherTextItem(QGraphicsItem * parent)
    : QGraphicsTextItem(parent)
{ }

QRectF FileWatcherTextItem::boundingRect() const
{
    return QRectF(0, 0, m_w, m_h);
}

void FileWatcherTextItem::setSize(int w, int h)
{
    m_w = w;
    m_h = h;

    prepareGeometryChange();
}
