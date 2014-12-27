/**
 * Copyright (C) 2002-2007 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <knotification.h>

#include "juk.h"

static const char description[] = I18N_NOOP("Jukebox and music manager by KDE");
static const char scott[]       = I18N_NOOP("Author, chief dork and keeper of the funk");
static const char michael[]     = I18N_NOOP("Assistant super-hero, fixer of many things");
static const char georg[]       = I18N_NOOP("More KDE Platform 4 porting efforts");
static const char daniel[]      = I18N_NOOP("System tray docking, \"inline\" tag editing,\nbug fixes, evangelism, moral support");
static const char tim[]         = I18N_NOOP("GStreamer port");
static const char stefan[]      = I18N_NOOP("Global keybindings support");
static const char stephen[]     = I18N_NOOP("Track announcement popups");
static const char frerich[]     = I18N_NOOP("Automagic track data guessing, bugfixes");
static const char zack[]        = I18N_NOOP("More automagical things, now using MusicBrainz");
static const char adam[]        = I18N_NOOP("Co-conspirator in MusicBrainz wizardry");
static const char matthias[]    = I18N_NOOP("Friendly, neighborhood aRts guru");
static const char maks[]        = I18N_NOOP("Making JuK friendlier to people with terabytes of music");
static const char antonio[]     = I18N_NOOP("DCOP interface");
static const char allan[]       = I18N_NOOP("FLAC and MPC support");
static const char nathan[]      = I18N_NOOP("Album cover manager");
static const char pascal[]      = I18N_NOOP("Gimper of splash screen");
static const char laurent[]     = I18N_NOOP("Porting to KDE 4 when no one else was around");
static const char giorgos[]     = I18N_NOOP("Badly-needed tag editor bugfixes.");
static const char sandsmark[]   = I18N_NOOP("Last.fm scrobbling support, lyrics, prepping for KDE Frameworks.");
static const char sho[]         = I18N_NOOP("MPRIS2 Interface implementation.");

int main(int argc, char *argv[])
{
    KAboutData aboutData("juk", 0, ki18n("JuK"),
                         "3.12", ki18n(description), KAboutData::License_GPL,
                         ki18n("© 2002–2014, Scott Wheeler, Michael Pyne, and others"),
                         KLocalizedString(),
                         "http://www.kde.org/applications/multimedia/juk/");

    aboutData.addAuthor(ki18n("Scott Wheeler"), ki18n(scott), "wheeler@kde.org");
    aboutData.addAuthor(ki18n("Michael Pyne"), ki18n(michael), "mpyne@kde.org");
    aboutData.addCredit(ki18n("Γιώργος Κυλάφας (Giorgos Kylafas)"), ki18n(giorgos), "gekylafas@gmail.com");
    aboutData.addCredit(ki18n("Daniel Molkentin"), ki18n(daniel), "molkentin@kde.org");
    aboutData.addCredit(ki18n("Tim Jansen"), ki18n(tim), "tim@tjansen.de");
    aboutData.addCredit(ki18n("Stefan Asserhäll"), ki18n(stefan), "stefan.asserhall@telia.com");
    aboutData.addCredit(ki18n("Stephen Douglas"), ki18n(stephen), "stephen_douglas@yahoo.com");
    aboutData.addCredit(ki18n("Frerich Raabe"), ki18n(frerich), "raabe@kde.org");
    aboutData.addCredit(ki18n("Zack Rusin"), ki18n(zack), "zack@kde.org");
    aboutData.addCredit(ki18n("Adam Treat"), ki18n(adam), "manyoso@yahoo.com");
    aboutData.addCredit(ki18n("Matthias Kretz"), ki18n(matthias), "kretz@kde.org");
    aboutData.addCredit(ki18n("Maks Orlovich"), ki18n(maks), "maksim@kde.org");
    aboutData.addCredit(ki18n("Antonio Larrosa Jimenez"), ki18n(antonio), "larrosa@kde.org");
    aboutData.addCredit(ki18n("Allan Sandfeld Jensen"), ki18n(allan), "kde@carewolf.com");
    aboutData.addCredit(ki18n("Nathan Toone"), ki18n(nathan), "nathan@toonetown.com");
    aboutData.addCredit(ki18n("Pascal Klein"), ki18n(pascal), "4pascal@tpg.com.au");
    aboutData.addCredit(ki18n("Laurent Montel"), ki18n(laurent), "montel@kde.org");
    aboutData.addCredit(ki18n("Georg Grabler"), ki18n(georg), "georg@grabler.net");
    aboutData.addCredit(ki18n("Martin Sandsmark"), ki18n(sandsmark), "martin.sandsmark@kde.org");
    aboutData.addCredit(ki18n("Eike Hein"), ki18n(sho), "hein@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[file(s)]", ki18n("File(s) to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KUniqueApplication::addCmdLineOptions();

    KUniqueApplication a;

    // If this flag gets set then JuK will quit if you click the cover on the track
    // announcement popup when JuK is only in the system tray (the systray has no widget).

    a.setQuitOnLastWindowClosed(false);

    // Create the main window and such

    JuK *juk = new JuK;

    if(a.isSessionRestored() && KMainWindow::canBeRestored(1))
        juk->restore(1, false /* don't show */);

    KConfigGroup config(KGlobal::config(), "Settings");
    if(!config.readEntry("StartDocked", false)) {
        juk->show();
    }
    else if(!a.isSessionRestored()) {
        QString message = i18n("JuK running in docked mode\nUse context menu in system tray to restore.");
        KNotification::event("dock_mode",i18n("JuK Docked"), message);
    }

    return a.exec();
}

// vim: set et sw=4 tw=0 sta fileencoding=utf8:
