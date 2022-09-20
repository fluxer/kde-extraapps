/* This file is part of the KDE project

   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgwebinterface.h"

#include "settings.h"

#include <KMessageBox>
#include <KLocale>

DlgWebinterface::DlgWebinterface(KDialog *parent)
    : QWidget(parent),
      m_passwdstore(nullptr)
{
    setupUi(this);

    readConfig();
    
    connect(parent, SIGNAL(accepted()), SLOT(saveSettings()));
    connect(webinterfacePwd, SIGNAL(textChanged(QString)), SIGNAL(changed()));
}

DlgWebinterface::~DlgWebinterface()
{
}

void DlgWebinterface::readConfig()
{
    if (Settings::webinterfaceEnabled()) {
        if (!m_passwdstore) {
            m_passwdstore = new KPasswdStore(this);
            m_passwdstore->setStoreID("KGet");
        }

        if (m_passwdstore->openStore(winId())) {
            webinterfacePwd->setText(m_passwdstore->getPasswd("Webinterface", winId()));
        } else {
            KMessageBox::error(nullptr, i18n("Could not open KPasswdStore"));
        }
    }
}

void DlgWebinterface::saveSettings()
{
    if (kcfg_WebinterfaceEnabled->isChecked()) {
        if (!m_passwdstore) {
            m_passwdstore = new KPasswdStore(this);
            m_passwdstore->setStoreID("KGet");
        }

        if (m_passwdstore->openStore(winId())) {
            m_passwdstore->storePasswd("Webinterface", webinterfacePwd->text(), winId());
        }
    }
    emit saved();
}

#include "moc_dlgwebinterface.cpp"
