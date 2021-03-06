/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>
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

#ifndef SYNDICATION_RSS2_ENCLOSURE_H
#define SYNDICATION_RSS2_ENCLOSURE_H

#include <syndication/elementwrapper.h>

#include <QDomElement>
#include <QString>

namespace Syndication {
namespace RSS2 {

/**
 * Describes a media object that is "attached" to the item.
 * The most common use case for enclosures are podcasts:
 * An audio file is attached to the feed and can be
 * automatically downloaded by a podcast client.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT Enclosure : public ElementWrapper
{
    public:


        /**
         * Default constructor, creates a null object, for which isNull() is
         * @c true.
         */
        Enclosure();

        /**
         * Creates an Enclosure object wrapping an @c &lt;enclosure> XML element.
         *
         * @param element The @c &lt;enclosure> element to wrap
         */
        explicit Enclosure(const QDomElement& element);

        /**
         * returns the URL of the enclosure
         */
        QString url() const;

        /**
         * returns the size of the enclosure in bytes
         */
        int length() const;

        /**
         * returns the mime type of the enclosure
         * (e.g. "audio/mpeg")
         */
        QString type() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_ENCLOSURE_H
