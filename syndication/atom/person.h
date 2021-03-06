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

#ifndef SYNDICATION_ATOM_PERSON_H
#define SYNDICATION_ATOM_PERSON_H

#include <syndication/elementwrapper.h>

#include <QDomElement>
#include <QString>

namespace Syndication {
namespace Atom {

/**
 * describes a person, with name and optional URI and e-mail address.
 * Used to describe authors and contributors of feeds/entries
 *
 *  @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Person : public ElementWrapper
{
    public:

        /**
         * creates a null person object
         */
       Person();

       /**
        * creates a Person object wrapping an Atom Person Construct (atom:author,
        * atom:contributor tags)
        * @param element a DOM element, should be a Atom Person Construct
        * (although not enforced), otherwise this object will not parse
        * anything useful
        */
       explicit Person(const QDomElement& element);

       /**
        * a human-readable name for the person. (required)
        * The name is a required  attribute of person constructs.
        *
        * @return a human-readable name of the person
        */
        QString name() const;

        /**
         * A URI associated with the person (optional). Usually the homepage.
         *
         * @return the URI of the person, or a null string if not specified
         */
        QString uri() const;

        /**
         * returns an e-mail address associated with the person. (optional)
         *
         * @return an e-mail address, or a null string if not specified
         */
        QString email() const;

        /**
         * description for debugging purposes
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_PERSON_H
