/*
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pppoewidget.h"
#include "ui_pppoe.h"

#include <NetworkManagerQt/PppoeSetting>

PppoeWidget::PppoeWidget(const NetworkManager::Setting::Ptr &setting, QWidget* parent, Qt::WindowFlags f):
    SettingWidget(setting, parent, f),
    m_ui(new Ui::PppoeWidget)
{
    m_ui->setupUi(this);

    connect(m_ui->cbShowPassword, SIGNAL(toggled(bool)), SLOT(showPassword(bool)));
    connect(m_ui->username, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()));

    KAcceleratorManager::manage(this);

    if (setting)
        loadConfig(setting);
}

PppoeWidget::~PppoeWidget()
{
    delete m_ui;
}

void PppoeWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::PppoeSetting::Ptr pppoeSetting = setting.staticCast<NetworkManager::PppoeSetting>();

    m_ui->service->setText(pppoeSetting->service());
    m_ui->username->setText(pppoeSetting->username());
    m_ui->password->setText(pppoeSetting->password());
}

QVariantMap PppoeWidget::setting(bool agentOwned) const
{
    Q_UNUSED(agentOwned);

    NetworkManager::PppoeSetting pppoeSetting;
    if (!m_ui->service->text().isEmpty())
        pppoeSetting.setService(m_ui->service->text());
    if (!m_ui->username->text().isEmpty())
        pppoeSetting.setUsername(m_ui->username->text());
    if (!m_ui->password->text().isEmpty())
        pppoeSetting.setPassword(m_ui->password->text());

    if (agentOwned) {
        pppoeSetting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
    }

    return pppoeSetting.toMap();
}

void PppoeWidget::showPassword(bool show)
{
    m_ui->password->setPasswordMode(!show);
}

bool PppoeWidget::isValid() const
{
    return !m_ui->username->text().isEmpty();
}
