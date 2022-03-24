/*  This file is part of the KDE project

    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "dlgtorrent.h"

#include "kget_export.h"

#include <QFile>
#include <QJsonDocument>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <libtorrent/settings_pack.hpp>

KGET_EXPORT_PLUGIN_CONFIG(DlgTorrentSettings)

static const int tableindexrole = 1234;
static const int tabletyperole = 4321;

DlgTorrentSettings::DlgTorrentSettings(QWidget *parent, const QVariantList &args)
    : KCModule(KGetFactory::componentData(), parent, args)
{
    ui.setupUi(this);

    load();

    connect(
        ui.settingsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
        this, SLOT(slotItemChanged(QTableWidgetItem*))
    );
}

DlgTorrentSettings::~DlgTorrentSettings()
{
}

void DlgTorrentSettings::load()
{
    QFile settingsfile(KStandardDirs::locateLocal("appdata", "torrentsettings.json"));
    QJsonDocument settingsjson;
    if (settingsfile.open(QFile::ReadOnly)) {
        settingsjson = QJsonDocument::fromJson(settingsfile.readAll());
    }

    const QVariantMap settingsmap = settingsjson.toVariant().toMap();
    lt::settings_pack ltsettings = lt::default_settings();
    foreach (const QString &key, settingsmap.keys()) {
        const int settingskey = key.toInt();
        const QVariant settingsvalue = settingsmap.value(key);
        switch (settingsvalue.type()) {
            case QVariant::ByteArray:
            case QVariant::String: {
                const QString settingsstring = settingsvalue.toString();
                ltsettings.set_str(settingskey, settingsstring.toStdString());
                break;
            }
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong: {
                const int settingsint = settingsvalue.toInt();
                ltsettings.set_int(settingskey, settingsint);
                break;
            }
            case QVariant::Bool: {
                const bool settingsbool = settingsvalue.toBool();
                ltsettings.set_int(settingskey, settingsbool);
                break;
            }
            default: {
                kWarning(5001) << "invalid setting type";
                break;
            }
        }
    }

    int tablerowcount = 0;
    // qDebug() << Q_FUNC_INFO << "string settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_string_settings; i++) {
        ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        const int settingindex = (lt::settings_pack::string_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_str(settingindex).c_str();

        QTableWidgetItem* tablewidget = new QTableWidgetItem();
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        tablewidget = new QTableWidgetItem();
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::String)));
        tablewidget->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        tablewidget->setText(QString::fromStdString(ltsettings.get_str(settingindex)));
        ui.settingsTableWidget->setItem(tablerowcount, 1, tablewidget);

        tablerowcount++;
    }

    // qDebug() << Q_FUNC_INFO << "int settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_int_settings; i++) {
        ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        const int settingindex = (lt::settings_pack::int_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_int(settingindex);

        QTableWidgetItem* tablewidget = new QTableWidgetItem();
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        tablewidget = new QTableWidgetItem();
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::Int)));
        tablewidget->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        tablewidget->setText(QString::number(ltsettings.get_int(settingindex)));
        ui.settingsTableWidget->setItem(tablerowcount, 1, tablewidget);

        tablerowcount++;
    }

    // qDebug() << Q_FUNC_INFO << "bool settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_bool_settings; i++) {
        ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        const int settingindex = (lt::settings_pack::bool_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_bool(settingindex);

        QTableWidgetItem* tablewidget = new QTableWidgetItem();;
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        tablewidget = new QTableWidgetItem();
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::Bool)));
        tablewidget->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        tablewidget->setCheckState(ltsettings.get_bool(settingindex) ? Qt::Checked : Qt::Unchecked);
        ui.settingsTableWidget->setItem(tablerowcount, 1, tablewidget);

        tablerowcount++;
    }

    emit changed(false);
}

void DlgTorrentSettings::save()
{
    QVariantMap settingsmap;
    for (int i = 0; i < ui.settingsTableWidget->rowCount(); i++) {
        QTableWidgetItem* tablewidget = ui.settingsTableWidget->item(i, 1);
        const int settingsindex = tablewidget->data(tableindexrole).toInt();
        const int settingstype = tablewidget->data(tabletyperole).toInt();
        const QString settingskey = QString::number(settingsindex);
        
        // qDebug() << Q_FUNC_INFO << settingsindex << settingstype;

        switch (settingstype) {
            case QVariant::String: {
                settingsmap.insert(settingskey, tablewidget->text());
                break;
            }
            case QVariant::Int: {
                settingsmap.insert(settingskey, tablewidget->text().toInt());
                break;
            }
            case QVariant::Bool: {
                settingsmap.insert(settingskey, tablewidget->checkState() == Qt::Checked ? true : false);
                break;
            }
            default: {
                kWarning() << "invalid setting type" << settingstype;
                break;
            }
        }
    }

    QJsonDocument settingsjson = QJsonDocument::fromVariant(settingsmap);
    if (settingsjson.isNull()) {
        kWarning() << "could not parse settings map";
        return;
    }

    QFile settingsfile(KStandardDirs::locateLocal("appdata", "torrentsettings.json"));
    if (!settingsfile.open(QFile::WriteOnly)) {
        kWarning() << "could not open settings file";
        return;
    }

    const QByteArray settingsdata = settingsjson.toJson();
    if (settingsfile.write(settingsdata) != settingsdata.size()) {
        kWarning() << "could not write settings file";
        return;
    }

    emit changed(false);
}

void DlgTorrentSettings::slotItemChanged(QTableWidgetItem* tablewidget)
{
    Q_UNUSED(tablewidget);
    emit changed(true);
}

#include "moc_dlgtorrent.cpp"
