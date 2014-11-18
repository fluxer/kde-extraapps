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

#include <kdebug.h>

#include <kcombobox.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <ktabwidget.h>
#include <klocale.h>

#include "ku_configdlg.h"
#include "ku_misc.h"

#include "ui_ku_generalsettings.h"
#include "ui_ku_filessettings.h"
#include "ui_ku_passwordpolicy.h"

KU_ConfigDlg::KU_ConfigDlg( KConfigSkeleton *config, QWidget *parent, const char *name ) :
    KConfigDialog( parent, QLatin1String( name ), config)
{
  setFaceType(List);
  setButtons(Default|Ok|Apply|Cancel|Help);
  setDefaultButton(Ok);
  setModal(true);
  KTabWidget *page1 = new KTabWidget( this );
  {
    Ui::KU_GeneralSettings ui;
    QFrame *frame = new QFrame ( page1 );
    ui.setupUi( frame );
    ui.kcfg_shell->addItem( i18n("<Empty>" ) );
    ui.kcfg_shell->addItems( readShells() );
    page1->addTab( frame, i18n("Connection") );
  }
  {
    Ui::KU_PasswordPolicy ui;
    QFrame *frame = new QFrame ( page1 );
    ui.setupUi( frame );
    page1->addTab( frame, i18n("Password Policy") );
    ui.kcfg_smax->setSuffix(ki18np(" day", " days"));
    ui.kcfg_sinact->setSuffix(ki18np(" day", " days"));
    ui.kcfg_swarn->setSuffix(ki18np(" day", " days"));
    ui.kcfg_smin->setSuffix(ki18np(" day", " days"));
  }
  addPage( page1, i18n("General"), QLatin1String( "kuser" ), i18n("General Settings") );

  {
    QFrame *page2 = new QFrame( this );
    fileui = new Ui::KU_FilesSettings();
    fileui->setupUi( page2 );
    addPage( page2, i18n("Files"), QLatin1String( "document-properties" ), i18n("File Source Settings") );
  }

  setHelp(QString(),QLatin1String( "kuser" ));
}

KU_ConfigDlg::~KU_ConfigDlg()
{
  delete fileui;
}

#include "ku_configdlg.moc"
