/*
 * Copyright (C) 2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * Copyright (C) 2011 UFO Coders <info@ufocoders.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sendfileitemaction.h"

#include <QList>
#include <QMenu>
#include <QAction>
#include <QWidget>
#include <QVariantList>

#include <KIcon>
#include <KPluginFactory>
#include <KPluginLoader>

#include <KDebug>
#include <KProcess>
#include <KLocalizedString>

K_PLUGIN_FACTORY(SendFileItemActionFactory, registerPlugin<SendFileItemAction>();)
K_EXPORT_PLUGIN(SendFileItemActionFactory("SendFileItemAction", "bluedevil"))

SendFileItemAction::SendFileItemAction(QObject *parent, const QVariantList &args)
    : KFileItemActionPlugin(parent)
{
    Q_UNUSED(args)

    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<QMapDeviceInfo>();

    m_kded = new org::kde::BlueDevil("org.kde.kded", "/modules/bluedevil",
                                     QDBusConnection::sessionBus(), this);
}

QList<QAction*> SendFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget) const
{
    Q_UNUSED(parentWidget)

    QList<QAction*> list;

    // Don't show the action for files that we can't send or when Bluetooth is offline.
    if (!fileItemInfos.isLocal() || !m_kded->isOnline()) {
        return list;
    }

    SendFileItemAction *hack = const_cast<SendFileItemAction*>(this);
    hack->m_fileItemInfos = fileItemInfos;

    QAction *menuAction = new QAction(KIcon("preferences-system-bluetooth"), i18n("Send via Bluetooth"), hack);
    QMenu *menu = new QMenu();

    const QMapDeviceInfo &devices = m_kded->allDevices().value();
    Q_FOREACH (const DeviceInfo &device, devices) {
        if (device["UUIDs"].contains("00001105-0000-1000-8000-00805F9B34FB")) {
            QAction *action = new QAction(KIcon(device["icon"]), device["name"], hack);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(deviceTriggered()));
            action->setData(device["UBI"]);
            menu->addAction(action);
        }
    }

    QAction *otherAction = new QAction(hack);
    connect(otherAction, SIGNAL(triggered(bool)), this, SLOT(otherTriggered()));
    if (menu->actions().isEmpty()) {
        otherAction->setText(i18nc("Find Bluetooth device", "Find Device..."));
    } else {
        menu->addSeparator();
        otherAction->setText(i18nc("Other Bluetooth device", "Other..."));
    }
    menu->addAction(otherAction);

    menuAction->setMenu(menu);
    list << menuAction;
    return list;
}

void SendFileItemAction::deviceTriggered()
{
    QStringList args;
    args.append("-u" + static_cast<QAction *>(sender())->data().toString());

    KUrl::List fileList =  m_fileItemInfos.urlList();
    Q_FOREACH(const KUrl &url,  fileList) {
        args.append("-f" + url.path());
    }
    kDebug() << args;
    KProcess process;
    process.setProgram("bluedevil-sendfile", args);
    process.startDetached();
}

void SendFileItemAction::otherTriggered()
{
    kDebug();
    QStringList args;

    KUrl::List fileList =  m_fileItemInfos.urlList();
    Q_FOREACH(const KUrl &url,  fileList) {
        args.append("-f" + url.path());
    }

    KProcess process;
    process.setProgram("bluedevil-sendfile", args);
    process.startDetached();
}
