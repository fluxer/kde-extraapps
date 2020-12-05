/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>
#include "kemumainwindow.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("kemu", 0, ki18n("KEmu"),
                         "1.0.0", ki18n("Simple QEMU frontend for KDE."),
                         KAboutData::License_GPL_V2,
                         ki18n("(c) 2016 Ivailo Monev"),
                         KLocalizedString(),
                        "http://github.com/fluxer/katana"
                        );

    aboutData.addAuthor(ki18n("Ivailo Monev"),
                        ki18n("Maintainer"),
                        "xakepa10@gmail.com");
    aboutData.setProgramIconName(QLatin1String("applications-engineering"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication *kemuapp = new KApplication();
    KEmuMainWindow *kemuwindow = new KEmuMainWindow();
    kemuwindow->show();
    return kemuapp->exec();
}
