// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "timeutils.h"

// Qt
#include <QFile>

// KDE
#include <KDateTime>
#include <KDebug>
#include <KFileItem>
#include <KExiv2>

// Local
#include <lib/urlutils.h>

namespace Gwenview
{

namespace TimeUtils
{

struct CacheItem
{
    KDateTime fileMTime;
    KDateTime realTime;

    void update(const KFileItem& fileItem)
    {
        KDateTime time = fileItem.time(KFileItem::ModificationTime);
        if (fileMTime == time) {
            return;
        }

        fileMTime = time;

        if (!updateFromExif(fileItem.url())) {
            realTime = time;
        }
    }

    bool updateFromExif(const KUrl& url)
    {
        if (!UrlUtils::urlIsFastLocalFile(url)) {
            return false;
        }
    
        const QString path = url.path();
        const KExiv2 kexiv2(path);
        // Ordered list of keys to try
        static QList<QByteArray> datelst = QList<QByteArray>()
            << QByteArray("Exif.Photo.DateTimeOriginal")
            << QByteArray("Exif.Image.DateTimeOriginal")
            << QByteArray("Exif.Photo.DateTimeDigitized")
            << QByteArray("Exif.Image.DateTime");
        QString exifvalue;
        foreach (const KExiv2Property &kexiv2property, kexiv2.metadata()) {
            if (datelst.contains(kexiv2property.name)) {
                exifvalue = kexiv2property.value;
                if (!exifvalue.isEmpty()) {
                    break;
                }
            }
        }
        if (exifvalue.isEmpty()) {
            kWarning() << "No date in exif header of" << path;
            return false;
        }

        KDateTime dt = KDateTime::fromString(exifvalue, "%Y:%m:%d %H:%M:%S");
        if (!dt.isValid()) {
            kWarning() << "Invalid date in exif header of" << path;
            return false;
        }

        realTime = dt;
        return true;
    }
};

typedef QHash<KUrl, CacheItem> Cache;

KDateTime dateTimeForFileItem(const KFileItem& fileItem, CachePolicy cachePolicy)
{
    if (cachePolicy == SkipCache) {
        CacheItem item;
        item.update(fileItem);
        return item.realTime;
    }

    static Cache cache;
    const KUrl url = fileItem.targetUrl();

    Cache::iterator it = cache.find(url);
    if (it == cache.end()) {
        it = cache.insert(url, CacheItem());
    }

    it.value().update(fileItem);
    return it.value().realTime;
}

} // namespace

} // namespace
