// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

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
#include "documentdirfinder.moc"

// Qt

// KDE
#include <KDebug>
#include <KDirLister>

// Local
#include <lib/mimetypeutils.h>

namespace Gwenview
{

struct DocumentDirFinderPrivate
{
    KUrl mRootUrl;
    KDirLister* mDirLister;

    KUrl mFoundDirUrl;
};

DocumentDirFinder::DocumentDirFinder(const KUrl& rootUrl)
: d(new DocumentDirFinderPrivate)
{
    d->mRootUrl = rootUrl;
    d->mDirLister = new KDirLister(this);
    connect(d->mDirLister, SIGNAL(itemsAdded(KUrl,KFileItemList)),
            SLOT(slotItemsAdded(KUrl,KFileItemList)));
    connect(d->mDirLister, SIGNAL(completed()),
            SLOT(slotCompleted()));
    d->mDirLister->openUrl(rootUrl);
}

DocumentDirFinder::~DocumentDirFinder()
{
    delete d;
}

void DocumentDirFinder::start()
{
    d->mDirLister->openUrl(d->mRootUrl);
}

void DocumentDirFinder::slotItemsAdded(const KUrl& dir, const KFileItemList& list)
{
    Q_FOREACH(const KFileItem & item, list) {
        MimeTypeUtils::Kind kind = MimeTypeUtils::fileItemKind(item);
        switch (kind) {
        case MimeTypeUtils::KIND_DIR:
        case MimeTypeUtils::KIND_ARCHIVE:
            if (d->mFoundDirUrl.isValid()) {
                // This is the second dir we find, stop now
                finish(dir, MultipleDirsFound);
                return;
            } else {
                // First dir
                d->mFoundDirUrl = item.url();
            }
            break;

        case MimeTypeUtils::KIND_RASTER_IMAGE:
        case MimeTypeUtils::KIND_SVG_IMAGE:
        case MimeTypeUtils::KIND_VIDEO:
            finish(dir, DocumentDirFound);
            return;

        case MimeTypeUtils::KIND_UNKNOWN:
        case MimeTypeUtils::KIND_FILE:
            break;
        }
    }
}

void DocumentDirFinder::slotCompleted()
{
    if (d->mFoundDirUrl.isValid()) {
        KUrl url = d->mFoundDirUrl;
        d->mFoundDirUrl = KUrl();
        d->mDirLister->openUrl(url);
    } else {
        finish(d->mRootUrl, NoDocumentFound);
    }
}

void DocumentDirFinder::finish(const KUrl& url, DocumentDirFinder::Status status)
{
    disconnect(d->mDirLister, 0, this, 0);
    emit done(url, status);
    deleteLater();
}

} // namespace
