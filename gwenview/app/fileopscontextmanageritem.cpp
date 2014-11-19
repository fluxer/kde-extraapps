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
#include "fileopscontextmanageritem.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QListView>
#include <QMenu>
#include <QMimeData>

// KDE
#include <KAction>
#include <KActionCollection>
#include <KActionCategory>
#include <KDebug>
#include <KFileItem>
#include <KFileItemActions>
#include <kio/paste.h>
#include <KLocale>
#include <KMimeTypeTrader>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KRun>
#include <KService>
#include <KXMLGUIClient>

// libkonq
#include <konqmimedata.h>
#include <konq_operations.h>

// Local
#include <lib/contextmanager.h>
#include <lib/eventwatcher.h>
#include <lib/gvdebug.h>
#include "fileoperations.h"
#include "sidebar.h"

namespace Gwenview
{

struct FileOpsContextManagerItemPrivate
{
    FileOpsContextManagerItem* q;
    QListView* mThumbnailView;
    KXMLGUIClient* mXMLGUIClient;
    SideBarGroup* mGroup;
    KAction* mCutAction;
    KAction* mCopyAction;
    KAction* mPasteAction;
    KAction* mCopyToAction;
    KAction* mMoveToAction;
    KAction* mLinkToAction;
    KAction* mRenameAction;
    KAction* mTrashAction;
    KAction* mDelAction;
    KAction* mRestoreAction;
    KAction* mShowPropertiesAction;
    KAction* mCreateFolderAction;
    KAction* mOpenWithAction;
    QList<QAction*> mRegularFileActionList;
    QList<QAction*> mTrashFileActionList;
    KService::List mServiceList;
    bool mInTrash;

    KUrl::List urlList() const
    {
        KUrl::List urlList;

        KFileItemList list = q->contextManager()->selectedFileItemList();
        if (list.count() > 0) {
            urlList = list.urlList();
        } else {
            KUrl url = q->contextManager()->currentUrl();
            Q_ASSERT(url.isValid());
            urlList << url;
        }
        return urlList;
    }

    void updateServiceList()
    {
        // This code is inspired from
        // kdebase/apps/lib/konq/konq_menuactions.cpp

        // Get list of all distinct mimetypes in selection
        QStringList mimeTypes;
        Q_FOREACH(const KFileItem & item, q->contextManager()->selectedFileItemList()) {
            const QString mimeType = item.mimetype();
            if (!mimeTypes.contains(mimeType)) {
                mimeTypes << mimeType;
            }
        }

        // Query trader
        mServiceList = KFileItemActions::associatedApplications(mimeTypes, QString());
    }

    QMimeData* selectionMimeData()
    {
        QMimeData* mimeData = new QMimeData;
        KFileItemList list = q->contextManager()->selectedFileItemList();
        list.urlList().populateMimeData(mimeData);
        return mimeData;
    }

    KUrl pasteTargetUrl() const
    {
        // If only one folder is selected, paste inside it, otherwise paste in
        // current
        KFileItemList list = q->contextManager()->selectedFileItemList();
        if (list.count() == 1 && list.first().isDir()) {
            return list.first().url();
        } else {
            return q->contextManager()->currentDirUrl();
        }
    }
};

static QAction* createSeparator(QObject* parent)
{
    QAction* action = new KAction(parent);
    action->setSeparator(true);
    return action;
}

FileOpsContextManagerItem::FileOpsContextManagerItem(ContextManager* manager, QListView* thumbnailView, KActionCollection* actionCollection, KXMLGUIClient* client)
: AbstractContextManagerItem(manager)
, d(new FileOpsContextManagerItemPrivate)
{
    d->q = this;
    d->mThumbnailView = thumbnailView;
    d->mXMLGUIClient = client;
    d->mGroup = new SideBarGroup(i18n("File Operations"));
    setWidget(d->mGroup);
    EventWatcher::install(d->mGroup, QEvent::Show, this, SLOT(updateSideBarContent()));

    d->mInTrash = false;

    connect(contextManager(), SIGNAL(selectionChanged()),
            SLOT(updateActions()));
    connect(contextManager(), SIGNAL(currentDirUrlChanged(KUrl)),
            SLOT(updateActions()));

    KActionCategory* file = new KActionCategory(i18nc("@title actions category", "File"), actionCollection);
    KActionCategory* edit = new KActionCategory(i18nc("@title actions category", "Edit"), actionCollection);

    d->mCutAction = edit->addAction(KStandardAction::Cut, this, SLOT(cut()));

    // Copied from Dolphin:
    // need to remove shift+del from cut action, else the shortcut for deletejob
    // doesn't work
    KShortcut cutShortcut = d->mCutAction->shortcut();
    cutShortcut.remove(Qt::SHIFT + Qt::Key_Delete, KShortcut::KeepEmpty);
    d->mCutAction->setShortcut(cutShortcut);

    d->mCopyAction = edit->addAction(KStandardAction::Copy, this, SLOT(copy()));
    d->mPasteAction = edit->addAction(KStandardAction::Paste, this, SLOT(paste()));

    d->mCopyToAction = file->addAction("file_copy_to", this, SLOT(copyTo()));
    d->mCopyToAction->setText(i18nc("Verb", "Copy To..."));
    d->mCopyToAction->setShortcut(Qt::Key_F7);

    d->mMoveToAction = file->addAction("file_move_to", this, SLOT(moveTo()));
    d->mMoveToAction->setText(i18nc("Verb", "Move To..."));
    d->mMoveToAction->setShortcut(Qt::Key_F8);

    d->mLinkToAction = file->addAction("file_link_to", this, SLOT(linkTo()));
    d->mLinkToAction->setText(i18nc("Verb: create link to the file where user wants", "Link To..."));
    d->mLinkToAction->setShortcut(Qt::Key_F9);

    d->mRenameAction = file->addAction("file_rename", this, SLOT(rename()));
    d->mRenameAction->setText(i18nc("Verb", "Rename..."));
    d->mRenameAction->setShortcut(Qt::Key_F2);

    d->mTrashAction = file->addAction("file_trash", this, SLOT(trash()));
    d->mTrashAction->setText(i18nc("Verb", "Trash"));
    d->mTrashAction->setIcon(KIcon("user-trash"));
    d->mTrashAction->setShortcut(Qt::Key_Delete);

    d->mDelAction = file->addAction("file_delete", this, SLOT(del()));
    d->mDelAction->setText(i18n("Delete"));
    d->mDelAction->setIcon(KIcon("edit-delete"));
    d->mDelAction->setShortcut(QKeySequence(Qt::ShiftModifier | Qt::Key_Delete));

    d->mRestoreAction = file->addAction("file_restore", this, SLOT(restore()));
    d->mRestoreAction->setText(i18n("Restore"));

    d->mShowPropertiesAction = file->addAction("file_show_properties", this, SLOT(showProperties()));
    d->mShowPropertiesAction->setText(i18n("Properties"));
    d->mShowPropertiesAction->setIcon(KIcon("document-properties"));

    d->mCreateFolderAction = file->addAction("file_create_folder", this, SLOT(createFolder()));
    d->mCreateFolderAction->setText(i18n("Create Folder..."));
    d->mCreateFolderAction->setIcon(KIcon("folder-new"));

    d->mOpenWithAction = file->addAction("file_open_with");
    d->mOpenWithAction->setText(i18n("Open With"));
    QMenu* menu = new QMenu;
    d->mOpenWithAction->setMenu(menu);
    connect(menu, SIGNAL(aboutToShow()),
            SLOT(populateOpenMenu()));
    connect(menu, SIGNAL(triggered(QAction*)),
            SLOT(openWith(QAction*)));

    d->mRegularFileActionList
            << d->mRenameAction
            << d->mTrashAction
            << d->mDelAction
            << createSeparator(this)
            << d->mCopyToAction
            << d->mMoveToAction
            << d->mLinkToAction
            << createSeparator(this)
            << d->mOpenWithAction
            << d->mShowPropertiesAction
            << createSeparator(this)
            << d->mCreateFolderAction
            ;

    d->mTrashFileActionList
            << d->mRestoreAction
            << d->mDelAction
            << createSeparator(this)
            << d->mShowPropertiesAction
            ;

    connect(QApplication::clipboard(), SIGNAL(dataChanged()),
            SLOT(updatePasteAction()));
    updatePasteAction();

    // Delay action update because it must happen *after* main window has called
    // createGUI(), otherwise calling d->mXMLGUIClient->plugActionList() will
    // fail.
    QMetaObject::invokeMethod(this, "updateActions", Qt::QueuedConnection);
}

FileOpsContextManagerItem::~FileOpsContextManagerItem()
{
    delete d->mOpenWithAction->menu();
    delete d;
}

void FileOpsContextManagerItem::updateActions()
{
    const int count = contextManager()->selectedFileItemList().count();
    const bool selectionNotEmpty = count > 0;
    const bool urlIsValid = contextManager()->currentUrl().isValid();
    const bool dirUrlIsValid = contextManager()->currentDirUrl().isValid();

    d->mInTrash = contextManager()->currentDirUrl().protocol() == "trash";

    d->mCutAction->setEnabled(selectionNotEmpty);
    d->mCopyAction->setEnabled(selectionNotEmpty);
    d->mCopyToAction->setEnabled(selectionNotEmpty);
    d->mMoveToAction->setEnabled(selectionNotEmpty);
    d->mLinkToAction->setEnabled(selectionNotEmpty);
    d->mTrashAction->setEnabled(selectionNotEmpty);
    d->mRestoreAction->setEnabled(selectionNotEmpty);
    d->mDelAction->setEnabled(selectionNotEmpty);
    d->mOpenWithAction->setEnabled(selectionNotEmpty);
    d->mRenameAction->setEnabled(count == 1);

    d->mCreateFolderAction->setEnabled(dirUrlIsValid);
    d->mShowPropertiesAction->setEnabled(dirUrlIsValid || urlIsValid);

    d->mXMLGUIClient->unplugActionList("file_action_list");
    QList<QAction*>& list = d->mInTrash ? d->mTrashFileActionList : d->mRegularFileActionList;
    d->mXMLGUIClient->plugActionList("file_action_list", list);

    updateSideBarContent();
}

void FileOpsContextManagerItem::updatePasteAction()
{
    QPair<bool, QString> info = KonqOperations::pasteInfo(d->pasteTargetUrl());
    d->mPasteAction->setEnabled(info.first);
    d->mPasteAction->setText(info.second);
}

void FileOpsContextManagerItem::updateSideBarContent()
{
    if (!d->mGroup->isVisible()) {
        return;
    }

    d->mGroup->clear();
    QList<QAction*>& list = d->mInTrash ? d->mTrashFileActionList : d->mRegularFileActionList;
    Q_FOREACH(QAction * action, list) {
        if (action->isEnabled() && !action->isSeparator()) {
            d->mGroup->addAction(action);
        }
    }
}

void FileOpsContextManagerItem::showProperties()
{
    KFileItemList list = contextManager()->selectedFileItemList();
    if (list.count() > 0) {
        KPropertiesDialog::showDialog(list, d->mGroup);
    } else {
        KUrl url = contextManager()->currentDirUrl();
        KPropertiesDialog::showDialog(url, d->mGroup);
    }
}

void FileOpsContextManagerItem::cut()
{
    QMimeData* mimeData = d->selectionMimeData();
    KonqMimeData::addIsCutSelection(mimeData, true);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileOpsContextManagerItem::copy()
{
    QMimeData* mimeData = d->selectionMimeData();
    KonqMimeData::addIsCutSelection(mimeData, false);
    QApplication::clipboard()->setMimeData(mimeData);
}

void FileOpsContextManagerItem::paste()
{
    const bool move = KonqMimeData::decodeIsCutSelection(QApplication::clipboard()->mimeData());
    KIO::pasteClipboard(d->pasteTargetUrl(), d->mGroup, move);
}

void FileOpsContextManagerItem::trash()
{
    FileOperations::trash(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::del()
{
    FileOperations::del(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::restore()
{
    KonqOperations::restoreTrashedItems(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::copyTo()
{
    FileOperations::copyTo(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::moveTo()
{
    FileOperations::moveTo(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::linkTo()
{
    FileOperations::linkTo(d->urlList(), d->mGroup);
}

void FileOpsContextManagerItem::rename()
{
    if (d->mThumbnailView->isVisible()) {
        QModelIndex index = d->mThumbnailView->currentIndex();
        d->mThumbnailView->edit(index);
    } else {
        FileOperations::rename(d->urlList().first(), d->mGroup);
    }
}

void FileOpsContextManagerItem::createFolder()
{
    KUrl url = contextManager()->currentDirUrl();
    KonqOperations::newDir(d->mGroup, url);
}

void FileOpsContextManagerItem::populateOpenMenu()
{
    QMenu* openMenu = d->mOpenWithAction->menu();
    qDeleteAll(openMenu->actions());

    d->updateServiceList();

    int idx = 0;
    Q_FOREACH(const KService::Ptr & service, d->mServiceList) {
        QString text = service->name().replace('&', "&&");
        QAction* action = openMenu->addAction(text);
        action->setIcon(KIcon(service->icon()));
        action->setData(idx);
        ++idx;
    }

    openMenu->addSeparator();
    QAction* action = openMenu->addAction(i18n("Other Application..."));
    action->setData(-1);
}

void FileOpsContextManagerItem::openWith(QAction* action)
{
    Q_ASSERT(action);
    KService::Ptr service;
    KUrl::List list = d->urlList();

    bool ok;
    int idx = action->data().toInt(&ok);
    GV_RETURN_IF_FAIL(ok);
    if (idx == -1) {
        // Other Application...
        KOpenWithDialog dlg(list, d->mGroup);
        if (!dlg.exec()) {
            return;
        }
        service = dlg.service();

        if (!service) {
            // User entered a custom command
            Q_ASSERT(!dlg.text().isEmpty());
            KRun::run(dlg.text(), list, d->mGroup);
            return;
        }
    } else {
        service = d->mServiceList.at(idx);
    }

    Q_ASSERT(service);
    KRun::run(*service, list, d->mGroup);
}

} // namespace
