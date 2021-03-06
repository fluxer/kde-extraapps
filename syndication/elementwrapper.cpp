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
#include "elementwrapper.h"
#include "constants.h"

#include <kurl.h>


#include <QtXml/qdom.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

namespace Syndication {

class ElementWrapper::ElementWrapperPrivate
{
    public:
        
    QDomElement element;
    QDomDocument ownerDoc;
    mutable QString xmlBase;
    mutable bool xmlBaseParsed;
    mutable QString xmlLang;
    mutable bool xmlLangParsed;
};

ElementWrapper::ElementWrapper() : d(new ElementWrapperPrivate)
{
    d->xmlBaseParsed = true;
    d->xmlLangParsed = true;
}

ElementWrapper::ElementWrapper(const ElementWrapper& other)
{
    *this = other;
}

ElementWrapper::ElementWrapper(const QDomElement& element) : d(new ElementWrapperPrivate)
{
    d->element = element;
    d->ownerDoc = element.ownerDocument(); //keep a copy of the (shared, thus cheap) document around to ensure the element isn't deleted too early (Bug 190068)
    d->xmlBaseParsed = false;
    d->xmlLangParsed = false;
}

ElementWrapper::~ElementWrapper()
{
}

ElementWrapper& ElementWrapper::operator=(const ElementWrapper& other)
{
    d = other.d;
    return *this;
}

bool ElementWrapper::operator==(const ElementWrapper& other) const
{
    return d->element == other.d->element;
}

bool ElementWrapper::isNull() const
{
    return d->element.isNull();
}

const QDomElement& ElementWrapper::element() const
{
    return d->element;
}

QString ElementWrapper::xmlBase() const
{
    if (!d->xmlBaseParsed) // xmlBase not computed yet
    {
        QDomElement current = d->element;

        /*
        An atom feed can contain nested xml:base elements, like this:

        <feed xml:base="http://example.com/foo.atom">
          <entry xml:base="subdir/">
            <link href="foo.html"/>
          </entry>
        </feed>

        To compute xml:base we explore the tree all the way up to the top.
        `bases` stores all the xml:base values from the deepest element up to
        the root element.
        */
        QStringList bases;
        while (!current.isNull())
        {
            if (current.hasAttributeNS(xmlNamespace(), QLatin1String("base")))
            {
                bases << current.attributeNS(xmlNamespace(), QLatin1String("base"));
            }
            
            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement())
                current = parent.toElement();
            else
                current = QDomElement();
        }
        while (!bases.isEmpty())
        {
            KUrl u(d->xmlBase, bases.takeLast());
            d->xmlBase = u.url();
        }
        
        d->xmlBaseParsed = true;
    }
    
    return d->xmlBase;
}

QString ElementWrapper::completeURI(const QString& uri) const
{
    KUrl u(xmlBase(), uri);
    
    if (u.isValid())
        return u.url();
    
    return uri;
}

QString ElementWrapper::xmlLang() const
{
    if (!d->xmlLangParsed) // xmlLang not computed yet
    {
        QDomElement current = d->element;
        
        while (!current.isNull())
        {
            if (current.hasAttributeNS(xmlNamespace(), QLatin1String("lang")))
            {
                d->xmlLang = current.attributeNS(xmlNamespace(), QLatin1String("lang"));
                return d->xmlLang;
            }
            
            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement())
                current = parent.toElement();
            else
                current = QDomElement();
        }
        d->xmlLangParsed = true;
    }
    return d->xmlLang;
}
        
QString ElementWrapper::extractElementText(const QString& tagName) const
{
    QDomElement el = d->element.namedItem(tagName).toElement();
    return el.isNull() ? QString() : el.text().trimmed();
}

QString ElementWrapper::extractElementTextNS(const QString& namespaceURI, const QString& localName) const
{
    QDomElement el = firstElementByTagNameNS(namespaceURI, localName);
    return el.isNull() ? QString() : el.text().trimmed();
}

QString ElementWrapper::childNodesAsXML(const QDomElement& parent)
{
    ElementWrapper wrapper(parent);
    
    if (parent.isNull())
        return QString();

    QDomNodeList list = parent.childNodes();
    
    QString str;
    QTextStream ts( &str, QIODevice::WriteOnly );
    
    // if there is a xml:base in our scope, first set it for
    // each child element so the xml:base shows up in the
    // serialization
    QString base = wrapper.xmlBase();


    for (int i = 0; i < list.count(); ++i)
    {
        QDomNode it = list.item(i);
        if (!base.isEmpty() && it.isElement() 
             && !it.toElement().hasAttributeNS(xmlNamespace(), QLatin1String("base")))
        {
            it.toElement().setAttributeNS(xmlNamespace(), QLatin1String("base"), base);
        }
            
        ts << it;
    }
    return str.trimmed();
}

QString ElementWrapper::childNodesAsXML() const
{
    return childNodesAsXML(d->element);
}

QList<QDomElement> ElementWrapper::elementsByTagName(const QString& tagName) const
{
    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.tagName() == tagName)
                elements.append(e);
        }
    }
    return elements;
}

QDomElement ElementWrapper::firstElementByTagNameNS(const QString& nsURI, const QString& localName) const
{
    if (isNull())
        return QDomElement();
    
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI)
                return e;
        }
    }
    
    return QDomElement();
}


QList<QDomElement> ElementWrapper::elementsByTagNameNS(const QString& nsURI, const QString& localName) const
{
    if (isNull())
        return QList<QDomElement>();
    
    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI)
                elements.append(e);
        }
    }
    return elements;
}

QString ElementWrapper::text() const
{
    return d->element.text();
}
        
QString ElementWrapper::attribute(const QString& name, const QString& defValue) const
{
    return d->element.attribute(name, defValue);
}
        
QString ElementWrapper::attributeNS(const QString& nsURI, const QString& localName, const QString& defValue) const
{
    return d->element.attributeNS(nsURI, localName, defValue);
}

bool ElementWrapper::hasAttribute(const QString& name) const
{
    return d->element.hasAttribute(name);
}

bool ElementWrapper::hasAttributeNS(const QString& nsURI, const QString& localName) const
{
    return d->element.hasAttributeNS(nsURI, localName);
}

} // namespace Syndication
