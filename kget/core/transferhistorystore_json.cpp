/*  This file is part of the KDE project

    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

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

#include "core/transferhistorystore.h"
#include "core/transferhistorystore_json_p.h"

#include <QDateTime>
#include <QFile>
#include <KDebug>

JsonStore::JsonStore(const QString &database)
    : TransferHistoryStore(),
    m_dbName(database)
{
}

JsonStore::~JsonStore()
{
}

void JsonStore::load()
{
    m_items.clear();
    QFile dbFile(m_dbName);
    if (dbFile.open(QFile::ReadOnly)) {
        const QByteArray dbData = dbFile.readAll();

        m_dbDoc = QJsonDocument::fromJson(dbData);
        if (m_dbDoc.isNull()) {
            kWarning(5001) << m_dbDoc.errorString();
        } else {
            const QVariantMap dbMap = m_dbDoc.toVariant().toMap();
            const QStringList dbKeys = dbMap.keys();

            int counter = 1;
            foreach (const QString &key, dbKeys) {
                const QVariantMap keyMap = dbMap.value(key).toMap();

                TransferHistoryItem item;
                item.setSource(key);
                item.setDest(keyMap.value("dest").toString());
                item.setState(keyMap.value("state").toInt());
                item.setDateTime(QDateTime::fromTime_t(keyMap.value("time").toUInt()));
                item.setSize(keyMap.value("size").toInt());

                m_items << item;
                emit elementLoaded(counter, dbKeys.size(), item);
                counter++;
            }
        }

        dbFile.close();
    } else {
        // may not exist yet
        kDebug(5001) << "could not open" << m_dbName;
    }

    emit loadFinished();
}

void JsonStore::clear()
{
    QFile::remove(m_dbName);
}

void JsonStore::saveItem(const TransferHistoryItem &item)
{
    saveItems(QList<TransferHistoryItem>() << item);
}

void JsonStore::saveItems(const QList<TransferHistoryItem> &items)
{
    QFile dbFile(m_dbName);
    if (dbFile.open(QFile::WriteOnly)) {
        QVariantMap dbMap = m_dbDoc.toVariant().toMap();

        foreach (const TransferHistoryItem &item, items) {
            QVariantMap itemMap;
            itemMap.insert("dest", item.dest());
            itemMap.insert("state", QString::number(item.state()));
            itemMap.insert("time", QString::number(item.dateTime().toTime_t()));
            itemMap.insert("size", QString::number(item.size()));

            dbMap.insert(item.source(), itemMap);
            m_dbDoc = QJsonDocument::fromVariant(dbMap);

            m_items << item;
        }

        const QByteArray dbData = m_dbDoc.toJson();
        if (dbFile.write(dbData.constData(), dbData.size()) != dbData.size()) {
            kWarning(5001) << "could not write data to" << m_dbName;;
        }

        dbFile.close();
    } else {
        kWarning(5001) << "could not open" << m_dbName;
    }

    emit saveFinished();
}

void JsonStore::deleteItem(const TransferHistoryItem &item)
{
    QFile dbFile(m_dbName);
    if (dbFile.open(QFile::WriteOnly)) {
        QVariantMap dbMap = m_dbDoc.toVariant().toMap();

        dbMap.remove(item.source());
        m_dbDoc = QJsonDocument::fromVariant(dbMap);

        m_items.removeAll(item);

        const QByteArray dbData = m_dbDoc.toJson();
        if (dbFile.write(dbData.constData(), dbData.size()) != dbData.size()) {
            kWarning(5001) << "could not write data to" << m_dbName;;
        }

        dbFile.close();
    } else {
        kWarning(5001) << "could not open" << m_dbName;
    }

    emit deleteFinished();
}

#include "moc_transferhistorystore_json_p.cpp"
