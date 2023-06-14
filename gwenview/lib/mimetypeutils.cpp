// vim: set tabstop=4 shiftwidth=4 expandtab
/*
Gwenview - A simple image viewer for KDE
Copyright 2006 Aurelien Gateau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "mimetypeutils.h"

// Qt
#include <QApplication>
#include <QStringList>

// KDE
#include <KApplication>
#include <KDebug>
#include <KFileItem>
#include <KIO/Job>
#include <KIO/JobClasses>
#include <KIO/NetAccess>
#include <KMimeType>
#include <KUrl>

#include <KImageIO>

// Local
#include <gvdebug.h>

namespace Gwenview
{

namespace MimeTypeUtils
{

static inline QString resolveAlias(const QString& name)
{
    KMimeType::Ptr ptr = KMimeType::mimeType(name, KMimeType::ResolveAliases);
    return ptr.isNull() ? name : ptr->name();
}

static void resolveAliasInList(QStringList* list)
{
    QStringList::Iterator
    it = list->begin(),
    end = list->end();
    for (; it != end; ++it) {
        *it = resolveAlias(*it);
    }
}

const QStringList& imageMimeTypes()
{
    static QStringList list;
    if (list.isEmpty()) {
        list = KImageIO::mimeTypes(KImageIO::Reading);

        list << "image/svg+xml" << "image/svg+xml-compressed";

        // need to invent more intelligent way to whitelist raws
        list << "image/x-nikon-nef" << "image/x-nikon-nrw"
             << "image/x-canon-cr2" << "image/x-canon-crw"
             << "image/x-pentax-pef" << "image/x-adobe-dng"
             << "image/x-sony-arw" << "image/x-minolta-mrw"
             << "image/x-panasonic-raw" << "image/x-panasonic-raw2"
             << "image/x-samsung-srw" << "image/x-olympus-orf"
             << "image/x-fuji-raf" << "image/x-kodak-dcr"
             << "image/x-kodak-kdc" << "image/x-kodak-k25"
             << "image/x-sigma-x3f";

        resolveAliasInList(&list);
    }

    return list;
}

QString urlMimeType(const KUrl& url)
{
    if (url.isEmpty()) {
        return "unknown";
    }
    // Try a simple guess, using extension for remote urls
    QString mimeType = KMimeType::findByUrl(url)->name();
    if (mimeType == "application/octet-stream") {
        kDebug() << "KMimeType::findByUrl() failed to find mimetype for" << url << ". Falling back to KIO::NetAccess::mimetype().";
        // No luck, look deeper. This can happens with http urls if the filename
        // does not provide any extension.
        mimeType = KIO::NetAccess::mimetype(url, KApplication::kApplication()->activeWindow());
    }
    return mimeType;
}

Kind mimeTypeKind(const QString& mimeType)
{
    if (imageMimeTypes().contains(mimeType)) {
        return KIND_IMAGE;
    }
    // if it is image but it is not one of the static MIME types attempt to open it anyway
    if (mimeType.startsWith(QLatin1String("image/"))) {
        return KIND_IMAGE;
    }
    if (mimeType == QLatin1String("inode/directory")) {
        return KIND_DIR;
    }
    return KIND_FILE;
}

Kind fileItemKind(const KFileItem& item)
{
    GV_RETURN_VALUE_IF_FAIL(!item.isNull(), KIND_UNKNOWN);
    return mimeTypeKind(item.mimetype());
}

Kind urlKind(const KUrl& url)
{
    return mimeTypeKind(urlMimeType(url));
}

} // namespace MimeTypeUtils
} // namespace Gwenview
