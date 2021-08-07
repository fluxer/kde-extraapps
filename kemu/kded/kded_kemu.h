/*  This file is part of the KDE libraries
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

#ifndef KDED_KEMU_H
#define KDED_KEMU_H

#include <QMap>
#include <QProcess>

#include "kdedmodule.h"

class KEmuModule: public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kemu")

public:
    KEmuModule(QObject *parent, const QList<QVariant>&);
    ~KEmuModule();

public Q_SLOTS:
    Q_SCRIPTABLE bool start(const QString &machine);
    Q_SCRIPTABLE bool stop(const QString &machine);

    Q_SCRIPTABLE bool isRunning(const QString &machine) const;
    Q_SCRIPTABLE QStringList running() const;
    Q_SCRIPTABLE QStringList machines() const;

Q_SIGNALS:
    Q_SCRIPTABLE void started(const QString &machine) const;
    Q_SCRIPTABLE void stopped(int status, const QString &error) const;
    Q_SCRIPTABLE void error(const QString &error) const;

private Q_SLOTS:
    void _machineFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QMap<QString,QProcess*> m_machines;
};
#endif // KDED_KEMU_H
