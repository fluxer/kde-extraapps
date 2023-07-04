// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
#include "moc_recursivedirmodel.cpp"

// Local
#include <lib/gvdebug.h>

// KDE
#include <KDebug>
#include <KDirLister>
#include <KDirModel>

// Qt

namespace Gwenview
{

struct RecursiveDirModelPrivate
{
    KDirLister* mDirLister;

    int rowForUrl(const KUrl &url) const
    {
        return mRowForUrl.value(url, -1);
    }

    void removeAt(int row)
    {
        KFileItem item = mList.takeAt(row);
        mRowForUrl.remove(item.url());

        // Decrease row value for all urls after the one we removed
        // ("row" now points to the item after the one we removed since we used takeAt)
        const int count = mList.count();
        for (; row < count; ++row) {
            KUrl url = mList.at(row).url();
            mRowForUrl[url]--;
        }
    }

    void addItem(const KFileItem& item)
    {
        mRowForUrl.insert(item.url(), mList.count());
        mList.append(item);
    }

    void clear()
    {
        mRowForUrl.clear();
        mList.clear();
    }

    // RecursiveDirModel can only access mList through this read-only getter.
    // This ensures it cannot introduce inconsistencies between mList and mRowForUrl.
    const KFileItemList& list() const
    {
        return mList;
    }

private:
    KFileItemList mList;
    QHash<KUrl, int> mRowForUrl;
};

RecursiveDirModel::RecursiveDirModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new RecursiveDirModelPrivate)
{
    d->mDirLister = new KDirLister(this);
    connect(
        d->mDirLister, SIGNAL(itemsAdded(KFileItemList)),
        this, SLOT(slotItemsAdded(KFileItemList))
    );
    connect(
        d->mDirLister, SIGNAL(itemsDeleted(KFileItemList)),
        this, SLOT(slotItemsDeleted(KFileItemList))
    );
    connect(
        d->mDirLister, SIGNAL(clear()),
        this, SLOT(slotClear())
    );
    connect(
        d->mDirLister, SIGNAL(completed()),
        this, SIGNAL(completed())
    );
}

RecursiveDirModel::~RecursiveDirModel()
{
    delete d;
}

KUrl RecursiveDirModel::url() const
{
    return d->mDirLister->url();
}

void RecursiveDirModel::setUrl(const KUrl& url)
{
    d->mDirLister->openUrl(url, true);
}

int RecursiveDirModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->list().count();
}

QVariant RecursiveDirModel::data(const QModelIndex& index, int role) const
{
    if (index.parent().isValid()) {
        return QVariant();
    }
    KFileItem item = d->list().value(index.row());
    if (item.isNull()) {
        kWarning() << "Invalid row" << index.row();
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        return item.text();
    case Qt::DecorationRole:
        return item.iconName();
    case KDirModel::FileItemRole:
        return QVariant(item);
    default:
        kWarning() << "Unhandled role" << role;
        break;
    }
    return QVariant();
}

void RecursiveDirModel::slotItemsAdded(const KFileItemList& newList)
{
    KFileItemList fileList;
    Q_FOREACH(const KFileItem& item, newList) {
        if (item.isFile()) {
            if (d->rowForUrl(item.url()) == -1) {
                fileList << item;
            }
        }
    }

    if (!fileList.isEmpty()) {
        beginInsertRows(QModelIndex(), d->list().count(), d->list().count() + fileList.count());
        Q_FOREACH(const KFileItem& item, fileList) {
            d->addItem(item);
        }
        endInsertRows();
    }
}

void RecursiveDirModel::slotItemsDeleted(const KFileItemList& list)
{
    Q_FOREACH(const KFileItem& item, list) {
        if (item.isDir()) {
            continue;
        }
        int row = d->rowForUrl(item.url());
        if (row == -1) {
            kWarning() << "Received itemsDeleted for an unknown item: this should not happen!";
            GV_FATAL_FAILS;
            continue;
        }
        beginRemoveRows(QModelIndex(), row, row);
        d->removeAt(row);
        endRemoveRows();
    }
}

void RecursiveDirModel::slotClear()
{
    beginResetModel();
    d->clear();
    endResetModel();
}

} // namespace
