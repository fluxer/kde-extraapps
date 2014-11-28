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

#include "playlistsearch.h"
#include "playlist.h"
#include "playlistitem.h"
#include "collectionlist.h"
#include "juk-exception.h"

#include <kdebug.h>

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

PlaylistSearch::PlaylistSearch() :
    m_mode(MatchAny)
{

}

PlaylistSearch::PlaylistSearch(const PlaylistList &playlists,
                               const ComponentList &components,
                               SearchMode mode,
                               bool searchNow) :
    m_playlists(playlists),
    m_components(components),
    m_mode(mode)
{
    if(searchNow)
        search();
}

void PlaylistSearch::search()
{
    m_items.clear();
    m_matchedItems.clear();
    m_unmatchedItems.clear();

    // This really isn't as bad as it looks.  Despite the four nexted loops
    // most of the time this will be searching one playlist for one search
    // component -- possibly for one column.

    // Also there should be some caching of previous searches in here and
    // allowance for appending and removing chars.  If one is added it
    // should only search the current list.  If one is removed it should
    // pop the previous search results off of a stack.

    foreach(Playlist *playlist, m_playlists) {
        if(!isEmpty()) {
            for(Q3ListViewItemIterator it(playlist); it.current(); ++it)
                checkItem(static_cast<PlaylistItem *>(*it));
        }
        else {
            m_items += playlist->items();
            m_matchedItems += playlist->items();
        }
    }
}

bool PlaylistSearch::checkItem(PlaylistItem *item)
{
    m_items.append(item);

    // set our default
    bool match = bool(m_mode);

    ComponentList::Iterator componentIt = m_components.begin();
    for(; componentIt != m_components.end(); ++componentIt) {

        bool componentMatches = (*componentIt).matches(item);

        if(componentMatches && m_mode == MatchAny) {
            match = true;
            break;
        }

        if(!componentMatches && m_mode == MatchAll) {
            match = false;
            break;
        }
    }

    if(match)
        m_matchedItems.append(item);
    else
        m_unmatchedItems.append(item);

    return match;
}

void PlaylistSearch::addComponent(const Component &c)
{
    m_components.append(c);
}

void PlaylistSearch::clearComponents()
{
    m_components.clear();
}

PlaylistSearch::ComponentList PlaylistSearch::components() const
{
    return m_components;
}

bool PlaylistSearch::isNull() const
{
    return m_components.isEmpty();
}

bool PlaylistSearch::isEmpty() const
{
    if(isNull())
        return true;

    ComponentList::ConstIterator it = m_components.begin();
    for(; it != m_components.end(); ++it) {
        if(!(*it).query().isEmpty() || !(*it).pattern().isEmpty())
            return false;
    }

    return true;
}

void PlaylistSearch::clearItem(PlaylistItem *item)
{
    m_items.removeAll(item);
    m_matchedItems.removeAll(item);
    m_unmatchedItems.removeAll(item);
}

////////////////////////////////////////////////////////////////////////////////
// Component public methods
////////////////////////////////////////////////////////////////////////////////

PlaylistSearch::Component::Component() :
    m_mode(Contains),
    m_searchAllVisible(true),
    m_caseSensitive(false)
{

}

PlaylistSearch::Component::Component(const QString &query,
                                     bool caseSensitive,
                                     const ColumnList &columns,
                                     MatchMode mode) :
    m_query(query),
    m_columns(columns),
    m_mode(mode),
    m_searchAllVisible(columns.isEmpty()),
    m_caseSensitive(caseSensitive),
    m_re(false)
{

}

PlaylistSearch::Component::Component(const QRegExp &query, const ColumnList& columns) :
    m_queryRe(query),
    m_columns(columns),
    m_mode(Exact),
    m_searchAllVisible(columns.isEmpty()),
    m_caseSensitive(false),
    m_re(true)
{

}

bool PlaylistSearch::Component::matches(PlaylistItem *item) const
{
    if((m_re && m_queryRe.isEmpty()) || (!m_re && m_query.isEmpty()))
        return false;

    if(m_columns.isEmpty()) {
        Playlist *p = static_cast<Playlist *>(item->listView());
        for(int i = 0; i < p->columns(); i++) {
            if(p->isColumnVisible(i))
                m_columns.append(i);
        }
    }


    for(ColumnList::Iterator it = m_columns.begin(); it != m_columns.end(); ++it) {

        if(m_re) {
            if(item->text(*it).contains(m_queryRe))
                return true;
            else
                break;
        }

        switch(m_mode) {
        case Contains:
            if(item->text(*it).contains(m_query, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive))
                return true;
            break;
        case Exact:
            if(item->text(*it).length() == m_query.length()) {
                if(m_caseSensitive) {
                    if(item->text(*it) == m_query)
                        return true;
                }
                else if(item->text(*it).toLower() == m_query.toLower())
                    return true;
            }
            break;
        case ContainsWord:
        {
            QString s = item->text(*it);
            int i = s.indexOf(m_query, 0, m_caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );

            if(i >= 0) {

                // If we found the pattern and the lengths are the same, then
                // this is a match.

                if(s.length() == m_query.length())
                    return true;

                // First: If the match starts at the beginning of the text or the
                // character before the match is not a word character

                // AND

                // Second: Either the pattern was found at the end of the text,
                // or the text following the match is a non-word character

                // ...then we have a match

                if((i == 0 || !s.at(i - 1).isLetterOrNumber()) &&
                   (i + m_query.length() == s.length() || !s.at(i + m_query.length()).isLetterOrNumber()))
                    return true;
                break;
            }
        }
        }
    }
    return false;
}

bool PlaylistSearch::Component::operator==(const Component &v) const
{
    return m_query == v.m_query &&
        m_queryRe == v.m_queryRe &&
        m_columns == v.m_columns &&
        m_mode == v.m_mode &&
        m_searchAllVisible == v.m_searchAllVisible &&
        m_caseSensitive == v.m_caseSensitive &&
        m_re == v.m_re;
}

////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &s, const PlaylistSearch &search)
{
    s << search.components()
      << qint32(search.searchMode());

    return s;
}

QDataStream &operator>>(QDataStream &s, PlaylistSearch &search)
{
    search.clearPlaylists();
    search.addPlaylist(CollectionList::instance());

    search.clearComponents();
    PlaylistSearch::ComponentList components;
    s >> components;
    PlaylistSearch::ComponentList::ConstIterator it = components.constBegin();
    for(; it != components.constEnd(); ++it)
        search.addComponent(*it);

    qint32 mode;
    s >> mode;
    search.setSearchMode(PlaylistSearch::SearchMode(mode));

    return s;
}

QDataStream &operator<<(QDataStream &s, const PlaylistSearch::Component &c)
{
    s << c.isPatternSearch()
      << (c.isPatternSearch() ? c.pattern().pattern() : c.query())
      << c.isCaseSensitive()
      << c.columns()
      << qint32(c.matchMode());

    return s;
}

QDataStream &operator>>(QDataStream &s, PlaylistSearch::Component &c)
{
    bool patternSearch;
    QString pattern;
    bool caseSensitive;
    ColumnList columns;
    qint32 mode;

    s >> patternSearch
      >> pattern
      >> caseSensitive
      >> columns
      >> mode;

    if(patternSearch)
        c = PlaylistSearch::Component(QRegExp(pattern), columns);
    else
        c = PlaylistSearch::Component(pattern, caseSensitive, columns, PlaylistSearch::Component::MatchMode(mode));

    return s;
}

// vim: set et sw=4 tw=0 sta:
