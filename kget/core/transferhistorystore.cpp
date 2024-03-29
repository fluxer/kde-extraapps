/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "core/transferhistorystore.h"
#include "core/transferhistorystore_xml_p.h"
#include "core/transferhistorystore_json_p.h"
#include "core/transfer.h"
#include "settings.h"

#include <QDateTime>
#include <QtXml/qdom.h>
#include <QFile>
#include <QList>
#include <QThread>
#include <QTimer>

#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

TransferHistoryItem::TransferHistoryItem() : QObject()
{}

TransferHistoryItem::TransferHistoryItem(const Transfer &transfer) : QObject()
{
    setDest(transfer.dest().pathOrUrl());
    setSource(transfer.source().url());
    setSize(transfer.totalSize());
    setDateTime(QDateTime::currentDateTime());

    setState(transfer.status());
}

TransferHistoryItem::TransferHistoryItem(const TransferHistoryItem &item) : QObject()
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());
}

void TransferHistoryItem::setDest(const QString &dest)
{
    m_dest = dest;
}

void TransferHistoryItem::setSource(const QString &source)
{
    m_source = source;
}

void TransferHistoryItem::setState(int state)
{
    m_state = state;
}

void TransferHistoryItem::setSize(int size)
{
    m_size = size;
}

void TransferHistoryItem::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

QString TransferHistoryItem::dest() const
{
    return m_dest;
}

QString TransferHistoryItem::source() const
{
    return m_source;
}

int TransferHistoryItem::state() const
{
    return m_state;
}

int TransferHistoryItem::size() const
{
    return m_size;
}

QDateTime TransferHistoryItem::dateTime() const
{
    return m_dateTime;
}

TransferHistoryItem& TransferHistoryItem::operator=(const TransferHistoryItem &item)
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());

    return *this;
}

bool TransferHistoryItem::operator==(const TransferHistoryItem& item) const
{
    return dest() == item.dest() && source() == item.source();
}

TransferHistoryStore::TransferHistoryStore() : QObject(),
    m_items()
{
}

TransferHistoryStore::~TransferHistoryStore()
{
}

QList <TransferHistoryItem> TransferHistoryStore::items() const
{
    return m_items;
}

TransferHistoryStore *TransferHistoryStore::getStore()
{
    switch(Settings::historyBackend())
    {
        case TransferHistoryStore::Xml:
            return new XmlStore(KStandardDirs::locateLocal("appdata", "transferhistory.kgt"));
        case TransferHistoryStore::Json:
        default:
            return new JsonStore(KStandardDirs::locateLocal("appdata", "transferhistory.json"));
    }
}

#include "moc_transferhistorystore.cpp"
