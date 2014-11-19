/***************************************************************************
 *   Copyright (C) 2012 by Volker Lanz <vl@fidra.de>                       *
 *   Copyright (C) 2013 by Andrius Štikonas <stikonas@gmail.com>           *
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

#include "fs/luks.h"

#include "gui/decryptluksdialog.h"

#include "util/capacity.h"
#include "util/externalcommand.h"

#include <QString>
#include <QUuid>

#include <KLocale>

namespace FS
{
	FileSystem::CommandSupportType luks::m_GetUsed = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_GetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Create = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Grow = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Shrink = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Move = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Check = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Copy = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_Backup = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_SetLabel = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_UpdateUUID = FileSystem::cmdSupportNone;
	FileSystem::CommandSupportType luks::m_GetUUID = FileSystem::cmdSupportNone;

	luks::luks(qint64 firstsector, qint64 lastsector, qint64 sectorsused, const QString& label) :
		FileSystem(firstsector, lastsector, sectorsused, label, FileSystem::Luks)
	{
	}

	void luks::init()
	{
		m_UpdateUUID = findExternal("cryptsetup") ? cmdSupportFileSystem : cmdSupportNone;
		m_Copy = cmdSupportCore;
		m_Move = cmdSupportCore;
		m_Backup = cmdSupportCore;
		m_GetUUID = findExternal("cryptsetup") ? cmdSupportFileSystem : cmdSupportNone;
	}

	bool luks::supportToolFound() const
	{
		return
// 			m_GetUsed != cmdSupportNone &&
// 			m_GetLabel != cmdSupportNone &&
// 			m_SetLabel != cmdSupportNone &&
// 			m_Create != cmdSupportNone &&
// 			m_Check != cmdSupportNone &&
			m_UpdateUUID != cmdSupportNone &&
// 			m_Grow != cmdSupportNone &&
// 			m_Shrink != cmdSupportNone &&
			m_Copy != cmdSupportNone &&
			m_Move != cmdSupportNone &&
			m_Backup != cmdSupportNone &&
			m_GetUUID != cmdSupportNone;
	}

	FileSystem::SupportTool luks::supportToolName() const
	{
		return SupportTool("cryptsetup", KUrl("https://code.google.com/p/cryptsetup/"));
	}

	qint64 luks::minCapacity() const
	{
		 return 3 * Capacity::unitFactor(Capacity::Byte, Capacity::MiB);
	}

	QString luks::mountTitle() const
	{
		return i18nc("@title:menu", "Decrypt");
	}

	QString luks::unmountTitle() const
	{
		return i18nc("@title:menu", "Deactivate");
	}

	bool luks::mount(const QString& deviceNode)
	{
		QPointer<DecryptLuksDialog> dlg = new DecryptLuksDialog(deviceNode);

		if (dlg->exec() == KDialog::Accepted)
		{
			std::vector<QString> commands;
			commands.push_back("echo");
			commands.push_back("cryptsetup");
			std::vector<QStringList> args;
			args.push_back(QStringList() << dlg->luksPassphrase().text());
			args.push_back(QStringList() << "luksOpen" << deviceNode << dlg->luksName().text());
			ExternalCommand cmd(commands, args);
			delete dlg;
			return cmd.run(-1) && cmd.exitCode() == 0;
		}

		delete dlg;
		return false;
	}

	bool luks::unmount(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksClose" << mapperName(deviceNode));
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	QString luks::readUUID(const QString& deviceNode) const
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksUUID" << deviceNode);
		if (cmd.run())
		{
			return cmd.output().simplified();
		}
		return "---";
	}

	bool luks::updateUUID(Report& report, const QString& deviceNode) const
	{
		QUuid uuid = QUuid::createUuid();

		ExternalCommand cmd(report, "cryptsetup", QStringList() << "luksUUID" << deviceNode << "--uuid" << uuid);
		return cmd.run(-1) && cmd.exitCode() == 0;
	}

	QString luks::mapperName (const QString& deviceNode)
	{
		ExternalCommand cmd("find", QStringList() << "/dev/mapper/" << "-exec" << "cryptsetup" << "status" << "{}" << ";");
		if (cmd.run())
		{
			QRegExp rxDeviceName("(/dev/mapper/[A-Za-z0-9-/]+) is active[A-Za-z0-9- \\.\n]+[A-Za-z0-9-: \n]+" + deviceNode);
			if (rxDeviceName.indexIn(cmd.output()) > -1)
				return rxDeviceName.cap(1);
		}
		return "";
	}

	QString luks::getCipherName(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxCipherName("(?:Cipher name:\\s+)([A-Za-z0-9-]+)");
			if (rxCipherName.indexIn(cmd.output()) > -1)
				return rxCipherName.cap(1);
		}
		return "---";
	}

	QString luks::getCipherMode(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxCipherMode("(?:Cipher mode:\\s+)([A-Za-z0-9-]+)");
			if (rxCipherMode.indexIn(cmd.output()) > -1)
				return rxCipherMode.cap(1);
		}
		return "---";
	}

	QString luks::getHashName(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxHash("(?:Hash spec:\\s+)([A-Za-z0-9-]+)");
			if (rxHash.indexIn(cmd.output()) > -1)
				return rxHash.cap(1);
		}
		return "---";
	}

	QString luks::getKeySize(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxKeySize("(?:MK bits:\\s+)(\\d+)");
			if (rxKeySize.indexIn(cmd.output()) > -1)
				return rxKeySize.cap(1);
		}
		return "---";
	}

	QString luks::getPayloadOffset(const QString& deviceNode)
	{
		ExternalCommand cmd("cryptsetup", QStringList() << "luksDump" << deviceNode);
		if (cmd.run())
		{
			QRegExp rxPayloadOffset("(?:Payload offset:\\s+)(\\d+)");
			if (rxPayloadOffset.indexIn(cmd.output()) > -1)
				return rxPayloadOffset.cap(1);
		}
		return "---";
	}
}
