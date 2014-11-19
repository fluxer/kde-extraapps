/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
#ifndef DOCUMENTFACTORY_H
#define DOCUMENTFACTORY_H

// Qt
#include <QObject>

#include <lib/document/document.h>

class QUndoGroup;

class KUrl;

namespace Gwenview
{

struct DocumentFactoryPrivate;

/**
 * This class holds all instances of Document.
 *
 * It keeps a cache of recently accessed documents to avoid reloading them.
 * To do so it keeps a last-access timestamp, which is updated to the
 * current time every time DocumentFactory::load() is called.
 */
class GWENVIEWLIB_EXPORT DocumentFactory : public QObject
{
    Q_OBJECT
public:
    static DocumentFactory* instance();
    ~DocumentFactory();

    /**
     * Loads the document associated with url, or returns an already cached
     * instance of Document::Ptr if there is any.
     * This method updates the last-access timestamp.
     */
    Document::Ptr load(const KUrl& url);

    /**
     * Returns a document if it has already been loaded once with load().
     * This method does not update the last-access timestamp.
     */
    Document::Ptr getCachedDocument(const KUrl&) const;

    QList<KUrl> modifiedDocumentList() const;

    bool hasUrl(const KUrl&) const;

    void clearCache();

    QUndoGroup* undoGroup();

    /**
     * Do not keep document whose url is @url in cache even if it has been
     * modified
     */
    void forget(const KUrl& url);

Q_SIGNALS:
    void modifiedDocumentListChanged();
    void documentChanged(const KUrl&);
    void documentBusyStateChanged(const KUrl&, bool);

private Q_SLOTS:
    void slotLoaded(const KUrl&);
    void slotSaved(const KUrl&, const KUrl&);
    void slotModified(const KUrl&);
    void slotBusyChanged(const KUrl&, bool);

private:
    DocumentFactory();

    DocumentFactoryPrivate* const d;
};

} // namespace
#endif /* DOCUMENTFACTORY_H */
