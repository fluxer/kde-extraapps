/*
 *   Copyright (C) 2008 Bruno Virlet <bvirlet@kdemail.net>
 *   Copyright (C) 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright (C) 2009 Jan G. Marker <jangerrit@weiler-marker.com>
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
#include "imageiconengine.h"

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

#include "audioplayercontrolconfigkeys.h"

Q_DECLARE_METATYPE(QList<QVariantMap>)

/** The variable PLAY contains the action label for play */
static const QString PLAY(QLatin1String("play"));
/** The variable APPEND contains the action label for append */
static const QString APPEND(QLatin1String("append"));
/** The variable NONE says that no action is needed */
static const QString NONE(QLatin1String("none"));

// for reference:
// https://specifications.freedesktop.org/mpris-spec/latest/

typedef QList<QDBusObjectPath> TrackListType;

AudioPlayerControlRunner::AudioPlayerControlRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);

    setObjectName(QLatin1String("Audio Player Control Runner"));
    setSpeed(AbstractRunner::SlowSpeed);

    qDBusRegisterMetaType<QList<QVariantMap> >();

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));

    reloadConfiguration();
}

AudioPlayerControlRunner::~AudioPlayerControlRunner()
{
}

void AudioPlayerControlRunner::prep()
{
    m_running = false;
    m_songsInPlaylist = 0;
    m_nextSongAvailable = false;
    m_prevSongAvailable = false;

    QDBusInterface player(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
        QLatin1String("/org/mpris/MediaPlayer2"), QLatin1String("org.mpris.MediaPlayer2.Player"));
    m_running = player.isValid();

    if (m_running) {
        QDBusInterface trackList(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
            QLatin1String("/org/mpris/MediaPlayer2"), QLatin1String("org.mpris.MediaPlayer2.TrackList"));
        m_songsInPlaylist = qvariant_cast<TrackListType>(trackList.property("Tracks")).size();

        // NOTE: Audacious implements both properties but not org.mpris.MediaPlayer2.TrackList, the
        // properties also are set to true even if there is only one track in the playlist - what
        // to do with that?
        // NOTE: VLC implements org.mpris.MediaPlayer2.TrackList but not the properties - what to
        // do with that?
        if (m_songsInPlaylist > 0) {
            m_nextSongAvailable = player.property("CanGoNext").toBool();
            m_prevSongAvailable = player.property("CanGoPrevious").toBool();
        }
    }
}

void AudioPlayerControlRunner::match(Plasma::RunnerContext &context)
{
    if (context.query().length() < 3) {
        return;
    }

    const QString term = context.query();

    QList<Plasma::QueryMatch> matches;

    if (m_useCommands) {
        QVariantList playcontrol;
        playcontrol  << QLatin1String("/org/mpris/MediaPlayer2") << QLatin1String("org.mpris.MediaPlayer2.Player");

        /* The commands */

        // Play
        // TODO: does not make sense if already playing (not paused)
        if (context.isValid() && m_comPlay.startsWith(term, Qt::CaseInsensitive) &&
            (!m_running || m_songsInPlaylist)) {
            QVariantList data = playcontrol;
            data << QLatin1String("Play") << NONE << QLatin1String("start");
            matches << createMatch(this, i18n("Start playing"), i18n("Audio player control"), QLatin1String("play"),
                                   KIcon(QLatin1String("media-playback-start")), data, 1.0);
        }

        if (!context.isValid() || !m_running) {
            // The interface of the player is not availalbe, so the rest of the commands
            // is not needed
            context.addMatches(term,matches);
            return;
        }

        if (context.isValid() && m_songsInPlaylist) {
            // The playlist isn't empty
            // Next song
            if (m_comNext.startsWith(term, Qt::CaseInsensitive) && m_nextSongAvailable) {
                QVariantList data = playcontrol;
                data << QLatin1String("Next") << NONE << QLatin1String("nostart");
                matches << createMatch(this, i18n("Play next song"), i18n("Audio player control"),
                                       QLatin1String("next"), KIcon(QLatin1String("media-skip-forward")), data, 1.0);
            }

            // Previous song
            if (context.isValid() && m_comPrev.startsWith(term, Qt::CaseInsensitive) && m_prevSongAvailable) {
                QVariantList data = playcontrol;
                data << QLatin1String("Previous") << NONE << QLatin1String("nostart");
                matches << createMatch(this, i18n("Play previous song"), i18n("Audio player control") ,
                                       QLatin1String("previous"), KIcon(QLatin1String("media-skip-backward")), data, 1.0);
            }
        } // --- if (m_songsInPlaylist)

        // Pause
        // TODO: does not make sense if not playing (paused)
        if (context.isValid() && m_comPause.startsWith(term, Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String("Pause") << NONE << QLatin1String("nostart");
            matches << createMatch(this, i18n("Pause playing"), i18n("Audio player control"),
                                   QLatin1String("pause"), KIcon(QLatin1String("media-playback-pause")), data, 1.0);
        }

        // Stop
        if (context.isValid() && m_comStop.startsWith(term, Qt::CaseInsensitive)) {
            QVariantList data = playcontrol;
            data << QLatin1String("Stop") << NONE << QLatin1String("nostart");
            matches << createMatch(this, i18n("Stop playing"), i18n("Audio player control"),
                                   QLatin1String("stop"), KIcon(QLatin1String("media-playback-stop")), data, 1.0);
        }

        // Set volume to
        if (context.isValid() && equals(term, QRegExp(m_comVolume + QLatin1String(" \\d{1,2}0{0,1}") ) ) ) {
            QVariantList data = playcontrol;
            int newVolume = getNumber(term , ' ');
            data << QLatin1String("Volume") << NONE << QLatin1String("nostart") << (newVolume / 100.0);
            matches << createMatch(this, i18n("Set volume to %1%" , newVolume),
                                   QLatin1String("volume"), i18n("Audio player control"), KIcon(QLatin1String("audio-volume-medium")), data, 1.0);
        }

        // Quit player
        if (context.isValid() && m_comQuit.startsWith(term, Qt::CaseInsensitive)) {
            QVariantList data;
            data  << QLatin1String("/org/mpris/MediaPlayer2") << QLatin1String("org.mpris.MediaPlayer2") << QLatin1String("Quit") << NONE
            << QLatin1String("nostart");
            matches << createMatch(this, i18n("Quit %1", m_player), QLatin1String(""),
                                   QLatin1String("quit"), KIcon(QLatin1String("application-exit")), data, 1.0);
        }
    } // --- if (m_useCommands)

    context.addMatches(term, matches);
}

void AudioPlayerControlRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    QVariantList data = match.data().value<QVariantList>();

    if (data[4].toString().compare(QLatin1String("start")) == 0) {
        // The players's interface isn't available but it should be started
        if (!startPlayer()) {
            return;
        }
    }

    // special case for properties
    if (data[2].toString().compare(QLatin1String("Volume")) == 0) {
        QDBusInterface player(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
            data[0].toString(), data[1].toString());
        player.setProperty(data[2].toByteArray(), data[5]);
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QString::fromLatin1("org.mpris.MediaPlayer2.%1").arg(m_player),
        data[0].toString(), data[1].toString(), data[2].toString());
    kDebug() << msg;
    QVariantList args;
    for (int i = 5; data.length() > i;++i) {
        args << data[i];
    }
    msg.setArguments(args);
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

QList<QAction*> AudioPlayerControlRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    QList<QAction*> ret;
    QVariantList data = match.data().value<QVariantList>();

    if (data.length() > 3 && data[3].toString().compare(NONE)) {
        if (!action(PLAY)) {
            addAction(PLAY, KIcon(QLatin1String("media-playback-start")), i18n("Play"));
            addAction(APPEND, KIcon(QLatin1String("media-track-add-amarok")), i18n("Append to playlist"));
        }

        const QStringList actions = data[3].toString().split(QLatin1Char( ',' ));
        for (int i = 0; i < actions.length(); ++i) {
            ret << action(actions[i]);
        }
    }

    return ret;
}

void AudioPlayerControlRunner::reloadConfiguration()
{
    KConfigGroup grp = config();
    m_player = grp.readEntry(CONFIG_PLAYER, "vlc");
    m_useCommands = grp.readEntry(CONFIG_COMMANDS, true);
    m_comPlay = grp.readEntry(CONFIG_PLAY, i18n("play"));
    m_comAppend = grp.readEntry(CONFIG_APPEND, i18n("append"));
    m_comPause = grp.readEntry(CONFIG_PAUSE, i18n("pause"));
    m_comNext = grp.readEntry(CONFIG_NEXT, i18n("next"));
    m_comPrev = grp.readEntry(CONFIG_PREV, i18n("prev"));
    m_comStop = grp.readEntry(CONFIG_STOP, i18n("stop"));
    m_comVolume = grp.readEntry(CONFIG_VOLUME, i18n("volume"));
    m_comQuit = grp.readEntry(CONFIG_QUIT, i18n("quit"));

    /* Adding the syntaxes for helping the user */
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

bool AudioPlayerControlRunner::playerRunning() const
{
    return m_running;
}

bool AudioPlayerControlRunner::startPlayer() const
{
    if (playerRunning()) {
        return true;
    }

    if (!KRun::run(m_player, KUrl::List(), 0)) {
        //We couldn't start the player
        KMessageBox::error(0, i18n("%1 not found", m_player),
            i18n("%1 was not found so the runner is unable to work.", m_player));
        return false;
    }

    /*while (!playerRunning()) {
        //Waiting for the player's interface to appear
        ;
    }*/

    return true;
}

bool AudioPlayerControlRunner::equals(const QString &text, QRegExp reg)
{
    reg.setCaseSensitivity(Qt::CaseInsensitive);
    return reg.exactMatch(text);
}

int AudioPlayerControlRunner::getNumber(const QString& term, const char character)
{
    return term.section(QLatin1Char(character), 1, 1).toInt();
}

#include "moc_audioplayercontrolrunner.cpp"
