/*
 *   Copyright (C) 2007, 2008, 2009, 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LANCELOT_APPLET_H
#define LANCELOT_APPLET_H

#include <QString>
#include <QPainter>

#include <KConfigDialog>

#include <Plasma/Applet>

#include "LancelotAppletConfig.h"
#include "../application/LancelotConfig.h"

class LancelotApplet: public Plasma::Applet
{
    Q_OBJECT

public:
    LancelotApplet(QObject * parent, const QVariantList &args);
    ~LancelotApplet();

    void init();
    virtual QList< QAction * > contextualActions();

public Q_SLOTS:
    void configChanged();

protected Q_SLOTS:
    void showLancelot();
    void toggleLancelot();
    void showLancelotSection(const QString & section);
    void toggleLancelotSection(const QString & section);
    void updateImmutability(const Plasma::ImmutabilityType immutable);
    void configAccepted();

protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
    virtual void createConfigurationInterface(KConfigDialog *parent);
    virtual void constraintsEvent(Plasma::Constraints
            constraints);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

protected Q_SLOTS:
    void applyConfig();
    void loadConfig();
    void saveConfig();
    void iconSizeChanged(int group);

private:
    class Private;
    Private * const d;

    LancelotAppletConfig m_config;
    LancelotConfig       m_configMenu;
};

K_EXPORT_PLASMA_APPLET(lancelot-launcher, LancelotApplet)

#endif /* LANCELOT_LAUNCHER_APPLET_H_ */
