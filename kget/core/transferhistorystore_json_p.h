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

#ifndef TRANSFERHISTORYSTORE_JSON_P_H
#define TRANSFERHISTORYSTORE_JSON_P_H

#include "transferhistorystore.h"

#include <QList>
#include <QJsonDocument>

class TransferHistoryItem;
class JsonStore : public TransferHistoryStore
{
    Q_OBJECT
public:
    JsonStore(const QString &database);
    ~JsonStore();

public slots:
    void load();
    void clear();
    void saveItem(const TransferHistoryItem &item);
    void saveItems(const QList<TransferHistoryItem> &items);
    void deleteItem(const TransferHistoryItem &item);

private:
    QString m_dbName;
    QJsonDocument m_dbDoc;
};

#endif
