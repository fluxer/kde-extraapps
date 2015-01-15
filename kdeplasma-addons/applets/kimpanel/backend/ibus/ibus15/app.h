/*
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APP_H
#define APP_H
#include <ibus.h>

#include <QApplication>
#include <QPair>
#include <QMap>
#include <QByteArray>
#include "panel.h"

class App : public QApplication {
    Q_OBJECT
public:
    typedef QPair< uint, uint > TriggerKey;
    App(int argc, char** argv);
    virtual ~App();
    void finalize();
    virtual bool x11EventFilter(XEvent* event );
    void setTriggerKeys(QList< TriggerKey > triggersList);
    void setDoGrab(bool doGrab);
    bool keyboardGrabbed() { return m_keyboardGrabbed; }
    void init();
    void nameAcquired();
    void nameLost();
    QByteArray normalizeIconName(const QByteArray& icon) const;
private Q_SLOTS:
    void clean();
    void grabKey();
    void ungrabKey();
    uint getPrimaryModifier(uint state);
    void keyRelease(const XEvent& event);
    void accept();
    void ungrabXKeyboard();
    bool grabXKeyboard();
private:
    bool m_init;
    IBusBus *m_bus;
    IBusPanelImpanel *m_impanel;
    QList< QPair< uint, uint > > m_triggersList;
    bool m_keyboardGrabbed;
    bool m_doGrab;
    QMap<QByteArray, QByteArray> m_iconMap;
};

#endif // APP_H
