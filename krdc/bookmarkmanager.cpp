/****************************************************************************
**
** Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
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

#include "bookmarkmanager.h"

#include "mainwindow.h"

#include <KBookmarkMenu>
#include <KStandardDirs>
#include <KDebug>

BookmarkManager::BookmarkManager(KActionCollection *collection, KMenu *menu, MainWindow *parent)
        : QObject(parent),
        KBookmarkOwner(),
        m_mainWindow(parent)
{
    const QString file = KStandardDirs::locateLocal("data", "krdc/bookmarks.xml");

    m_manager = KBookmarkManager::managerForFile(file, "krdc");
    m_manager->setUpdate(true);
    m_bookmarkMenu = new KBookmarkMenu(m_manager, this, menu, collection);

    KBookmarkGroup root = m_manager->root();
    KBookmark bm = root.first();
    while (!bm.isNull()) {
        if (bm.metaDataItem("krdc-history") == "historyfolder") // get it also when user renamed it
            break;
        bm = root.next(bm);
    }

    if (bm.isNull()) {
        kDebug(5010) << "History folder not found. Create it.";
        bm = m_manager->root().createNewFolder(i18n("History"));
        bm.setMetaDataItem("krdc-history", "historyfolder");
        m_manager->emitChanged();
    }

    m_historyGroup = bm.toGroup();
}

BookmarkManager::~BookmarkManager()
{
    delete m_bookmarkMenu;
}

void BookmarkManager::addHistoryBookmark(RemoteView *view)
{
    KBookmark bm = m_historyGroup.first();
    const QString urlString = urlForView(view);
    const KUrl url = KUrl(urlString);
    while (!bm.isNull()) {
        if (bm.url() == url) {
            kDebug(5010) << "Found URL. Move it at the history start.";
            m_historyGroup.moveBookmark(bm, KBookmark());
            bm.updateAccessMetadata();
            m_manager->emitChanged(m_historyGroup);
            return;
        }
        bm = m_historyGroup.next(bm);
    }

    if (bm.isNull()) {
        kDebug(5010) << "Add new history bookmark.";
        bm = m_historyGroup.addBookmark(titleForUrl(urlString), urlString);
        bm.updateAccessMetadata();
        m_historyGroup.moveBookmark(bm, KBookmark());
        m_manager->emitChanged(m_historyGroup);
    }
}

void BookmarkManager::openBookmark(const KBookmark &bm, Qt::MouseButtons, Qt::KeyboardModifiers)
{
    emit openUrl(bm.url());
}

void BookmarkManager::openFolderinTabs(const KBookmarkGroup &bookmarkGroup)
{
    KBookmark bm = bookmarkGroup.first();
    while (!bm.isNull()) {
        emit openUrl(bm.url());
        bm = bookmarkGroup.next(bm);
    }
}

bool BookmarkManager::addBookmarkEntry() const
{
    return true;
}

bool BookmarkManager::editBookmarkEntry() const
{
    return true;
}

QString BookmarkManager::currentUrl() const
{
    RemoteView *view = m_mainWindow->currentRemoteView();
    if (view)
        return urlForView(view);
    else
        return QString();
}

QString BookmarkManager::urlForView(RemoteView *view) const
{
    return view->url().prettyUrl(KUrl::RemoveTrailingSlash);

}

QString BookmarkManager::currentTitle() const
{
    return titleForUrl(currentUrl());
}

QString BookmarkManager::titleForUrl(const QString &url) const
{
    return QUrl::fromPercentEncoding(url.toUtf8());

}

bool BookmarkManager::supportsTabs() const
{
    return true;
}

QList<QPair<QString, QString> > BookmarkManager::currentBookmarkList() const
{
    QList<QPair<QString, QString> > list;

    QMapIterator<QWidget *, RemoteView *> iter(m_mainWindow->remoteViewList());

    while (iter.hasNext()) {
        RemoteView *next = iter.next().value();
        const QString url = next->url().prettyUrl(KUrl::RemoveTrailingSlash);
        list << QPair<QString, QString>(url, url);
    }

    return list;
}

void BookmarkManager::addManualBookmark(const QString &url, const QString &text)
{
    m_manager->root().addBookmark(url, text);
    emit m_manager->emitChanged();
}

KBookmarkManager* BookmarkManager::getManager()
{
    return m_manager;
}

const QStringList BookmarkManager::findBookmarkAddresses(const KBookmarkGroup &group, const QString &url)
{
    QStringList bookmarkAddresses = QStringList();
    KBookmark bm = group.first();
    while (!bm.isNull()) {
        if (bm.isGroup()) {
            bookmarkAddresses.append(findBookmarkAddresses(bm.toGroup(), url));
        }

        if (bm.url() == url) {
            bookmarkAddresses.append(bm.address());
        }
        bm = group.next(bm);
    }
    return bookmarkAddresses;
}

void BookmarkManager::removeByUrl(KBookmarkManager *manager, const QString &url, bool ignoreHistory, const QString updateTitle)
{
    foreach(const QString &address, findBookmarkAddresses(manager->root(), url)) {
        KBookmark bm = manager->findByAddress(address);
        if (ignoreHistory && bm.parentGroup().metaDataItem("krdc-history") == "historyfolder") {
            if (!updateTitle.isEmpty()) {
                kDebug(5010) << "Update" << bm.fullText();
                bm.setFullText(updateTitle);
            }
        } else {
            if (!bm.isGroup()) { // please don't delete groups... happened in testing
                kDebug(5010) << "Delete" << bm.fullText();
                bm.parentGroup().deleteBookmark(bm);
            }
        }
    }

    manager->emitChanged();
}

void BookmarkManager::updateTitle(KBookmarkManager *manager, const QString &url, const QString &title)
{
    foreach(const QString &address, findBookmarkAddresses(manager->root(), url)) {
        KBookmark bm = manager->findByAddress(address);
        bm.setFullText(title);
        kDebug(5010) << "Update" << bm.fullText();
    }
    manager->emitChanged();
}

#include "bookmarkmanager.moc"
