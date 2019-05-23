/***************************************************************************
 *   Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                       *
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

#if !defined(SCANPROGRESSDIALOG__H)

#define SCANPROGRESSDIALOG__H

#include <kprogressdialog.h>

#include <QShowEvent>

class ScanProgressDialog : public KProgressDialog
{
	Q_OBJECT

	public:
		ScanProgressDialog(QWidget* parent);

	protected:
		virtual void showEvent(QShowEvent* e);

	public:
		void setProgress(quint32 p) { progressBar()->setValue(p); }
		void setDeviceName(const QString& d);
};

#endif

