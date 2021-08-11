/*  This file is part of the KDE libraries
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

#include "transferTorrent.h"

#include <klocale.h>
#include <kdebug.h>

#include <boost/make_shared.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/magnet_uri.hpp>

TransferTorrent::TransferTorrent(TransferGroup* parent, TransferFactory* factory,
                         Scheduler* scheduler, const KUrl &source, const KUrl &dest,
                         const QDomElement* e)
    : Transfer(parent, factory, scheduler, source, dest, e),
    m_timerid(0), m_ltsession(nullptr)
{
    setCapabilities(Transfer::Cap_Resuming);

    // FIXME: once downloaded and KGet is restarted transfer is not seeded

    lt::settings_pack ltsettings;
    ltsettings.set_int(lt::settings_pack::alert_mask,
        lt::alert::status_notification
        | lt::alert::error_notification
        | lt::alert::progress_notification
        | lt::alert::stats_notification);
    // TODO: configurable limits
#if 0
    ltsettings.set_int(settings_pack::connections_limit, 60);
    ltsettings.set_int(settings_pack::download_rate_limit, 2048);
    ltsettings.set_int(settings_pack::upload_rate_limit, 1024);
#endif

    m_ltsession = new lt::session(ltsettings);
}

TransferTorrent::~TransferTorrent()
{
    if (m_timerid != 0) {
        killTimer(m_timerid);
    }
    delete m_ltsession;
}

void TransferTorrent::start()
{
    if (status() == Job::Finished) {
        kDebug(5001) << "TransferTorrent::start: transfer is already finished";
        return;
    }

    kDebug(5001) << "TransferTorrent::start";

    const KUrl sourceurl = Transfer::source();
    const QString sourcestring = sourceurl.url();
    const QByteArray destination = Transfer::directory().toLocalFile().toLocal8Bit();

    kDebug(5001) << "TransferTorrent::start: source" << sourceurl;
    kDebug(5001) << "TransferTorrent::start: destination" << destination;

    lt::add_torrent_params ltparams;
    if (sourcestring.startsWith("magnet:")) {
        const QByteArray source = sourcestring.toLocal8Bit();

        lt::error_code lterror = lt::errors::no_error;
        lt::parse_magnet_uri(source.constData(), ltparams, lterror);

        if (lterror != lt::errors::no_error) {
            kError(5001) << "TransferTorrent::start" << lterror.message().c_str();

            // TODO: translate the error message
            setStatus(Job::Aborted, lterror.message().c_str(), SmallIcon("dialog-error"));
            setTransferChange(Transfer::Tc_Status, true);
            return;
        }
    } else if (sourcestring.endsWith(".torrent")) {
        const QByteArray source = sourceurl.toLocalFile().toLocal8Bit();

        ltparams.ti = boost::make_shared<lt::torrent_info>(source.constData());
        if (!ltparams.ti->is_valid()) {
            kError(5001) << "TransferTorrent::start: invalid torrent file";

            setStatus(Job::Aborted, i18n("Invalid torrent file"), SmallIcon("dialog-error"));
            setTransferChange(Transfer::Tc_Status, true);
            return;
        }
        m_totalSize = ltparams.ti->total_size();
        setTransferChange(Transfer::Tc_TotalSize, true);
    } else {
        kError(5001) << "TransferTorrent::start: invalid source" << sourceurl;

        setStatus(Job::Aborted, i18n("Invalid source URL"), SmallIcon("dialog-error"));
        setTransferChange(Transfer::Tc_Status, true);
        return;
    }

    ltparams.save_path = destination.constData();
    m_lthandle = m_ltsession->add_torrent(ltparams);

    setStatus(Job::Running);
    setTransferChange(Transfer::Tc_Status, true);

    m_timerid = startTimer(1000);
}

void TransferTorrent::stop()
{
    if (status() == Job::Stopped) {
        return;
    }

    kDebug(5001) << "TransferTorrent::stop";

    if (m_timerid != 0) {
        killTimer(m_timerid);
        m_timerid = 0;
    }

    m_downloadSpeed = 0;
    m_uploadSpeed = 0;
    setStatus(Job::Stopped);
    setTransferChange(Transfer::Tc_Status | Transfer::Tc_DownloadSpeed | Transfer::Tc_UploadSpeed, true);
}

void TransferTorrent::deinit(Transfer::DeleteOptions options)
{
    // TODO: delete resume data?
}

bool TransferTorrent::isWorking() const
{
    return (m_timerid != 0);
}

void TransferTorrent::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerid) {
        event->ignore();
        return;
    }

    kDebug(5001) << "TransferTorrent::timerEvent";

    std::vector<lt::alert*> ltalerts;
    m_ltsession->pop_alerts(&ltalerts);

    foreach (lt::alert const* ltalert, ltalerts) {
        if (lt::alert_cast<lt::state_update_alert>(ltalert)) {
            const lt::torrent_status ltstatus = m_lthandle.status();

            m_percent = (ltstatus.progress * 100.0);
            m_downloadedSize = ltstatus.total_done;
            m_downloadSpeed = ltstatus.download_rate;
            m_uploadedSize = ltstatus.all_time_upload;
            m_uploadSpeed = ltstatus.upload_rate;

            kDebug(5001) << "TransferTorrent::timerEvent: percent" << m_percent;
            kDebug(5001) << "TransferTorrent::timerEvent: downloaded size" << m_downloadedSize;
            kDebug(5001) << "TransferTorrent::timerEvent: download speed" << m_downloadSpeed;
            kDebug(5001) << "TransferTorrent::timerEvent: upload size" << m_uploadedSize;
            kDebug(5001) << "TransferTorrent::timerEvent: upload speed" << m_uploadSpeed;

            switch (ltstatus.state) {
                case lt::torrent_status::queued_for_checking:
                case lt::torrent_status::checking_files:
                case lt::torrent_status::checking_resume_data: {
                    setStatus(Job::Running, i18n("Checking..."));
                    setTransferChange(Transfer::Tc_Status, true);
                    break;
                }
                case lt::torrent_status::allocating: {
                    setStatus(Job::Running, i18n("Allocating disk space..."));
                    setTransferChange(Transfer::Tc_Status, true);
                    break;
                }
                case lt::torrent_status::finished:
                case lt::torrent_status::seeding: {
                    setStatus(Job::FinishedKeepAlive, i18n("Seeding..."));
                    setTransferChange(Transfer::Tc_Status, true);
                    break;
                }
                case lt::torrent_status::downloading_metadata:
                case lt::torrent_status::downloading: {
                    setStatus(Job::Running);
                    setTransferChange(Transfer::Tc_Status, true);
                    break;
                }
            }

            setTransferChange(
                Transfer::Tc_Percent
                | Transfer::Tc_DownloadedSize | Transfer::Tc_DownloadSpeed
                | Transfer::Tc_UploadedSize | Transfer::Tc_UploadSpeed
                , true
            );
        } else if (lt::alert_cast<lt::torrent_finished_alert>(ltalert)) {
            kDebug(5001) << "TransferTorrent::timerEvent: transfer finished";

            m_lthandle.save_resume_data();

            // TODO: keep alive only if seeding is enabled
            setStatus(Job::FinishedKeepAlive);
            setTransferChange(Transfer::Tc_Status, true);
        } else if (lt::alert_cast<lt::torrent_error_alert>(ltalert)) {
            kError(5001) << "TransferTorrent::timerEvent" << ltalert->message().c_str();

            killTimer(m_timerid);

            // TODO: translate the error message
            setStatus(Job::Aborted, ltalert->message().c_str(), SmallIcon("dialog-error"));
            setTransferChange(Transfer::Tc_Status, true);
        } else {
            kDebug(5001) << "TransferTorrent::timerEvent" << ltalert->message().c_str();
        }
    }

    m_ltsession->post_torrent_updates();

    event->accept();
}

#include "moc_transferTorrent.cpp"
