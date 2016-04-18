/***************************************************************************
 *   Copyright (C) 2005-2014 by the Quassel Project                        *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This file is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU Library General Public License (LGPL)   *
 *   as published by the Free Software Foundation; either version 2 of the *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QApplication>
#include <QMenu>

#include "systemtray.h"

#include "action.h"
#include "actioncollection.h"
#include "client.h"
#include "qtui.h"

#include <KIcon>
#include <KMenu>
#include <KWindowInfo>
#include <KWindowSystem>

SystemTray::SystemTray(QWidget *parent)
    : QObject(parent),
    _mode(Invalid),
    _state(Passive),
    _shouldBeVisible(true),
    // XXX: re-use kuassel icon with a state?
    _passiveIcon(KIcon("kuassel-inactive")),
    _activeIcon(KIcon("kuassel")),
    _needsAttentionIcon(KIcon("message")),
    _trayMenu(0),
    _associatedWidget(parent)
{
    Q_ASSERT(parent);
}


SystemTray::~SystemTray()
{
    _trayMenu->deleteLater();
}


QWidget *SystemTray::associatedWidget() const
{
    return _associatedWidget;
}


void SystemTray::init()
{
    ActionCollection *coll = QtUi::actionCollection("General");
    _minimizeRestoreAction = new Action(i18n("&Minimize"), this, this, SLOT(minimizeRestore()));

    KMenu *kmenu;
    _trayMenu = kmenu = new KMenu();
    kmenu->addTitle(_activeIcon, "Kuassel");

    _trayMenu->setTitle("Kuassel");


    _trayMenu->addAction(coll->action("ConnectCore"));
    _trayMenu->addAction(coll->action("DisconnectCore"));
    _trayMenu->addAction(coll->action("CoreInfo"));
    _trayMenu->addSeparator();
    _trayMenu->addAction(_minimizeRestoreAction);
    _trayMenu->addAction(coll->action("Quit"));

    connect(_trayMenu, SIGNAL(aboutToShow()), SLOT(trayMenuAboutToShow()));

    NotificationSettings notificationSettings;
    notificationSettings.initAndNotify("Systray/Animate", this, SLOT(enableAnimationChanged(QVariant)), true);
}


void SystemTray::trayMenuAboutToShow()
{
    if (GraphicalUi::isMainWidgetVisible())
        _minimizeRestoreAction->setText(i18n("&Minimize"));
    else
        _minimizeRestoreAction->setText(i18n("&Restore"));
}


void SystemTray::setMode(Mode mode_)
{
    if (mode_ != _mode) {
        _mode = mode_;
        if (_trayMenu) {
            if (_mode == Legacy) {
                _trayMenu->setWindowFlags(Qt::Popup);
            }
            else {
                _trayMenu->setWindowFlags(Qt::Window);
            }
        }
    }
}


QIcon SystemTray::stateIcon() const
{
    return stateIcon(state());
}


QIcon SystemTray::stateIcon(State state) const
{
    switch (state) {
    case Passive:
        return _passiveIcon;
    case Active:
        return _activeIcon;
    case NeedsAttention:
        return _needsAttentionIcon;
    }
    return QIcon();
}


void SystemTray::setState(State state)
{
    if (_state != state) {
        _state = state;
    }
}


void SystemTray::setAlert(bool alerted)
{
    if (alerted)
        setState(NeedsAttention);
    else
        setState(Client::isConnected() ? Active : Passive);
}


void SystemTray::setVisible(bool visible)
{
    _shouldBeVisible = visible;
}


void SystemTray::setToolTip(const QString &title, const QString &subtitle)
{
    _toolTipTitle = title;
    _toolTipSubTitle = subtitle;
    emit toolTipChanged(title, subtitle);
}


void SystemTray::showMessage(const QString &title, const QString &message, MessageIcon icon, int millisecondsTimeoutHint, uint id)
{
    Q_UNUSED(title)
    Q_UNUSED(message)
    Q_UNUSED(icon)
    Q_UNUSED(millisecondsTimeoutHint)
    Q_UNUSED(id)
}


void SystemTray::activate(SystemTray::ActivationReason reason)
{
    emit activated(reason);
}


void SystemTray::minimizeRestore()
{
    GraphicalUi::toggleMainWidget();
}


void SystemTray::enableAnimationChanged(const QVariant &v)
{
    _animationEnabled = v.toBool();
    emit animationEnabledChanged(v.toBool());
}
