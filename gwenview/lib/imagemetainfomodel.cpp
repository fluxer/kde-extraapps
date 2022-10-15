// vim: set tabstop=4 shiftwidth=4 expandtab:
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
// Self
#include "imagemetainfomodel.h"

// Qt

// KDE
#include <KDebug>
#include <KFileItem>
#include <KGlobal>
#include <KLocale>
#include <KExiv2>

// Local

namespace Gwenview
{

enum GroupRow {
    NoGroupSpace = -2,
    NoGroup = -1,
    GeneralGroup,
    ExifGroup,
    IptcGroup,
    XmpGroup
};

class MetaInfoGroup
{
public:
    enum {
        InvalidRow = -1
    };

    class Entry
    {
    public:
        Entry(const QString& key, const QString& label, const QString& value)
            : mKey(key), mLabel(label.trimmed()), mValue(value.trimmed())
        {}

        QString key() const
        {
            return mKey;
        }
        QString label() const
        {
            return mLabel;
        }

        QString value() const
        {
            return mValue;
        }
        void setValue(const QString& value)
    {
            mValue = value.trimmed();
        }

        void appendValue(const QString& value)
    {
            if (mValue.length() > 0) {
                mValue += '\n';
            }
            mValue += value.trimmed();
        }

    private:
        QString mKey;
        QString mLabel;
        QString mValue;
    };

    MetaInfoGroup(const QString& label)
        : mLabel(label)
        {}

    ~MetaInfoGroup()
    {
        qDeleteAll(mList);
    }

    void clear()
    {
        qDeleteAll(mList);
        mList.clear();
        mRowForKey.clear();
    }

    void addEntry(const QString& key, const QString& label, const QString& value)
    {
        addEntry(new Entry(key, label, value));
    }

    void addEntry(Entry* entry)
    {
        mList << entry;
        mRowForKey[entry->key()] = mList.size() - 1;
    }

    void getInfoForKey(const QString& key, QString* label, QString* value) const
    {
        Entry* entry = getEntryForKey(key);
        if (entry) {
            *label = entry->label();
            *value = entry->value();
        }
    }

    QString getKeyAt(int row) const
    {
        Q_ASSERT(row < mList.size());
        return mList[row]->key();
    }

    QString getLabelForKeyAt(int row) const
    {
        Q_ASSERT(row < mList.size());
        return mList[row]->label();
    }

    QString getValueForKeyAt(int row) const
    {
        Q_ASSERT(row < mList.size());
        return mList[row]->value();
    }

    void setValueForKeyAt(int row, const QString& value)
    {
        Q_ASSERT(row < mList.size());
        mList[row]->setValue(value);
    }

    int getRowForKey(const QString& key) const
    {
        return mRowForKey.value(key, InvalidRow);
    }

    int size() const
    {
        return mList.size();
    }

    QString label() const
    {
        return mLabel;
    }

    const QList<Entry*>& entryList() const {
        return mList;
    }

private:
    Entry* getEntryForKey(const QString& key) const
    {
        int row = getRowForKey(key);
        if (row == InvalidRow) {
            return 0;
        }
        return mList[row];
    }

    QList<Entry*> mList;
    QHash<QString, int> mRowForKey;
    QString mLabel;
};

struct ImageMetaInfoModelPrivate
{
    QVector<MetaInfoGroup*> mMetaInfoGroupVector;
    ImageMetaInfoModel* q;

    void clearGroup(MetaInfoGroup* group, const QModelIndex& parent)
    {
        if (group->size() > 0) {
            q->beginRemoveRows(parent, 0, group->size() - 1);
            group->clear();
            q->endRemoveRows();
        }
    }

    void setGroupEntryValue(GroupRow groupRow, const QString& key, const QString& value)
    {
        MetaInfoGroup* group = mMetaInfoGroupVector[groupRow];
        int entryRow = group->getRowForKey(key);
        if (entryRow == MetaInfoGroup::InvalidRow) {
            kWarning() << "No row for key" << key;
            return;
        }
        group->setValueForKeyAt(entryRow, value);
        QModelIndex groupIndex = q->index(groupRow, 0);
        QModelIndex entryIndex = q->index(entryRow, 1, groupIndex);
        emit q->dataChanged(entryIndex, entryIndex);
    }

    QVariant displayData(const QModelIndex& index) const
    {
        if (index.internalId() == NoGroup) {
            if (index.column() != 0) {
                return QVariant();
            }
            QString label = mMetaInfoGroupVector[index.row()]->label();
            return QVariant(label);
        }

        if (index.internalId() == NoGroupSpace) {
            return QVariant(QString());
        }

        MetaInfoGroup* group = mMetaInfoGroupVector[index.internalId()];
        if (index.column() == 0) {
            return group->getLabelForKeyAt(index.row());
        } else {
            return group->getValueForKeyAt(index.row());
        }
    }

    void initGeneralGroup()
    {
        MetaInfoGroup* group = mMetaInfoGroupVector[GeneralGroup];
        group->addEntry("General.Name", i18nc("@item:intable Image file name", "Name"), QString());
        group->addEntry("General.Size", i18nc("@item:intable", "File Size"), QString());
        group->addEntry("General.Time", i18nc("@item:intable", "File Time"), QString());
        group->addEntry("General.ImageSize", i18nc("@item:intable", "Image Size"), QString());
        group->addEntry("General.Comment", i18nc("@item:intable", "Comment"), QString());
    }
};

ImageMetaInfoModel::ImageMetaInfoModel()
: d(new ImageMetaInfoModelPrivate)
{
    d->q = this;
    d->mMetaInfoGroupVector.resize(4);
    d->mMetaInfoGroupVector[GeneralGroup] = new MetaInfoGroup(i18nc("@title:group General info about the image", "General"));
    d->mMetaInfoGroupVector[ExifGroup] = new MetaInfoGroup("EXIF");
    d->mMetaInfoGroupVector[IptcGroup] = new MetaInfoGroup("IPTC");
    d->mMetaInfoGroupVector[XmpGroup]  = new MetaInfoGroup("XMP");
    d->initGeneralGroup();
}

ImageMetaInfoModel::~ImageMetaInfoModel()
{
    qDeleteAll(d->mMetaInfoGroupVector);
    delete d;
}

void ImageMetaInfoModel::setUrl(const KUrl& url)
{
    KFileItem item(KFileItem::Unknown, KFileItem::Unknown, url);
    QString sizeString = KGlobal::locale()->formatByteSize(item.size());

    d->setGroupEntryValue(GeneralGroup, "General.Name", item.name());
    d->setGroupEntryValue(GeneralGroup, "General.Size", sizeString);
    d->setGroupEntryValue(GeneralGroup, "General.Time", item.timeString());

    KExiv2 kexiv2(url.path());
    const KExiv2::DataMap kexiv2datamap = kexiv2.data();
    QList<QByteArray> exifkeys;
    QList<QByteArray> iptckeys;
    QList<QByteArray> xmpkeys;
    foreach (const QByteArray &kexiv2key, kexiv2datamap.keys()) {
        if (kexiv2key.startsWith("Exif.")) {
            exifkeys.append(kexiv2key);
        } else if (kexiv2key.startsWith("Xmp.")) {
            iptckeys.append(kexiv2key);
        } else if (kexiv2key.startsWith("Iptc.")) {
            xmpkeys.append(kexiv2key);
        } else {
            kWarning() << "Unknown Exif2 key" << kexiv2key;
        }
    }

    MetaInfoGroup* exifGroup = d->mMetaInfoGroupVector[ExifGroup];
    MetaInfoGroup* iptcGroup = d->mMetaInfoGroupVector[IptcGroup];
    MetaInfoGroup* xmpGroup  = d->mMetaInfoGroupVector[XmpGroup];
    QModelIndex exifIndex = index(ExifGroup, 0);
    QModelIndex iptcIndex = index(IptcGroup, 0);
    QModelIndex xmpIndex  = index(XmpGroup, 0);
    d->clearGroup(exifGroup, exifIndex);
    d->clearGroup(iptcGroup, iptcIndex);
    d->clearGroup(xmpGroup,  xmpIndex);

    if (!exifkeys.isEmpty()) {
        beginInsertRows(exifIndex, 0, exifkeys.size() - 1);
        foreach (const QByteArray &exifkey, exifkeys) {
            exifGroup->addEntry(exifkey, kexiv2.label(exifkey), kexiv2datamap.value(exifkey));
        }
        endInsertRows();
    }

    if (!iptckeys.isEmpty()) {
        beginInsertRows(iptcIndex, 0, iptckeys.size() - 1);
        foreach (const QByteArray &iptckey, iptckeys) {
            iptcGroup->addEntry(iptckey, kexiv2.label(iptckey), kexiv2datamap.value(iptckey));
        }
        endInsertRows();
    }

    if (!xmpkeys.isEmpty()) {
        beginInsertRows(xmpIndex, 0, xmpkeys.size() - 1);
        foreach (const QByteArray &xmpkey, xmpkeys) {
            xmpGroup->addEntry(xmpkey, kexiv2.label(xmpkey), kexiv2datamap.value(xmpkey));
        }
        endInsertRows();
    }
}

void ImageMetaInfoModel::setImageSize(const QSize& size)
{
    QString imageSize;
    if (size.isValid()) {
        imageSize = i18nc(
                        "@item:intable %1 is image width, %2 is image height",
                        "%1x%2", size.width(), size.height());

        double megaPixels = size.width() * size.height() / 1000000.;
        if (megaPixels > 0.1) {
            QString megaPixelsString = QString::number(megaPixels, 'f', 1);
            imageSize += ' ';
            imageSize += i18nc(
                             "@item:intable %1 is number of millions of pixels in image",
                             "(%1MP)", megaPixelsString);
        }
    } else {
        imageSize = '-';
    }
    d->setGroupEntryValue(GeneralGroup, "General.ImageSize", imageSize);
}

void ImageMetaInfoModel::getInfoForKey(const QString& key, QString* label, QString* value) const
{
    MetaInfoGroup* group;
    if (key.startsWith(QLatin1String("General"))) {
        group = d->mMetaInfoGroupVector[GeneralGroup];
    } else if (key.startsWith(QLatin1String("Exif"))) {
        group = d->mMetaInfoGroupVector[ExifGroup];
    } else if (key.startsWith(QLatin1String("Iptc"))) {
        group = d->mMetaInfoGroupVector[IptcGroup];
    } else if (key.startsWith(QLatin1String("Xmp"))) {
        group = d->mMetaInfoGroupVector[XmpGroup];
    } else {
        kWarning() << "Unknown metainfo key" << key;
        return;
    }
    group->getInfoForKey(key, label, value);
}

QString ImageMetaInfoModel::getValueForKey(const QString& key) const
{
    QString label, value;
    getInfoForKey(key, &label, &value);
    return value;
}

QString ImageMetaInfoModel::keyForIndex(const QModelIndex& index) const
{
    if (index.internalId() == NoGroup) {
        return QString();
    }
    MetaInfoGroup* group = d->mMetaInfoGroupVector[index.internalId()];
    return group->getKeyAt(index.row());
}

QModelIndex ImageMetaInfoModel::index(int row, int col, const QModelIndex& parent) const
{
    if (col < 0 || col > 1) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        // This is a group
        if (row < 0 || row >= d->mMetaInfoGroupVector.size()) {
            return QModelIndex();
        }
        return createIndex(row, col, col == 0 ? NoGroup : NoGroupSpace);
    } else {
        // This is an entry
        int group = parent.row();
        if (row < 0 || row >= d->mMetaInfoGroupVector[group]->size()) {
            return QModelIndex();
        }
        return createIndex(row, col, group);
    }
}

QModelIndex ImageMetaInfoModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    if (index.internalId() == NoGroup || index.internalId() == NoGroupSpace) {
        return QModelIndex();
    } else {
        return createIndex(index.internalId(), 0, NoGroup);
    }
}

int ImageMetaInfoModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return d->mMetaInfoGroupVector.size();
    } else if (parent.internalId() == NoGroup) {
        return d->mMetaInfoGroupVector[parent.row()]->size();
    } else {
        return 0;
    }
}

int ImageMetaInfoModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

QVariant ImageMetaInfoModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return d->displayData(index);
    default:
        return QVariant();
    }
}

QVariant ImageMetaInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
        return QVariant();
    }

    QString caption;
    if (section == 0) {
        caption = i18nc("@title:column", "Property");
    } else if (section == 1) {
        caption = i18nc("@title:column", "Value");
    } else {
        kWarning() << "Unknown section" << section;
    }

    return QVariant(caption);
}

} // namespace
