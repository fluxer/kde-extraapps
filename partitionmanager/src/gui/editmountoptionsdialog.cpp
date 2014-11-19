/***************************************************************************
 *   Copyright (C) 2009,2010 by Volker Lanz <vl@fidra.de>                  *
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

#include "gui/editmountoptionsdialog.h"
#include "gui/editmountoptionsdialogwidget.h"

#include <QString>
#include <QStringList>

#include <kdebug.h>

EditMountOptionsDialog::EditMountOptionsDialog(QWidget* parent, const QStringList& options) :
	KDialog(parent),
	m_DialogWidget(new EditMountOptionsDialogWidget(this, options))
{
	setMainWidget(&widget());
	setCaption(i18nc("@title:window", "Edit additional mount options"));

	restoreDialogSize(KConfigGroup(KGlobal::config(), "editMountOptionsDialog"));
}

/** Destroys an EditMOuntOptionsDialog instance */
EditMountOptionsDialog::~EditMountOptionsDialog()
{
	KConfigGroup kcg(KGlobal::config(), "editMountOptionsDialog");
	saveDialogSize(kcg);
}

QStringList EditMountOptionsDialog::options()
{
	QStringList rval;
	const QStringList lines = widget().editOptions().toPlainText().split('\n');
	foreach (const QString& line, lines)
		rval.append(line.simplified().toLower());
	return rval;
}
