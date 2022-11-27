/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>                 *
 *   Copyright (C) 2009 by Matthias Fuchs <mat69@gmx.net>                  *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kgetappletutils.h"

#include <plasma/svg.h>
#include <plasma/theme.h>
#include <plasma/widgets/label.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/pushbutton.h>

#include <QtDBus/QDBusConnectionInterface>
#include <QGraphicsLinearLayout>
#include <QPainter>
#include <QRect>
#include <QVBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QTimer>

#include <KIcon>
#include <KLocale>
#include <KPushButton>

/** Error widget **/

ErrorWidget::ErrorWidget(const QString &message, QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent)
{
    m_interface = QDBusConnection::sessionBus().interface();

    m_layout = new QGraphicsLinearLayout(this);
    m_layout->setOrientation(Qt::Vertical);

    m_errorLabel = new Plasma::Label(this);
    m_errorLabel->setText(message);
    m_errorLabel->nativeWidget()->setAlignment(Qt::AlignCenter);

    m_icon = new Plasma::IconWidget(KIcon("dialog-warning"),"", this);

    m_launchButton = new Plasma::PushButton(this);
    m_launchButton->setText(i18n("Launch KGet"));
    m_launchButton->nativeWidget()->setIcon(KIcon("kget"));

    m_layout->addItem(m_errorLabel);
    m_layout->addItem(m_icon);
    m_layout->addItem(m_launchButton);

    setLayout(m_layout);

    connect(m_launchButton, SIGNAL(clicked()), SLOT(launchKGet()));
}

ErrorWidget::~ErrorWidget()
{
    delete m_errorLabel;
    delete m_icon;
    delete m_launchButton;
}

void ErrorWidget::launchKGet()
{
    QProcess::startDetached("kget");
    checkKGetStatus();
}

void ErrorWidget::checkKGetStatus()
{
    if (m_interface->isServiceRegistered("org.kde.kget")) {
        emit kgetStarted();
    } else {
        QTimer::singleShot(1000, this, SLOT(checkKGetStatus()));
    }
}

#include "moc_kgetappletutils.cpp"
