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

#ifndef _AUTHENTICATION_CONFIGURATION_WIDGET_H_
#define _AUTHENTICATION_CONFIGURATION_WIDGET_H_

#include <QHBoxLayout>
#include <QGroupBox>

#include <klocale.h>
#include <kurlrequester.h>

#include "globals.h"
#include "logging.h"
#include "fileList.h"

#include "logLevel.h"

#include "authenticationConfiguration.h"
#include "authenticationLogMode.h"

#include "logModeConfigurationWidget.h"


class AuthenticationConfigurationWidget : public LogModeConfigurationWidget {

	Q_OBJECT

	public:
		AuthenticationConfigurationWidget() :
			LogModeConfigurationWidget(i18n("Authentication Log"),QLatin1String(  AUTHENTICATION_MODE_ICON ), i18n("Authentication Log"))
			{

			QVBoxLayout* layout = new QVBoxLayout();
			this->setLayout(layout);

			//Authentication log file
			QGroupBox* authenticationBox=new QGroupBox(i18n("Authentication Log File"));
			QHBoxLayout* authenticationLayout = new QHBoxLayout();
			authenticationBox->setLayout(authenticationLayout);

			layout->addWidget(authenticationBox);

			authenticationLayout->addWidget(new QLabel(i18n("Authentication log file:")));

			authenticationUrlRequester=new KUrlRequester(authenticationBox);
			authenticationUrlRequester->setMode(KFile::File);

			authenticationUrlRequester->setToolTip(i18n("You can type or choose the authentication log file (example: <i>/var/log/auth.log</i>)."));
			authenticationUrlRequester->setWhatsThis(i18n("You can type or choose here the authentication log file. This file will be analyzed when you select the <b>Authentication log</b> menu. Generally, its name is <i>/var/log/auth.log</i>"));
			authenticationLayout->addWidget(authenticationUrlRequester);

			connect(authenticationUrlRequester, SIGNAL(textChanged(const QString&)), this, SIGNAL(configurationChanged()));

			layout->addStretch();

		}

		~AuthenticationConfigurationWidget() {

		}

	public slots:

		void saveConfig() {
			AuthenticationConfiguration* authenticationConfiguration = Globals::instance()->findLogMode(QLatin1String( AUTHENTICATION_LOG_MODE_ID ))->logModeConfiguration<AuthenticationConfiguration*>();

			authenticationConfiguration->setAuthenticationPath(authenticationUrlRequester->url().path());
		}

		void readConfig() {
			AuthenticationConfiguration* authenticationConfiguration = Globals::instance()->findLogMode(QLatin1String( AUTHENTICATION_LOG_MODE_ID ))->logModeConfiguration<AuthenticationConfiguration*>();

			authenticationUrlRequester->setUrl(KUrl(authenticationConfiguration->authenticationPath()));
		}

		void defaultConfig() {
			//TODO Find a way to read the configuration per default
			readConfig();
		}

	protected:
		bool isValid() const {
			if (authenticationUrlRequester->url().path().isEmpty()==false) {
				return true;
			}

			return false;

		}

	private:
		KUrlRequester* authenticationUrlRequester;

};

#endif // _AUTHENTICATION_CONFIGURATION_WIDGET_H_
