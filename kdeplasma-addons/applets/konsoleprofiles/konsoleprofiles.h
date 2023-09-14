/*  This file is part of the KDE project
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KONSOLEPROFILES_H
#define KONSOLEPROFILES_H

#include <Plasma/PopupApplet>

class KonsoleProfilesWidget;

class KonsoleProfilesApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    KonsoleProfilesApplet(QObject *parent, const QVariantList &args);
    ~KonsoleProfilesApplet();

    // Plasma::Applet reimplementation
    void init() final;
    // Plasma::PopupApplet reimplementation
    QGraphicsWidget* graphicsWidget() final;

private:
    friend KonsoleProfilesWidget;
    KonsoleProfilesWidget *m_konsoleprofileswidget;
};

K_EXPORT_PLASMA_APPLET(konsoleprofiles, KonsoleProfilesApplet)

#endif // KONSOLEPROFILES_H
