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

#include <QUrl>
#include <KDebug>
#include <KLocale>
#include <ksettings.h>

#include "kded_kemu.h"
#include "kpluginfactory.h"

K_PLUGIN_FACTORY(KEmuModuleFactory, registerPlugin<KEmuModule>();)
K_EXPORT_PLUGIN(KEmuModuleFactory("kemu"))

KEmuModule::KEmuModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
    KGlobal::locale()->insertCatalog("kemu");
}

KEmuModule::~KEmuModule()
{
    foreach(QProcess* machineProcess, m_machines) {
        const QString machine = m_machines.key(machineProcess);
        kDebug() << "stopping machine" << machine;
        machineProcess->terminate();
        machineProcess->deleteLater();
        m_machines.remove(machine);
    }
}

bool KEmuModule::start(const QString &machine)
{
    if (m_machines.contains(machine)) {
        emit error(i18n("Machine is already running: %1", machine));
        return false;
    }

    KSettings settings("kemu", KSettings::SimpleConfig);
    const bool enable = settings.value(machine + "/enable").toBool();
    if (!enable) {
        emit error(i18n("Machine is not enabled: %1", machine));
        return false;
    }

    kDebug() << "loading machine settings" << machine;
    const QUrl cdrom = settings.value(machine + "/cdrom").toUrl();
    const QUrl harddisk = settings.value(machine + "/harddisk").toUrl();
    const QString system = settings.value(machine + "/system").toString();
    const QString video = settings.value(machine + "/video", "virtio").toString();
    const QString audio = settings.value(machine + "/audio", "ac97").toString();
    const int ram = settings.value(machine + "/ram", 512).toInt();
    const int cpu = settings.value(machine + "/cpu", 1).toInt();
    const bool kvm = settings.value(machine + "/kvm", false).toBool();
    const bool acpi = settings.value(machine + "/acpi", false).toBool();
    const QString args = settings.value(machine + "/args").toString();

    if (cdrom.isEmpty() && harddisk.isEmpty()) {
        emit error(i18n("Either CD-ROM or Hard Disk image must be set"));
        return false;
    }

    kDebug() << "starting machine" << machine;
    QStringList machineArgs;
    machineArgs << "-name" << machine;
    if (!cdrom.isEmpty()) {
        machineArgs << "-cdrom" << cdrom.toString();
    }
    if (!harddisk.isEmpty()) {
        machineArgs << "-hda" << harddisk.toString();
    }
    machineArgs << "-vga" << video;
    machineArgs << "-soundhw" << audio;
    machineArgs << "-m" << QString::number(ram);
    machineArgs << "-smp" << QString::number(cpu);
    if (kvm) {
        machineArgs << "-enable-kvm";
    }
    if (!acpi) {
        machineArgs << "-no-acpi";
    }
    if (!args.isEmpty()) {
        foreach (const QString argument, args.split(" ")) {
            machineArgs << argument;
        }
    }

    QProcess* machineProcess = new QProcess(this);
    machineProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_machines.insert(machine, machineProcess);
    connect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(_machineFinished(int,QProcess::ExitStatus)));
    machineProcess->start(system, machineArgs);
    const bool success = machineProcess->waitForStarted();
    emit started(machine);
    return success;
}

bool KEmuModule::stop(const QString &machine)
{
    if (!m_machines.contains(machine)) {
        emit error(i18n("Machine is not running: %1", machine));
        return false;
    }

    kDebug() << "stopping machine" << machine;
    QProcess* machineProcess = m_machines.take(machine);
    machineProcess->terminate();
    machineProcess->deleteLater();
    return true;
}

bool KEmuModule::isRunning(const QString &machine) const
{
    return m_machines.contains(machine);
}

QStringList KEmuModule::running() const
{
    return m_machines.keys();
}

QStringList KEmuModule::machines() const
{
    QStringList addedMachines;
    KSettings settings("kemu", KSettings::SimpleConfig);
    foreach(const QString key, settings.keys()) {
        const int sepindex = key.indexOf("/");
        if (sepindex < 1) {
            continue;
        }
        QString machine = key.left(sepindex);
        if (addedMachines.contains(machine)) {
            continue;
        }
        if (settings.value(machine + "/enable").toBool() == true) {
            addedMachines.append(machine);
        } else {
            kDebug() << "garbage machine" << machine;
        }
    }
    return addedMachines;
}

void KEmuModule::_machineFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *machineProcess = qobject_cast<QProcess*>(sender());
    disconnect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(_machineFinished(int,QProcess::ExitStatus)));
    emit stopped(exitCode, QString(machineProcess->readAll()));

    const QString machine = m_machines.key(machineProcess);
    m_machines.remove(machine);
    machineProcess->deleteLater();
}

#include "moc_kded_kemu.cpp"
