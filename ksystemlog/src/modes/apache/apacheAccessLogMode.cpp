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

#include "apacheAccessLogMode.h"

#include <QList>

#include <kicon.h>
#include <klocale.h>

#include "logging.h"
#include "logMode.h"

#include "apacheAccessAnalyzer.h"
#include "apacheAccessItemBuilder.h"
#include "apacheConfigurationWidget.h"
#include "apacheConfiguration.h"


ApacheAccessLogMode::ApacheAccessLogMode(ApacheConfiguration* apacheConfiguration, ApacheConfigurationWidget* apacheConfigurationWidget) :
	LogMode(QLatin1String( APACHE_ACCESS_LOG_MODE_ID ), i18n("Apache Access Log"), QLatin1String( APACHE_ACCESS_MODE_ICON )) {

	d->logModeConfiguration = apacheConfiguration;
	d->logModeConfigurationWidget = apacheConfigurationWidget;

	d->itemBuilder = new ApacheAccessItemBuilder();

	//Apache Log Action
	d->action = createDefaultAction();
	d->action->setToolTip(i18n("Display the Apache Access log."));
	d->action->setWhatsThis(i18n("Displays the Apache Access log in the current tab. Apache is the main used Web server in the world. This log saves all requests performed by the Apache web server."));

}

ApacheAccessLogMode::~ApacheAccessLogMode() {

}

Analyzer* ApacheAccessLogMode::createAnalyzer() {
	return new ApacheAccessAnalyzer(this);
}

QList<LogFile> ApacheAccessLogMode::createLogFiles() {
	ApacheConfiguration* apacheConfiguration = logModeConfiguration<ApacheConfiguration*>();
	return apacheConfiguration->findNoModeLogFiles(apacheConfiguration->apacheAccessPaths());
}
