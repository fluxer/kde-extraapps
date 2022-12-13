/***************************************************************************
 *   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#if !defined(EXTERNALCOMMAND__H)

#define EXTERNALCOMMAND__H

#include "util/libpartitionmanagerexport.h"

#include <QProcess>
#include <QStringList>
#include <QString>

class Report;

/** An external command.

	Runs an external command as a child process.

	@author Volker Lanz <vl@fidra.de>
	@author Andrius Štikonas <andrius@stikonas.eu>
*/
class LIBPARTITIONMANAGERPRIVATE_EXPORT ExternalCommand : public QProcess
{
	Q_OBJECT
	Q_DISABLE_COPY(ExternalCommand)

	public:
		explicit ExternalCommand(const QString& cmd, const QStringList& args = QStringList());
		explicit ExternalCommand(Report& report, const QString& cmd, const QStringList& args = QStringList());
		~ExternalCommand();

	public:
		bool start(int timeout = 30000);
		bool waitFor(int timeout = 30000);
		bool run(int timeout = 30000);

		const QString& output() const { return m_Output; } /**< @return the command output */

		Report* report() { return m_Report; } /**< @return pointer to the Report or NULL */
		QString command() { return m_Command; } /**< @return the command to run */
		QStringList args() { return m_Args; } /**< @return the arguments */

	protected:
		void setup();

	protected slots:
		void onReadOutput();

	private:
		Report *m_Report;
		QString m_Command;
		QStringList m_Args;
		QString m_Output;
};

#endif
