/***************************************************** vim:set ts=4 sw=4 sts=4:
  Jovie

  The KDE Text-to-Speech Daemon.
  -----------------------------
  Copyright 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Copyright 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// KDE Includes.
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtDBus/QtDBus>

// KTTSD includes.
#include "jovie.h"

int main (int argc, char *argv[]){
    KAboutData aboutdata("jovie", 0, ki18n("Jovie"),
         "0.6.0", ki18n("Text-to-speech synthesis daemon"),
         KAboutData::License_GPL, ki18n("(C) 2002, José Pablo Ezequiel Fernández"));
    aboutdata.addAuthor(ki18n("Jeremy Whiting"), ki18n("Current Maintainer"), "jpwhiting@kde.org");
    aboutdata.addAuthor(ki18n("José Pablo Ezequiel Fernández"),ki18n("Original Author"),"pupeno@pupeno.com");
    aboutdata.addAuthor(ki18n("Gary Cramblitt"), ki18n("Previous Maintainer"),"garycramblitt@comcast.net");
    aboutdata.addAuthor(ki18n("Gunnar Schmi Dt"), ki18n("Contributor"),"gunnar@schmi-dt.de");
    aboutdata.addAuthor(ki18n("Olaf Schmidt"), ki18n("Contributor"),"ojschmidt@kde.org");
    aboutdata.addAuthor(ki18n("Paul Giannaros"), ki18n("Contributor"), "ceruleanblaze@gmail.com");
    aboutdata.addAuthor(ki18n("Simion Ploscariu"), ki18n("Contributor"), "simion314@gmail.com");
    aboutdata.addCredit(ki18n("Jorge Luis Arzola"), ki18n("Testing"), "arzolacub@hotmail.com");
    aboutdata.addCredit(ki18n("David Powell"), ki18n("Testing"), "achiestdragon@gmail.com");
    aboutdata.setProgramIconName(QLatin1String( "preferences-desktop-text-to-speech" ));

    KCmdLineArgs::init( argc, argv, &aboutdata );
    KUniqueApplication::addCmdLineOptions();

    //KUniqueApplication::setOrganizationDomain("kde.org");
    //KUniqueApplication::setApplicationName("jovie");
    KUniqueApplication app;
    app.setQuitOnLastWindowClosed(false);

    if (!KUniqueApplication::start()) {
        kDebug() << "Jovie is already running";
        return (0);
    }

    if (QDBusConnection::sessionBus().interface()->registerService(QLatin1String( "org.kde.KSpeech" ))
        != QDBusConnectionInterface::ServiceRegistered) {
        kDebug() << "Could not register on KSpeech";
    }

    if (QDBusConnection::sessionBus().interface()->registerService(QLatin1String( "org.kde.kttsd" ))
        != QDBusConnectionInterface::ServiceRegistered) {
        kDebug() << "Could not register on kttsd";
    }

    KCrash::setFlags(KCrash::AutoRestart);

    // This app is started automatically, no need for session management
    app.disableSessionManagement();

    kDebug() << "main: Creating Jovie Service";
    Jovie* service = Jovie::Instance();
    service->init();

    // kDebug() << "Entering event loop.";
    return app.exec();
    delete service;
}
