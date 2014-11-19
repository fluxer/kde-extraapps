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

#ifndef _STATUS_BAR_
#define _STATUS_BAR_


#include <kstatusbar.h>

class QString;



namespace KSystemLog {

class StatusBarPrivate;

/**
 * Status Bar
 */
class StatusBar : public KStatusBar {
	
	Q_OBJECT
	
public:
	explicit StatusBar(QWidget* parent);
	
	virtual ~StatusBar();
	
	void changeLineCountMessage(const QString& lineCountMessage);
	void changeLastModification(const QTime& lastModification);
	
	void changeMessage(const QString& message);
	
private slots:
	void toggleHistory();
	void selectLastHistory();
private:
	
	StatusBarPrivate* const d;
	
};

}

#endif // _STATUS_BAR_
