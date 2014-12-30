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

#include <cstdlib>

#include "monoapplication.h"
#include <KAboutData>
#include "kcmdlinewrapper.h"

#include "cliparser.h"
#include "quassel.h"

int main(int argc, char **argv)
{
    Quassel::setupBuildInfo();
    QCoreApplication::setApplicationName(Quassel::buildInfo().applicationName);

    // We need to explicitly initialize the required resources when linking statically
    Q_INIT_RESOURCE(sql);
    Q_INIT_RESOURCE(pics);

    AbstractCliParser *cliParser;

    // We need to init KCmdLineArgs first
    // TODO: build an AboutData compat class to replace our aboutDlg strings
    KAboutData aboutData("kuassel", "kdelibs4", ki18n("Kuassel IRC"), Quassel::buildInfo().plainVersionString.toUtf8(),
        ki18n("A modern, distributed IRC client"),
        KAboutData::License_GPL_V2,
        ki18n("(c) 2005-2014 Quassel Project\n"
              "(c) 2014-2015 Katana Development Team"
        ));
    // Quassel is dual license
    aboutData.addLicense(KAboutData::License_GPL_V3);
    // that's just the core team
    aboutData.addCredit( ki18n("Manuel Nickschas"), ki18n("Quassel project founder"), "sput@quassel-irc.org");
    aboutData.addCredit( ki18n("Marcus Eggenberger"), ki18n("Quassel project motivator"), "egs@quassel-irc.org");
    aboutData.addCredit( ki18n("Alexander von Renteln"), ki18n("Quassel windows maintainer"), "egs@quassel-irc.org");
    // contributors...
    aboutData.addCredit( ki18n("Daniel Albers"), ki18n("Master Of Translation, many fixes and enhancements"));
    aboutData.addCredit( ki18n("Liudas Alisauskas"), ki18n("Lithuanian translation"));
    aboutData.addCredit( ki18n("Terje Andersen"), ki18n("Norwegian translation, documentation"));
    aboutData.addCredit( ki18n("Jens Arnold"), ki18n("Postgres migration fixes"));
    aboutData.addCredit( ki18n("Adolfo Jayme Barrientos"), ki18n("Spanish translation"));
    aboutData.addCredit( ki18n("Mattia Basaglia"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Pete Beardmore"), ki18n("Linewrap for input line"));
    aboutData.addCredit( ki18n("Rafael Belmonte"), ki18n("Spanish translation"));
    aboutData.addCredit( ki18n("Sergiu Bivol"), ki18n("Romanian translation"));
    aboutData.addCredit( ki18n("Bruno Brigras"), ki18n("Crash fixes"));
    aboutData.addCredit( ki18n("Florent Castelli"), ki18n("Sanitize topic handling"));
    aboutData.addCredit( ki18n("Theo Chatzimichos"), ki18n("Greek translation"));
    aboutData.addCredit( ki18n("Yuri Chornoivan"), ki18n("Ukrainian translation"));
    aboutData.addCredit( ki18n("Tomáš Chvátal"), ki18n("Czech translation"));
    aboutData.addCredit( ki18n("\"Condex\""), ki18n("Galician translation"));
    aboutData.addCredit( ki18n("Joshua Corbin"), ki18n("Various fixes"));
    aboutData.addCredit( ki18n("\"cordata\""), ki18n("Esperanto translation"));
    aboutData.addCredit( ki18n("Matthias Coy"), ki18n("German translation"));
    aboutData.addCredit( ki18n("\"derpella\""), ki18n("Polish translation"));
    aboutData.addCredit( ki18n("\"Dorian\""), ki18n("French translation"));
    aboutData.addCredit( ki18n("Luke Faraone"), ki18n("Doc fixes"));
    aboutData.addCredit( ki18n("Chris Fuenty"), ki18n("SASL support"));
    aboutData.addCredit( ki18n("Kevin Funk"), ki18n("German translation"));
    aboutData.addCredit( ki18n("Fabiano Francesconi"), ki18n("Italian translation"));
    aboutData.addCredit( ki18n("Leo Franchi"), ki18n("OSX improvements"));
    aboutData.addCredit( ki18n("Sebastien Fricker"), ki18n("Audio backend improvements"));
    aboutData.addCredit( ki18n("Alf Gaida"), ki18n("Language improvements"));
    aboutData.addCredit( ki18n("Aurélien Gâteau"), ki18n("Message Indicator support"));
    aboutData.addCredit( ki18n("Marco Genise"), ki18n("Ideas, hacking, motivation"));
    aboutData.addCredit( ki18n("Felix Geyer"), ki18n("Certificate handling improvements"));
    aboutData.addCredit( ki18n("Volkan Gezer"), ki18n("Turkish translation"));
    aboutData.addCredit( ki18n("Sjors Gielen"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Sebastian Goth"), ki18n("Many improvements and features"));
    aboutData.addCredit( ki18n("Michael Groh"), ki18n("German translation, fixes"));
    aboutData.addCredit( ki18n("\"Gryllida\""), ki18n("IRC parser improvements"));
    aboutData.addCredit( ki18n("H. İbrahim Güngör"), ki18n("Turkish translation"));
    aboutData.addCredit( ki18n("Jiri Grönroos"), ki18n("Finnish translation"));
    aboutData.addCredit( ki18n("Chris H"), ki18n("Various improvements"));
    aboutData.addCredit( ki18n("Edward Hades"), ki18n("Russian translation"));
    aboutData.addCredit( ki18n("John Hand"), ki18n("Former All-Seeing Eye logo"));
    aboutData.addCredit( ki18n("Adam Harwood"), ki18n("ChatView improvements"));
    aboutData.addCredit( ki18n("Jonas Heese"), ki18n("Project founder, various improvements"));
    aboutData.addCredit( ki18n("Thomas Hogh"), ki18n("Windows builder"));
    aboutData.addCredit( ki18n("Johannes Huber"), ki18n("Many fixes and features, bug triaging"));
    aboutData.addCredit( ki18n("Theofilos Intzoglou"), ki18n("Greek translation"));
    aboutData.addCredit( ki18n("Jovan Jojkić"), ki18n("Serbian translation"));
    aboutData.addCredit( ki18n("Allan Jude"), ki18n("Documentation improvements"));
    aboutData.addCredit( ki18n("Michael Kedzierski"), ki18n("Mac fixes"));
    aboutData.addCredit( ki18n("Scott Kitterman<b></dt><dd>Kubuntu nightly packager, (packaging/build system) bughunter"));
    aboutData.addCredit( ki18n("Paul Klumpp"), ki18n("Initial design and mainwindow layout"));
    aboutData.addCredit( ki18n("Maia Kozheva"), ki18n("Russian translation"));
    aboutData.addCredit( ki18n("Tae-Hoon Kwon"), ki18n("Korean translation"));
    aboutData.addCredit( ki18n("\"Larso\""), ki18n("Finnish translation"));
    aboutData.addCredit( ki18n("Patrick Lauer"), ki18n("Gentoo packaging"));
    aboutData.addCredit( ki18n("Chris Le Sueur"), ki18n("Various fixes and improvements"));
    aboutData.addCredit( ki18n("Jerome Leclanche"), ki18n("Context menu fixes"));
    aboutData.addCredit( ki18n("Hendrik Leppkes"), ki18n("Various features"));
    aboutData.addCredit( ki18n("Jason Lynch"), ki18n("Bugfixes"));
    aboutData.addCredit( ki18n("Awad Mackie"), ki18n("ChatView improvements"));
    aboutData.addCredit( ki18n("Michael Marley"), ki18n("Various fixes and improvements"));
    aboutData.addCredit( ki18n("Martin Mayer"), ki18n("German translation"));
    aboutData.addCredit( ki18n("Daniel Meltzer"), ki18n("Various fixes and improvements"));
    aboutData.addCredit( ki18n("Sebastian Meyer"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Daniel E. Moctezuma"), ki18n("Japanese translation"));
    aboutData.addCredit( ki18n("Chris Moeller"), ki18n("Various fixes and improvements"));
    aboutData.addCredit( ki18n("Thomas Müller"), ki18n("Fixes, Debian packaging"));
    aboutData.addCredit( ki18n("Gábor Németh"), ki18n("Hungarian translation"));
    aboutData.addCredit( ki18n("Per Nielsen"), ki18n("Danish translation"));
    aboutData.addCredit( ki18n("J-P Nurmi"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Marco Paolone"), ki18n("Italian translation"));
    aboutData.addCredit( ki18n("Bas Pape"), ki18n("Many fixes and improvements, bug and patch triaging, tireless community support"));
    aboutData.addCredit( ki18n("Bruno Patri"), ki18n("French translation"));
    aboutData.addCredit( ki18n("Drew Patridge"), ki18n("BluesTheme stylesheet"));
    aboutData.addCredit( ki18n("Celeste Paul"), ki18n("Usability Queen"));
    aboutData.addCredit( ki18n("Vit Pelcak"), ki18n("Czech translation"));
    aboutData.addCredit( ki18n("Regis Perrin"), ki18n("French translation"));
    aboutData.addCredit( ki18n("Diego Petten&ograve;"), ki18n("Gentoo maintainer, build system improvements"));
    aboutData.addCredit( ki18n("Simon Philips"), ki18n("Dutch translation"));
    aboutData.addCredit( ki18n("Daniel Pielmeier"), ki18n("Gentoo maintainer"));
    aboutData.addCredit( ki18n("Nuno Pinheiro"), ki18n("Tons of Oxygen icons including our application icon"));
    aboutData.addCredit( ki18n("David Planella"), ki18n("Translation system fixes"));
    aboutData.addCredit( ki18n("Jure Repinc"), ki18n("Slovenian translation"));
    aboutData.addCredit( ki18n("Patrick von Reth"), ki18n("MinGW support, SNORE backend, Windows packager"));
    aboutData.addCredit( ki18n("Dirk Rettschlag"), ki18n("Various fixes and new features"));
    aboutData.addCredit( ki18n("Miguel Revilla"), ki18n("Spanish translation"));
    aboutData.addCredit( ki18n("Jaak Ristioja"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("David Roden"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Henning Rohlfs"), ki18n("Various fixes"));
    aboutData.addCredit( ki18n("Stella Rouzi"), ki18n("Greek translation"));
    aboutData.addCredit( ki18n("\"salnx\""), ki18n("Highlight configuration improvements"));
    aboutData.addCredit( ki18n("Martin Sandsmark"), ki18n("Core fixes, Quasseldroid"));
    aboutData.addCredit( ki18n("David Sansome"), ki18n("OSX Notification Center support"));
    aboutData.addCredit( ki18n("Dennis Schridde"), ki18n("D-Bus notifications"));
    aboutData.addCredit( ki18n("Jussi Schultink"), ki18n("Tireless tester, {ku|U}buntu tester and lobbyist, liters of delicious Finnish alcohol"));
    aboutData.addCredit( ki18n("Tim Schumacher"), ki18n("Fixes and feedback"));
    aboutData.addCredit( ki18n("\"sfionov\""), ki18n("Russian translation"));
    aboutData.addCredit( ki18n("Harald Sitter"), ki18n("{ku|U}buntu packager, motivator, promoter"));
    aboutData.addCredit( ki18n("Ramanathan Sivagurunathan"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Stefanos Sofroniou"), ki18n("Greek translation"));
    aboutData.addCredit( ki18n("Rüdiger Sonderfeld"), ki18n("Emacs keybindings"));
    aboutData.addCredit( ki18n("Alexander Stein"), ki18n("Tray icon fix"));
    aboutData.addCredit( ki18n("Daniel Steinmetz"), ki18n("Early beta tester and bughunter (on Vista&trade;!)"));
    aboutData.addCredit( ki18n("Jesper Thomschütz"), ki18n("Various fixes"));
    aboutData.addCredit( ki18n("Arthur Titeica"), ki18n("Romanian translation"));
    aboutData.addCredit( ki18n("\"ToBeFree\""), ki18n("German translation"));
    aboutData.addCredit( ki18n("Edward Toroshchin"), ki18n("Russian translation"));
    aboutData.addCredit( ki18n("Adam Tulinius"), ki18n("Early beta tester and bughunter, Danish translation"));
    aboutData.addCredit( ki18n("Deniz Türkoglu"), ki18n("Mac fixes"));
    aboutData.addCredit( ki18n("Frederik M.J. Vestre"), ki18n("Norwegian translation"));
    aboutData.addCredit( ki18n("Atte Virtanen"), ki18n("Finnish translation"));
    aboutData.addCredit( ki18n("Pavel Volkovitskiy"), ki18n("Early beta tester and bughunter"));
    aboutData.addCredit( ki18n("Roscoe van Wyk"), ki18n("Fixes"));
    aboutData.addCredit( ki18n("Zé"), ki18n("Portuguese translation"));
    aboutData.addCredit( ki18n("Benjamin Zeller"), ki18n("Windows build system fixes"));
    aboutData.addCredit( ki18n("\"zeugma\""), ki18n("Turkish translation"));

    // and finally special thanks to...
    aboutData.addCredit( ki18n("John Hand"), ki18n("the original Quassel icon - The All-Seeing Eye"));
    aboutData.addCredit( ki18n("Qt Software formerly known as Trolltech"), ki18n("creating Qt and Qtopia, and for sponsoring development of QuasselTopia with Greenphones and more"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    cliParser = new KCmdLineWrapper();
    Quassel::setCliParser(cliParser);

    // Initialize CLI arguments
    // NOTE: We can't use tr() at this point, since app is not yet created

    // put shared client&core arguments here
    cliParser->addSwitch("debug", 'd', "Enable debug output");
    cliParser->addSwitch("help", 'h', "Display this help and exit");
    cliParser->addSwitch("version", 'v', "Display version information");
    cliParser->addOption("configdir <path>", 'c', "Specify the directory holding configuration files, the SQlite database and the SSL certificate");
    cliParser->addOption("datadir <path>", 0, "DEPRECATED - Use --configdir instead");

    // put client-only arguments here
    cliParser->addOption("qss <file.qss>", 0, "Load a custom application stylesheet");
    cliParser->addSwitch("debugbufferswitches", 0, "Enables debugging for bufferswitches");
    cliParser->addSwitch("debugmodel", 0, "Enables debugging for models");
    cliParser->addSwitch("hidewindow", 0, "Start the client minimized to the system tray");
    // put core-only arguments here
    cliParser->addOption("listen <address>[,<address[,...]]>", 0, "The address(es) quasselcore will listen on", "::,0.0.0.0");
    cliParser->addOption("port <port>", 'p', "The port quasselcore will listen at", QString("4242"));
    cliParser->addSwitch("norestore", 'n', "Don't restore last core's state");
    cliParser->addOption("loglevel <level>", 'L', "Loglevel Debug|Info|Warning|Error", "Info");
#ifdef HAVE_SYSLOG
    cliParser->addSwitch("syslog", 0, "Log to syslog");
#endif
    cliParser->addOption("logfile <path>", 'l', "Log to a file");
    cliParser->addOption("select-backend <backendidentifier>", 0, "Switch storage backend (migrating data if possible)");
    cliParser->addSwitch("add-user", 0, "Starts an interactive session to add a new core user");
    cliParser->addOption("change-userpass <username>", 0, "Starts an interactive session to change the password of the user identified by username");
    cliParser->addSwitch("oidentd", 0, "Enable oidentd integration");
    cliParser->addOption("oidentd-conffile <file>", 0, "Set path to oidentd configuration file");
#ifdef HAVE_SSL
    cliParser->addSwitch("require-ssl", 0, "Require SSL for client connections");
#endif
    cliParser->addSwitch("enable-experimental-dcc", 0, "Enable highly experimental and unfinished support for CTCP DCC (DANGEROUS)");

    // service arguments
    cliParser->addOption("url <url>", 0, "irc:// URL or server hostname");

    // the KDE version needs this extra call to parse argc/argv before app is instantiated
    if (!cliParser->init()) {
        cliParser->usage();
        return EXIT_FAILURE;
    }

    MonolithicApplication app(argc, argv);


    if (!app.init()) return EXIT_FAILURE;
    return app.exec();
}
