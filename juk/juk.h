/**
 * Copyright (C) 2002-2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JUK_H
#define JUK_H

#include <kxmlguiwindow.h>

#include "playermanager.h"
#include <QKeyEvent>


class KToggleAction;
class KGlobalAccel;

class SliderAction;
class StatusLabel;
class SystemTray;
class PlayerManager;
class PlaylistSplitter;
class Scrobbler;

class JuK : public KXmlGuiWindow
{
    Q_OBJECT

public:
    JuK(QWidget* parent = 0);
    virtual ~JuK();

    static JuK* JuKInstance();

    PlayerManager *playerManager() const;

    // Use a null cover for failure
    void coverDownloaded(const QPixmap &cover);

private:
    void setupLayout();
    void setupActions();
    void setupGlobalAccels();

    void keyPressEvent(QKeyEvent *);

    void activateScrobblerIfEnabled();

    /**
     * readSettings() is separate from readConfig() in that it contains settings
     * that need to be read before the GUI is setup.
     */
    void readSettings();
    void readConfig();
    void saveConfig();

    virtual bool queryClose();

private slots:
    void slotSetupSystemTray();
    void slotShowHide();
    void slotAboutToQuit();
    void slotQuit();
    void slotToggleSystemTray(bool enabled);
    void slotEditKeys();
    void slotConfigureTagGuesser();
    void slotConfigureFileRenamer();
    void slotConfigureScrobbling();
    void slotUndo();
    void slotCheckAlbumNextAction(bool albumRandomEnabled);
    void slotProcessArgs();
    void slotClearOldCovers();

private:
    PlaylistSplitter *m_splitter;
    StatusLabel *m_statusLabel;
    SystemTray *m_systemTray;

    KToggleAction *m_randomPlayAction;
    KToggleAction *m_toggleSystemTrayAction;
    KToggleAction *m_toggleDockOnCloseAction;
    KToggleAction *m_togglePopupsAction;
    KToggleAction *m_toggleSplashAction;

    PlayerManager *m_player;
    Scrobbler     *m_scrobbler;

    bool m_startDocked;
    bool m_showSplash;
    bool m_shuttingDown;

    static JuK* m_instance;
};

#endif

// vim: set et sw=4 tw=0 sta:
