/*
 *   Copyright (C) 2008 Bruno Virlet <bvirlet@kdemail.net>
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

#ifndef AUDIOPLAYERCONTROLRUNNER_H
#define AUDIOPLAYERCONTROLRUNNER_H

#include <plasma/abstractrunner.h>

#include <KIcon>
#include <KUrl>

#include <QDBusPendingCallWatcher>

class AudioPlayerControlRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    AudioPlayerControlRunner(QObject *parent, const QVariantList& args);
    ~AudioPlayerControlRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);
    QList<QAction*> actionsForMatch(const Plasma::QueryMatch &match);

    void reloadConfiguration();

private:
    /** A own method to create a match easily
      * @param context the terms context
      * @param runner the instance of AudioPlayerControlRunner
      * @param term the search term
      * @param term the main text of the match
      * @param subtext the subtext of the match
      * @param icon the icon of the match
      * @param data the data of the match
      * @param relevance the relevance of the match
      * @return the created match
      */
    Plasma::QueryMatch createMatch(Plasma::AbstractRunner* runner,
                                   const QString &title, const QString &subtext,
                                   const QString &id, const KIcon &icon,
                                   const QVariantList &data, const float &relevance);

    /** Starts the player detached from the current progress if it isn't started already.
      * @return @c true if it was successful, @c false in any other case
      */
    bool startPlayer();

    /** Looks for the number in the command term (it isn't case sensitive)
      * @param text the term
      * @param character the separator
      * @return the number as int
      */
    static int getNumber(const QString& term, const char character);

private slots:
    void prep();

private:
    /** The player this runner controls */
    QString m_player;

    //The commands
    /** Command for play a song */
    QString m_comPlay;
    /** Command for append a song */
    QString m_comAppend;
    /** Command for pause playing */
    QString m_comPause;
    /** Command for stop playing */
    QString m_comStop;
    /** Command for playing the next song */
    QString m_comNext;
    /** Command for playing the previous song */
    QString m_comPrev;
    /** Command for changing the volume */
    QString m_comVolume;
    /** Command for quit the player */
    QString m_comQuit;

    /** Use the commands */
    bool m_useCommands;

    /** The running state of the player */
    bool m_running;

    /** @c true if player can play song */
    bool m_canPlay;

    /** @c true if a next song is available */
    bool m_canGoNext;

    /** @c true if a previous song is available */
    bool m_canGoPrevious;

    /** The playback state of the player */
    QString m_playbackStatus;

    /** The player identity */
    QString m_identity;
};

K_EXPORT_PLASMA_RUNNER(audioplayercontrol, AudioPlayerControlRunner)

#endif
