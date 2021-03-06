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

#include "parser.h"
#include "constants.h"
#include "content.h"
#include "document.h"

#include <documentsource.h>




#include <QtXml/qdom.h>

#include <QtXml/qdom.h>

#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtCore/QString>

namespace Syndication {
namespace Atom {

class Parser::ParserPrivate
{
    public:
    static QDomDocument convertAtom0_3(const QDomDocument& document);
    static QDomNode convertNode(QDomDocument& doc, const QDomNode& node, const QHash<QString, QString>& nameMapper);
};
        
bool Parser::accept(const Syndication::DocumentSource& source) const
{
    QDomElement root = source.asDomDocument().documentElement();
    return !root.isNull() && (root.namespaceURI() == atom1Namespace() || root.namespaceURI() == atom0_3Namespace());
}

Syndication::SpecificDocumentPtr Parser::parse(const Syndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull())
    {
        // if this is not atom, return an invalid feed document
        return FeedDocumentPtr(new FeedDocument());
    }
    
    QDomElement feed = doc.namedItem(QLatin1String("feed")).toElement();
    
    bool feedValid = !feed.isNull();

    if (feedValid && feed.attribute(QLatin1String("version"))
        == QLatin1String("0.3"))
    {
        doc = ParserPrivate::convertAtom0_3(doc);
        feed = doc.namedItem(QLatin1String("feed")).toElement();
        
    }

    feedValid = !feed.isNull() && feed.namespaceURI() == atom1Namespace();
    
    if (feedValid)
    {
        return FeedDocumentPtr(new FeedDocument(feed));
    }

    QDomElement entry = doc.namedItem(QLatin1String("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == atom1Namespace();

    if (entryValid)
    {
        return EntryDocumentPtr(new EntryDocument(feed));
    }

    // if this is not atom, return an invalid feed document
    return FeedDocumentPtr(new FeedDocument());
}

QString Parser::format() const
{
    return QLatin1String("atom");
}

QDomNode Parser::ParserPrivate::convertNode(QDomDocument& doc, const QDomNode& node, const QHash<QString, QString>& nameMapper)
{
    if (!node.isElement())
        return node.cloneNode(true);
    
    bool isAtom03Element = node.namespaceURI() == atom0_3Namespace();
    QDomElement oldEl = node.toElement();
            
    // use new namespace
    QString newNS = isAtom03Element ? atom1Namespace() : node.namespaceURI();
    
    QString newName = node.localName();
    
    // rename tags that are listed in the nameMapper
    if (isAtom03Element && nameMapper.contains(node.localName()))
        newName = nameMapper[node.localName()];
    
    QDomElement newEl = doc.createElementNS(newNS, newName);
    
    QDomNamedNodeMap attributes = oldEl.attributes();
    
    // copy over attributes
    for (int i = 0; i < attributes.count(); ++i)
    {
        QDomAttr attr = attributes.item(i).toAttr();
        if (attr.namespaceURI().isEmpty())
            newEl.setAttribute(attr.name(), attr.value());
        else
            newEl.setAttributeNS(attr.namespaceURI(), attr.name(), attr.value());
    }
    
    bool isTextConstruct = newNS == atom1Namespace() 
            && (newName == QLatin1String("title")
            || newName == QLatin1String("rights")
            || newName == QLatin1String("subtitle")
            || newName == QLatin1String("summary"));
    
    // for atom text constructs, map to new type schema (which only allows text, type, xhtml)
    
    if (isTextConstruct)
    {
        QString oldType = newEl.attribute(QLatin1String("type"), QLatin1String("text/plain") );
        QString newType;
        
        Content::Format format = Content::mapTypeToFormat(oldType);
        switch (format)
        {
            case Content::XML:
                newType = QLatin1String("xhtml");
                break;
            case Content::EscapedHTML:
                newType = QLatin1String("html");
                break;
            case Content::PlainText:
            case Content::Binary:
            default:
                newType = QLatin1String("text");
                
        }
        
        newEl.setAttribute(QLatin1String("type"), newType);
    }
    else
    {
        // for generator, rename the "url" attribute to "uri"
        
        bool isGenerator = newNS == atom1Namespace() && newName == QLatin1String("generator");        
        if (isGenerator && newEl.hasAttribute(QLatin1String("url")))
            newEl.setAttribute(QLatin1String("uri"), newEl.attribute(QLatin1String("url")));
    }
    
    // process child nodes recursively and append them
    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.count(); ++i)
    {
        newEl.appendChild(convertNode(doc, children.item(i), nameMapper));
    }
    
    return newEl;
}

QDomDocument Parser::ParserPrivate::convertAtom0_3(const QDomDocument& doc03)
{
    QDomDocument doc = doc03.cloneNode(false).toDocument();
    
    // these are the tags that were renamed in 1.0
    QHash<QString, QString> nameMapper;
    nameMapper.insert(QLatin1String("issued"), QLatin1String("published"));
    nameMapper.insert(QLatin1String("modified"), QLatin1String("updated"));
    nameMapper.insert(QLatin1String("url"), QLatin1String("uri"));
    nameMapper.insert(QLatin1String("copyright"), QLatin1String("rights"));
    nameMapper.insert(QLatin1String("tagline"), QLatin1String("subtitle"));
    
    QDomNodeList children = doc03.childNodes();
    
    for (int i = 0; i < children.count(); ++i)
    {
        doc.appendChild(convertNode(doc, children.item(i), nameMapper));
    }

    return doc;
}

Parser::Parser() : d(0) {}
Parser::~Parser() {}
Parser::Parser(const Parser& other) : AbstractParser(other), d(0) {}
Parser& Parser::operator=(const Parser& /*other*/) { return *this; }

} // namespace Atom
} // namespace Syndication
