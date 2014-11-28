/***********************************************************************
 * Copyright 2012  Eike Hein <hein@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "mpris2/mediaplayer2player.h"
#include "juk.h"
#include "playermanager.h"
#include "coverinfo.h"
#include "playlist.h"
#include "playlistitem.h"
#include "tag.h"
#include "filehandle.h"

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariant>
#include <QFile>

#include <KUrl>
#include <KStandardDirs>

static QByteArray idFromPlaylistItem(const PlaylistItem *item)
{
    return QByteArray("/org/kde/juk/tid_") +
           QByteArray::number(item->trackId(), 16).rightJustified(8, '0');
}

MediaPlayer2Player::MediaPlayer2Player(QObject* parent)
    : QDBusAbstractAdaptor(parent)
    , m_player(JuK::JuKInstance()->playerManager())
{
    connect(m_player, SIGNAL(signalItemChanged(FileHandle)), this, SLOT(currentSourceChanged()));
    connect(m_player, SIGNAL(signalPlay()), this, SLOT(stateUpdated()));
    connect(m_player, SIGNAL(signalPause()), this, SLOT(stateUpdated()));
    connect(m_player, SIGNAL(signalStop()), this, SLOT(stateUpdated()));
    connect(m_player, SIGNAL(totalTimeChanged(int)), this, SLOT(totalTimeChanged()));
    connect(m_player, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));
    connect(m_player, SIGNAL(volumeChanged(float)), this, SLOT(volumeChanged(float)));
    connect(m_player, SIGNAL(seeked(int)), this, SLOT(seeked(int)));
}

MediaPlayer2Player::~MediaPlayer2Player()
{
}

bool MediaPlayer2Player::CanGoNext() const
{
    return true;
}

void MediaPlayer2Player::Next() const
{
    m_player->forward();
}

bool MediaPlayer2Player::CanGoPrevious() const
{
    return true;
}

void MediaPlayer2Player::Previous() const
{
    m_player->back();
}

bool MediaPlayer2Player::CanPause() const
{
    return true;
}

void MediaPlayer2Player::Pause() const
{
    m_player->pause();
}

void MediaPlayer2Player::PlayPause() const
{
    m_player->playPause();
}

void MediaPlayer2Player::Stop() const
{
    m_player->stop();
}

bool MediaPlayer2Player::CanPlay() const
{
    return true;
}

void MediaPlayer2Player::Play() const
{
    m_player->play();
}

void MediaPlayer2Player::SetPosition(const QDBusObjectPath& TrackId, qlonglong Position) const
{
    PlaylistItem *playingItem = Playlist::playingItem();

    if (!playingItem) {
        return;
    }

    // Verify the SetPosition call is against the currently playing track
    QByteArray currentTrackId = idFromPlaylistItem(playingItem);

    if (TrackId.path().toLatin1() == currentTrackId) {
        m_player->seek(Position / 1000);
    }
}

void MediaPlayer2Player::OpenUri(QString Uri) const
{
    KUrl url(Uri);

    // JuK does not yet support KIO
    if (url.isLocalFile()) {
        m_player->play(url.toLocalFile());
    }
}

QString MediaPlayer2Player::PlaybackStatus() const
{
    if (m_player->playing()) {
        return QLatin1String("Playing");
    }
    else if (m_player->paused()) {
        return QLatin1String("Paused");
    }

    return QLatin1String("Stopped");
}

QString MediaPlayer2Player::LoopStatus() const
{
    // TODO: Implement, although this is orthogonal to the PlayerManager
    return "None";
}

void MediaPlayer2Player::setLoopStatus(const QString& loopStatus) const
{
    Q_UNUSED(loopStatus)
}

double MediaPlayer2Player::Rate() const
{
    return 1.0;
}

void MediaPlayer2Player::setRate(double rate) const
{
    Q_UNUSED(rate)
}

bool MediaPlayer2Player::Shuffle() const
{
    // TODO: Implement
    return false;
}

void MediaPlayer2Player::setShuffle(bool shuffle) const
{
    Q_UNUSED(shuffle)
    // TODO: Implement
}

QVariantMap MediaPlayer2Player::Metadata() const
{
    QVariantMap metaData;

    // The track ID is annoying since it must result in a valid DBus object
    // path, and the regex for that is, and I quote: [a-zA-Z0-9_]*, along with
    // the normal / delimiters for paths.
    PlaylistItem *item = Playlist::playingItem();
    if (!item)
        return metaData;

    FileHandle playingFile = item->file();
    QByteArray playingTrackFileId = idFromPlaylistItem(item);

    metaData["mpris:trackid"] =
        QVariant::fromValue<QDBusObjectPath>(
                QDBusObjectPath(playingTrackFileId.constData()));

    metaData["xesam:album"] = playingFile.tag()->album();
    metaData["xesam:title"] = playingFile.tag()->title();
    metaData["xesam:artist"] = QStringList(playingFile.tag()->artist());
    metaData["xesam:genre"]  = QStringList(playingFile.tag()->genre());

    metaData["mpris:length"] = qint64(playingFile.tag()->seconds() * 1000000);
    metaData["xesam:url"] = QString::fromLatin1(
            KUrl::fromLocalFile(playingFile.absFilePath()).toEncoded());

    if(playingFile.coverInfo()->hasCover()) {
        QString fallbackFileName = KStandardDirs::locateLocal("tmp",
                QString("juk-cover-%1.png").arg(item->trackId()));

        QString path = fallbackFileName;
        if(!QFile::exists(path)) {
            path = playingFile.coverInfo()->localPathToCover(fallbackFileName);
        }

        metaData["mpris:artUrl"] = QString::fromLatin1(QUrl::fromLocalFile(
                path).toEncoded());
    }

    return metaData;
}

double MediaPlayer2Player::Volume() const
{
    return m_player->volume();
}

void MediaPlayer2Player::setVolume(double volume) const
{
    if (volume < 0.0)
        volume = 0.0;
    if (volume > 1.0)
        volume = 1.0;
    m_player->setVolume(volume);
}

qlonglong MediaPlayer2Player::Position() const
{
    return m_player->currentTimeMSecs() * 1000;
}

double MediaPlayer2Player::MinimumRate() const
{
    return 1.0;
}

double MediaPlayer2Player::MaximumRate() const
{
    return 1.0;
}

bool MediaPlayer2Player::CanSeek() const
{
    return m_player->seekable();
}

void MediaPlayer2Player::Seek(qlonglong Offset) const
{
    m_player->seek(((m_player->currentTimeMSecs() * 1000) + Offset) / 1000);
}

bool MediaPlayer2Player::CanControl() const
{
    return true;
}

void MediaPlayer2Player::currentSourceChanged() const
{
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    properties["CanSeek"] = CanSeek();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::stateUpdated() const
{
    QVariantMap properties;
    properties["PlaybackStatus"] = PlaybackStatus();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::totalTimeChanged() const
{
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::seekableChanged(bool seekable) const
{
    QVariantMap properties;
    properties["CanSeek"] = seekable;
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::volumeChanged(float newVol) const
{
    Q_UNUSED(newVol)

    QVariantMap properties;
    properties["Volume"] = Volume();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::seeked(int newPos) const
{
    // casts int to uint64
    emit Seeked(newPos);
}

void MediaPlayer2Player::signalPropertiesChange(const QVariantMap& properties) const
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties", "PropertiesChanged" );

    msg << "org.mpris.MediaPlayer2.Player";
    msg << properties;
    msg << QStringList();

    QDBusConnection::sessionBus().send(msg);
}
