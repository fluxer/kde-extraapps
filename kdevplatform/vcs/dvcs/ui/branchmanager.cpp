/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "branchmanager.h"

#include <QListWidget>
#include <QInputDialog>

#include <KJob>
#include <KMessageBox>
#include <KDebug>

#include "../dvcsjob.h"
#include "../dvcsplugin.h"
#include <vcs/models/brancheslistmodel.h>
#include "ui_branchmanager.h"

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

using namespace KDevelop;

BranchManager::BranchManager(const QString& repository, KDevelop::DistributedVersionControlPlugin* executor, QWidget *parent)
    : KDialog(parent)
    , m_repository(repository)
    , m_dvcPlugin(executor)
{
    setButtons(KDialog::Close);
    setWindowTitle(i18n("Branch Manager"));

    m_ui = new Ui::BranchDialogBase;
    QWidget* w = new QWidget(this);
    m_ui->setupUi(w);
    setMainWidget(w);

    m_model = new BranchesListModel(this);
    m_model->initialize(m_dvcPlugin, repository);
    m_ui->branchView->setModel(m_model);

    QString branchName = m_model->currentBranch();
    // apply initial selection
    QList< QStandardItem* > items = m_model->findItems(branchName);
    if (!items.isEmpty()) {
        m_ui->branchView->setCurrentIndex(items.first()->index());
    }

    m_ui->newButton->setIcon(KIcon("list-add"));
    connect(m_ui->newButton, SIGNAL(clicked()), this, SLOT(createBranch()));
    m_ui->deleteButton->setIcon(KIcon("list-remove"));
    connect(m_ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteBranch()));
    m_ui->renameButton->setIcon(KIcon("edit-rename"));
    connect(m_ui->renameButton, SIGNAL(clicked()), this, SLOT(renameBranch()));
    m_ui->checkoutButton->setIcon(KIcon("dialog-ok-apply"));
    connect(m_ui->checkoutButton, SIGNAL(clicked()), this, SLOT(checkoutBranch()));

    // checkout branch on double-click
    connect(m_ui->branchView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(checkoutBranch()));
}

BranchManager::~BranchManager()
{
    delete m_ui;
}

void BranchManager::createBranch()
{
    const QModelIndex currentBranchIdx = m_ui->branchView->currentIndex();
    if (!currentBranchIdx.isValid()) {
        KMessageBox::messageBox(this, KMessageBox::Error,
                                i18n("You must select a base branch from the list before creating a new branch."));
        return;
    }
    QString baseBranch = currentBranchIdx.data().toString();
    bool branchNameEntered = false;
    QString newBranch = QInputDialog::getText(this, i18n("New branch"), i18n("Name of the new branch:"),
            QLineEdit::Normal, QString(), &branchNameEntered);
    if (!branchNameEntered)
        return;

    if (!m_model->findItems(newBranch).isEmpty())
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Branch \"%1\" already exists.\n"
                                        "Please, choose another name.",
                                        newBranch));
    }
    else
        m_model->createBranch(baseBranch, newBranch);
}

void BranchManager::deleteBranch()
{
    QString baseBranch = m_ui->branchView->selectionModel()->selection().indexes().first().data().toString();

    if (baseBranch == m_model->currentBranch())
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                    i18n("Currently at the branch \"%1\".\n"
                                            "To remove it, please change to another branch.",
                                            baseBranch));
        return;
    }

    int ret = KMessageBox::messageBox(this, KMessageBox::WarningYesNo, 
                                      i18n("Are you sure you want to irreversibly remove the branch '%1'?", baseBranch));
    if (ret == KMessageBox::Yes)
        m_model->removeBranch(baseBranch);
}

void BranchManager::renameBranch()
{
    QModelIndex currentIndex = m_ui->branchView->currentIndex();
    if (!currentIndex.isValid())
        return;

    m_ui->branchView->edit(currentIndex);
}

void BranchManager::checkoutBranch()
{
    QString branch = m_ui->branchView->currentIndex().data().toString();
    if (branch == m_model->currentBranch())
    {
        KMessageBox::messageBox(this, KMessageBox::Sorry,
                                i18n("Already on branch \"%1\"\n",
                                        branch));
        return;
    }

    kDebug() << "Switching to" << branch << "in" << m_repository;
    KDevelop::VcsJob *branchJob = m_dvcPlugin->switchBranch(m_repository, branch);
//     connect(branchJob, SIGNAL(finished(KJob*)), m_model, SIGNAL(resetCurrent()));
    
    ICore::self()->runController()->registerJob(branchJob);
    close();
}
