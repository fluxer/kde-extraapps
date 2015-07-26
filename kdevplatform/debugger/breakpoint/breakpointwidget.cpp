/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "breakpointwidget.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>

#include <KIcon>
#include <KLocalizedString>
#include <KPassivePopup>
#include <KDebug>

#include "breakpointdetails.h"
#include "../breakpoint/breakpoint.h"
#include "../breakpoint/breakpointmodel.h"
#include <interfaces/idebugcontroller.h>
#include <util/autoorientedsplitter.h>

#define IF_DEBUG(x)
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <util/placeholderitemproxymodel.h>

using namespace KDevelop;

BreakpointWidget::BreakpointWidget(IDebugController *controller, QWidget *parent)
: QWidget(parent), m_firstShow(true), m_debugController(controller),
  breakpointDisableAll_(0), breakpointEnableAll_(0), breakpointRemoveAll_(0)
{
    setWindowTitle(i18nc("@title:window", "Debugger Breakpoints"));
    setWhatsThis(i18nc("@info:whatsthis", "Displays a list of breakpoints with "
                                          "their current status. Clicking on a "
                                          "breakpoint item allows you to change "
                                          "the breakpoint and will take you "
                                          "to the source in the editor window."));
    setWindowIcon( KIcon("process-stop") );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    QSplitter *s = new AutoOrientedSplitter(this);
    layout->addWidget(s);
    m_splitter = s;

    m_breakpointsView = new QTableView(s);
    m_breakpointsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_breakpointsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_breakpointsView->horizontalHeader()->setHighlightSections(false);
    m_breakpointsView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    details_ = new BreakpointDetails(s);

    s->setStretchFactor(0, 2);

    m_breakpointsView->verticalHeader()->hide();

    PlaceholderItemProxyModel* proxyModel = new PlaceholderItemProxyModel(this);
    proxyModel->setSourceModel(m_debugController->breakpointModel());
    proxyModel->setColumnHint(Breakpoint::LocationColumn, i18n("New code breakpoint ..."));
    proxyModel->setColumnHint(Breakpoint::ConditionColumn, i18n("Enter condition ..."));
    m_breakpointsView->setModel(proxyModel);
    connect(proxyModel, SIGNAL(dataInserted(int, QVariant)), SLOT(slotDataInserted(int, QVariant)));
    m_proxyModel = proxyModel;

    connect(m_breakpointsView, SIGNAL(activated(QModelIndex)), this, SLOT(slotOpenFile(QModelIndex)));
    connect(m_breakpointsView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(modelReset()), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(slotUpdateBreakpointDetail()));


    connect(m_debugController->breakpointModel(),
            SIGNAL(hit(KDevelop::Breakpoint*)),
            SLOT(breakpointHit(KDevelop::Breakpoint*)));

    connect(m_debugController->breakpointModel(),
            SIGNAL(error(KDevelop::Breakpoint*,QString,int)),
            SLOT(breakpointError(KDevelop::Breakpoint*,QString,int)));

    setupPopupMenu();
}

void BreakpointWidget::setupPopupMenu()
{
    popup_ = new QMenu(this);

    QMenu* newBreakpoint = popup_->addMenu( i18nc("New breakpoint", "&New") );
    newBreakpoint->setIcon(KIcon("list-add"));

    QAction* action = newBreakpoint->addAction(
        i18nc("Code breakpoint", "&Code"),
        this,
        SLOT(slotAddBlankBreakpoint()) );
    // Use this action also to provide a local shortcut
    action->setShortcut(QKeySequence(Qt::Key_B + Qt::CTRL,
                                        Qt::Key_C));
    addAction(action);

    newBreakpoint->addAction(
        i18nc("Data breakpoint", "Data &Write"),
        this, SLOT(slotAddBlankWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data read breakpoint", "Data &Read"),
        this, SLOT(slotAddBlankReadWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data access breakpoint", "Data &Access"),
        this, SLOT(slotAddBlankAccessWatchpoint()));

    QAction* breakpointDelete = popup_->addAction(
        KIcon("edit-delete"),
        i18n( "&Delete" ),
        this,
        SLOT(slotRemoveBreakpoint()));
    breakpointDelete->setShortcut(Qt::Key_Delete);
    breakpointDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(breakpointDelete);


    popup_->addSeparator();
    breakpointDisableAll_ = popup_->addAction(i18n("Disable &All"), this, SLOT(slotDisableAllBreakpoints()));
    breakpointEnableAll_ = popup_->addAction(i18n("&Enable All"), this, SLOT(slotEnableAllBreakpoints()));
    breakpointRemoveAll_ = popup_->addAction(i18n("&Remove All"), this, SLOT(slotRemoveAllBreakpoints()));

    connect(popup_,SIGNAL(aboutToShow()), this, SLOT(slotPopupMenuAboutToShow()));
}


void BreakpointWidget::contextMenuEvent(QContextMenuEvent* event)
{
    popup_->popup(event->globalPos());
}

void BreakpointWidget::slotPopupMenuAboutToShow()
{
    if (m_debugController->breakpointModel()->rowCount() < 2) {
        breakpointDisableAll_->setDisabled(true);
        breakpointEnableAll_->setDisabled(true);
        breakpointRemoveAll_->setDisabled(true);
    } else {
        breakpointRemoveAll_->setEnabled(true);
        bool allDisabled = true;
        bool allEnabled = true;
        for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
            Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
            if (bp->enabled())
                allDisabled = false;
            else
                allEnabled = false;
        }
        breakpointDisableAll_->setDisabled(allDisabled);
        breakpointEnableAll_->setDisabled(allEnabled);
    }
       
}

void BreakpointWidget::showEvent(QShowEvent *)
{
    if (m_firstShow) {
        QHeaderView* header = m_breakpointsView->horizontalHeader();

        for (int i = 0; i < m_breakpointsView->model()->columnCount(); ++i) {
            if(i == Breakpoint::LocationColumn){
                continue;
            }
            m_breakpointsView->resizeColumnToContents(i);
        }
        //for some reasons sometimes width can be very small about 200... But it doesn't matter as we use tooltip anyway.
        int width = m_breakpointsView->size().width();

        header->resizeSection(Breakpoint::LocationColumn, width > 400 ? width/2 : header->sectionSize(Breakpoint::LocationColumn)*2 );
        m_firstShow = false;
    }
}

void BreakpointWidget::edit(KDevelop::Breakpoint *n)
{
    QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(n, Breakpoint::LocationColumn));
    m_breakpointsView->setCurrentIndex(index);
    m_breakpointsView->edit(index);
}

void BreakpointWidget::slotDataInserted(int column, const QVariant& value)
{
    Breakpoint* breakpoint = m_debugController->breakpointModel()->addCodeBreakpoint();
    breakpoint->setData(column, value);
}

void BreakpointWidget::slotAddBlankBreakpoint()
{
    edit(m_debugController->breakpointModel()->addCodeBreakpoint());
}

void BreakpointWidget::slotAddBlankWatchpoint()
{
    edit(m_debugController->breakpointModel()->addWatchpoint());
}

void BreakpointWidget::slotAddBlankReadWatchpoint()
{
    edit(m_debugController->breakpointModel()->addReadWatchpoint());
}


void KDevelop::BreakpointWidget::slotAddBlankAccessWatchpoint()
{
    edit(m_debugController->breakpointModel()->addAccessWatchpoint());
}


void BreakpointWidget::slotRemoveBreakpoint()
{
    QItemSelectionModel* sel = m_breakpointsView->selectionModel();
    QModelIndexList selected = sel->selectedIndexes();
    IF_DEBUG( kDebug() << selected; )
    if (!selected.isEmpty()) {
        m_debugController->breakpointModel()->removeRow(selected.first().row());
    }
}

void BreakpointWidget::slotRemoveAllBreakpoints()
{
    m_debugController->breakpointModel()->removeRows(0, m_debugController->breakpointModel()->rowCount());
}


void BreakpointWidget::slotUpdateBreakpointDetail()
{
    QModelIndexList selected = m_breakpointsView->selectionModel()->selectedIndexes();
    IF_DEBUG( kDebug() << selected; )
    if (selected.isEmpty()) {
        details_->setItem(0);
    } else {
        details_->setItem(m_debugController->breakpointModel()->breakpoint(selected.first().row()));
    }
}

void BreakpointWidget::breakpointHit(KDevelop::Breakpoint* b)
{
    IF_DEBUG( kDebug() << b; )
    const QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(b, 0));
    m_breakpointsView->selectionModel()->select(
        index,
        QItemSelectionModel::Rows
        | QItemSelectionModel::ClearAndSelect);
}

void BreakpointWidget::breakpointError(KDevelop::Breakpoint* b, const QString& msg, int column)
{
    IF_DEBUG( kDebug() << b << msg << column; )

    // FIXME: we probably should prevent this error notification during
    // initial setting of breakpoint, to avoid a cloud of popups.
    if (!m_breakpointsView->isVisible())
        return;

    const QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(b, column));
    QPoint p = m_breakpointsView->visualRect(index).topLeft();
    p = m_breakpointsView->mapToGlobal(p);

    KPassivePopup *pop = new KPassivePopup(m_breakpointsView);
    pop->setPopupStyle(KPassivePopup::Boxed);
    pop->setAutoDelete(true);
    // FIXME: the icon, too.
    pop->setView("", msg);
    pop->setTimeout(-1);
    pop->show(p);
}

void BreakpointWidget::slotOpenFile(const QModelIndex& breakpointIdx)
{
    if (breakpointIdx.column() != Breakpoint::LocationColumn){
        return;
    }
    Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(breakpointIdx.row());
    if (!bp || bp->line() == -1 || bp->url().isEmpty() ){
        return;
    }

   ICore::self()->documentController()->openDocument(bp->url().pathOrUrl(KUrl::RemoveTrailingSlash), KTextEditor::Cursor(bp->line(), 0), IDocumentController::DoNotFocus);
}

void BreakpointWidget::slotDisableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    }
}

void BreakpointWidget::slotEnableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Checked);
    }
}


#include "moc_breakpointwidget.cpp"
