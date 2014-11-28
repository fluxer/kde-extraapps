/**
 * Copyright (C) 2004 Michael Pyne <mpyne@kde.org>
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

#ifndef K3BEXPORTER_H
#define K3BEXPORTER_H

#include <kaction.h>

#include "playlistexporter.h"
#include "playlistitem.h"

class PlaylistBox;

class PlaylistAction : public KAction
{
    Q_OBJECT
    
public:
    PlaylistAction(const QString &userText, const QIcon &pix, const char *slot, const KShortcut &cut = KShortcut());

    typedef QMap<const Playlist *, QObject *> PlaylistRecipientMap;

    /**
     * Defines a QObject to call (using the m_slot SLOT) when an action is
     * emitted from a Playlist.
     */
    void addCallMapping(const Playlist *p, QObject *obj);

    protected slots:
    void slotActivated();

    private:
    QByteArray m_slot;
    PlaylistRecipientMap m_playlistRecipient;
};

/**
 * Class that will export the selected items of a playlist to K3b.
 */
class K3bExporter : public PlaylistExporter
{
    Q_OBJECT

public:
    K3bExporter(Playlist *parent = 0);

    /**
     * Returns a KAction that can be used to invoke the export.
     *
     * @return action used to start the export.
     */
    virtual KAction *action();

    Playlist *playlist() const { return m_parent; }
    void setPlaylist(Playlist *playlist) { m_parent = playlist; }

protected:
    void exportPlaylistItems(const PlaylistItemList &items);

private slots:
    void slotExport();

private:
    enum K3bOpenMode { AudioCD, DataCD, Abort };

    // Private method declarations
    void exportViaCmdLine(const PlaylistItemList &items);
    K3bOpenMode openMode();

    // Private member variable declarations
    Playlist *m_parent;

    // Static member used to avoid adding more than one action to KDE's
    // action list.
    static PlaylistAction *m_action;
};

/**
 * Class to export EVERY item in a playlist to K3b.  Used with the PlaylistBox
 * class to implement context-menus there.
 *
 * @see PlaylistBox
 */
class K3bPlaylistExporter : public K3bExporter
{
    Q_OBJECT
public:
    K3bPlaylistExporter(PlaylistBox *parent = 0);

    virtual KAction *action();

private slots:
    void slotExport();

private:
    PlaylistBox *m_playlistBox;
};

#endif /* K3BEXPORTER_H */

// vim: set et sw=4 tw=0 sta:
