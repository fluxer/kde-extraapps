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

#ifndef SYNDICATION_RSS2_CLOUD_H
#define SYNDICATION_RSS2_CLOUD_H

#include <syndication/elementwrapper.h>

#include <QDomElement>
#include <QString>

namespace Syndication {
namespace RSS2
{
/**
   * Cloud information for an RSS channel.
   * It specifies a web service that supports the rssCloud interface which can
   * be implemented in HTTP-POST, XML-RPC or SOAP 1.1.
   * Its purpose is to allow processes to register with a cloud to be notified
   * of updates to the channel,
   * implementing a lightweight publish-subscribe protocol for RSS feeds.
   *
   * Example:
   *
   * Domain="rpc.sys.com", port="80", path="/RPC2"
   * registerProcedure="myCloud.rssPleaseNotify" protocol="xml-rpc"
   *
   * In this example, to request notification on the channel it appears in,
   * you would send an XML-RPC message to rpc.sys.com on port 80, with a path
   * of /RPC2. The procedure to call is myCloud.rssPleaseNotify.
   *
   * For more information on the rssCloud interface see
   * http://blogs.law.harvard.edu/tech/soapMeetsRss#rsscloudInterface
   *
   * (Note: This explanation was taken from
   * http://blogs.law.harvard.edu/tech/rss )
   *
   * @author Frank Osterfeld
   */
class SYNDICATION_EXPORT Cloud : public ElementWrapper
{
    public:

        /**
         * Default constructor, creates a null object, for which isNull()
         * is @p true.
         */
        Cloud();

        /**
         * Creates a Cloud object wrapping a @c &lt;cloud> XML element.
         *
         * @param element The @c &lt;cloud> element to wrap
         */
        explicit Cloud(const QDomElement& element);

        /**
         * the remote domain
         */
        QString domain() const;

        /**
         * the remote port to connect to
         */
        int port() const;

        /** 
         * the remote path to connect to
         */
        QString path() const;

        /**
         * register procedure, e.g. "myCloud.rssPleaseNotify"
         */
        QString registerProcedure() const;

        /**
         * protocol used for publish-subscribe, e.g. "xml-rpc"
         */
        QString protocol() const;

        /**
         * Returns a description of the object for debugging purposes.
         *
         * @return debug string
         */
        QString debugInfo() const;
};

} // namespace RSS2
} // namespace Syndication

#endif // SYNDICATION_RSS2_CLOUD_H
