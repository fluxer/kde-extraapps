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

#include <QDBusInterface>
#include <QDBusReply>
#include <KDebug>
#include <KIcon>
#include <KMessageBox>

#include "krunner_kemu.h"

KEmuControlRunner::KEmuControlRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);

    setObjectName(QLatin1String("QEMU virtual machine manager runner"));
    setSpeed(AbstractRunner::SlowSpeed);

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));
}

KEmuControlRunner::~KEmuControlRunner()
{
}

void KEmuControlRunner::prep()
{
    QList<Plasma::RunnerSyntax> syntaxes;

    syntaxes << Plasma::RunnerSyntax("vm start :q:", i18n("Starts virtual machine"));
    syntaxes << Plasma::RunnerSyntax("vm stop :q:", i18n("Stops virtual machine"));

    setSyntaxes(syntaxes);
}

void KEmuControlRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < 7) {
        return;
    }

    QDBusInterface interface("org.kde.kded", "/modules/kemu", "org.kde.kemu");
    const QStringList machines = interface.call("machines").arguments().at(0).toStringList();
    if (term.startsWith("vm start")) {
        foreach (const QString &machine, machines) {
            const bool isrunning = interface.call("isRunning", machine).arguments().at(0).toBool();
            if (!isrunning) {
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::PossibleMatch);
                match.setIcon(KIcon(QLatin1String("system-run")));
                match.setText(i18n("Start %1 virtual machine", machine));
                match.setData(QStringList() << "start" << machine);
                context.addMatch(term, match);
            }
        }
    } else if (term.startsWith("vm stop")) {
        foreach (const QString &machine, machines) {
            const bool isrunning = interface.call("isRunning", machine).arguments().at(0).toBool();
            if (isrunning) {
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::PossibleMatch);
                match.setIcon(KIcon(QLatin1String("system-shutdown")));
                match.setText(i18n("Stop %1 virtual machine", machine));
                match.setData(QStringList() << "stop" << machine);
                context.addMatch(term, match);
            }
        }
    }
}

void KEmuControlRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QStringList data = match.data().toStringList();
    Q_ASSERT(data.size() == 2);
    const QString command = data.at(0);
    const QString machine = data.at(1);

    QDBusInterface interface("org.kde.kded", "/modules/kemu", "org.kde.kemu");
    if (command == "start") {
        QDBusReply<bool> reply = interface.call("start", machine);
        if (!reply.value()) {
            KMessageBox::error(nullptr, i18n("Unable to start VM"),
                i18n("Unable to start %1 virtual machine.", machine));
        }
    } else if (command == "stop") {
        QDBusReply<bool> reply = interface.call("stop", machine);
        if (!reply.value()) {
            KMessageBox::error(nullptr, i18n("Unable to stop VM"),
                i18n("Unable to stop %1 virtual machine.", machine));
        }
    } else {
        kWarning() << "invalid command" << command;
    }
}

#include "moc_krunner_kemu.cpp"
