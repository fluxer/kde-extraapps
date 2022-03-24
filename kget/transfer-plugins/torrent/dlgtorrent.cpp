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
#include <QSpinBox>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

KGET_EXPORT_PLUGIN_CONFIG(DlgTorrentSettings)

static const int tableindexrole = 1234;
static const int tabletyperole = 4321;

DlgTorrentSettings::DlgTorrentSettings(QWidget *parent, const QVariantList &args)
    : KCModule(KGetFactory::componentData(), parent, args)
{
    m_ui.setupUi(this);

    load();

    connect(
        m_ui.settingsTableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
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
                ltsettings.set_bool(settingskey, settingsbool);
                break;
            }
            default: {
                kWarning() << "invalid setting type";
                break;
            }
        }
    }

    loadSettings(ltsettings);
    emit changed(false);
}

void DlgTorrentSettings::save()
{
    QVariantMap settingsmap;
    for (int i = 0; i < m_ui.settingsTableWidget->rowCount(); i++) {
        const QTableWidgetItem* tablewidget0 = m_ui.settingsTableWidget->item(i, 0);
        const int settingsindex = tablewidget0->data(tableindexrole).toInt();
        const int settingstype = tablewidget0->data(tabletyperole).toInt();
        const QString settingskey = QString::number(settingsindex);
        
        // qDebug() << Q_FUNC_INFO << settingsindex << settingstype;

        switch (settingstype) {
            case QVariant::String: {
                const QTableWidgetItem* tablewidget1 = m_ui.settingsTableWidget->item(i, 1);
                settingsmap.insert(settingskey, tablewidget1->text());
                break;
            }
            case QVariant::Int: {
                const QSpinBox* tablecellwidget = qobject_cast<QSpinBox*>(m_ui.settingsTableWidget->cellWidget(i, 1));
                settingsmap.insert(settingskey, tablecellwidget->value());
                break;
            }
            case QVariant::Bool: {
                const QTableWidgetItem* tablewidget1 = m_ui.settingsTableWidget->item(i, 1);
                settingsmap.insert(settingskey, tablewidget1->checkState() == Qt::Checked ? true : false);
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

void DlgTorrentSettings::defaults()
{
    const lt::settings_pack ltsettings = lt::default_settings();
    loadSettings(ltsettings);
    emit changed(true);
}

void DlgTorrentSettings::slotItemChanged(QTableWidgetItem* tablewidget)
{
    Q_UNUSED(tablewidget);
    emit changed(true);
}

void DlgTorrentSettings::slotSpinBoxChanged(const int value)
{
    Q_UNUSED(value);
    emit changed(true);
}

void DlgTorrentSettings::loadSettings(const lt::settings_pack &ltsettings)
{
    int tablerowcount = 0;
    // qDebug() << Q_FUNC_INFO << "string settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_string_settings; i++) {
        const int settingindex = (lt::settings_pack::string_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_str(settingindex).c_str();

        m_ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        QTableWidgetItem* tablewidget = new QTableWidgetItem();
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::String)));
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        m_ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        tablewidget = new QTableWidgetItem();
        tablewidget->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        tablewidget->setText(QString::fromStdString(ltsettings.get_str(settingindex)));
        m_ui.settingsTableWidget->setItem(tablerowcount, 1, tablewidget);

        tablerowcount++;
    }

    // qDebug() << Q_FUNC_INFO << "int settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_int_settings; i++) {
        const int settingindex = (lt::settings_pack::int_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_int(settingindex);

        if (settingindex == lt::settings_pack::alert_mask) {
            continue;
        }

        m_ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        QTableWidgetItem* tablewidget = new QTableWidgetItem();
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::Int)));
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        m_ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        QSpinBox* tablecellwidget = new QSpinBox();
        tablecellwidget->setRange(-INT_MAX, INT_MAX);
        tablecellwidget->setValue(ltsettings.get_int(settingindex));
        connect(
            tablecellwidget, SIGNAL(valueChanged(int)),
            this, SLOT(slotSpinBoxChanged(int))
        );
        m_ui.settingsTableWidget->setCellWidget(tablerowcount, 1, tablecellwidget);

        tablerowcount++;
    }

    // qDebug() << Q_FUNC_INFO << "bool settings";
    for (int i = 0; i < lt::settings_pack::settings_counts_t::num_bool_settings; i++) {

        const int settingindex = (lt::settings_pack::bool_type_base + i);
        // qDebug() << Q_FUNC_INFO << lt::name_for_setting(settingindex) << ltsettings.get_bool(settingindex);

        m_ui.settingsTableWidget->setRowCount(tablerowcount + 1);

        QTableWidgetItem* tablewidget = new QTableWidgetItem();;
        tablewidget->setData(tableindexrole, QVariant(settingindex));
        tablewidget->setData(tabletyperole, QVariant(int(QVariant::String)));
        tablewidget->setFlags(Qt::ItemIsEnabled);
        tablewidget->setText(QString::fromStdString(lt::name_for_setting(settingindex)));
        m_ui.settingsTableWidget->setItem(tablerowcount, 0, tablewidget);

        tablewidget = new QTableWidgetItem();
        tablewidget->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        tablewidget->setCheckState(ltsettings.get_bool(settingindex) ? Qt::Checked : Qt::Unchecked);
        m_ui.settingsTableWidget->setItem(tablerowcount, 1, tablewidget);

        tablerowcount++;
    }
}

#include "moc_dlgtorrent.cpp"
