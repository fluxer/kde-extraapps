/***************************************************************************
 *   Copyright (C) 2005-2014 by the Quassel Project                        *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
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

#include "appearancesettingspage.h"

#include "buffersettings.h"
#include "qtui.h"
#include "qtuisettings.h"
#include "qtuistyle.h"

#include <KLocale>

#include <QCheckBox>
#include <QFileDialog>
#include <QStyleFactory>
#include <QFile>
#include <QDir>

AppearanceSettingsPage::AppearanceSettingsPage(QWidget *parent)
    : SettingsPage(i18n("Interface"), QString(), parent)
{
    ui.setupUi(this);

#ifdef Q_OS_MAC
    ui.minimizeOnClose->hide();
#endif
#ifdef QT_NO_SYSTEMTRAYICON
    ui.useSystemTrayIcon->hide();
#endif

    initAutoWidgets();
    initStyleComboBox();

    foreach(QComboBox *comboBox, findChildren<QComboBox *>()) {
        connect(comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(widgetHasChanged()));
    }
    foreach(QCheckBox *checkBox, findChildren<QCheckBox *>()) {
        connect(checkBox, SIGNAL(clicked()), this, SLOT(widgetHasChanged()));
    }

    connect(ui.chooseStyleSheet, SIGNAL(clicked()), SLOT(chooseStyleSheet()));

    connect(ui.userNoticesInDefaultBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.userNoticesInStatusBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.userNoticesInCurrentBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));

    connect(ui.serverNoticesInDefaultBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.serverNoticesInStatusBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.serverNoticesInCurrentBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));

    connect(ui.errorMsgsInDefaultBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.errorMsgsInStatusBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
    connect(ui.errorMsgsInCurrentBuffer, SIGNAL(clicked(bool)), this, SLOT(widgetHasChanged()));
}


void AppearanceSettingsPage::initStyleComboBox()
{
    QStringList styleList = QStyleFactory::keys();
    ui.styleComboBox->addItem(i18n("<System Default>"));
    foreach(QString style, styleList) {
        ui.styleComboBox->addItem(style);
    }
}


void AppearanceSettingsPage::defaults()
{
    ui.styleComboBox->setCurrentIndex(0);

    SettingsPage::defaults();
    widgetHasChanged();
}


void AppearanceSettingsPage::load()
{
    QtUiSettings uiSettings;

    // Gui Style
    QString style = uiSettings.value("Style", QString("")).toString();
    if (style.isEmpty()) {
        ui.styleComboBox->setCurrentIndex(0);
    }
    else {
        ui.styleComboBox->setCurrentIndex(ui.styleComboBox->findText(style, Qt::MatchExactly));
    }
    ui.styleComboBox->setProperty("storedValue", ui.styleComboBox->currentIndex());

    // bufferSettings:
    BufferSettings bufferSettings;
    int redirectTarget = bufferSettings.userNoticesTarget();
    SettingsPage::load(ui.userNoticesInDefaultBuffer, redirectTarget & BufferSettings::DefaultBuffer);
    SettingsPage::load(ui.userNoticesInStatusBuffer, redirectTarget & BufferSettings::StatusBuffer);
    SettingsPage::load(ui.userNoticesInCurrentBuffer, redirectTarget & BufferSettings::CurrentBuffer);

    redirectTarget = bufferSettings.serverNoticesTarget();
    SettingsPage::load(ui.serverNoticesInDefaultBuffer, redirectTarget & BufferSettings::DefaultBuffer);
    SettingsPage::load(ui.serverNoticesInStatusBuffer, redirectTarget & BufferSettings::StatusBuffer);
    SettingsPage::load(ui.serverNoticesInCurrentBuffer, redirectTarget & BufferSettings::CurrentBuffer);

    redirectTarget = bufferSettings.errorMsgsTarget();
    SettingsPage::load(ui.errorMsgsInDefaultBuffer, redirectTarget & BufferSettings::DefaultBuffer);
    SettingsPage::load(ui.errorMsgsInStatusBuffer, redirectTarget & BufferSettings::StatusBuffer);
    SettingsPage::load(ui.errorMsgsInCurrentBuffer, redirectTarget & BufferSettings::CurrentBuffer);

    SettingsPage::load();
    setChangedState(false);
}


void AppearanceSettingsPage::save()
{
    QtUiSettings uiSettings;

    if (ui.styleComboBox->currentIndex() < 1) {
        uiSettings.setValue("Style", QString(""));
    }
    else {
        uiSettings.setValue("Style", ui.styleComboBox->currentText());
        QApplication::setStyle(ui.styleComboBox->currentText());
    }
    ui.styleComboBox->setProperty("storedValue", ui.styleComboBox->currentIndex());

    bool needsStyleReload =
        ui.useCustomStyleSheet->isChecked() != ui.useCustomStyleSheet->property("storedValue").toBool()
        || (ui.useCustomStyleSheet->isChecked() && ui.customStyleSheetPath->text() != ui.customStyleSheetPath->property("storedValue").toString());

    BufferSettings bufferSettings;
    int redirectTarget = 0;
    if (ui.userNoticesInDefaultBuffer->isChecked())
        redirectTarget |= BufferSettings::DefaultBuffer;
    if (ui.userNoticesInStatusBuffer->isChecked())
        redirectTarget |= BufferSettings::StatusBuffer;
    if (ui.userNoticesInCurrentBuffer->isChecked())
        redirectTarget |= BufferSettings::CurrentBuffer;
    bufferSettings.setUserNoticesTarget(redirectTarget);

    redirectTarget = 0;
    if (ui.serverNoticesInDefaultBuffer->isChecked())
        redirectTarget |= BufferSettings::DefaultBuffer;
    if (ui.serverNoticesInStatusBuffer->isChecked())
        redirectTarget |= BufferSettings::StatusBuffer;
    if (ui.serverNoticesInCurrentBuffer->isChecked())
        redirectTarget |= BufferSettings::CurrentBuffer;
    bufferSettings.setServerNoticesTarget(redirectTarget);

    redirectTarget = 0;
    if (ui.errorMsgsInDefaultBuffer->isChecked())
        redirectTarget |= BufferSettings::DefaultBuffer;
    if (ui.errorMsgsInStatusBuffer->isChecked())
        redirectTarget |= BufferSettings::StatusBuffer;
    if (ui.errorMsgsInCurrentBuffer->isChecked())
        redirectTarget |= BufferSettings::CurrentBuffer;
    bufferSettings.setErrorMsgsTarget(redirectTarget);

    SettingsPage::save();
    setChangedState(false);
    if (needsStyleReload)
        QtUi::style()->reload();
}

void AppearanceSettingsPage::chooseStyleSheet()
{
    QString dir = ui.customStyleSheetPath->property("storedValue").toString();
    if (!dir.isEmpty() && QFile(dir).exists())
        dir = QDir(dir).absolutePath();
    else
        dir = QDir(Quassel::findDataFilePath("default.qss")).absolutePath();

    QString name = QFileDialog::getOpenFileName(this, i18n("Please choose a stylesheet file"), dir, "*.qss");
    if (!name.isEmpty())
        ui.customStyleSheetPath->setText(name);
}


void AppearanceSettingsPage::widgetHasChanged()
{
    setChangedState(testHasChanged());
}


bool AppearanceSettingsPage::testHasChanged()
{
    if (ui.styleComboBox->currentIndex() != ui.styleComboBox->property("storedValue").toInt()) return true;

    if (SettingsPage::hasChanged(ui.userNoticesInStatusBuffer)) return true;
    if (SettingsPage::hasChanged(ui.userNoticesInDefaultBuffer)) return true;
    if (SettingsPage::hasChanged(ui.userNoticesInCurrentBuffer)) return true;

    if (SettingsPage::hasChanged(ui.serverNoticesInStatusBuffer)) return true;
    if (SettingsPage::hasChanged(ui.serverNoticesInDefaultBuffer)) return true;
    if (SettingsPage::hasChanged(ui.serverNoticesInCurrentBuffer)) return true;

    if (SettingsPage::hasChanged(ui.errorMsgsInStatusBuffer)) return true;
    if (SettingsPage::hasChanged(ui.errorMsgsInDefaultBuffer)) return true;
    if (SettingsPage::hasChanged(ui.errorMsgsInCurrentBuffer)) return true;

    return false;
}
