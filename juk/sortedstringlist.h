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

#ifndef SORTEDSTRINGLIST_H
#define SORTEDSTRINGLIST_H

#include <qstringlist.h>

class SortedStringList
{
public:
    SortedStringList();
    ~SortedStringList();

    /**
     * Insert the value.  Returns true if the item was already in the list
     * or false otherwise.
     */
    bool insert(const QString &value);
    bool contains(const QString &value) const;
    bool remove(const QString &value);

    /**
     * Returns a sorted list of the values.
     * Warning, this method is expensive and shouldn't be used except when
     * necessary.
     */
    QStringList values() const;

private:
    class Node;

    Node *find(const QString &value) const;
    /**
     * The insertion implementation.  Returns true if the item was already
     * present in the list.
     */
    bool BSTInsert(const QString &value);
    void traverse(const Node *n, QStringList &list) const;

    Node *treeMinimum(Node *n) const;
    Node *treeSuccessor(Node *n) const;

    Node *m_root;
};

#endif

// vim: set et sw=4 tw=0 sta:
