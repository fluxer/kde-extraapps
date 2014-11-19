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

#ifndef _LOG_MODE_CONFIGURATION_H_
#define _LOG_MODE_CONFIGURATION_H_

#include <QObject>
#include <QList>
#include <QStringList>

#include "logFile.h"


class KSystemLogConfig;

class LogModeConfiguration : public QObject {
	
	Q_OBJECT
	
	public:
		LogModeConfiguration();
		
		virtual ~LogModeConfiguration();
		
		LogFile findGenericLogFile(const QString& file);
		QList<LogFile> findGenericLogFiles(const QStringList& files);
		
		QList<LogFile> findNoModeLogFiles(const QStringList& stringList);
		
	protected:
		KSystemLogConfig* configuration;
};

#endif // _LOG_MODE_CONFIGURATION_H_
