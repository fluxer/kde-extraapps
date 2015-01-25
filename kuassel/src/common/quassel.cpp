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

#include "quassel.h"

#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QHostAddress>
#include <QLibraryInfo>
#include <QSettings>
#include <QUuid>

#include <kstandarddirs.h>

#include "bufferinfo.h"
#include "identity.h"
#include "logger.h"
#include "message.h"
#include "network.h"
#include "protocol.h"
#include "syncableobject.h"
#include "types.h"

#include "../../version.h"

Quassel::BuildInfo Quassel::_buildInfo;
AbstractCliParser *Quassel::_cliParser = 0;
Quassel::RunMode Quassel::_runMode;
QString Quassel::_configDirPath;
QStringList Quassel::_dataDirPaths;
bool Quassel::_initialized = false;
bool Quassel::DEBUG = false;
QString Quassel::_coreDumpFileName;
Quassel *Quassel::_instance = 0;
bool Quassel::_handleCrashes = true;
Quassel::LogLevel Quassel::_logLevel = InfoLevel;
QFile *Quassel::_logFile = 0;
bool Quassel::_logToSyslog = false;

Quassel::Quassel()
{
    Q_ASSERT(!_instance);
    _instance = this;

    // We catch SIGTERM and SIGINT (caused by Ctrl+C) to graceful shutdown Quassel.
    signal(SIGTERM, handleSignal);
    signal(SIGINT, handleSignal);
}


Quassel::~Quassel()
{
    if (logFile()) {
        logFile()->close();
        logFile()->deleteLater();
    }
    delete _cliParser;
}


bool Quassel::init()
{
    if (_initialized)
        return true;  // allow multiple invocations because of MonolithicApplication

    if (_handleCrashes) {
        // we have crashhandler for win32 and unix (based on execinfo).
#if defined(HAVE_EXECINFO)
        // we only handle crashes ourselves if coredumps are disabled
        struct rlimit *limit = (rlimit *)malloc(sizeof(struct rlimit));
        int rc = getrlimit(RLIMIT_CORE, limit);

        if (rc == -1 || !((long)limit->rlim_cur > 0 || limit->rlim_cur == RLIM_INFINITY)) {
        signal(SIGABRT, handleSignal);
        signal(SIGSEGV, handleSignal);
        signal(SIGBUS, handleSignal);
    }
    free(limit);
#endif /* HAVE_EXECINFO */
    }

    _initialized = true;
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    registerMetaTypes();

    Network::setDefaultCodecForServer("ISO-8859-1");
    Network::setDefaultCodecForEncoding("UTF-8");
    Network::setDefaultCodecForDecoding("ISO-8859-15");

    if (isOptionSet("help")) {
        cliParser()->usage();
        return false;
    }

    if (isOptionSet("version")) {
        std::cout << qPrintable("Kuassel IRC: " + Quassel::buildInfo().plainVersionString) << std::endl;
        return false;
    }

    DEBUG = isOptionSet("debug");

    // set up logging
    if (Quassel::runMode() != Quassel::ClientOnly) {
        if (isOptionSet("loglevel")) {
            QString level = optionValue("loglevel");

            if (level == "Debug") _logLevel = DebugLevel;
            else if (level == "Info") _logLevel = InfoLevel;
            else if (level == "Warning") _logLevel = WarningLevel;
            else if (level == "Error") _logLevel = ErrorLevel;
        }

        QString logfilename = optionValue("logfile");
        if (!logfilename.isEmpty()) {
            _logFile = new QFile(logfilename);
            if (!_logFile->open(QIODevice::Append | QIODevice::Text)) {
                qWarning() << "Could not open log file" << logfilename << ":" << _logFile->errorString();
                _logFile->deleteLater();
                _logFile = 0;
            }
        }
#ifdef HAVE_SYSLOG
        _logToSyslog = isOptionSet("syslog");
#endif
    }

    return true;
}


void Quassel::quit()
{
    QCoreApplication::quit();
}


//! Register our custom types with Qt's Meta Object System.
/**  This makes them available for QVariant and in signals/slots, among other things.
*
*/
void Quassel::registerMetaTypes()
{
    // Complex types
    qRegisterMetaType<Message>("Message");
    qRegisterMetaType<BufferInfo>("BufferInfo");
    qRegisterMetaType<NetworkInfo>("NetworkInfo");
    qRegisterMetaType<Network::Server>("Network::Server");
    qRegisterMetaType<Identity>("Identity");

    qRegisterMetaTypeStreamOperators<Message>("Message");
    qRegisterMetaTypeStreamOperators<BufferInfo>("BufferInfo");
    qRegisterMetaTypeStreamOperators<NetworkInfo>("NetworkInfo");
    qRegisterMetaTypeStreamOperators<Network::Server>("Network::Server");
    qRegisterMetaTypeStreamOperators<Identity>("Identity");

    qRegisterMetaType<IdentityId>("IdentityId");
    qRegisterMetaType<BufferId>("BufferId");
    qRegisterMetaType<NetworkId>("NetworkId");
    qRegisterMetaType<UserId>("UserId");
    qRegisterMetaType<AccountId>("AccountId");
    qRegisterMetaType<MsgId>("MsgId");

    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<QUuid>("QUuid");

    qRegisterMetaTypeStreamOperators<IdentityId>("IdentityId");
    qRegisterMetaTypeStreamOperators<BufferId>("BufferId");
    qRegisterMetaTypeStreamOperators<NetworkId>("NetworkId");
    qRegisterMetaTypeStreamOperators<UserId>("UserId");
    qRegisterMetaTypeStreamOperators<AccountId>("AccountId");
    qRegisterMetaTypeStreamOperators<MsgId>("MsgId");

    qRegisterMetaType<Protocol::SessionState>("Protocol::SessionState");

    // Versions of Qt prior to 4.7 didn't define QVariant as a meta type
    if (!QMetaType::type("QVariant")) {
        qRegisterMetaType<QVariant>("QVariant");
        qRegisterMetaTypeStreamOperators<QVariant>("QVariant");
    }
}

void Quassel::setupBuildInfo()
{
    _buildInfo.applicationName = "kuassel";
    _buildInfo.baseVersion = QUASSEL_VERSION_STRING;
    _buildInfo.plainVersionString = QString("v%1").arg(_buildInfo.baseVersion);
}


//! Signal handler for graceful shutdown.
void Quassel::handleSignal(int sig)
{
    switch (sig) {
    case SIGTERM:
    case SIGINT:
        qWarning("%s", qPrintable(QString("Caught signal %1 - exiting.").arg(sig)));
        if (_instance)
            _instance->quit();
        else
            QCoreApplication::quit();
        break;
    case SIGABRT:
    case SIGSEGV:
    case SIGBUS:
        logBacktrace(coreDumpFileName());
        exit(EXIT_FAILURE);
        break;
    default:
        break;
    }
}


void Quassel::logFatalMessage(const char *msg)
{
    QFile dumpFile(coreDumpFileName());
    dumpFile.open(QIODevice::Append);
    QTextStream dumpStream(&dumpFile);

    dumpStream << "Fatal: " << msg << '\n';
    dumpStream.flush();
    dumpFile.close();
}


Quassel::Features Quassel::features()
{
    Features feats = 0;
    for (int i = 1; i <= NumFeatures; i <<= 1)
        feats |= (Feature)i;

    return feats;
}


const QString &Quassel::coreDumpFileName()
{
    if (_coreDumpFileName.isEmpty()) {
        QDir configDir(configDirPath());
        _coreDumpFileName = configDir.absoluteFilePath(QString("Quassel-Crash-%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmm")));
        QFile dumpFile(_coreDumpFileName);
        dumpFile.open(QIODevice::Append);
        QTextStream dumpStream(&dumpFile);
        dumpStream << "Kuassel IRC: " << _buildInfo.baseVersion << '\n';
        qDebug() << "Kuassel IRC: " << _buildInfo.baseVersion;
        dumpStream.flush();
        dumpFile.close();
    }
    return _coreDumpFileName;
}


QString Quassel::configDirPath()
{
    if (!_configDirPath.isEmpty())
        return _configDirPath;

    _configDirPath = KStandardDirs().localkdedir() + QDir::separator() + buildInfo().applicationName + QDir::separator();

    QDir qDir(_configDirPath);
    if (!qDir.exists(_configDirPath)) {
        if (!qDir.mkpath(_configDirPath)) {
            qCritical() << "Unable to create config directory:" << qPrintable(qDir.absolutePath());
            return QString();
        }
    }

    return _configDirPath;
}


QStringList Quassel::dataDirPaths()
{
    if (_dataDirPaths.isEmpty()) {
        _dataDirPaths = KStandardDirs().resourceDirs("data");
    }
    return _dataDirPaths;
}

QString Quassel::findDataFilePath(const QString &fileName)
{
    return KStandardDirs().locate("data", buildInfo().applicationName + QDir::separator() +  fileName);
}


QStringList Quassel::scriptDirPaths()
{
    QStringList res(configDirPath() + "scripts/");
    foreach(QString path, dataDirPaths())
    res << path + "scripts/";
    return res;
}

