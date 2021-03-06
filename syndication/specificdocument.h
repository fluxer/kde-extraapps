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
#ifndef SYNDICATION_SPECIFICDOCUMENT_H
#define SYNDICATION_SPECIFICDOCUMENT_H

#include "ksyndication_export.h"

#include <boost/shared_ptr.hpp>

#include <QString>

namespace Syndication {

class DocumentVisitor;
class SpecificDocument;

//@cond PRIVATE
typedef boost::shared_ptr<SpecificDocument> SpecificDocumentPtr;
//@endcond

/**
 * Document interface for format-specific feed documents as parsed from a
 * document source (see DocumentSource).
 * The Document classes from the several syndication formats must implement
 * this interface. It's main purpose is to provide access for document visitors
 * (see DocumentVisitor).
 * Usually it is not necessary to access the format-specific document at all,
 * use Feed for a format-agnostic interface to all feed documents supported by
 * the library.
 *
 * @author Frank Osterfeld
 */
class SYNDICATION_EXPORT SpecificDocument
{
    public:

        /**
         * virtual dtor
         */
        virtual ~SpecificDocument();

        /**
         * This must be implemented for the double dispatch
         * technique (Visitor pattern).
         *
         * The usual implementation is
         * @code
         * return visitor->visit(this);
         * @endcode
         *
         * See also DocumentVisitor.
         *
         * @param visitor the visitor "visiting" this object
         */
        virtual bool accept(DocumentVisitor* visitor) = 0;

        /**
         * Returns whether this document is valid or not.
         * Invalid documents do not contain any useful
         * information.
         */
        virtual bool isValid() const = 0;

        /**
         * Returns a description of the document for debugging purposes.
         *
         * @return debug string
         */
        virtual QString debugInfo() const = 0;
};

} // namespace Syndication

#endif // SYNDICATION_SPECIFICDOCUMENT_H

