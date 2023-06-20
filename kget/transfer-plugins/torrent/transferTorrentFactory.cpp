/*  This file is part of the KDE project

    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "transferTorrentFactory.h"
#include "transferTorrent.h"
#include "core/scheduler.h"
#include "core/transfergroup.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN(TransferTorrentFactory)

TransferTorrentFactory::TransferTorrentFactory(QObject *parent, const QVariantList &args)
    : TransferFactory(parent, args)
{
}

TransferTorrentFactory::~TransferTorrentFactory()
{
}

Transfer* TransferTorrentFactory::createTransfer(const KUrl &srcUrl, const KUrl &destUrl,
                                                 TransferGroup* parent,
                                                 Scheduler* scheduler)
{
    kDebug() << "TransferTorrentFactory::createTransfer";
    if (isSupported(srcUrl)) {
        return new TransferTorrent(parent, this, scheduler, srcUrl, destUrl);
    }
    return nullptr;
}

bool TransferTorrentFactory::isSupported(const KUrl &url) const
{
    const QString urlstring = url.url();
    return (urlstring.startsWith("magnet:") || urlstring.endsWith(".torrent"));
}

QStringList TransferTorrentFactory::addsProtocols() const
{
    // magnet is not an actual protocol but register it anyway
    return QStringList() << "magnet";
}