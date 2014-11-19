/***************************************************************************
 *   Copyright (C) 2006      by Tobias Koenig <tokoe@kde.org>              *
 *   Copyright (C) 2007      by Pino Toscano <pino@kde.org>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _OKULAR_TEXTPAGE_P_H_
#define _OKULAR_TEXTPAGE_P_H_

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtGui/QTransform>

class SearchPoint;
class TinyTextEntity;
class RegionText;

namespace Okular
{

class PagePrivate;
typedef QList< TinyTextEntity* > TextList;

/**
 * Returns whether the two strings match.
 * Satisfies the condition that if two strings match then their lengths are equal.
 */
typedef bool ( *TextComparisonFunction )( const QStringRef & from, const QStringRef & to );

/**
 * A list of RegionText. It keeps a bunch of TextList with their bounding rectangles
 */
typedef QList<RegionText> RegionTextList;

class TextPagePrivate
{
    public:
        TextPagePrivate();
        ~TextPagePrivate();

        RegularAreaRect * findTextInternalForward( int searchID, const QString &query,
                                                   TextComparisonFunction comparer,
                                                   const TextList::ConstIterator &start,
                                                   int start_offset,
                                                   const TextList::ConstIterator &end);
        RegularAreaRect * findTextInternalBackward( int searchID, const QString &query,
                                                    TextComparisonFunction comparer,
                                                    const TextList::ConstIterator &start,
                                                    int start_offset,
                                                    const TextList::ConstIterator &end );

        /**
         * Copy a TextList to m_words, the pointers of list are adopted
         */
        void setWordList(const TextList &list);

        /**
         * Make necessary modifications in the TextList to make the text order correct, so
         * that textselection works fine
         */
        void correctTextOrder();

        // variables those can be accessed directly from TextPage
        TextList m_words;
        QMap< int, SearchPoint* > m_searchPoints;
        PagePrivate *m_page;

    private:
        RegularAreaRect * searchPointToArea(const SearchPoint* sp);
};

}

#endif
