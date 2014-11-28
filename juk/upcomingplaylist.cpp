/**
 * Copyright (C) 2002-2004, 2008 Michael Pyne <mpyne@kde.org>
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

#include "upcomingplaylist.h"
#include "juk-exception.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kaction.h>

#include "playlistitem.h"
#include "playlistcollection.h"
#include "tracksequencemanager.h"
#include "collectionlist.h"
#include "actioncollection.h"

using namespace ActionCollection;

UpcomingPlaylist::UpcomingPlaylist(PlaylistCollection *collection) :
    Playlist(collection, true),
    m_active(false),
    m_oldIterator(0)
{
    setName(i18n("Play Queue"));
    setAllowDuplicates(true);
    setSorting(-1);
}

UpcomingPlaylist::~UpcomingPlaylist()
{
    removeIteratorOverride();
}

void UpcomingPlaylist::initialize()
{
    // Prevent duplicate initialization.

    if(m_active)
        return;

    m_active = true;

    m_oldIterator = manager()->takeIterator();
    manager()->installIterator(new UpcomingSequenceIterator(this));

    if(!m_oldIterator->current())
        m_oldIterator->prepareToPlay(CollectionList::instance());
    else
        manager()->iterator()->setCurrent(m_oldIterator->current());
}

void UpcomingPlaylist::appendItems(const PlaylistItemList &itemList)
{
    initialize();

    if(itemList.isEmpty())
        return;

    PlaylistItem *after = static_cast<PlaylistItem *>(lastItem());

    foreach(PlaylistItem *playlistItem, itemList) {
        after = createItem(playlistItem, after);
        m_playlistIndex.insert(after, playlistItem->playlist());
    }

    dataChanged();
    slotWeightDirty();
}

void UpcomingPlaylist::playNext()
{
    initialize();

    PlaylistItem *next = TrackSequenceManager::instance()->nextItem();

    if(next) {
        setPlaying(next);
        Playlist *source = m_playlistIndex[next];
        if(source) {
            PlaylistList l;
            l.append(this);
            source->synchronizePlayingItems(l, false);
        }
    }
    else {
        removeIteratorOverride();

        // Normally we continue to play the currently playing item that way
        // a user can continue to hear their song when deselecting Play Queue.
        // However we're technically still "playing" when the queue empties and
        // we reinstall the old iterator so in this situation manually advance
        // to the next track. (Otherwise we hear the same song twice in a row
        // during the transition)

        setPlaying(manager()->nextItem());
    }
}

void UpcomingPlaylist::clearItem(PlaylistItem *item)
{
    m_playlistIndex.remove(item);
    Playlist::clearItem(item);
}

void UpcomingPlaylist::addFiles(const QStringList &files, PlaylistItem *after)
{
    CollectionList::instance()->addFiles(files, after);

    PlaylistItemList l;
    foreach(const QString &file, files) {
        FileHandle f(file);
        PlaylistItem *i = CollectionList::instance()->lookup(f.absFilePath());
        if(i)
            l.append(i);
    }

    appendItems(l);
}

QMap< PlaylistItem::Pointer, QPointer<Playlist> > &UpcomingPlaylist::playlistIndex()
{
    return m_playlistIndex;
}

void UpcomingPlaylist::removeIteratorOverride()
{
    if(!m_active)
        return;

    m_active = false; // Allow re-initialization.

    if(!m_oldIterator)
        return;

    // Install the old track iterator.

    manager()->installIterator(m_oldIterator);

    // If we have an item that is currently playing, allow it to keep playing.
    // Otherwise, just reset to the default iterator (probably not playing
    // anything.)
    // XXX: Reset to the last playing playlist?

    m_oldIterator->reset();
    if(playingItem())
        m_oldIterator->setCurrent(playingItem()->collectionItem());

    setPlaying(manager()->currentItem(), true);

    Watched::currentChanged();
}

TrackSequenceManager *UpcomingPlaylist::manager() const
{
    return TrackSequenceManager::instance();
}

UpcomingPlaylist::UpcomingSequenceIterator::UpcomingSequenceIterator(UpcomingPlaylist *playlist) :
    TrackSequenceIterator(), m_playlist(playlist)
{
}

UpcomingPlaylist::UpcomingSequenceIterator::UpcomingSequenceIterator(const UpcomingSequenceIterator &other) :
    TrackSequenceIterator(other), m_playlist(other.m_playlist)
{
}

UpcomingPlaylist::UpcomingSequenceIterator::~UpcomingSequenceIterator()
{
}

void UpcomingPlaylist::UpcomingSequenceIterator::advance()
{
    PlaylistItem *item = m_playlist->firstChild();

    if(item) {
        PlaylistItem *next = static_cast<PlaylistItem *>(item->nextSibling());
        m_playlist->clearItem(item);
        setCurrent(next);
    }
}

void UpcomingPlaylist::UpcomingSequenceIterator::backup()
{
}

UpcomingPlaylist::UpcomingSequenceIterator *UpcomingPlaylist::UpcomingSequenceIterator::clone() const
{
    return new UpcomingSequenceIterator(*this);
}

void UpcomingPlaylist::UpcomingSequenceIterator::setCurrent(PlaylistItem *currentItem)
{
    if(!currentItem) {
        TrackSequenceIterator::setCurrent(currentItem);
        return;
    }

    // If the upcoming playlist is playing something, clear it out since
    // apparently the user didn't want to hear it.

    PlaylistItem *playingItem = m_playlist->playingItem();
    if(playingItem && playingItem->playlist() == m_playlist && currentItem != playingItem)
        m_playlist->clearItem(playingItem);

    // If a different playlist owns this item, add it to the upcoming playlist

    Playlist *p = currentItem->playlist();

    if(p != m_playlist) {
        PlaylistItem *i = m_playlist->createItem(currentItem, (PlaylistItem *) 0);
        m_playlist->playlistIndex().insert(i, p);
        m_playlist->dataChanged();
        m_playlist->slotWeightDirty();
    }
    else {
        // if(p == m_playlist) {

        // Bump this item up to the top
        m_playlist->takeItem(currentItem);
        m_playlist->insertItem(currentItem);
    }

    TrackSequenceIterator::setCurrent(m_playlist->firstChild());
}

void UpcomingPlaylist::UpcomingSequenceIterator::reset()
{
    setCurrent(0);
}

void UpcomingPlaylist::UpcomingSequenceIterator::prepareToPlay(Playlist *)
{
    if(!m_playlist->items().isEmpty())
        setCurrent(m_playlist->firstChild());
}

QDataStream &operator<<(QDataStream &s, const UpcomingPlaylist &p)
{
    PlaylistItemList l = const_cast<UpcomingPlaylist *>(&p)->items();

    s << qint32(l.count());

    foreach(const PlaylistItem *playlistItem, l)
        s << playlistItem->file().absFilePath();

    return s;
}

QDataStream &operator>>(QDataStream &s, UpcomingPlaylist &p)
{
    QString fileName;
    PlaylistItem *newItem = 0;
    qint32 count;

    s >> count;

    for(qint32 i = 0; i < count; ++i) {
        s >> fileName;
        if(fileName.isEmpty())
            throw BICStreamException();

        newItem = p.createItem(FileHandle(fileName), newItem, false);
    }

    return s;
}

// vim: set et sw=4 tw=0 sta:
