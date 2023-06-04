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

#ifndef TRANSFER_TORRENT_FACTORY_H
#define TRANSFER_TORRENT_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;

class TransferTorrentFactory : public TransferFactory
{
    Q_OBJECT
public:
    TransferTorrentFactory(QObject *parent, const QVariantList &args);
    ~TransferTorrentFactory();

    Transfer* createTransfer(const KUrl &srcUrl, const KUrl &destUrl,
                             TransferGroup* parent, Scheduler* scheduler);
    bool isSupported(const KUrl &url) const;
    QStringList addsProtocols() const;
};

#endif // TRANSFER_TORRENT_FACTORY_H
