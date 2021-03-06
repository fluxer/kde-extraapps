/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef SYNDICATION_ATOM_PARSER_H
#define SYNDICATION_ATOM_PARSER_H

#include <syndication/abstractparser.h>

#include <QString>
#include <QHash>

namespace Syndication {

class SpecificDocument;
class DocumentSource;

namespace Atom {

/**
 * parser implementation for Atom 1.0 and 0.3.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Parser : public Syndication::AbstractParser
{
    public:

        /** default constructor */
        Parser();

        /** destructor */
        virtual ~Parser();

        /**
         * returns whether the source looks like an Atom 1.0 or 0.3
         * document, by checking the root element.
         * @param source document source to check
         */
        bool accept(const Syndication::DocumentSource& source) const;

        /**
         * parses either an EntryDocument or a FeedDocument from a
         * document source. If the source is not an atom document,
         * an invalid FeedDocument is returned.
         * @see SpecificDocument::isValid()
         * @param source the document source to parse
         */
        Syndication::SpecificDocumentPtr parse(
                const Syndication::DocumentSource& source) const;

        /**
         * returns the format string for this parser implementation, which is
         * @c "atom"
         * @return @c "atom"
         */
        QString format() const;

    private:

        Parser(const Parser& other);
        Parser& operator=(const Parser& other);
        class ParserPrivate;
        ParserPrivate* const d;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_PARSER_H
