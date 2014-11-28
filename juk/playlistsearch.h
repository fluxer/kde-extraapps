/**
 * Copyright (C) 2003-2004 Scott Wheeler <wheeler@kde.org>
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

#ifndef PLAYLISTSEARCH_H
#define PLAYLISTSEARCH_H

#include <QRegExp>
#include <QList>

class Playlist;
class PlaylistItem;

typedef QList<int> ColumnList;
typedef QList<PlaylistItem *> PlaylistItemList;
typedef QList<Playlist *> PlaylistList;

class PlaylistSearch
{
public:
    class Component;
    typedef QList<Component> ComponentList;

    enum SearchMode { MatchAny = 0, MatchAll = 1 };

    PlaylistSearch();
    PlaylistSearch(const PlaylistList &playlists,
                   const ComponentList &components,
                   SearchMode mode = MatchAny,
                   bool searchNow = true);

    void search();
    bool checkItem(PlaylistItem *item);

    PlaylistItemList searchedItems() const { return m_items; }
    PlaylistItemList matchedItems() const { return m_matchedItems; }
    PlaylistItemList unmatchedItems() const { return m_unmatchedItems; }

    void addPlaylist(Playlist *p) { m_playlists.append(p); }
    void clearPlaylists() { m_playlists.clear(); }
    PlaylistList playlists() const { return m_playlists; }

    void addComponent(const Component &c);
    void clearComponents();
    ComponentList components() const;

    void setSearchMode(SearchMode m) { m_mode = m; }
    SearchMode searchMode() const { return m_mode; }

    bool isNull() const;
    bool isEmpty() const;

    /**
     * This is used to clear an item from the matched and unmatched lists.  This
     * is useful because it can prevent keeping a dangling pointer around without
     * requiring invalidating the search.
     */
    void clearItem(PlaylistItem *item);

private:
    PlaylistList m_playlists;
    ComponentList m_components;
    SearchMode m_mode;

    PlaylistItemList m_items;
    PlaylistItemList m_matchedItems;
    PlaylistItemList m_unmatchedItems;
};

/**
 * A search is built from several search components.  These corespond to to lines
 * in the search bar.
 */

class PlaylistSearch::Component
{
public:
    enum MatchMode { Contains = 0, Exact = 1, ContainsWord = 2 };

    /**
     * Create an empty search component.  This is only provided for use by
     * QValueList and should not be used in any other context.
     */
    Component();

    /**
     * Create a query component.  This defaults to searching all visible coulumns.
     */
    Component(const QString &query,
              bool caseSensitive = false,
              const ColumnList &columns = ColumnList(),
              MatchMode mode = Contains);

    /**
     * Create a query component.  This defaults to searching all visible coulumns.
     */
    Component(const QRegExp &query, const ColumnList &columns = ColumnList());

    QString query() const { return m_query; }
    QRegExp pattern() const { return m_queryRe; }
    ColumnList columns() const { return m_columns; }

    bool matches(PlaylistItem *item) const;
    bool isPatternSearch() const { return m_re; }
    bool isCaseSensitive() const { return m_caseSensitive; }
    MatchMode matchMode() const { return m_mode; }

    bool operator==(const Component &v) const;

private:
    QString m_query;
    QRegExp m_queryRe;
    mutable ColumnList m_columns;
    MatchMode m_mode;
    bool m_searchAllVisible;
    bool m_caseSensitive;
    bool m_re;
};

/**
 * Streams \a search to the stream \a s.
 * \note This does not save the playlist list, but instead will assume that the
 * search is just relevant to the collection list.  This is all that is presently
 * needed by JuK.
 */
QDataStream &operator<<(QDataStream &s, const PlaylistSearch &search);

/**
 * Streams \a search from the stream \a s.
 * \note This does not save the playlist list, but instead will assume that the
 * search is just relevant to the collection list.  This is all that is presently
 * needed by JuK.
 */
QDataStream &operator>>(QDataStream &s, PlaylistSearch &search);

QDataStream &operator<<(QDataStream &s, const PlaylistSearch::Component &c);
QDataStream &operator>>(QDataStream &s, PlaylistSearch::Component &c);

#endif

// vim: set et sw=4 tw=0 sta:
