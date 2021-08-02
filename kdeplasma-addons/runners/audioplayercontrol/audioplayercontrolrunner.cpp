/*
 *   Copyright (C) 2008 Bruno Virlet <bvirlet@kdemail.net>
 *   Copyright (C) 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright (C) 2009 Jan G. Marker <jangerrit@weiler-marker.com>
 *   Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "audioplayercontrolrunner.h"
#include "audioplayercontrolconfigkeys.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/qdbuspendingcall.h>
#include <QtDBus/QDBusMessage>

#include <KMessageBox>
#include <KDebug>
#include <KIcon>
#include <KRun>
#include <KUrl>

// for reference:
// https://specifications.freedesktop.org/mpris-spec/latest/

AudioPlayerControlRunner::AudioPlayerControlRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);

    setObjectName(QLatin1String("Audio Player Control Runner"));
    setSpeed(AbstractRunner::SlowSpeed);

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));

    reloadConfiguration();
}

AudioPlayerControlRunner::~AudioPlayerControlRunner()
{
}

void AudioPlayerControlRunner::prep()
{
    m_running = false;
    m_canPlay = false;
    m_canGoNext = false;
    m_canGoPrevious = false;
    m_playbackStatus = QString();
    m_identity = m_player;

    QDBusInterface player(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
        QLatin1String("/org/mpris/MediaPlayer2"), QLatin1String("org.mpris.MediaPlayer2.Player"));
    m_running = player.isValid();

    if (m_running) {
        QDBusInterface mediaplayer2(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
            QLatin1String("/org/mpris/MediaPlayer2"), QLatin1String("org.mpris.MediaPlayer2"));
        m_identity = mediaplayer2.property("Identity").toString();
        m_playbackStatus = player.property("PlaybackStatus").toString();
        // NOTE: Bogus values set by Audacious for the properties bellow
        m_canPlay = player.property("CanPlay").toBool();
        m_canGoNext = player.property("CanGoNext").toBool();
        m_canGoPrevious = player.property("CanGoPrevious").toBool();
    }

    // qDebug() << Q_FUNC_INFO << m_running << m_canPlay << m_canGoNext << m_canGoPrevious << m_playbackStatus << m_identity;
}

void AudioPlayerControlRunner::match(Plasma::RunnerContext &context)
{
    if (context.query().length() < 3) {
        return;
    }

    const QString term = context.query();

    QList<Plasma::QueryMatch> matches;

    if (context.isValid() ) {
        QVariantList playcontrol;
        playcontrol  << QLatin1String("/org/mpris/MediaPlayer2") << QLatin1String("org.mpris.MediaPlayer2.Player");

        // Play
        if (m_comPlay.startsWith(term, Qt::CaseInsensitive) &&
            (!m_running || (m_canPlay && m_playbackStatus != QLatin1String("Playing")))) {
            QVariantList data = playcontrol;
            data << QLatin1String("Play") << QLatin1String("start");
            matches << createMatch(this, i18n("Start playing"), i18n("Audio player control"),
                QLatin1String("play"), KIcon(QLatin1String("media-playback-start")), data, 1.0);
        }

        if (!m_running) {
            // The interface of the player is not availalbe, so the rest of the commands are not needed
            context.addMatches(term, matches);
            return;
        }

        // Next song
        if (m_comNext.startsWith(term, Qt::CaseInsensitive) && m_canGoNext) {
            QVariantList data = playcontrol;
            data << QLatin1String("Next") << QLatin1String("nostart");
            matches << createMatch(this, i18n("Play next song"), i18n("Audio player control"),
                QLatin1String("next"), KIcon(QLatin1String("media-skip-forward")), data, 1.0);
        }

        // Previous song
        if (m_comPrev.startsWith(term, Qt::CaseInsensitive) && m_canGoPrevious) {
            QVariantList data = playcontrol;
            data << QLatin1String("Previous") << QLatin1String("nostart");
            matches << createMatch(this, i18n("Play previous song"), i18n("Audio player control") ,
                QLatin1String("previous"), KIcon(QLatin1String("media-skip-backward")), data, 1.0);
        }

        // Pause
        if (m_comPause.startsWith(term, Qt::CaseInsensitive) && m_playbackStatus == QLatin1String("Playing")) {
            QVariantList data = playcontrol;
            data << QLatin1String("Pause") << QLatin1String("nostart");
            matches << createMatch(this, i18n("Pause playing"), i18n("Audio player control"),
                QLatin1String("pause"), KIcon(QLatin1String("media-playback-pause")), data, 1.0);
        }

        // Stop
        if (m_comStop.startsWith(term, Qt::CaseInsensitive) && m_playbackStatus == QLatin1String("Playing")) {
            QVariantList data = playcontrol;
            data << QLatin1String("Stop") << QLatin1String("nostart");
            matches << createMatch(this, i18n("Stop playing"), i18n("Audio player control"),
                QLatin1String("stop"), KIcon(QLatin1String("media-playback-stop")), data, 1.0);
        }

        // Set volume to
        QRegExp volumeRegxp(m_comVolume + QLatin1String(" \\d{1,2}0{0,1}"));
        volumeRegxp.setCaseSensitivity(Qt::CaseInsensitive);
        if (volumeRegxp.exactMatch(term)) {
            QVariantList data = playcontrol;
            int newVolume = getNumber(term , ' ');
            data << QLatin1String("Volume") << QLatin1String("nostart") << (newVolume / 100.0);
            matches << createMatch(this, i18n("Set volume to %1%" , newVolume),
                QLatin1String("volume"), i18n("Audio player control"), KIcon(QLatin1String("audio-volume-medium")), data, 1.0);
        }

        // Quit player
        if (m_comQuit.startsWith(term, Qt::CaseInsensitive)) {
            QVariantList data;
            data << QLatin1String("/org/mpris/MediaPlayer2") << QLatin1String("org.mpris.MediaPlayer2")
                << QLatin1String("Quit") << QLatin1String("nostart");
            matches << createMatch(this, i18n("Quit %1", m_identity), QLatin1String(""),
                QLatin1String("quit"), KIcon(QLatin1String("application-exit")), data, 1.0);
        }
    }

    context.addMatches(term, matches);
}

void AudioPlayerControlRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    QVariantList data = match.data().value<QVariantList>();

    if (data[3].toString().compare(QLatin1String("start")) == 0) {
        // The players's interface isn't available but it should be started
        if (!startPlayer()) {
            return;
        }
    }

    // Special case for properties
    if (data[2].toString().compare(QLatin1String("Volume")) == 0) {
        QDBusInterface player(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
            data[0].toString(), data[1].toString());
        player.setProperty(data[2].toByteArray(), data[4]);
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
        data[0].toString(), data[1].toString(), data[2].toString());
    kDebug() << msg;
    QVariantList args;
    for (int i = 4; data.length() > i;++i) {
        args << data[i];
    }
    msg.setArguments(args);
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

void AudioPlayerControlRunner::reloadConfiguration()
{
    KConfigGroup grp = config();
    m_player = grp.readEntry(CONFIG_PLAYER, "vlc");
    m_comPlay = grp.readEntry(CONFIG_PLAY, i18n("play"));
    m_comPause = grp.readEntry(CONFIG_PAUSE, i18n("pause"));
    m_comNext = grp.readEntry(CONFIG_NEXT, i18n("next"));
    m_comPrev = grp.readEntry(CONFIG_PREV, i18n("prev"));
    m_comStop = grp.readEntry(CONFIG_STOP, i18n("stop"));
    m_comVolume = grp.readEntry(CONFIG_VOLUME, i18n("volume"));
    m_comQuit = grp.readEntry(CONFIG_QUIT, i18n("quit"));

    // Adding the syntaxes for helping the user
    QList<Plasma::RunnerSyntax> syntaxes;

    syntaxes << Plasma::RunnerSyntax(m_comPlay, i18n("Plays a song from playlist"));
    syntaxes << Plasma::RunnerSyntax(m_comPause,i18n("Pauses the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comNext, i18n("Plays the next song in the playlist if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comPrev, i18n("Plays the previous song if one is available"));
    syntaxes << Plasma::RunnerSyntax(m_comStop, i18n("Stops the playing"));
    syntaxes << Plasma::RunnerSyntax(m_comVolume + QLatin1String(" :q:"), i18n("Sets the volume to :q:"));
    syntaxes << Plasma::RunnerSyntax(m_comQuit, i18n("Quits the player"));

    setSyntaxes(syntaxes);
}

Plasma::QueryMatch AudioPlayerControlRunner::createMatch(Plasma::AbstractRunner* runner,
        const QString &title, const QString &subtext, const QString &id,
        const KIcon &icon, const QVariantList &data, const float &relevance)
{
    Plasma::QueryMatch match(runner);
    match.setText(title);
    match.setSubtext(subtext);
    match.setId(id);
    match.setIcon(icon);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}

bool AudioPlayerControlRunner::startPlayer()
{
    if (m_running) {
        return true;
    }

    if (!KRun::run(m_player, KUrl::List(), Q_NULLPTR)) {
        // We couldn't start the player
        KMessageBox::error(Q_NULLPTR, i18n("%1 not found", m_player),
            i18n("%1 was not found so the runner is unable to work.", m_player));
        return false;
    }

    QElapsedTimer waitTimeout;
    waitTimeout.start();
    while (!m_running && waitTimeout.elapsed() < 30000) {
        // Waiting for the player's interface to appear
        QDBusInterface player(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
            QLatin1String("/org/mpris/MediaPlayer2"), QLatin1String("org.mpris.MediaPlayer2.Player"));
        m_running = player.isValid();
        QCoreApplication::processEvents();
    }

    if (m_running) {
        prep();
        return true;
    }

    return false;
}

int AudioPlayerControlRunner::getNumber(const QString& term, const char character)
{
    return term.section(QLatin1Char(character), 1, 1).toInt();
}

#include "moc_audioplayercontrolrunner.cpp"
