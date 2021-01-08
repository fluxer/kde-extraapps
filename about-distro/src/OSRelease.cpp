/*
  Copyright (C) 2014 Harald Sitter <apachelogger@ubuntu.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License or (at your option) version 3 or any later version
  accepted by the membership of KDE e.V. (or its successor approved
  by the membership of KDE e.V.), which shall act as a proxy
  defined in Section 14 of version 3 of the license.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OSRelease.h"

#include <QtCore/QFile>

#include <KDebug>
#include <KLocalizedString>
#include <KShell>

static void setVar(QString *var, const QString &value) 
{
    // Values may contain quotation marks, strip them as we have no use for them.
    KShell::Errors error;
    QStringList args = KShell::splitArgs(value, KShell::NoOptions, &error);
    kDebug() << args << args.size();
    if (error != KShell::NoError) { // Failed to parse.
        return;
    }
    *var = args.join(QLatin1String(" "));
}

static void setVar(QStringList *var, const QString &value)
{
    // Instead of passing the verbatim value we manually strip any initial quotes
    // and then run it through KShell. At this point KShell will actually split
    // by spaces giving us the final QStringList.
    // NOTE: Splitting like this does not actually allow escaped substrings to
    //       be handled correctly, so "kitteh \"french fries\"" would result in
    //       three list entries. I'd argue that if someone makes an id like that
    //       they are at fault for the bogus parsing here though as id explicitly
    //       is required to not contain spaces even if more advanced shell escaping
    //       is also allowed...
    QString value_ = value;
    if (value_.at(0) == QLatin1Char('"') && value_.at(value_.size()-1) == QLatin1Char('"')) {
        value_.remove(0, 1);
        value_.remove(-1, 1);
    }
    KShell::Errors error;
    QStringList args = KShell::splitArgs(value_, KShell::NoOptions, &error);
    if (error != KShell::NoError) { // Failed to parse.
        return;
    }
    *var = args;
}

// https://www.freedesktop.org/software/systemd/man/os-release.html
OSRelease::OSRelease()
{
    // NOTE: The os-release specification defines default values for specific
    //       fields which means that even if we can not read the os-release file
    //       we have sort of expected default values to use.
    // TODO: it might still be handy to indicate to the outside whether
    //       fallback values are being used or not.

    // Set default values for non-optional fields.
    logo = QLatin1String("start-here-kde");
    prettyName = QLatin1String("Linux");

    static const QStringList OSReleaseFiles = QStringList()
        << QLatin1String("/etc/os-release")
        << QLatin1String("/usr/lib/os-release");

    foreach (const QString &releaseFile, OSReleaseFiles) {
        QFile file(releaseFile);
        if (!file.exists()) {
            continue;
        }
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        while (!file.atEnd()) {
            QString line = file.readLine();
            if (line.startsWith(QLatin1Char('#'))) {
                // Comment line
                continue;
            }

            QStringList comps = line.split(QLatin1Char('='));
            if (comps.size() != 2) {
                // Invalid line.
                continue;
            }

            QString key = comps.at(0);
            QString value = comps.at(1).trimmed();
            if (key == QLatin1String("LOGO")) {
                setVar(&logo, value);
            } else if (key == QLatin1String("VERSION_ID")) {
                setVar(&versionId, value);
            } else if (key == QLatin1String("PRETTY_NAME")) {
                setVar(&prettyName, value);
            } else if (key == QLatin1String("HOME_URL")) {
                setVar(&homeUrl, value);
            }
            // only these above are used right now
        }
    }
}
