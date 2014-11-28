/**
 * Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>
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

#ifndef PLAYLISTINTERFACE_H
#define PLAYLISTINTERFACE_H

#include <QList>

class FileHandle;
class PlaylistObserver;

/**
 * An interface implemented by PlaylistInterface to make it possible to watch
 * for changes in the PlaylistInterface.  This is a semi-standard observer
 * pattern from i.e. Design Patterns.
 */

class Watched
{
public:
    void addObserver(PlaylistObserver *observer);
    void removeObserver(PlaylistObserver *observer);

    /**
     * Call this to remove all objects observing this class unconditionally (for example, when
     * you're being destructed).
     */
    void clearObservers();

    /**
     * This is triggered when the currently playing item has been changed.
     */
    virtual void currentChanged();

    /**
     * This is triggered when the data in the playlist -- i.e. the tag content
     * changes.
     */
    virtual void dataChanged();

protected:
    virtual ~Watched();

private:
    QList<PlaylistObserver *> m_observers;
};

/**
 * This is a simple interface that should be used by things that implement a
 * playlist-like API.
 */

class PlaylistInterface : public Watched
{
public:
    virtual QString name() const = 0;
    virtual FileHandle currentFile() const = 0;
    virtual int time() const = 0;
    virtual int count() const = 0;

    virtual void playNext() = 0;
    virtual void playPrevious() = 0;
    virtual void stop() = 0;

    virtual bool playing() const = 0;
};

class PlaylistObserver
{
public:
    virtual ~PlaylistObserver();

    /**
     * This method must be implemented in concrete implementations; it should
     * define what action should be taken in the observer when the currently
     * playing item changes.
     */
    virtual void updateCurrent() = 0;

    /**
     * This method must be implemented in concrete implementations; it should
     * define what action should be taken when the data of the PlaylistItems in
     * the playlist changes.
     */
    virtual void updateData() = 0;

    void clearWatched() { m_playlist = 0; }

protected:
    PlaylistObserver(PlaylistInterface *playlist);
    const PlaylistInterface *playlist() const;

private:
    PlaylistInterface *m_playlist;
};

#endif

// vim: set et sw=4 tw=0 sta:
