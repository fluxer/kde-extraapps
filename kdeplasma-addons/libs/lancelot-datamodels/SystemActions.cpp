/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "SystemActions.h"
#include "SystemActions_p.h"

#include <QDBusInterface>

#include <KRun>
#include <KIcon>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KAuthorized>
#include <KMessageBox>
#include <KJob>

#include <Plasma/AbstractRunner>

#include <kworkspace/kworkspace.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCall>
#include <solid/powermanagement.h>

#include "screensaver_interface.h"

#define ID_MENU_LEAVE       "menu-leave"
#define ID_MENU_SWITCH_USER "menu-switch-user"

#define ID_LOCK_SCREEN  "lock-screen"
#define ID_LOGOUT       "leave-logout"
#define ID_REBOOT       "leave-reboot"
#define ID_POWEROFF     "leave-poweroff"
#define ID_SUSPEND_DISK "suspend-disk"
#define ID_SUSPEND_RAM  "suspend-ram"

namespace Lancelot {
namespace Models {

SystemActions * SystemActions::Private::instance = NULL;

SystemActions::Private::Private(SystemActions * parent)
    : delayedActivateItemIndex(-1),
      switchUserModel(NULL),
      q(parent)
{
}

SystemActions::SystemActions()
    : StandardActionTreeModel(NULL),
      d(new Private(this))
{
}

SystemActions::SystemActions(Item * r)
    : StandardActionTreeModel(r),
      d(new Private(this))
{
}

SystemActions::~SystemActions()
{
    delete d;
}

SystemActions * SystemActions::self()
{
    if (!SystemActions::Private::instance) {
        SystemActions::Private::instance = new SystemActions();
        SystemActions::Private::instance->load();
    }
    return SystemActions::Private::instance;
}

Lancelot::ActionTreeModel * SystemActions::action(const QString & id, bool execute)
{
    QList < Lancelot::ActionTreeModel * > subs;
    subs << this;

    while (!subs.isEmpty()) {
        Lancelot::StandardActionTreeModel * model =
            dynamic_cast < Lancelot::StandardActionTreeModel * >
                (subs.takeFirst());
        if (!model) {
            continue;
        }

        for (int i = 0; i < model->size(); i++) {
            if (id == model->data(i).toString()) {
                if (model->isCategory(i)) {
                    return model->child(i);
                }

                if (execute) {
                    model->activated(i);
                }

                return NULL;
            } else {
                if (model->isCategory(i)) {
                    subs << model->child(i);
                }
            }
        }
    }
    return NULL;
}

QStringList SystemActions::actions() const
{
    QStringList result;
    result << ID_MENU_LEAVE
           << ID_MENU_SWITCH_USER
           << ID_LOCK_SCREEN
           << ID_LOGOUT
           << ID_REBOOT
           << ID_POWEROFF;

    QSet <Solid::PowerManagement::SleepState> sleepstates = Solid::PowerManagement::supportedSleepStates();
    foreach (const Solid::PowerManagement::SleepState &sleepstate, sleepstates) {
        if (sleepstate == Solid::PowerManagement::SuspendState) {
            result << ID_SUSPEND_RAM;
        } else if (sleepstate == Solid::PowerManagement::HibernateState) {
            result << ID_SUSPEND_DISK;
        }
    }
    return result;
}

QString SystemActions::actionTitle(const QString & id) const
{
    if (id == ID_MENU_LEAVE) {
        return i18n("&Leave");
    } else if (id == ID_MENU_SWITCH_USER) {
        return i18n("Switch &User");
    } else if (id == ID_LOCK_SCREEN) {
        return i18n("Loc&k Session");
    } else if (id == ID_LOGOUT) {
        return i18n("Log &Out");
    } else if (id == ID_REBOOT) {
        return i18n("Re&boot");
    } else if (id == ID_POWEROFF) {
        return i18n("&Shut Down");
    } else if (id == ID_SUSPEND_DISK) {
        return i18n("Suspend to D&isk");
    } else if (id == ID_SUSPEND_RAM) {
        return i18n("Suspend to &RAM");
    }

    return QString();
}

QIcon SystemActions::actionIcon(const QString & id) const
{
    if (id == ID_MENU_LEAVE) {
        return KIcon("system-shutdown");
    } else if (id == ID_MENU_SWITCH_USER) {
        return KIcon("system-switch-user");
    } else if (id == ID_LOCK_SCREEN) {
        return KIcon("system-lock-screen");
    } else if (id == ID_LOGOUT) {
        return KIcon("system-log-out");
    } else if (id == ID_REBOOT) {
        return KIcon("system-reboot");
    } else if (id == ID_POWEROFF) {
        return KIcon("system-shutdown");
    } else if (id == ID_SUSPEND_DISK) {
        return KIcon("system-suspend-hibernate");
    } else if (id == ID_SUSPEND_RAM) {
        return KIcon("system-suspend");
    }

    return QIcon();
}

void SystemActions::load()
{
#define ItemFor( ID ) \
    StandardActionTreeModel::Item ( \
        actionTitle( ID ), QString(), actionIcon( ID ), ID)

    add(new ItemFor(ID_LOCK_SCREEN));

    StandardActionTreeModel::Item * item;

    QStringList acts = actions();

    item = new ItemFor(ID_MENU_LEAVE);
    item->children << new ItemFor(ID_LOGOUT);
    item->children << new ItemFor(ID_REBOOT);
    item->children << new ItemFor(ID_POWEROFF);
    if (acts.contains(ID_SUSPEND_DISK)) {
        item->children << new ItemFor(ID_SUSPEND_DISK);
    }
    if (acts.contains(ID_SUSPEND_RAM)) {
        item->children << new ItemFor(ID_SUSPEND_RAM);
    }
    add(item);

    d->switchUserModel = new Lancelot::ActionTreeModelProxy(
            new Sessions()
            );
    add(new ItemFor(ID_MENU_SWITCH_USER));
#undef ItemFor

    emit updated();
}

Lancelot::StandardActionTreeModel * SystemActions::createChild(int index)
{
    Item * childItem = root()->children.value(index);
    Lancelot::StandardActionTreeModel * model =
            new SystemActions(childItem);
    return model;
}

bool SystemActions::isCategory(int index) const
{
    if (index < 0 || index >= root()->children.size()) {
        return false;
    }

    if (root()->children.at(index)->data.toString()
            == ID_MENU_SWITCH_USER) {
        return true;
    }

    return Lancelot::StandardActionTreeModel::isCategory(index);
}

Lancelot::ActionTreeModel * SystemActions::child(int index)
{
    if (index < 0 || index >= root()->children.size()) {
        return NULL;
    }

    if (root()->children.at(index)->data.toString()
            == ID_MENU_SWITCH_USER) {
        return d->switchUserModel;
    }

    return Lancelot::StandardActionTreeModel::child(index);
}

void SystemActions::activate(int index)
{
    if (index < 0 || index >= root()->children.size()) {
        d->delayedActivateItemIndex = -1;
        return;
    }

    d->delayedActivateItemIndex = index;
    QTimer::singleShot(0, d, SLOT(delayedActivate()));
}

void SystemActions::Private::delayedActivate()
{
    if (delayedActivateItemIndex < 0) {
        return;
    }

    QString cmd = q->root()->children.at(delayedActivateItemIndex)->data.toString();

    if (cmd == ID_SUSPEND_DISK || cmd == ID_SUSPEND_RAM) {

        if (cmd == ID_SUSPEND_DISK) {
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, 0, 0);
        } else {
            Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, 0, 0);
        }

        ApplicationConnector::self()->hide(true);
        return;

    } else if (cmd == ID_LOCK_SCREEN) {
        org::freedesktop::ScreenSaver screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", QDBusConnection::sessionBus());

        if (screensaver.isValid()) {
            ApplicationConnector::self()->hide(true);
            screensaver.Lock();
        } else {
            KMessageBox::error(
                    0,
                    i18n("<p>Lancelot can not lock your screen at the moment."
                         "</p>"),
                    i18n("Session locking error"));
        }

        return;
    }

    KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
    KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeDefault; // abuse as "nothing"

    // KWorkSpace related

    if (cmd == ID_LOGOUT) {
        type = KWorkSpace::ShutdownTypeNone;
    } else if (cmd == ID_REBOOT) {
        type = KWorkSpace::ShutdownTypeReboot;
    } else if (cmd == ID_POWEROFF) {
        type = KWorkSpace::ShutdownTypeHalt;
    }

    if (type != KWorkSpace::ShutdownTypeDefault) {
        ApplicationConnector::self()->hide(true);
        KWorkSpace::requestShutDown(confirm, type);
        return;
    }
}

// Sessions
Sessions::Sessions()
    : BaseModel(), d(new Private())
{
    load();
}

Sessions::~Sessions()
{
    delete d;
}

void Sessions::load()
{
    if (KAuthorized::authorizeKAction("start_new_session") &&
        d->dm.isSwitchable() &&
        d->dm.numReserve() >= 0) {
        add(
            i18n("New Session"), "",
            KIcon("system-switch-user"), ID_MENU_SWITCH_USER);
    }

    SessList sessions;
    d->dm.localSessions(sessions);

    foreach (const SessEnt& session, sessions) {
        if (session.self) {
            continue;
        }

        QString name = KDisplayManager::sess2Str(session);

        add(
            name, QString(),
            KIcon(session.vt ? "utilities-terminal" : "user-identity"),
            name//session.session
        );
    }

    if (size() == 0) {
        add(
                i18n("Display manager error"), QString(),
                KIcon("dialog-warning"),
                QString("display-manager-error")
           );
    }
}

void Sessions::activate(int index)
{
    QString data = itemAt(index).data.toString();

    if (data.isEmpty()) {
        return;
    }

    hideApplicationWindow();

    if (data == "display-manager-error") {
        KMessageBox::error(
                0,
                i18n("<p>Lancelot can not find your display manager. "
                     "This means that it not able to retrieve the list of currently "
                     "running sessions, or start a new one.</p>"),
                i18n("Display manager error"));
    } else if (data == ID_MENU_SWITCH_USER) {
        //TODO: update this message when it changes
        // in sessionrunner
        int result = KMessageBox::warningContinueCancel(
                0,
                i18n("<p>You have chosen to open another desktop session.<br />"
                    "The current session will be hidden "
                    "and a new login screen will be displayed.<br />"
                    "An F-key is assigned to each session; "
                    "F%1 is usually assigned to the first session, "
                    "F%2 to the second session and so on. "
                    "You can switch between sessions by pressing "
                    "Ctrl, Alt and the appropriate F-key at the same time. "
                    "Additionally, the KDE Panel and Desktop menus have "
                    "actions for switching between sessions.</p>",
                    7, 8),
                i18n("Warning - New Session"),
                KGuiItem(i18n("&Start New Session"), "fork"),
                KStandardGuiItem::cancel(),
                ":confirmNewSession",
                KMessageBox::PlainCaption | KMessageBox::Notify);

        if (result == KMessageBox::Cancel) {
            return;
        }

        QDBusInterface screensaver("org.freedesktop.ScreenSaver",
                "/ScreenSaver", "org.freedesktop.ScreenSaver");
        screensaver.call( "Lock" );

        d->dm.startReserve();
        return;
    }

    SessList sessions;
    if (d->dm.localSessions(sessions)) {
        foreach (const SessEnt &session, sessions) {
            if (data == KDisplayManager::sess2Str(session)) {
                d->dm.lockSwitchVT(session.vt);
                break;
            }
        }
    }

    hideApplicationWindow();
}

} // namespace Models
} // namespace Lancelot
