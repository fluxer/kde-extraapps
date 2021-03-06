/*
 *  Copyright (c) 1998 Denis Perchine <dyp@perchine.com>
 *  Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>
 *  Former maintainer: Adriaan de Groot <groot@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef _KU_CONFIGDLG_H_
#define _KU_CONFIGDLG_H_

#include <QList>
#include <QProgressDialog>

#include <kconfigdialog.h>
#include <kprogressdialog.h>
#include <kio/job.h>

QT_BEGIN_NAMESPACE
class Ui_KU_FilesSettings;
QT_END_NAMESPACE

namespace KIO { class Job; }

class KU_ConfigDlg : public KConfigDialog {
  Q_OBJECT
public:
  KU_ConfigDlg( KConfigSkeleton *config, QWidget* parent, const char * name = 0 );
  ~KU_ConfigDlg();
private:
  Ui_KU_FilesSettings *fileui;
  QString mErrorMsg;
};

#endif // _KU_CONFIGDLG_H_
