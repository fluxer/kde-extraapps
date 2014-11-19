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

#include "gui/editmountpointdialog.h"
#include "gui/editmountpointdialogwidget.h"

#include "core/partition.h"

#include <kmessagebox.h>
#include <kdebug.h>
#include <kguiitem.h>
#include <kstandardguiitem.h>

EditMountPointDialog::EditMountPointDialog(QWidget* parent, Partition& p) :
	KDialog(parent),
	m_Partition(p),
	m_DialogWidget(new EditMountPointDialogWidget(this, partition()))
{
	setMainWidget(&widget());
	setCaption(i18nc("@title:window", "Edit mount point for <filename>%1</filename>", p.deviceNode()));

	restoreDialogSize(KConfigGroup(KGlobal::config(), "editMountPointDialog"));
}

/** Destroys an EditMOuntOptionsDialog instance */
EditMountPointDialog::~EditMountPointDialog()
{
	KConfigGroup kcg(KGlobal::config(), "editMountPointDialog");
	saveDialogSize(kcg);
}

void EditMountPointDialog::accept()
{
	if (KMessageBox::warningContinueCancel(this,
			i18nc("@info", "<para>Are you sure you want to save the changes you made to the system table file <filename>/etc/fstab</filename>?</para>"
			"<para><warning>This will overwrite the existing file on your hard drive now. This <strong>can not be undone</strong>.</warning></para>"),
			i18nc("@title:window", "Really save changes?"),
			KGuiItem(i18nc("@action:button", "Save changes"), "arrow-right"),
			KStandardGuiItem::cancel(),
			"reallyWriteMountPoints") == KMessageBox::Cancel)
		return;

	if (widget().acceptChanges() && widget().writeMountpoints("/etc/fstab"))
		partition().setMountPoint(widget().editPath().text());

	KDialog::accept();
}
