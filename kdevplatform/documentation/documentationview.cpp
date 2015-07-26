/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "documentationview.h"
#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QCompleter>
#include <QLayout>
#include <QTextBrowser>
#include <KLineEdit>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <interfaces/icore.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>
#include "documentationfindwidget.h"

using namespace KDevelop;

DocumentationView::DocumentationView(QWidget* parent, ProvidersModel* m)
    : QWidget(parent), mProvidersModel(m)
{
    setWindowIcon(KIcon("documentation"));
    setWindowTitle(i18n("Documentation"));

    setLayout(new QVBoxLayout(this));
    layout()->setMargin(0);
    layout()->setSpacing(0);
    
    //TODO: clean this up, simply use addAction as that will create a toolbar automatically
    //      use custom KAction's with createWidget for mProviders and mIdentifiers
    mActions=new KToolBar(this);
    // set window title so the QAction from QToolBar::toggleViewAction gets a proper name set
    mActions->setWindowTitle(i18n("Documentation Tool Bar"));
    mActions->setToolButtonStyle(Qt::ToolButtonIconOnly);
    int iconSize=style()->pixelMetric(QStyle::PM_SmallIconSize);
    mActions->setIconSize(QSize(iconSize, iconSize));
    
    mFindDoc = new DocumentationFindWidget;
    mFindDoc->hide();
    
    mBack=mActions->addAction(KIcon("go-previous"), i18n("Back"));
    mForward=mActions->addAction(KIcon("go-next"), i18n("Forward"));
    mFind=mActions->addAction(KIcon("edit-find"), i18n("Find"), mFindDoc, SLOT(show()));
    mActions->addSeparator();
    mActions->addAction(KIcon("go-home"), i18n("Home"), this, SLOT(showHome()));
    mProviders=new QComboBox(mActions);
    mProviders->setFocusPolicy(Qt::NoFocus);
    
    mIdentifiers=new KLineEdit(mActions);
    mIdentifiers->setClearButtonShown(true);
    mIdentifiers->setCompleter(new QCompleter(mIdentifiers));
//     mIdentifiers->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mIdentifiers->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    /* vertical size policy should be left to the style. */
    mIdentifiers->setSizePolicy(QSizePolicy::Expanding, mIdentifiers->sizePolicy().verticalPolicy());
    connect(mIdentifiers, SIGNAL(returnPressed()), SLOT(changedSelection()));
    connect(mIdentifiers->completer(), SIGNAL(activated(QModelIndex)), SLOT(changeProvider(QModelIndex)));
    
    mActions->addWidget(mProviders);
    mActions->addWidget(mIdentifiers);
    
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    connect(mBack, SIGNAL(triggered()), this, SLOT(browseBack()));
    connect(mForward, SIGNAL(triggered()), this, SLOT(browseForward()));
    mCurrent=mHistory.end();
    
    layout()->addWidget(mActions);
    layout()->addWidget(new QWidget(this));
    layout()->addWidget(mFindDoc);
    
    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

void DocumentationView::initialize()
{
    mProviders->setModel(mProvidersModel);
    connect(mProviders, SIGNAL(activated(int)), SLOT(changedProvider(int)));
    foreach(KDevelop::IDocumentationProvider* p, mProvidersModel->providers()) {
        connect(dynamic_cast<QObject*>(p), SIGNAL(addHistory(KSharedPtr<KDevelop::IDocumentation>)),
                SLOT(addHistory(KSharedPtr<KDevelop::IDocumentation>)));
    }
    connect(mProvidersModel, SIGNAL(providersChanged()), this, SLOT(emptyHistory()));
    
    if(mProvidersModel->rowCount()>0)
        changedProvider(0);
}

void DocumentationView::browseBack()
{
    mCurrent--;
    mBack->setEnabled(mCurrent!=mHistory.begin());
    mForward->setEnabled(true);
    
    updateView();
}

void DocumentationView::browseForward()
{
    mCurrent++;
    mForward->setEnabled(mCurrent+1!=mHistory.end());
    mBack->setEnabled(true);

    updateView();
}

void DocumentationView::showHome()
{
    KDevelop::IDocumentationProvider* prov=mProvidersModel->provider(mProviders->currentIndex());
    
    showDocumentation(prov->homePage());
}

void DocumentationView::changedSelection()
{
    changeProvider(mIdentifiers->completer()->currentIndex());
}

void DocumentationView::changeProvider(const QModelIndex& idx)
{
    if(idx.isValid())
    {
        KDevelop::IDocumentationProvider* prov=mProvidersModel->provider(mProviders->currentIndex());
        KSharedPtr<KDevelop::IDocumentation> doc=prov->documentationForIndex(idx);
        if(doc)
            showDocumentation(doc);
    }
}

void DocumentationView::showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)
{
    kDebug(9529) << "showing" << doc->name();
    
    addHistory(doc);
    updateView();
}

void DocumentationView::addHistory(KSharedPtr< KDevelop::IDocumentation > doc)
{
    mBack->setEnabled( !mHistory.isEmpty() );
    mForward->setEnabled(false);

    // clear all history following the current item, unless we're already
    // at the end (otherwise this code crashes when history is empty, which
    // happens when addHistory is first called on startup to add the
    // homepage)
    if (mCurrent+1 < mHistory.end()) {
        mHistory.erase(mCurrent+1, mHistory.end());
    }

    mHistory.append(doc);
    mCurrent=mHistory.end()-1;

    // NOTE: we assume an existing widget was used to navigate somewhere
    //       but this history entry actually contains the new info for the
    //       title... this is ugly and should be refactored somehow
    if (mIdentifiers->completer()->model() == (*mCurrent)->provider()->indexModel()) {
        mIdentifiers->setText((*mCurrent)->name());
    }
}

void DocumentationView::emptyHistory()
{
    mHistory.clear();
    mCurrent=mHistory.end();
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    if(mProviders->count() > 0) {
        mProviders->setCurrentIndex(0);
        changedProvider(0);
    }
}

void DocumentationView::updateView()
{
    mProviders->setCurrentIndex(mProvidersModel->rowForProvider((*mCurrent)->provider()));
    mIdentifiers->completer()->setModel((*mCurrent)->provider()->indexModel());
    mIdentifiers->setText((*mCurrent)->name());
    
    QLayoutItem* lastview=layout()->takeAt(1);
    Q_ASSERT(lastview);
    
    if(lastview->widget()->parent()==this)
        lastview->widget()->deleteLater();
    
    delete lastview;
    
    mFindDoc->setEnabled(false);
    QWidget* w=(*mCurrent)->documentationWidget(mFindDoc, this);
    Q_ASSERT(w);
    
    mFind->setEnabled(mFindDoc->isEnabled());
    if(!mFindDoc->isEnabled())
        mFindDoc->hide();
    
    QLayoutItem* findW=layout()->takeAt(1);
    layout()->addWidget(w);
    layout()->addItem(findW);
}

void DocumentationView::changedProvider(int row)
{
    mIdentifiers->completer()->setModel(mProvidersModel->provider(row)->indexModel());
    mIdentifiers->clear();
    
    showHome();
}

////////////// ProvidersModel //////////////////

ProvidersModel::ProvidersModel(QObject* parent)
    : QAbstractListModel(parent)
    , mProviders(ICore::self()->documentationController()->documentationProviders())
{
    connect(ICore::self()->pluginController(), SIGNAL(pluginUnloaded(KDevelop::IPlugin*)), SLOT(unloaded(KDevelop::IPlugin*)));
    connect(ICore::self()->pluginController(), SIGNAL(pluginLoaded(KDevelop::IPlugin*)), SLOT(loaded(KDevelop::IPlugin*)));
    connect(ICore::self()->documentationController(), SIGNAL(providersChanged()), SLOT(reloadProviders()));
}

void ProvidersModel::reloadProviders()
{
    beginResetModel();
    mProviders = ICore::self()->documentationController()->documentationProviders();
    endResetModel();
    emit providersChanged();
}

QVariant ProvidersModel::data(const QModelIndex& index, int role) const
{
    QVariant ret;
    switch (role)
    {
    case Qt::DisplayRole:
        ret=provider(index.row())->name();
        break;
    case Qt::DecorationRole:
        ret=provider(index.row())->icon();
        break;
    }
    return ret;
}

void ProvidersModel::unloaded(KDevelop::IPlugin* p)
{
    IDocumentationProvider* prov=p->extension<IDocumentationProvider>();
    int idx=-1;
    if (prov)
        idx = mProviders.indexOf(prov);

    if (idx>=0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        mProviders.removeAt(idx);
        endRemoveRows();
        emit providersChanged();
    }

    IDocumentationProviderProvider* provProv=p->extension<IDocumentationProviderProvider>();
    if (provProv) {
        foreach (IDocumentationProvider* p, provProv->providers()) {
            idx = mProviders.indexOf(p);
            if (idx>=0) {
                beginRemoveRows(QModelIndex(), idx, idx);
                mProviders.removeAt(idx);
                endRemoveRows();
            }
        }
        emit providersChanged();
    }
}

void ProvidersModel::loaded(IPlugin* p)
{
    IDocumentationProvider* prov=p->extension<IDocumentationProvider>();

    if (prov && !mProviders.contains(prov)) {
        beginInsertRows(QModelIndex(), 0, 0);
        mProviders.append(prov);
        endInsertRows();
        emit providersChanged();
    }
    IDocumentationProviderProvider* provProv=p->extension<IDocumentationProviderProvider>();
    if (provProv) {
        beginInsertRows(QModelIndex(), 0, 0);
        mProviders.append(provProv->providers());
        endInsertRows();
        emit providersChanged();
    }
}

int ProvidersModel::rowForProvider(IDocumentationProvider* provider) { return mProviders.indexOf(provider); }
IDocumentationProvider* ProvidersModel::provider(int pos) const { return mProviders[pos]; }
QList< IDocumentationProvider* > ProvidersModel::providers() { return mProviders; }
int ProvidersModel::rowCount(const QModelIndex&) const { return mProviders.count(); }

