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

#ifndef _LOG_MODE_ACTION_H_
#define _LOG_MODE_ACTION_H_

#include <QObject>
#include <QList>
#include <QAction>

class LogModeActionPrivate;

class LogModeAction : public QObject {
	
	Q_OBJECT
	
public:
	
	enum Category {
			RootCategory,
	        ServicesCategory,
	        OthersCategory
	    };
    Q_DECLARE_FLAGS(Categories, Category)

    LogModeAction();
    
	virtual ~LogModeAction();
	
	virtual QList<QAction*> innerActions() = 0;
	
	virtual QAction* actionMenu() = 0;
	
	void setInToolBar(bool inToolBar);
	
	bool isInToolBar();
	
	void setCategory(Category category);
	
	Category category();
	
private:
	LogModeActionPrivate* const d;

};

#endif // _LOG_MODE_ACTION_H_
