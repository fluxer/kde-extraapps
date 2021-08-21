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

#include <QTimer>
#include <klocale.h>
#include <kdebug.h>

#include <boost/make_shared.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/magnet_uri.hpp>

// NOTE: error_code comparison is bogus and breaks translatelterror() too,
// possibly silently fixed via:
// https://github.com/boostorg/system/commit/2fa0a00583a3a791092568d2ade793314181926e
#define BOOST_ERROR_EQUAL_OPERATOR_IS_BORKED

static const int LTPollInterval = 1000;

enum LTPriorities {
    Disabled = 0,
    LowPriority = 1,
    NormalPriority = 4,
    HighPriority = 7,
};

static QString translatelterror(lt::error_code lterror)
{
    if (lterror == lt::errors::no_error) {
        return i18n("Not an error");
    } else if (lterror == lt::errors::file_collision) {
        return i18n("Two torrents has files which end up overwriting each other");
    } else if (lterror == lt::errors::failed_hash_check) {
        return i18n("A piece did not match its piece hash");
    } else if (lterror == lt::errors::torrent_is_no_dict) {
        return i18n("The .torrent file does not contain a bencoded dictionary at its top level");
    } else if (lterror == lt::errors::torrent_missing_info) {
        return i18n("The .torrent file does not have an ``info`` dictionary");
    } else if (lterror == lt::errors::torrent_info_no_dict) {
        return i18n("The .torrent file's ``info`` entry is not a dictionary");
    } else if (lterror == lt::errors::torrent_missing_piece_length) {
        return i18n("The .torrent file does not have a ``piece length`` entry");
    } else if (lterror == lt::errors::torrent_missing_name) {
        return i18n("The .torrent file does not have a ``name`` entry");
    } else if (lterror == lt::errors::torrent_invalid_name) {
        return i18n("The .torrent file's name entry is invalid");
    } else if (lterror == lt::errors::torrent_invalid_length) {
        return i18n("The length of a file, or of the whole .torrent file is invalid");
    } else if (lterror == lt::errors::torrent_file_parse_failed) {
        return i18n("Failed to parse a file entry in the .torrent");
    } else if (lterror == lt::errors::torrent_missing_pieces) {
        return i18n("The ``pieces`` field is missing or invalid in the .torrent file");
    } else if (lterror == lt::errors::torrent_invalid_hashes) {
        return i18n("The ``pieces`` string has incorrect length");
    } else if (lterror == lt::errors::too_many_pieces_in_torrent) {
        return i18n("The .torrent file has more pieces than is supported by libtorrent");
    } else if (lterror == lt::errors::invalid_swarm_metadata) {
        return i18n("The metadata (.torrent file) that was received from the swarm matched the info-hash, but failed to be parsed");
    } else if (lterror == lt::errors::invalid_bencoding) {
        return i18n("The file or buffer is not correctly bencoded");
    } else if (lterror == lt::errors::no_files_in_torrent) {
        return i18n("The .torrent file does not contain any files");
    } else if (lterror == lt::errors::invalid_escaped_string) {
        return i18n("The string was not properly url-encoded as expected");
    } else if (lterror == lt::errors::session_is_closing) {
        return i18n("Operation is not permitted since the session is shutting down");
    } else if (lterror == lt::errors::duplicate_torrent) {
        return i18n("There's already a torrent with that info-hash added to the session");
    } else if (lterror == lt::errors::invalid_torrent_handle) {
        return i18n("The supplied torrent_handle is not referring to a valid torrent");
    } else if (lterror == lt::errors::invalid_entry_type) {
        return i18n("The type requested from the entry did not match its type");
    } else if (lterror == lt::errors::missing_info_hash_in_uri) {
        return i18n("The specified URI does not contain a valid info-hash");
    } else if (lterror == lt::errors::file_too_short) {
        return i18n("One of the files in the torrent was unexpectedly small");
    } else if (lterror == lt::errors::unsupported_url_protocol) {
        return i18n("The URL used an unknown protocol");
    } else if (lterror == lt::errors::url_parse_error) {
        return i18n("The URL did not conform to URL syntax and failed to be parsed");
    } else if (lterror == lt::errors::peer_sent_empty_piece) {
        return i18n("The peer sent a 'piece' message of length 0");
    } else if (lterror == lt::errors::invalid_file_tag) {
        return i18n("A bencoded structure was corrupt and failed to be parsed parse_failed");
    } else if (lterror == lt::errors::missing_info_hash) {
        return i18n("The fast resume file was missing or had an invalid info-hash");
    } else if (lterror == lt::errors::mismatching_info_hash) {
        return i18n("The info-hash did not match the torrent");
    } else if (lterror == lt::errors::invalid_hostname) {
        return i18n("The URL contained an invalid hostname");
    } else if (lterror == lt::errors::invalid_port) {
        return i18n("The URL had an invalid port");
    } else if (lterror == lt::errors::port_blocked) {
        return i18n("The port is blocked by the port-filter");
    } else if (lterror == lt::errors::expected_close_bracket_in_address) {
        return i18n("The IPv6 address was expected to end with ']'");
    } else if (lterror == lt::errors::destructing_torrent) {
        return i18n("The torrent is being destructed");
    } else if (lterror == lt::errors::timed_out) {
        return i18n("The connection timed out");
    } else if (lterror == lt::errors::upload_upload_connection) {
        return i18n("The peer is upload only, and we are upload only");
    } else if (lterror == lt::errors::uninteresting_upload_peer) {
        return i18n("The peer is upload only, and we're not interested in it");
    } else if (lterror == lt::errors::invalid_info_hash) {
        return i18n("The peer sent an unknown info-hash");
    } else if (lterror == lt::errors::torrent_paused) {
        return i18n("The torrent is paused, preventing the operation from succeeding");
    } else if (lterror == lt::errors::invalid_have) {
        return i18n("The peer sent an invalid have message, either wrong size or referring to a piece that doesn't exist in the torrent");
    } else if (lterror == lt::errors::invalid_bitfield_size) {
        return i18n("The bitfield message had the incorrect size");
    } else if (lterror == lt::errors::too_many_requests_when_choked) {
        return i18n("The peer kept requesting pieces after it was choked, possible abuse attempt");
    } else if (lterror == lt::errors::invalid_piece) {
        return i18n("The peer sent a piece message that does not correspond to a piece request sent by the client");
    } else if (lterror == lt::errors::no_memory) {
        return i18n("memory allocation failed");
    } else if (lterror == lt::errors::torrent_aborted) {
        return i18n("The torrent is aborted, preventing the operation to succeed");
    } else if (lterror == lt::errors::self_connection) {
        return i18n("The peer is a connection to ourself, no point in keeping it");
    } else if (lterror == lt::errors::invalid_piece_size) {
        return i18n("The peer sent a piece message with invalid size");
    } else if (lterror == lt::errors::timed_out_no_interest) {
        return i18n("The peer has not been interesting or interested in us for too long");
    } else if (lterror == lt::errors::timed_out_inactivity) {
        return i18n("The peer has not said anything in a long time");
    } else if (lterror == lt::errors::timed_out_no_handshake) {
        return i18n("The peer did not send a handshake within a reasonable amount of time");
    } else if (lterror == lt::errors::timed_out_no_request) {
        return i18n("The peer has been unchoked for too long without requesting any data");
    } else if (lterror == lt::errors::invalid_choke) {
        return i18n("The peer sent an invalid choke message");
    } else if (lterror == lt::errors::invalid_unchoke) {
        return i18n("The peer send an invalid unchoke message");
    } else if (lterror == lt::errors::invalid_interested) {
        return i18n("The peer sent an invalid interested message");
    } else if (lterror == lt::errors::invalid_not_interested) {
        return i18n("The peer sent an invalid not-interested message");
    } else if (lterror == lt::errors::invalid_request) {
        return i18n("The peer sent an invalid piece request message");
    } else if (lterror == lt::errors::invalid_hash_list) {
        return i18n("The peer sent an invalid hash-list message");
    } else if (lterror == lt::errors::invalid_hash_piece) {
        return i18n("The peer sent an invalid hash-piece message");
    } else if (lterror == lt::errors::invalid_cancel) {
        return i18n("The peer sent an invalid cancel message");
    } else if (lterror == lt::errors::invalid_dht_port) {
        return i18n("The peer sent an invalid DHT port-message");
    } else if (lterror == lt::errors::invalid_suggest) {
        return i18n("The peer sent an invalid suggest piece-message");
    } else if (lterror == lt::errors::invalid_have_all) {
        return i18n("The peer sent an invalid have all-message");
    } else if (lterror == lt::errors::invalid_have_none) {
        return i18n("The peer sent an invalid have none-message");
    } else if (lterror == lt::errors::invalid_reject) {
        return i18n("The peer sent an invalid reject message");
    } else if (lterror == lt::errors::invalid_allow_fast) {
        return i18n("The peer sent an invalid allow fast-message");
    } else if (lterror == lt::errors::invalid_extended) {
        return i18n("The peer sent an invalid extension message ID");
    } else if (lterror == lt::errors::invalid_message) {
        return i18n("The peer sent an invalid message ID");
    } else if (lterror == lt::errors::sync_hash_not_found) {
        return i18n("The synchronization hash was not found in the encrypted handshake");
    } else if (lterror == lt::errors::invalid_encryption_constant) {
        return i18n("The encryption constant in the handshake is invalid");
    } else if (lterror == lt::errors::no_plaintext_mode) {
        return i18n("The peer does not support plaintext, which is the selected mode");
    } else if (lterror == lt::errors::no_rc4_mode) {
        return i18n("The peer does not support rc4, which is the selected mode");
    } else if (lterror == lt::errors::unsupported_encryption_mode) {
        return i18n("The peer does not support any of the encryption modes that the client supports");
    } else if (lterror == lt::errors::unsupported_encryption_mode_selected) {
        return i18n("The peer selected an encryption mode that the client did not advertise and does not support");
    } else if (lterror == lt::errors::invalid_pad_size) {
        return i18n("The pad size used in the encryption handshake is of invalid size");
    } else if (lterror == lt::errors::invalid_encrypt_handshake) {
        return i18n("The encryption handshake is invalid");
    } else if (lterror == lt::errors::no_incoming_encrypted) {
        return i18n("The client is set to not support incoming encrypted connections and this is an encrypted connection");
    } else if (lterror == lt::errors::no_incoming_regular) {
        return i18n("The client is set to not support incoming regular bittorrent connections");
    } else if (lterror == lt::errors::duplicate_peer_id) {
        return i18n("The client is already connected to this peer-ID");
    } else if (lterror == lt::errors::torrent_removed) {
        return i18n("Torrent was removed");
    } else if (lterror == lt::errors::packet_too_large) {
        return i18n("The packet size exceeded the upper sanity check-limit");
    } else if (lterror == lt::errors::http_error) {
        return i18n("The web server responded with an error");
    } else if (lterror == lt::errors::missing_location) {
        return i18n("The web server response is missing a location header");
    } else if (lterror == lt::errors::invalid_redirection) {
        return i18n("The web seed redirected to a path that no longer matches the .torrent directory structure");
    } else if (lterror == lt::errors::redirecting) {
        return i18n("The connection was closed because it redirected to a different URL");
    } else if (lterror == lt::errors::invalid_range) {
        return i18n("The HTTP range header is invalid");
    } else if (lterror == lt::errors::no_content_length) {
        return i18n("The HTTP response did not have a content length");
    } else if (lterror == lt::errors::banned_by_ip_filter) {
        return i18n("The IP is blocked by the IP filter");
    } else if (lterror == lt::errors::too_many_connections) {
        return i18n("At the connection limit");
    } else if (lterror == lt::errors::peer_banned) {
        return i18n("The peer is marked as banned");
    } else if (lterror == lt::errors::stopping_torrent) {
        return i18n("The torrent is stopping, causing the operation to fail");
    } else if (lterror == lt::errors::too_many_corrupt_pieces) {
        return i18n("The peer has sent too many corrupt pieces and is banned");
    } else if (lterror == lt::errors::torrent_not_ready) {
        return i18n("The torrent is not ready to receive peers");
    } else if (lterror == lt::errors::peer_not_constructed) {
        return i18n("The peer is not completely constructed yet");
    } else if (lterror == lt::errors::session_closing) {
        return i18n("The session is closing, causing the operation to fail");
    } else if (lterror == lt::errors::optimistic_disconnect) {
        return i18n("The peer was disconnected in order to leave room for a potentially better peer");
    } else if (lterror == lt::errors::torrent_finished) {
        return i18n("The torrent is finished");
    } else if (lterror == lt::errors::no_router) {
        return i18n("No UPnP router found");
    } else if (lterror == lt::errors::metadata_too_large) {
        return i18n("The metadata message says the metadata exceeds the limit");
    } else if (lterror == lt::errors::invalid_metadata_request) {
        return i18n("The peer sent an invalid metadata request message");
    } else if (lterror == lt::errors::invalid_metadata_size) {
        return i18n("The peer advertised an invalid metadata size");
    } else if (lterror == lt::errors::invalid_metadata_offset) {
        return i18n("The peer sent a message with an invalid metadata offset");
    } else if (lterror == lt::errors::invalid_metadata_message) {
        return i18n("The peer sent an invalid metadata message");
    } else if (lterror == lt::errors::pex_message_too_large) {
        return i18n("The peer sent a peer exchange message that was too large");
    } else if (lterror == lt::errors::invalid_pex_message) {
        return i18n("The peer sent an invalid peer exchange message");
    } else if (lterror == lt::errors::invalid_lt_tracker_message) {
        return i18n("The peer sent an invalid tracker exchange message");
    } else if (lterror == lt::errors::too_frequent_pex) {
        return i18n("The peer sent an pex messages too often");
    } else if (lterror == lt::errors::no_metadata) {
        return i18n("The operation failed because it requires the torrent to have the metadata (.torrent file) and it doesn't have it yet");
    } else if (lterror == lt::errors::invalid_dont_have) {
        return i18n("The peer sent an invalid ``dont_have`` message");
    } else if (lterror == lt::errors::requires_ssl_connection) {
        return i18n("The peer tried to connect to an SSL torrent without connecting over SSL");
    } else if (lterror == lt::errors::invalid_ssl_cert) {
        return i18n("The peer tried to connect to a torrent with a certificate for a different torrent");
    } else if (lterror == lt::errors::not_an_ssl_torrent) {
        return i18n("The torrent is not an SSL torrent, and the operation requires an SSL torrent");
    } else if (lterror == lt::errors::banned_by_port_filter) {
        return i18n("Peer was banned because its listen port is within a banned port range");
    } else if (lterror == lt::errors::unsupported_protocol_version) {
        return i18n("The NAT-PMP router responded with an unsupported protocol version");
    } else if (lterror == lt::errors::natpmp_not_authorized) {
        return i18n("You are not authorized to map ports on this NAT-PMP router");
    } else if (lterror == lt::errors::network_failure) {
        return i18n("The NAT-PMP router failed because of a network failure");
    } else if (lterror == lt::errors::no_resources) {
        return i18n("The NAT-PMP router failed because of lack of resources");
    } else if (lterror == lt::errors::unsupported_opcode) {
        return i18n("The NAT-PMP router failed because an unsupported opcode was sent");
    } else if (lterror == lt::errors::missing_file_sizes) {
        return i18n("The resume data file is missing the 'file sizes' entry");
    } else if (lterror == lt::errors::no_files_in_resume_data) {
        return i18n("The resume data file 'file sizes' entry is empty");
    } else if (lterror == lt::errors::missing_pieces) {
        return i18n("The resume data file is missing the 'pieces' and 'slots' entry");
    } else if (lterror == lt::errors::mismatching_number_of_files) {
        return i18n("The number of files in the resume data does not match the number of files in the torrent");
    } else if (lterror == lt::errors::mismatching_file_size) {
        return i18n("One of the files on disk has a different size than in the fast resume file");
    } else if (lterror == lt::errors::mismatching_file_timestamp) {
        return i18n("One of the files on disk has a different timestamp than in the fast resume file");
    } else if (lterror == lt::errors::not_a_dictionary) {
        return i18n("The resume data file is not a dictionary");
    } else if (lterror == lt::errors::invalid_blocks_per_piece) {
        return i18n("The 'blocks per piece' entry is invalid in the resume data file");
    } else if (lterror == lt::errors::missing_slots) {
        return i18n("The resume file is missing the 'slots' entry");
    } else if (lterror == lt::errors::too_many_slots) {
        return i18n("The resume file contains more slots than the torrent");
    } else if (lterror == lt::errors::invalid_slot_list) {
        return i18n("The 'slot' entry is invalid in the resume data");
    } else if (lterror == lt::errors::invalid_piece_index) {
        return i18n("One index in the 'slot' list is invalid");
    } else if (lterror == lt::errors::pieces_need_reorder) {
        return i18n("The pieces on disk needs to be re-ordered for the specified allocation mode");
    } else if (lterror == lt::errors::resume_data_not_modified) {
        return i18n("The resume data is not modified");
    } else if (lterror == lt::errors::http_parse_error) {
        return i18n("The HTTP header was not correctly formatted");
    } else if (lterror == lt::errors::http_missing_location) {
        return i18n("The HTTP response was in the 300-399 range but lacked a location header");
    } else if (lterror == lt::errors::http_failed_decompress) {
        return i18n("The HTTP response was encoded with gzip or deflate but decompressing it failed");
    } else if (lterror == lt::errors::no_i2p_router) {
        return i18n("The URL specified an i2p address, but no i2p router is configured");
    } else if (lterror == lt::errors::no_i2p_endpoint) {
        return i18n("i2p acceptor is not available yet, can't announce without endpoint");
    } else if (lterror == lt::errors::scrape_not_available) {
        return i18n("The tracker URL doesn't support transforming it into a scrape URL");
    } else if (lterror == lt::errors::invalid_tracker_response) {
        return i18n("Invalid tracker response");
    } else if (lterror == lt::errors::invalid_peer_dict) {
        return i18n("Invalid peer dictionary entry");
    } else if (lterror == lt::errors::tracker_failure) {
        return i18n("Tracker sent a failure message");
    } else if (lterror == lt::errors::invalid_files_entry) {
        return i18n("Missing or invalid 'files' entry");
    } else if (lterror == lt::errors::invalid_hash_entry) {
        return i18n("Missing or invalid 'hash' entry");
    } else if (lterror == lt::errors::invalid_peers_entry) {
        return i18n("Missing or invalid 'peers' and 'peers6' entry");
    } else if (lterror == lt::errors::invalid_tracker_response_length) {
        return i18n("udp tracker response packet has invalid size");
    } else if (lterror == lt::errors::invalid_tracker_transaction_id) {
        return i18n("Invalid transaction id in udp tracker response");
    } else if (lterror == lt::errors::invalid_tracker_action) {
        return i18n("Invalid action field in udp tracker response");
    }
    return i18n("Unknown error");
}

class TorrentFileModel : public FileModel
{
public:
    TorrentFileModel(const QList<KUrl> &files, const KUrl &destDirectory, QObject *parent);
    Qt::ItemFlags flags(const QModelIndex &index) const final;

    Job::Status transferstatus;
};

TorrentFileModel::TorrentFileModel(const QList<KUrl> &files, const KUrl &destDirectory, QObject *parent)
    : FileModel(files, destDirectory, parent)
{
}

Qt::ItemFlags TorrentFileModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    if (index.column() == FileItem::File) {
        // TODO: this really should be done for other transfer plugins too, it does not make sense
        // to disable file transfer once it is already finished
        // TODO: this does not (and cannot) account for parent item (directory) flags
        if (transferstatus == Job::Finished || transferstatus == Job::FinishedKeepAlive) {
            return Qt::ItemIsSelectable;
        }
    }

    return FileModel::flags(index);
}

TransferTorrent::TransferTorrent(TransferGroup* parent, TransferFactory* factory,
                         Scheduler* scheduler, const KUrl &source, const KUrl &dest,
                         const QDomElement* e)
    : Transfer(parent, factory, scheduler, source, dest, e),
    m_timerid(0), m_ltsession(nullptr), m_filemodel(nullptr)
{
    setCapabilities(Transfer::Cap_SpeedLimit | Transfer::Cap_Resuming | Transfer::Cap_MultipleMirrors);

    lt::settings_pack ltsettings;
    ltsettings.set_int(lt::settings_pack::alert_mask,
        lt::alert::status_notification
        | lt::alert::error_notification
        | lt::alert::progress_notification
        | lt::alert::stats_notification);

    m_ltsession = new lt::session(ltsettings);
}

TransferTorrent::~TransferTorrent()
{
    if (m_lthandle.is_valid()) {
        m_lthandle.save_resume_data();
    }

    if (m_timerid != 0) {
        killTimer(m_timerid);
        m_timerid = 0;
    }

    delete m_ltsession;
}

void TransferTorrent::setSpeedLimits(int uploadLimit, int downloadLimit)
{
    m_lthandle.set_upload_limit(uploadLimit * 1024);
    m_lthandle.set_download_limit(downloadLimit * 1024);
}

QHash<KUrl, QPair<bool, int> > TransferTorrent::availableMirrors(const KUrl &file) const
{
    Q_UNUSED(file);

    QHash<KUrl, QPair<bool, int> > result;

    const std::vector<lt::announce_entry> lttrackers = m_lthandle.trackers();
    foreach (const lt::announce_entry &lttracker, lttrackers) {
        result.insert(KUrl(lttracker.url.c_str()), QPair<bool,int>(true, 1));
    }

    return result;
}

void TransferTorrent::setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    Q_UNUSED(file);

    std::vector<lt::announce_entry> lttrackers;
    foreach (const KUrl &fileurl, mirrors.keys()) {
        const QPair<bool, int> mirrorpair = mirrors.value(fileurl);
        if (mirrorpair.first == true) {
            const QByteArray filestring = fileurl.prettyUrl().toLocal8Bit();
            lttrackers.push_back(lt::announce_entry(filestring.constData()));
        }
    }

    m_lthandle.replace_trackers(lttrackers);
}

void TransferTorrent::start()
{
    if (status() == Job::Running) {
        return;
    }

    const KUrl sourceurl = source();
    const QString sourcestring = sourceurl.url();
    const QByteArray destination = directory().toLocalFile().toLocal8Bit();

    kDebug(5001) << "source" << sourceurl;
    kDebug(5001) << "destination" << destination;

    lt::add_torrent_params ltparams;
    if (sourcestring.startsWith("magnet:")) {
        const QByteArray source = sourcestring.toLocal8Bit();

        lt::error_code lterror = lt::errors::no_error;
        lt::parse_magnet_uri(source.constData(), ltparams, lterror);

#ifdef BOOST_ERROR_EQUAL_OPERATOR_IS_BORKED
        if (lterror) {
#else
        if (lterror != lt::errors::no_error) {
#endif
            kError(5001) << lterror.message().c_str();

            const QString errormesssage = translatelterror(lterror);
            setError(errormesssage, SmallIcon("dialog-error"), Job::NotSolveable);
            setLog(errormesssage, Transfer::Log_Error);
            setTransferChange(Transfer::Tc_Status | Transfer::Tc_Log, true);
            return;
        }
    } else if (sourcestring.endsWith(".torrent")) {
        const QByteArray source = sourceurl.toLocalFile().toLocal8Bit();

#if LIBTORRENT_VERSION_MAJOR <= 1 && LIBTORRENT_VERSION_MINOR <= 1
        ltparams.ti = boost::make_shared<lt::torrent_info>(source.constData());
#else
        ltparams.ti = std::make_shared<lt::torrent_info>(std::string(source.constData()));
#endif
        if (!ltparams.ti->is_valid()) {
            kError(5001) << "invalid torrent file";

            const QString errormesssage = i18n("Invalid torrent file");
            setError(errormesssage, SmallIcon("dialog-error"), Job::NotSolveable);
            setLog(errormesssage, Transfer::Log_Error);
            setTransferChange(Transfer::Tc_Status | Transfer::Tc_Log, true);
            return;
        }
        m_totalSize = ltparams.ti->total_size();
        setTransferChange(Transfer::Tc_TotalSize, true);
    } else {
        kError(5001) << "invalid source" << sourceurl;

        const QString errormesssage = i18n("Invalid source URL");
        setError(errormesssage, SmallIcon("dialog-error"), Job::NotSolveable);
        setLog(errormesssage, Transfer::Log_Error);
        setTransferChange(Transfer::Tc_Status | Transfer::Tc_Log, true);
        return;
    }

    ltparams.save_path = destination.constData();
#if LIBTORRENT_VERSION_MAJOR <= 1 && LIBTORRENT_VERSION_MINOR <= 1
    ltparams.file_priorities = m_priorities;
#else
    std::vector<lt::download_priority_t> priorities;
    for (const boost::uint8_t priority: m_priorities) {
        priorities.push_back(lt::download_priority_t(priority));
    }
    ltparams.file_priorities = priorities;
#endif
    ltparams.upload_limit = (m_uploadLimit * 1024);
    ltparams.download_limit = (m_downloadLimit * 1024);
    m_lthandle = m_ltsession->add_torrent(ltparams);

    setStatus(Job::Running);
    setTransferChange(Transfer::Tc_Status, true);

    Q_ASSERT(m_timerid == 0);
    m_timerid = startTimer(LTPollInterval);
}

void TransferTorrent::stop()
{
    if (status() == Job::Stopped) {
        return;
    }

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
    Q_ASSERT(m_ltsession);
    if (options & Transfer::DeleteFiles && m_lthandle.is_valid()) {
        m_ltsession->remove_torrent(m_lthandle, lt::session_handle::delete_files);
    }

    if (options & Transfer::DeleteTemporaryFiles && m_lthandle.is_valid()) {
        m_ltsession->remove_torrent(m_lthandle, lt::session_handle::delete_partfile);
    }
}

bool TransferTorrent::isStalled() const
{
    const lt::torrent_status ltstatus = m_lthandle.status();
    return (status() == Job::Running && downloadSpeed() == 0 && ltstatus.state == lt::torrent_status::finished);
}

bool TransferTorrent::isWorking() const
{
    return (m_timerid != 0);
}

QList<KUrl> TransferTorrent::files() const
{
    QList<KUrl> result;

    if (m_lthandle.torrent_file()) {
        const lt::file_storage ltstorage = m_lthandle.torrent_file()->files();
        if (ltstorage.is_valid()) {
            for (int i = 0; i < ltstorage.num_files(); i++) {
                result.append(KUrl(ltstorage.file_path(i).c_str()));
            }
        }
    }

    return result;
}

FileModel* TransferTorrent::fileModel()
{
    if (!m_filemodel) {
        m_filemodel = new TorrentFileModel(files(), directory(), this);
        connect(m_filemodel, SIGNAL(checkStateChanged()), this, SLOT(slotCheckStateChanged()));
    }

    const Job::Status transferstatus = status();
    m_filemodel->transferstatus = transferstatus;
    if (m_lthandle.torrent_file()) {
        const lt::file_storage ltstorage = m_lthandle.torrent_file()->files();
        if (ltstorage.is_valid()) {
            for (int i = 0; i < ltstorage.num_files(); i++) {
                const KUrl fileurl = KUrl(ltstorage.file_path(i).c_str());

                Job::Status filestatus = transferstatus;
                // priority has no effect on finished/seeded torrents
                const int ltpriority = (m_priorities.size() > i ? m_priorities.at(i) : LTPriorities::NormalPriority);
                const lt::torrent_status ltstatus = m_lthandle.status();
                if (ltstatus.state == lt::torrent_status::seeding || ltstatus.state == lt::torrent_status::finished) {
                    filestatus = Job::Finished;
                }
                if (ltpriority == LTPriorities::Disabled) {
                    filestatus = Job::Stopped;
                }

                const Qt::CheckState filestate = (ltpriority == LTPriorities::Disabled ? Qt::Unchecked : Qt::Checked);
                QModelIndex fileindex = m_filemodel->index(fileurl, FileItem::File);
                m_filemodel->setData(fileindex, filestate, Qt::CheckStateRole);

                QModelIndex statusindex = m_filemodel->index(fileurl, FileItem::Status);
                m_filemodel->setData(statusindex, filestatus);

                const qlonglong filesize = ltstorage.file_size(i);
                QModelIndex sizeindex = m_filemodel->index(fileurl, FileItem::Size);
                m_filemodel->setData(sizeindex, filesize);
            }
        }
    }

    return m_filemodel;
}

void TransferTorrent::slotCheckStateChanged()
{
    Q_ASSERT(m_filemodel);

    int counter = 0;
    m_priorities.clear();
    foreach (const KUrl &url, files()) {
        const QModelIndex fileindex = m_filemodel->index(url, FileItem::File);
        const int checkstate = m_filemodel->data(fileindex, Qt::CheckStateRole).toInt();
        if (checkstate != int(Qt::Unchecked)) {
            kDebug(5001) << "will downloand" << url;

            m_priorities.push_back(LTPriorities::NormalPriority);
            m_lthandle.file_priority(counter, LTPriorities::NormalPriority);
        } else {
            kDebug(5001) << "will not downloand" << url;

            m_lthandle.file_priority(counter, LTPriorities::Disabled);
            m_priorities.push_back(LTPriorities::Disabled);
        }
        counter++;
    }
}

void TransferTorrent::save(const QDomElement &element)
{
    QDomElement elementcopy = element;
    QString prioritiesstring;
    for (int i = 0; i < m_priorities.size(); i++) {
        const boost::uint8_t priority = m_priorities.at(i);
        if (i == 0) {
            prioritiesstring.append(QString::number(priority));
        } else {
            prioritiesstring.append(QString::fromLatin1(",") + QString::number(priority));
        }
    }
    elementcopy.setAttribute("FilePriorities", prioritiesstring);

    Transfer::save(elementcopy);
}

void TransferTorrent::load(const QDomElement *element)
{
    Transfer::load(element);

    m_priorities.clear();
    if (element) {
        const QStringList priorities = element->attribute("FilePriorities").split(",");
        foreach (const QString priority, priorities) {
            m_priorities.push_back(boost::uint8_t(priority.toInt()));
        }
    }
}

void TransferTorrent::init()
{
    // start even if transfer is finished so that torrent is seeded
    const bool shouldstart = (policy() != Job::Stop);
    if (shouldstart) {
        // do it after load(), this is racy
        QTimer::singleShot(2000, this, SLOT(slotDelayedStart()));
    }
}

void TransferTorrent::slotDelayedStart()
{
    start();
}

void TransferTorrent::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != m_timerid) {
        event->ignore();
        return;
    }

    Q_ASSERT(m_ltsession);
    std::vector<lt::alert*> ltalerts;
    m_ltsession->pop_alerts(&ltalerts);

    foreach (lt::alert const* ltalert, ltalerts) {
        if (lt::alert_cast<lt::state_update_alert>(ltalert)) {
            const lt::torrent_status ltstatus = m_lthandle.status();

            m_percent = (ltstatus.progress * 100.0);
            m_downloadedSize = ltstatus.total_done;
            m_downloadSpeed = ltstatus.download_rate;
            m_uploadedSize = (ltstatus.total_upload + ltstatus.all_time_upload);
            m_uploadSpeed = ltstatus.upload_rate;

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
            m_lthandle.save_resume_data();

            setStatus(Job::FinishedKeepAlive);
            setTransferChange(Transfer::Tc_Status, true);
        } else if (lt::alert_cast<lt::torrent_error_alert>(ltalert)) {
            kError(5001) << ltalert->message().c_str();

            const lt::torrent_error_alert* lterror = lt::alert_cast<lt::torrent_error_alert>(ltalert);

            killTimer(m_timerid);
            m_timerid = 0;

            const QString errormesssage = translatelterror(lterror->error);
            setError(errormesssage, SmallIcon("dialog-error"), Job::ManualSolve);
            setLog(errormesssage, Transfer::Log_Error);
            setTransferChange(Transfer::Tc_Status | Transfer::Tc_Log, true);
        }
    }

    m_ltsession->post_torrent_updates();

    event->accept();
}

#include "moc_transferTorrent.cpp"
