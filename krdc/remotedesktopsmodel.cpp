/****************************************************************************
**
** Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
** Copyright (C) 2009 Tony Murray <murraytony@gmail.com>
**
** This file is part of KDE.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; see the file COPYING. If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#include "remotedesktopsmodel.h"
#include "bookmarkmanager.h"

#include <KStandardDirs>
#include <KDebug>
#include <KLocale>

RemoteDesktopsModel::RemoteDesktopsModel(QObject *parent)
        : QAbstractTableModel(parent)
{
    const QString file = KStandardDirs::locateLocal("data", "krdc/bookmarks.xml");
    m_manager = KBookmarkManager::managerForFile(file, "krdc");
    m_manager->setUpdate(true);
    connect(m_manager, SIGNAL(changed(QString,QString)), SLOT(bookmarksChanged()));
    buildModelFromBookmarkGroup(m_manager->root());

#ifdef BUILD_ZEROCONF
    // Add RDP and NX if they start announcing via Zeroconf:
    m_protocols["_rfb._tcp"] = "vnc";

    zeroconfBrowser = new KDNSSD();
    connect(zeroconfBrowser, SIGNAL(finished()), this, SLOT(servicesChanged()));
    zeroconfBrowser->startBrowse("_rfb._tcp");
    kDebug(5010) << "Browsing for zeroconf hosts.";
#endif
}

RemoteDesktopsModel::~RemoteDesktopsModel()
{
}

int RemoteDesktopsModel::columnCount(const QModelIndex &) const
{
    return 6;  // same as count of RemoteDesktopsModel::DisplayItems enum
}

int RemoteDesktopsModel::rowCount(const QModelIndex &) const
{
    return remoteDesktops.size();
}

Qt::ItemFlags RemoteDesktopsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.column() == RemoteDesktopsModel::Favorite) {
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    return Qt::ItemIsEnabled;
}

bool RemoteDesktopsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole && index.column() == RemoteDesktopsModel::Favorite) {
        bool checked = (Qt::CheckState)value.toUInt() == Qt::Checked;
        remoteDesktops[index.row()].favorite = checked;

        RemoteDesktop rd = remoteDesktops.at(index.row());
        if (checked) {
            KBookmarkGroup root = m_manager->root();
            root.addBookmark(rd.title, rd.url);
            m_manager->emitChanged(root);
        } else {
            BookmarkManager::removeByUrl(m_manager, rd.url, true, rd.title);
        }
        return true;
    }
    return false;
}

QVariant RemoteDesktopsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    RemoteDesktop item = remoteDesktops.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case RemoteDesktopsModel::Favorite:
            return item.favorite;
        case RemoteDesktopsModel::Title:
            return item.title;
        case RemoteDesktopsModel::LastConnected:
            return QVariant(QDateTime(item.lastConnected));
        case RemoteDesktopsModel::VisitCount:
            return item.visits;
        case RemoteDesktopsModel::Created:
            if (item.created.isNull()) return QVariant();
            return KGlobal::locale()->formatDateTime(item.created.toLocalTime(), QLocale::NarrowFormat);
        case RemoteDesktopsModel::Source:
            switch (item.source) {
            case RemoteDesktop::Bookmarks:
                return i18nc("Where each displayed link comes from", "Bookmarks");
            case RemoteDesktop::History:
                return i18nc("Where each displayed link comes from", "History");
            case RemoteDesktop::Zeroconf:
                return i18nc("Where each displayed link comes from", "Zeroconf");
            case RemoteDesktop::None:
                return i18nc("Where each displayed link comes from", "None");
            }
        default:
            return QVariant();
        }

    case Qt::CheckStateRole:
        if (index.column() == RemoteDesktopsModel::Favorite)
            return item.favorite ? Qt::Checked : Qt::Unchecked;
        return QVariant();

    case Qt::ToolTipRole:
        switch(index.column()) {
        case RemoteDesktopsModel::Favorite:
            if (item.favorite) {
                return i18nc("Remove the selected url from the bookarks menu", "Remove the bookmark for %1.", item.title);
            } else {
                return i18nc("Add the selected url to the bookmarks menu", "Bookmark %1.", item.title);
            }
        case RemoteDesktopsModel::LastConnected:
            if (!item.lastConnected.isNull()) {
                return KGlobal::locale()->formatDateTime(item.lastConnected.toLocalTime(), QLocale::NarrowFormat);
            }
            break; // else show default tooltip
        case RemoteDesktopsModel::Created:
            if (!item.created.isNull()) {
                return KGlobal::locale()->formatDateTime(item.created.toLocalTime(), QLocale::NarrowFormat);
            }
            break; // else show default tooltip
        default:
            break;
        }
        return item.url;  //use the url for the tooltip
        
    case 10001: //url for dockwidget
        return item.url;

    case 10002: //filter
        return QUrl::fromPercentEncoding(QString(item.url + item.title).toUtf8()); // return both user visible title and url data, percent encoded

    case 10003: //title for dockwidget
        return item.title;

    default:
        return QVariant();
    }
}

QVariant RemoteDesktopsModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case RemoteDesktopsModel::Favorite:
            return QVariant(); // the favorite column is to small for a header
        case RemoteDesktopsModel::Title:
            return i18nc("Header of the connections list, title/url for remote connection", "Remote Desktop");
        case RemoteDesktopsModel::LastConnected:
            return i18nc("Header of the connections list, the last time this connection was initiated", "Last Connected");
        case RemoteDesktopsModel::VisitCount:
            return i18nc("Header of the connections list, the number of times this connection has been visited", "Visits");
        case RemoteDesktopsModel::Created:
            return i18nc("Header of the connections list, the time when this entry was created", "Created");
        case RemoteDesktopsModel::Source:
            return i18nc("Header of the connections list, where this entry comes from", "Source");
        }
    }
    return QVariant();
}

// does not trigger view update, you must do this by hand after using this function
void RemoteDesktopsModel::removeAllItemsFromSources(RemoteDesktop::Sources sources)
{
    QMutableListIterator<RemoteDesktop> iter(remoteDesktops);
    while (iter.hasNext()) {
        iter.next();
        // if it matches any of the specified sources, remove it
        if ((iter.value().source & sources) > 0)
            iter.remove();
    }
}

void RemoteDesktopsModel::bookmarksChanged()
{
    kDebug(5010);
    removeAllItemsFromSources(RemoteDesktop::Bookmarks | RemoteDesktop::History);
    buildModelFromBookmarkGroup(m_manager->root());
    reset();
}

// Danger Will Roobinson, confusing code ahead!
void RemoteDesktopsModel::buildModelFromBookmarkGroup(const KBookmarkGroup &group)
{
    KBookmark bm = group.first();
    while (!bm.isNull()) {
        if (bm.isGroup()) {
            // recurse subfolders and treat it special if it is the history folder
            buildModelFromBookmarkGroup(bm.toGroup());
        } else { // not a group

            RemoteDesktop item;
            item.title = bm.fullText();
            item.url = bm.url().url();
            int index = remoteDesktops.indexOf(item); //search for this url to see if we need to update it
            bool newItem = index < 0; // do we need to create a new item?

            // we want to merge all copies of a url into one link, so if the item exists, update it
            if (group.metaDataItem("krdc-history") == "historyfolder") {
                // set source and favorite (will override later if needed)
                item.source = RemoteDesktop::History;
                item.favorite = false;

                // since we are in the history folder collect statitics and add them
                QDateTime connected = QDateTime();
                QDateTime created = QDateTime();
                bool ok = false;
                // first the created datetime
                created.setTime_t(bm.metaDataItem("time_added").toLongLong(&ok));
                if (ok) (newItem ? item : remoteDesktops[index]).created = created;
                // then the last visited datetime
                ok = false;
                connected.setTime_t(bm.metaDataItem("time_visited").toLongLong(&ok));
                if (ok) (newItem ? item : remoteDesktops[index]).lastConnected = connected;
                // finally the visited count
                ok = false;
                int visits = bm.metaDataItem("visit_count").toInt(&ok);
                if (ok) (newItem ? item : remoteDesktops[index]).visits = visits;
            } else {
                if (newItem) {
                    // if this is a new item, just add the rest of the required data
                    item.lastConnected = QDateTime();
                    item.created = QDateTime();
                    item.visits = 0;
                    item.favorite = true;
                    item.source = RemoteDesktop::Bookmarks;
                } else {
                    // otherwise override these fields with the info from the bookmark
                    remoteDesktops[index].title = bm.fullText();
                    remoteDesktops[index].favorite = true;
                    remoteDesktops[index].source = RemoteDesktop::Bookmarks;
                }
            }
            // if we have a new item, add it
            if (newItem)
                remoteDesktops.append(item);
        }
        bm = group.next(bm); // next item in the group
    }
}

#ifdef BUILD_ZEROCONF
void RemoteDesktopsModel::servicesChanged()
{
    //redo list because it is easier than finding and removing one that disappeared
    KUrl url;
    removeAllItemsFromSources(RemoteDesktop::Zeroconf);
    foreach(const KDNSSDService &service, zeroconfBrowser->services()) {
        url.setScheme(m_protocols[service.type].toLower());
        url.setHost(service.hostname);
        url.setPort(service.port);

        RemoteDesktop item;
        item.url = url.url();

        if (!remoteDesktops.contains(item)) {
            item.title = service.name;
            item.source = RemoteDesktop::Zeroconf;
            item.created = QDateTime::currentDateTime();
            item.favorite = false;
            item.visits = 0;
            remoteDesktops.append(item);
        }
    }
    reset();
}
#endif

#include "moc_remotedesktopsmodel.cpp"
