/**
 * Copyright (C) 2003 Stefan Asserhall <stefan.asserhall@telia.com>
 * Copyright (C) 2007, 2008 Michael Pyne <mpyne@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEYDIALOG_H
#define KEYDIALOG_H

#include <KDialog>

class QButtonGroup;

class KActionCollection;
class KShortcutsEditor;

class KeyDialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a KeyDialog called @p name as a child of @p parent.
     */
    explicit KeyDialog(KActionCollection *actionCollection, QWidget *parent = 0);

    /**
     * Destructor. Deletes all resources used by a KeyDialog object.
     */
    virtual ~KeyDialog();

    /**
     * This is a member function, provided to allow inserting both global
     * accelerators and actions.
     */
    static int configure(KActionCollection *actionCollection, QWidget *parent = 0);

    /**
     * This is a member function, provided to create a global accelerator with
     * standard keys.
     */
    static void setupActionShortcut(const QString &actionName);

private:

    /**
     * Groups of keys that can be selected in the dialog.  If you add or remove
     * a group you must also adjust struct KeyInfo::shortcut to have enough
     * entries for all groups, and then adjust the keyInfo[] array.
     */
    enum KeyGroup { NoKeys = 0, StandardKeys = 1, MultimediaKeys = 2 };

    int configure();

private slots:
    void slotKeys(int group);
    void slotDefault();

private:
    KActionCollection *m_actionCollection;
    KShortcutsEditor  *m_pKeyChooser;
    QButtonGroup      *m_group;

    struct KeyInfo;

    static const KeyInfo keyInfo[];
    static const uint    keyInfoCount;
};

#endif // KEYDIALOG_H

// vim: set et sw=4 tw=0 sta:
