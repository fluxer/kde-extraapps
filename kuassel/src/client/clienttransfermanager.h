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

#ifndef CLIENTTRANSFERMANAGER_H
#define CLIENTTRANSFERMANAGER_H

#include "transfermanager.h"

#include <QUuid>

class ClientTransfer;

class ClientTransferManager : public TransferManager
{
    Q_OBJECT
    SYNCABLE_OBJECT

public:
    ClientTransferManager(QObject *parent = 0);

    const ClientTransfer *transfer(const QUuid &uuid) const;

public slots:
    void onCoreTransferAdded(const QUuid &uuid);
    void onTransferInitDone();

signals:
    void transferAdded(const ClientTransfer *transfer);

private slots:
    void onTransferAdded(const Transfer *transfer);

};

#endif
