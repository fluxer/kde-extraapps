/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHOWDESKTOP_H
#define SHOWDESKTOP_H

//#define MINIMIZE_ONLY

#include <Plasma/Applet>

class ShowDesktop : public Plasma::Applet
{
    Q_OBJECT
public:
    ShowDesktop(QObject *parent, const QVariantList &args);

    void init() final;

public slots:
    void minimizeAll();
#ifndef MINIMIZE_ONLY
    void reset();
    void delay();
#endif

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;
    
private slots:
    void iconSizeChanged(int group);

private:
    bool m_wm2ShowingDesktop;
#ifndef MINIMIZE_ONLY
    bool m_down;
    bool m_goingDown;
#endif
};

K_EXPORT_PLASMA_APPLET(showdesktop, ShowDesktop)

#endif // SHOWDESKTOP_H
