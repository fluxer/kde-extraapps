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

#ifndef _XORG_CONFIGURATION_H_
#define _XORG_CONFIGURATION_H_

#include <QStringList>

#include "logModeConfiguration.h"

#include "logging.h"
#include "defaults.h"
#include "ksystemlogConfig.h"

#include "xorgLogMode.h"

class XorgConfigurationPrivate {
public:
	QStringList xorgPaths;
};

class XorgConfiguration : public LogModeConfiguration {

	Q_OBJECT

	public:
		XorgConfiguration() :
			d(new XorgConfigurationPrivate()) {

			configuration->setCurrentGroup(QLatin1String( "XorgLogMode" ));

			QStringList defaultXorgPaths;
			defaultXorgPaths << QLatin1String( "/var/log/Xorg.0.log" );
			configuration->addItemStringList(QLatin1String( "LogFilesPaths" ), d->xorgPaths, defaultXorgPaths, QLatin1String( "LogFilesPaths" ));

		}

		virtual ~XorgConfiguration() {
			delete d;
		}

		QStringList xorgPaths() const {
			return d->xorgPaths;
		}

		void setXorgPaths(const QStringList& xorgPaths) {
			d->xorgPaths = xorgPaths;
		}

	private:
		XorgConfigurationPrivate* const d;

};

#endif // _XORG_CONFIGURATION_H_
