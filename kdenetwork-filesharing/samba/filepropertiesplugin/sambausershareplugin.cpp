/*
  Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>
  Copyright (c) 2011 Rodrigo Belem <rclbelem@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QFileInfo>
#include <QStringList>
#include <QStandardItemModel>
#include <QDBusInterface>
#include <QDBusReply>

#include <kvbox.h>
#include <kuser.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <ksambashare.h>
#include <ksambasharedata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <KPluginFactory>
#include <KPluginLoader>

#include "sambausershareplugin.h"
#include "model.h"
#include "delegate.h"

K_PLUGIN_FACTORY(SambaUserSharePluginFactory, registerPlugin<SambaUserSharePlugin>();)
K_EXPORT_PLUGIN(SambaUserSharePluginFactory("fileshare_propsdlgplugin"))

SambaUserSharePlugin::SambaUserSharePlugin(QObject *parent, const QList<QVariant> &args)
    : KPropertiesDialogPlugin(qobject_cast<KPropertiesDialog *>(parent))
    , url()
    , shareData()
{
    url = properties->kurl().path(KUrl::RemoveTrailingSlash);
    if (url.isEmpty()) {
        return;
    }

    QFileInfo pathInfo(url);
    if (!pathInfo.permission(QFile::ReadUser | QFile::WriteUser)) {
        return;
    }

    KGlobal::locale()->insertCatalog("kfileshare");

    KVBox *vbox = new KVBox();
    properties->addPage(vbox, i18n("&Share"));
    properties->setFileSharingPage(vbox);

    // smbd is usually found as /usr/sbin/smbd
    const QString exePath = "/usr/sbin:/usr/bin:/sbin:/bin";
    if (KStandardDirs::findExe("smbd", exePath).isEmpty()) {

        QWidget *widget = new QWidget(vbox);
        QVBoxLayout *vLayout = new QVBoxLayout(widget);
        vLayout->setAlignment(Qt::AlignJustify);
        vLayout->setSpacing(KDialog::spacingHint());
        vLayout->setMargin(0);

        vLayout->addWidget(new QLabel(i18n("Samba is not installed on your system."), widget));

        // align items on top
        vLayout->addStretch();

        return;
    }

    QWidget *widget = new QWidget(vbox);
    propertiesUi.setupUi(widget);

    QList<KSambaShareData> shareList = KSambaShare::instance()->getSharesByPath(url);

    if (!shareList.isEmpty()) {
        shareData = shareList.at(0); // FIXME: using just the first in the list for a while
    }

    setupModel();
    setupViews();
    load();

    connect(propertiesUi.sambaChk, SIGNAL(toggled(bool)), this, SLOT(toggleShareStatus(bool)));
    connect(propertiesUi.sambaChk, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(propertiesUi.sambaNameEdit, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
    connect(propertiesUi.sambaNameEdit, SIGNAL(textChanged(QString)), this, SLOT(checkShareName(QString)));
    connect(propertiesUi.sambaAllowGuestChk, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(changed()));

    for (int i = 0; i < model->rowCount(); ++i) {
        propertiesUi.tableView->openPersistentEditor(model->index(i, 1, QModelIndex()));
    }
}

SambaUserSharePlugin::~SambaUserSharePlugin()
{
}

void SambaUserSharePlugin::setupModel()
{
    model = new UserPermissionModel(shareData, this);
}

void SambaUserSharePlugin::setupViews()
{
    propertiesUi.tableView->setModel(model);
    propertiesUi.tableView->setSelectionMode(QAbstractItemView::NoSelection);
    propertiesUi.tableView->setItemDelegate(new UserPermissionDelegate(this));
}

void SambaUserSharePlugin::load()
{
    bool guestAllowed = false;
    bool sambaShared = KSambaShare::instance()->isDirectoryShared(url);

    propertiesUi.sambaChk->setChecked(sambaShared);
    toggleShareStatus(sambaShared);
    if (sambaShared) {
        guestAllowed = (bool) shareData.guestPermission();
    }
    propertiesUi.sambaAllowGuestChk->setChecked(guestAllowed);

    propertiesUi.sambaNameEdit->setText(shareData.name());
}

void SambaUserSharePlugin::applyChanges()
{
    KSambaShareData::UserShareError result;

    if (propertiesUi.sambaChk->isChecked()) {
        if (shareData.setAcl(model->getAcl()) != KSambaShareData::UserShareAclOk) {
            return;
        }

        shareData.setName(propertiesUi.sambaNameEdit->text());

        shareData.setPath(url);

        KSambaShareData::GuestPermission guestOk(shareData.guestPermission());

        guestOk = (propertiesUi.sambaAllowGuestChk->isChecked() == false)
                  ? KSambaShareData::GuestsNotAllowed : KSambaShareData::GuestsAllowed;

        shareData.setGuestPermission(guestOk);

        result = shareData.save();
    } else if (KSambaShare::instance()->isDirectoryShared(url)) {
        result = shareData.remove();
    }

#warning "the error reporting could use some love"
    // Some error types that do not apply but may in the future:
    // UserSharePathNotDirectory
    if (result == KSambaShareData::UserShareExceedMaxShares) {
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Maximum shared exceeded.</qt>"));
    } else if (result == KSambaShareData::UserShareNameInvalid) {
        // Is that even supposed to happend?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The username for sharing is invalid.</qt>"));
    } else if (result == KSambaShareData::UserShareNameInUse) {
        // Is that even supposed to happend?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The username for sharing is already in use.</qt>"));
    } else if (result == KSambaShareData::UserSharePathInvalid) {
        // Is that even supposed to happend?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The path chosen is invalid.</qt>"));
    } else if (result == KSambaShareData::UserSharePathNotExists) {
        // The path can be gone all of a sudden
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The path chosen does not exists.</qt>"));
    } else if (result == KSambaShareData::UserSharePathNotAbsolute) {
        // Is that even supposed to happend?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The path chosen is not absolute.</qt>"));
    } else if (result == KSambaShareData::UserSharePathNotAllowed) {
        // Not allowed, ok
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>The path chosen is not allowed.</qt>"));
    } else if (result == KSambaShareData::UserShareAclInvalid) {
        // Invalid ACL?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Invalid Advanced Control List (ACL).</qt>"));
    } else if (result == KSambaShareData::UserShareAclUserNotValid) {
        // Invalid ACL?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Invalid user in Advanced Control List (ACL).</qt>"));
    } else if (result == KSambaShareData::UserShareGuestsInvalid) {
        // Invalid ACL?
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Invalid guest user.</qt>"));
    } else if (result == KSambaShareData::UserShareGuestsNotAllowed) {
        // Guest not allowed, ok
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Guest user not allowed.</qt>"));
    } else if (result == KSambaShareData::UserShareSystemError) {
        // Something strange happened, Bob
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>System error.</qt>"));
    } else if (!result == KSambaShareData::UserShareOk) {
        // Something strange happened, Bob
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
            i18n("<qt>Unexpected error occurred.</qt>"));
    }
}

void SambaUserSharePlugin::toggleShareStatus(bool checked)
{
    propertiesUi.sambaNameEdit->setEnabled(checked);
    propertiesUi.sambaAllowGuestChk->setEnabled(checked);
    propertiesUi.tableView->setEnabled(checked);
    if (checked && propertiesUi.sambaNameEdit->text().isEmpty()) {
        propertiesUi.sambaNameEdit->setText(getNewShareName());
    }
}

void SambaUserSharePlugin::checkShareName(const QString &name)
{
    bool disableButton = false;

    if (name.isEmpty()) {
        disableButton = true;
    } else if (!KSambaShare::instance()->isShareNameAvailable(name)) {
        // There is another Share with the same name
        KMessageBox::sorry(qobject_cast<KPropertiesDialog *>(this),
                i18n("<qt>There is already a share with the name <strong>%1</strong>.<br /> Please choose another name.</qt>",
                    propertiesUi.sambaNameEdit->text()));
        propertiesUi.sambaNameEdit->selectAll();
        disableButton = true;
    }

    if (disableButton) {
        properties->enableButtonOk(false);
        propertiesUi.sambaNameEdit->setFocus();
        return;
    }

    if (!properties->isButtonEnabled(KPropertiesDialog::Ok)) {
        properties->enableButtonOk(true);
    }
}

QString SambaUserSharePlugin::getNewShareName()
{
    QString shareName = KUrl(url).fileName();

    if (!propertiesUi.sambaNameEdit->text().isEmpty()) {
        shareName = propertiesUi.sambaNameEdit->text();
    }

    // Windows could have problems with longer names
    shareName = shareName.left(12);

    return shareName;
}

#include "moc_sambausershareplugin.cpp"
