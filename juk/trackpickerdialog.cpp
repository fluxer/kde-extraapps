/**
 * Copyright (C) 2003-2004 Scott Wheeler <wheeler@kde.org>
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

#include <config-juk.h>

#include "trackpickerdialog.h"

#if HAVE_TUNEPIMP


#include <k3listview.h>
#include <klocale.h>

#define NUMBER(x) (x == 0 ? QString::null : QString::number(x))	//krazy:exclude=nullstrassign for old broken gcc

class TrackPickerItem : public K3ListViewItem
{
public:
    TrackPickerItem(K3ListView *parent, const KTRMResult &result) :
        K3ListViewItem(parent, parent->lastChild(),
                      result.title(), result.artist(), result.album(),
                      NUMBER(result.track()), NUMBER(result.year())),
        m_result(result) {}
    KTRMResult result() const { return m_result; }

private:
    KTRMResult m_result;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

TrackPickerDialog::TrackPickerDialog(const QString &name,
                                     const KTRMResultList &results,
                                     QWidget *parent) :
    KDialog(parent)
{
    setObjectName(name.toAscii());
    setModal(true);
    setCaption(i18n("Internet Tag Guesser"));
    setButtons(Ok | Cancel);
    showButtonSeparator(true);

    m_base = new TrackPickerDialogBase(this);
    setMainWidget(m_base);

    m_base->fileLabel->setText(name);
    m_base->trackList->setSorting(-1);

    for(KTRMResultList::ConstIterator it = results.begin(); it != results.end(); ++it)
        new TrackPickerItem(m_base->trackList, *it);

    m_base->trackList->setSelected(m_base->trackList->firstChild(), true);

    connect(m_base->trackList, SIGNAL(doubleClicked(Q3ListViewItem*,QPoint,int)),
            this, SLOT(accept()));

    setMinimumWidth(qMax(400, width()));
}

TrackPickerDialog::~TrackPickerDialog()
{

}

KTRMResult TrackPickerDialog::result() const
{
    if(m_base->trackList->selectedItem())
        return static_cast<TrackPickerItem *>(m_base->trackList->selectedItem())->result();
    else
        return KTRMResult();
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

int TrackPickerDialog::exec()
{
    int dialogCode = KDialog::exec();

    // Only return true if an item was selected.

    if(m_base->trackList->selectedItem())
        return dialogCode;
    else
        return Rejected;
}

#include "trackpickerdialog.moc"

#endif // HAVE_TUNEPIMP

// vim: set et sw=4 tw=0 sta:
