/***************************************************************************
 *   Copyright (C) 2005-2014 by the Quassel Project                        *
 *   devel@quassel-irc.org                                                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************
 * Parts of this API have been shamelessly stolen from KDE's kaction.h     *
 ***************************************************************************/

#ifndef ACTION_H_
#define ACTION_H_

#include <KAction>

class Action : public KAction
{
    Q_OBJECT

public:
    explicit Action(QObject *parent);
    Action(const QString &text, QObject *parent, const QObject *receiver = 0, const char *slot = 0, const QKeySequence &shortcut = 0);
    Action(const QIcon &icon, const QString &text, QObject *parent, const QObject *receiver = 0, const char *slot = 0, const QKeySequence &shortcut = 0);

private:
    void init();
};



#endif
