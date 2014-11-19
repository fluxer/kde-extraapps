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

#ifndef _FILE_LIST_H_
#define _FILE_LIST_H_

#include <QWidget>
#include <kurl.h>

#include "fileListHelper.h"

#include "ui_fileListBase.h"

class QVBoxLayout;

class FileList : public QWidget, public Ui::FileListBase {

	Q_OBJECT
	
	public:
		FileList(QWidget* parent, const QString& descriptionText);
		virtual ~FileList();
		
		int count() const;
		
		bool isEmpty() const;
		
		QStringList paths();

		void addPaths(const QStringList& paths);
		
	public slots:
		void removeAllItems();
		
	signals:
		void fileListChanged();

	private slots:
		
		void updateButtons();
		
		void removeSelectedItem();
		void moveUpItem();
		void moveDownItem();
		
	protected slots:
		virtual void addItem();
		
		void modifyItem();
		void modifyItem(QListWidgetItem* item);

	protected:
		void removeItem(int id);
		
		void moveItem(int direction);
		void unselectAllItems();

		/**
		 * Convenient method which returns the layout which manage the button list
		 */
		QVBoxLayout* buttonsLayout();

		FileListHelper fileListHelper;
};

#endif //_FILE_LIST_H_
