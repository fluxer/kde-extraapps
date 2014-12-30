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

#include "qtuiapplication.h"

#include <QStringList>

#include <KStandardDirs>

#include "client.h"
#include "cliparser.h"
#include "mainwin.h"
#include "qtui.h"
#include "qtuisettings.h"

QtUiApplication::QtUiApplication(int &argc, char **argv)
    : KUniqueApplication(),
    Quassel(),
    _aboutToQuit(false)
{
    Q_UNUSED(argc); Q_UNUSED(argv);

    // We need to setup KDE's data dirs
    QStringList dataDirs = KGlobal::dirs()->findDirs("data", "");
    for (int i = 0; i < dataDirs.count(); i++)
        dataDirs[i].append("kuassel/");
    dataDirs.append(":/data/");
    setDataDirPaths(dataDirs);


    disableCrashhandler();
    setRunMode(Quassel::ClientOnly);

#if QT_VERSION < 0x050000
    qInstallMsgHandler(Client::logMessage);
#else
    qInstallMessageHandler(Client::logMessage);
#endif
}


bool QtUiApplication::init()
{
    if (Quassel::init()) {
        // session resume
        QtUi *gui = new QtUi();
        Client::init(gui);
        // init gui only after the event loop has started
        // QTimer::singleShot(0, gui, SLOT(init()));
        gui->init();
        resumeSessionIfPossible();
        return true;
    }
    return false;
}


QtUiApplication::~QtUiApplication()
{
    Client::destroy();
}


void QtUiApplication::quit()
{
    QtUi::mainWindow()->quit();
}


void QtUiApplication::commitData(QSessionManager &manager)
{
    Q_UNUSED(manager)
    _aboutToQuit = true;
}


void QtUiApplication::saveState(QSessionManager &manager)
{
    //qDebug() << QString("saving session state to id %1").arg(manager.sessionId());
    // AccountId activeCore = Client::currentCoreAccount().accountId(); // FIXME store this!
    SessionSettings s(manager.sessionId());
    s.setSessionAge(0);
    QtUi::mainWindow()->saveStateToSettings(s);
}


void QtUiApplication::resumeSessionIfPossible()
{
    // load all sessions
    if (isSessionRestored()) {
        qDebug() << QString("restoring from session %1").arg(sessionId());
        SessionSettings s(sessionId());
        s.sessionAging();
        s.setSessionAge(0);
        QtUi::mainWindow()->restoreStateFromSettings(s);
        s.cleanup();
    }
    else {
        SessionSettings s(QString("1"));
        s.sessionAging();
        s.cleanup();
    }
}
