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
#include "moc_sorteddirmodel.cpp"

// Qt
#include <QTimer>

// KDE
#include <KDebug>
#include <KDateTime>
#include <KDirLister>

// Local
#include <lib/timeutils.h>
#include <KDirModel>

namespace Gwenview
{

AbstractSortedDirModelFilter::AbstractSortedDirModelFilter(SortedDirModel* model)
: QObject(model)
, mModel(model)
{
    if (mModel) {
        mModel->addFilter(this);
    }
}

AbstractSortedDirModelFilter::~AbstractSortedDirModelFilter()
{
    if (mModel) {
        mModel->removeFilter(this);
    }
}

struct SortedDirModelPrivate
{
    KDirModel* mSourceModel;
    QStringList mBlackListedExtensions;
    QList<AbstractSortedDirModelFilter*> mFilters;
    QTimer mDelayedApplyFiltersTimer;
    MimeTypeUtils::Kinds mKindFilter;
};

SortedDirModel::SortedDirModel(QObject* parent)
: KDirSortFilterProxyModel(parent)
, d(new SortedDirModelPrivate)
{
    d->mSourceModel = new KDirModel(this);
    setSourceModel(d->mSourceModel);
    d->mDelayedApplyFiltersTimer.setInterval(0);
    d->mDelayedApplyFiltersTimer.setSingleShot(true);
    connect(&d->mDelayedApplyFiltersTimer, SIGNAL(timeout()), SLOT(doApplyFilters()));
}

SortedDirModel::~SortedDirModel()
{
    delete d;
}

MimeTypeUtils::Kinds SortedDirModel::kindFilter() const
{
    return d->mKindFilter;
}

void SortedDirModel::setKindFilter(MimeTypeUtils::Kinds kindFilter)
{
    if (d->mKindFilter == kindFilter) {
        return;
    }
    d->mKindFilter = kindFilter;
    applyFilters();
}

void SortedDirModel::adjustKindFilter(MimeTypeUtils::Kinds kinds, bool set)
{
    MimeTypeUtils::Kinds kindFilter = d->mKindFilter;
    if (set) {
        kindFilter |= kinds;
    } else {
        kindFilter &= ~kinds;
    }
    setKindFilter(kindFilter);
}

void SortedDirModel::addFilter(AbstractSortedDirModelFilter* filter)
{
    d->mFilters << filter;
    applyFilters();
}

void SortedDirModel::removeFilter(AbstractSortedDirModelFilter* filter)
{
    d->mFilters.removeAll(filter);
    applyFilters();
}

KDirLister* SortedDirModel::dirLister() const
{
    return d->mSourceModel->dirLister();
}

void SortedDirModel::reload()
{
    dirLister()->updateDirectory();
}

void SortedDirModel::setBlackListedExtensions(const QStringList& list)
{
    d->mBlackListedExtensions = list;
}

KFileItem SortedDirModel::itemForIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return KFileItem();
    }

    QModelIndex sourceIndex = mapToSource(index);
    return d->mSourceModel->itemForIndex(sourceIndex);
}

KUrl SortedDirModel::urlForIndex(const QModelIndex& index) const
{
    KFileItem item = itemForIndex(index);
    return item.isNull() ? KUrl() : item.url();
}

KFileItem SortedDirModel::itemForSourceIndex(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return KFileItem();
    }
    return d->mSourceModel->itemForIndex(sourceIndex);
}

QModelIndex SortedDirModel::indexForItem(const KFileItem& item) const
{
    if (item.isNull()) {
        return QModelIndex();
    }

    QModelIndex sourceIndex = d->mSourceModel->indexForItem(item);
    return mapFromSource(sourceIndex);
}

QModelIndex SortedDirModel::indexForUrl(const KUrl& url) const
{
    if (!url.isValid()) {
        return QModelIndex();
    }
    QModelIndex sourceIndex = d->mSourceModel->indexForUrl(url);
    return mapFromSource(sourceIndex);
}

bool SortedDirModel::filterAcceptsRow(int row, const QModelIndex& parent) const
{
    QModelIndex index = d->mSourceModel->index(row, 0, parent);
    KFileItem fileItem = d->mSourceModel->itemForIndex(index);

    MimeTypeUtils::Kinds kind = MimeTypeUtils::fileItemKind(fileItem);
    if (d->mKindFilter != MimeTypeUtils::Kinds() && !(d->mKindFilter & kind)) {
        return false;
    }

    if (kind != MimeTypeUtils::KIND_DIR) {
        int dotPos = fileItem.name().lastIndexOf('.');
        if (dotPos >= 1) {
            QString extension = fileItem.name().mid(dotPos + 1).toLower();
            if (d->mBlackListedExtensions.contains(extension)) {
                return false;
            }
        }

        Q_FOREACH(const AbstractSortedDirModelFilter * filter, d->mFilters) {
            if (!filter->acceptsIndex(index)) {
                return false;
            }
        }
    }
    return KDirSortFilterProxyModel::filterAcceptsRow(row, parent);
}

void SortedDirModel::applyFilters()
{
    d->mDelayedApplyFiltersTimer.start();
}

void SortedDirModel::doApplyFilters()
{
    QSortFilterProxyModel::invalidateFilter();
}

bool SortedDirModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const KFileItem leftItem = itemForSourceIndex(left);
    const KFileItem rightItem = itemForSourceIndex(right);

    const bool leftIsDir = leftItem.isDir();
    const bool rightIsDir = rightItem.isDir();

    if (leftIsDir != rightIsDir) {
        return leftIsDir;
    }

    if (sortRole() != KDirModel::ModifiedTime) {
        return KDirSortFilterProxyModel::lessThan(left, right);
    }

    const KDateTime leftDate = TimeUtils::dateTimeForFileItem(leftItem);
    const KDateTime rightDate = TimeUtils::dateTimeForFileItem(rightItem);

    return leftDate < rightDate;
}

bool SortedDirModel::hasDocuments() const
{
    const int count = rowCount();
    if (count == 0) {
        return false;
    }
    for (int row = 0; row < count; ++row) {
        const QModelIndex idx = index(row, 0);
        const KFileItem item = itemForIndex(idx);
        if (!item.isDir()) {
            return true;
        }
    }
    return false;
}

void SortedDirModel::setDirLister(KDirLister* dirLister)
{
    d->mSourceModel->setDirLister(dirLister);
}

} //namespace
