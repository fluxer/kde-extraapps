/**
 * Copyright (C) 2004 Michael Pyne <mpyne@kde.org>
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

#ifndef FILERENAMEROPTIONS_H
#define FILERENAMEROPTIONS_H

#include <kdialog.h>
#include "ui_filerenameroptionsbase.h"
#include "tagrenameroptions.h"

/**
 * Base widget implementing the options for a particular tag type.
 *
 * @author Michael Pyne <mpyne@kde.org>
 */
class FileRenamerTagOptions : public QWidget, public Ui::FileRenamerTagOptionsBase
{
    Q_OBJECT
public:
    FileRenamerTagOptions(QWidget *parent, const TagRenamerOptions &options);
    virtual ~FileRenamerTagOptions() {}

    const TagRenamerOptions &options() const { return m_options; }

protected slots:
    virtual void slotBracketsChanged();
    virtual void slotTrackWidthChanged();
    virtual void slotEmptyActionChanged();

private:
    TagRenamerOptions m_options;
};

/**
 * This defines the dialog that actually gets the options from the user.
 *
 * @author Michael Pyne <mpyne@kde.org>
 */
class TagOptionsDialog : public KDialog
{
    Q_OBJECT

public:
    TagOptionsDialog(QWidget *parent, const TagRenamerOptions &options, unsigned categoryNumber);

    const TagRenamerOptions &options() const { return m_options; }

    protected slots:
    virtual void accept();

private:

    // Private methods

    void loadConfig(); // Loads m_options from KConfig
    void saveConfig(); // Saves m_options to KConfig

    // Private members

    FileRenamerTagOptions *m_widget;
    TagRenamerOptions m_options;
    unsigned m_categoryNumber;
};

#endif /* FILERENAMEROPTIONS_H */

// vim: set et sw=4 tw=0 sta:
