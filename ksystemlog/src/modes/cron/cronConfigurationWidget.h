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

#ifndef _CRON_CONFIGURATION_WIDGET_H_
#define _CRON_CONFIGURATION_WIDGET_H_

#include "logModeConfigurationWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include <klocale.h>

#include "globals.h"
#include "logging.h"

#include "fileList.h"

#include "logLevel.h"

#include "cronConfiguration.h"
#include "cronLogMode.h"

class CronConfigurationWidget : public LogModeConfigurationWidget {

	Q_OBJECT

	public:
		CronConfigurationWidget() :
			LogModeConfigurationWidget(i18n("Cron Log"),QLatin1String( CRON_MODE_ICON ), i18n("Cron Log"))
			{

			QVBoxLayout* layout = new QVBoxLayout();
			this->setLayout(layout);

			QString description = i18n("<p>These files will be analyzed to show the <b>Cron Logs</b> (i.e. planned tasks logs). <a href='man:/cron'>More information...</a></p>");

			fileList = new FileList(this, description);

			connect(fileList, SIGNAL(fileListChanged()), this, SIGNAL(configurationChanged()));

			layout->addWidget(fileList);

			processFilterGroup = new QGroupBox(i18n("Enable Process Filtering"));
			processFilterGroup->setCheckable(true);

			connect(processFilterGroup, SIGNAL(clicked(bool)), this, SLOT(toggleProcessFilterEnabling(bool)));
			connect(processFilterGroup, SIGNAL(clicked(bool)), this, SIGNAL(configurationChanged()));

			layout->addWidget(processFilterGroup);

			QHBoxLayout* processFilterLayout = new QHBoxLayout();

			processFilterGroup->setLayout(processFilterLayout);

			processFilterLabel = new QLabel(i18n("Only keeps lines which matches this process :"));
			processFilter = new QLineEdit(this);

			processFilterLabel->setBuddy(processFilter);
			connect(processFilter, SIGNAL(textEdited(const QString&)), this, SIGNAL(configurationChanged()));

			processFilterLayout->addWidget(processFilterLabel);
			processFilterLayout->addWidget(processFilter);

		}

		virtual ~CronConfigurationWidget() {

		}

		bool isValid() const {
			if (fileList->isEmpty() == true) {
				logDebug() << "Cron configuration not valid" << endl;
				return false;
			}

			if (processFilterGroup->isChecked() && processFilter->text().isEmpty()) {
				logDebug() << "Cron configuration not valid" << endl;
				return false;
			}

			logDebug() << "Cron configuration valid" << endl;
			return true;
		}

		void saveConfig() {
			logDebug() << "Saving config from Cron Options..." << endl;

			CronConfiguration* cronConfiguration = Globals::instance()->findLogMode(QLatin1String( CRON_LOG_MODE_ID ))->logModeConfiguration<CronConfiguration*>();
			cronConfiguration->setCronPaths(fileList->paths());

			if (processFilterGroup->isChecked() == false) {
				cronConfiguration->setProcessFilter(QLatin1String( "" ));
			}
			else {
				cronConfiguration->setProcessFilter(processFilter->text());
			}
		}

		void readConfig() {
			CronConfiguration* cronConfiguration = Globals::instance()->findLogMode(QLatin1String( CRON_LOG_MODE_ID ))->logModeConfiguration<CronConfiguration*>();

			fileList->removeAllItems();

			fileList->addPaths(cronConfiguration->cronPaths());

			if (cronConfiguration->processFilter().isEmpty()) {
				processFilterGroup->setChecked(false);
			}
			else {
				processFilterGroup->setChecked(true);
				processFilter->setText(cronConfiguration->processFilter());
			}


		}

		void defaultConfig() {
			//TODO Find a way to read the configuration per default
			readConfig();
		}

	private slots:
		void toggleProcessFilterEnabling(bool enabled) {
			processFilter->setEnabled(enabled);
			processFilterLabel->setEnabled(enabled);
		}

	private:
		FileList* fileList;

		QGroupBox* processFilterGroup;

		QLineEdit* processFilter;
		QLabel* processFilterLabel;


};

#endif // _CRON_CONFIGURATION_WIDGET_H_
