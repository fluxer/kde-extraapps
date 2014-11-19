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

#include "postfixLogMode.h"

#include <QList>

#include <kicon.h>
#include <klocale.h>

#include "logging.h"
#include "logMode.h"

#include "postfixAnalyzer.h"
#include "postfixConfigurationWidget.h"
#include "postfixConfiguration.h"

#include "logModeItemBuilder.h"

PostfixLogMode::PostfixLogMode() :
	LogMode(QLatin1String( POSTFIX_LOG_MODE_ID ), i18n("Postfix Log"),QLatin1String( POSTFIX_MODE_ICON )) {

	d->logModeConfiguration = new PostfixConfiguration();

	d->logModeConfigurationWidget = new PostfixConfigurationWidget();

	d->itemBuilder = new LogModeItemBuilder();

	d->action = createDefaultAction();
	d->action->setToolTip(i18n("Display the Postfix log."));
	d->action->setWhatsThis(i18n("Displays the Postfix log in the current tab. Postfix is the most known and used mail server in the Linux world."));

}

PostfixLogMode::~PostfixLogMode() {

}

Analyzer* PostfixLogMode::createAnalyzer() {
	return new PostfixAnalyzer(this);
}

QList<LogFile> PostfixLogMode::createLogFiles() {
	return logModeConfiguration<PostfixConfiguration*>()->findGenericLogFiles();
}
