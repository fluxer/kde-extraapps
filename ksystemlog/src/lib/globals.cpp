/***************************************************************************
 *   KSystemLog, a system log viewer tool                                  *
 *   Copyright (C) 2007 by Nicolas Ternisien                               *
 *   nicolas.ternisien@gmail.com                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "globals.h"

#include <QString>
#include <QList>
#include <QMap>

#include <klocale.h>

#include "defaults.h"
#include "logLevel.h"
#include "logMode.h"
#include "logFile.h"

#include "analyzer.h"
#include "logModeAction.h"
#include "logModeFactory.h"
#include "logModeConfiguration.h"
#include "logModeConfigurationWidget.h"

Globals* Globals::self = NULL;

Globals* Globals::instance() {
	if (Globals::self == NULL) {
		Globals::self = new Globals();
	}

	return Globals::self;
}

class GlobalsPrivate {
public:

	/**
	 * Existing Log modes.
	 */
	QMap<QString, LogMode*> logModes;

	QList<LogModeAction*> logModeActions;

	/*
	LogMode* noMode;
	*/

	/**
	 * Existing Log levels. The id value corresponds to the index in the vector
	 */
	QList<LogLevel*> logLevels;

	/**
	 * These value are only pointers to item of the previous vector,
	 * they are provided for convenience
	 */
	LogLevel* noLogLevel;
	LogLevel* debugLogLevel;
	LogLevel* informationLogLevel;
	LogLevel* noticeLogLevel;
	LogLevel* warningLogLevel;
	LogLevel* errorLogLevel;
	LogLevel* criticalLogLevel;
	LogLevel* alertLogLevel;
	LogLevel* emergencyLogLevel;

};

Globals::Globals() :
	d(new GlobalsPrivate()) {

	setupLogLevels();

}

Globals::~Globals() {

	foreach(LogMode* logMode, d->logModes) {
		delete logMode;
	}
	d->logModes.clear();

	foreach(LogModeAction* logModeAction, d->logModeActions) {
		delete logModeAction;
	}
	d->logModeActions.clear();

	foreach(LogLevel* logLevel, d->logLevels) {
		delete logLevel;
	}
	d->logLevels.clear();

	delete d;

}

/*
void Globals::setupLogModes() {
	d->noMode=new LogMode("noLogMode", i18n("No Log"), NO_MODE_ICON);
	d->logModes.append(d->noMode);

	d->sambaMode=new LogMode("sambaLogMode", i18n("Samba Log"), SAMBA_MODE_ICON);
	d->logModes.append(d->sambaMode);

	d->cupsMode=new LogMode("cupsLogMode", i18n("CUPS Log"), CUPS_MODE_ICON);
	d->logModes.append(d->cupsMode);

	d->cupsAccessMode=new LogMode("cupsAccessLogMode", i18n("CUPS Access Log"), CUPS_ACCESS_MODE_ICON);
	d->logModes.append(d->cupsAccessMode);

	d->postfixMode=new LogMode("postfixLogMode", i18n("Postfix Log"), POSTFIX_MODE_ICON);
	d->logModes.append(d->postfixMode);

}
*/

void Globals::setupLogLevels() {
	d->noLogLevel=new LogLevel(NONE_LOG_LEVEL_ID, i18n("None"), QLatin1String( "help-contents" ), QColor(208, 210, 220));
	d->logLevels.append(d->noLogLevel);

	d->debugLogLevel=new LogLevel(DEBUG_LOG_LEVEL_ID, i18n("Debug"), QLatin1String( "attach" ), QColor(156, 157, 165));
	d->logLevels.append(d->debugLogLevel);

	d->informationLogLevel=new LogLevel(INFORMATION_LOG_LEVEL_ID, i18n("Information"), QLatin1String( "dialog-information" ), QColor(36, 49, 103) /*QColor(0, 0, 0)*/);
	d->logLevels.append(d->informationLogLevel);

	d->noticeLogLevel=new LogLevel(NOTICE_LOG_LEVEL_ID, i18n("Notice"), QLatin1String( "book2" ), QColor(36, 138, 22));
	d->logLevels.append(d->noticeLogLevel);

	d->warningLogLevel=new LogLevel(WARNING_LOG_LEVEL_ID, i18n("Warning"), QLatin1String( "dialog-warning" ), QColor(238, 144, 21));
	d->logLevels.append(d->warningLogLevel);

	d->errorLogLevel=new LogLevel(ERROR_LOG_LEVEL_ID, i18n("Error"), QLatin1String( "dialog-close" ), QColor(173, 28, 28));
	d->logLevels.append(d->errorLogLevel);

	d->criticalLogLevel=new LogLevel(CRITICAL_LOG_LEVEL_ID, i18n("Critical"), QLatin1String( "exec" ), QColor(214, 26, 26));
	d->logLevels.append(d->criticalLogLevel);

	d->alertLogLevel=new LogLevel(ALERT_LOG_LEVEL_ID, i18n("Alert"), QLatin1String( "bell" ), QColor(214, 0, 0));
	d->logLevels.append(d->alertLogLevel);

	d->emergencyLogLevel=new LogLevel(EMERGENCY_LOG_LEVEL_ID, i18n("Emergency"), QLatin1String( "application-exit" ), QColor(255, 0, 0));
	d->logLevels.append(d->emergencyLogLevel);

}


QList<LogMode*> Globals::logModes() {
	return d->logModes.values();
}

QList<LogLevel*> Globals::logLevels() {
	return d->logLevels;
}


LogLevel* Globals::noLogLevel() {
	return d->noLogLevel;
}
LogLevel* Globals::debugLogLevel() {
	return d->debugLogLevel;
}
LogLevel* Globals::informationLogLevel() {
	return d->informationLogLevel;
}
LogLevel* Globals::noticeLogLevel() {
	return d->noticeLogLevel;
}
LogLevel* Globals::warningLogLevel() {
	return d->warningLogLevel;
}
LogLevel* Globals::errorLogLevel() {
	return d->errorLogLevel;
}
LogLevel* Globals::criticalLogLevel() {
	return d->criticalLogLevel;
}
LogLevel* Globals::alertLogLevel() {
	return d->alertLogLevel;
}
LogLevel* Globals::emergencyLogLevel() {
	return d->emergencyLogLevel;
}

void Globals::registerLogModeFactory(LogModeFactory* logModeFactory) {
	QList<LogMode*> logModes = logModeFactory->createLogModes();

	foreach (LogMode* logMode, logModes) {
		//Log mode
		d->logModes.insert(logMode->id(), logMode);
	}

	//Log mode Actions
	LogModeAction* logModeAction = logModeFactory->createLogModeAction();
	if (logModeAction != NULL) {
		d->logModeActions.append(logModeAction);
	}

	delete logModeFactory;
}

LogMode* Globals::findLogMode(const QString& logModeName) {
	return d->logModes.value(logModeName);
}

QList<LogModeAction*> Globals::logModeActions() {
	return d->logModeActions;
}
