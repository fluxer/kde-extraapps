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

#ifndef TRANSFER_TORRENT_H
#define TRANSFER_TORRENT_H

#include "core/transfer.h"
#include "core/filemodel.h"

#include <QTimerEvent>
#include <kdirwatch.h>

#include <vector>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_handle.hpp>

class TorrentFileModel;

class TransferTorrent : public Transfer
{
    Q_OBJECT
    // Transfer reimplementations
public:
    TransferTorrent(TransferGroup* parent, TransferFactory* factory,
                    Scheduler* scheduler, const KUrl &src, const KUrl &dest,
                    const QDomElement* e = 0);
    ~TransferTorrent();

public:
    void init() final;
    void deinit(Transfer::DeleteOptions options) final;
    QList<KUrl> files() const final;
    FileModel* fileModel() final;
    void save(const QDomElement &element) final;
    void load(const QDomElement *element) final;
    QHash<KUrl, QPair<bool, int> > availableMirrors(const KUrl &file) const final;
    void setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors) final;

    // Job reimplementations
    void start() final;
    void stop() final;
    bool isStalled() const final;
    bool isWorking() const final;

protected:
    // QObject reimplementation
    void timerEvent(QTimerEvent *event) final;
    // Transfer reimplementation
    void setSpeedLimits(int uploadLimit, int downloadLimit) final;

private Q_SLOTS:
    void slotCheckStateChanged();
    void slotSettingsDirty(const QString &settings);

private:
    void applySettings();

    bool m_startonload;
    int m_polltimerid;
    int m_resumetimerid;
    lt::session* m_ltsession;
    lt::torrent_handle m_lthandle;
    std::vector<char> m_ltresumedata;
    TorrentFileModel* m_filemodel;
    std::vector<boost::uint8_t> m_priorities;
    KDirWatch* m_dirwatch;
};

#endif // TRANSFER_TORRENT_H
