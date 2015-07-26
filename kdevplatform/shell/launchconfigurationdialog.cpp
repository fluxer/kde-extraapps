/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "launchconfigurationdialog.h"
#include <QtGui/QSplitter>
#include <QtGui/QTreeView>
#include <QtGui/QStackedWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>

#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <ktabwidget.h>
#include <kmessagebox.h>

#include <interfaces/launchconfigurationpage.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "core.h"
#include "runcontroller.h"
#include "launchconfiguration.h"

#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>
#include <interfaces/launchconfigurationtype.h>
#include <QLayout>
#include <QMenu>
#include <QLabel>

namespace KDevelop
{
    
bool launchConfigGreaterThan(KDevelop::LaunchConfigurationType* a, KDevelop::LaunchConfigurationType* b)
{
    return a->name()>b->name();
}

//TODO: Maybe use KPageDialog instead, might make the model stuff easier and the default-size stuff as well
LaunchConfigurationDialog::LaunchConfigurationDialog(QWidget* parent): KDialog(parent), currentPageChanged( false )
{
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    setButtonFocus( KDialog::Ok );
    setWindowTitle( i18n( "Launch Configurations" ) );
    button( KDialog::Apply )->setEnabled( false );
    
    setupUi( mainWidget() );
    mainWidget()->layout()->setContentsMargins( 0, 0, 0, 0 );
    splitter->setSizes(QList<int>() << 260 << 620);
    
    addConfig->setIcon( KIcon("list-add") );
    addConfig->setEnabled( false );
    addConfig->setToolTip(i18nc("@info:tooltip", "Add a new launch configuration."));
    deleteConfig->setIcon( KIcon("list-remove") );
    deleteConfig->setEnabled( false );
    deleteConfig->setToolTip(i18nc("@info:tooltip", "Delete selected launch configuration."));
    
    model = new LaunchConfigurationsModel( tree );
    tree->setModel( model );
    tree->setExpandsOnDoubleClick( true );
    tree->setSelectionBehavior( QAbstractItemView::SelectRows );
    tree->setSelectionMode( QAbstractItemView::SingleSelection );
    tree->setUniformRowHeights( true );
    tree->setItemDelegate( new LaunchConfigurationModelDelegate() );
    tree->setColumnHidden(1, true);
    for(int row=0; row<model->rowCount(); row++) {
        tree->setExpanded(model->index(row, 0), true);
    }
    
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(doTreeContextMenu(QPoint)) );
    connect( deleteConfig, SIGNAL(clicked()), this, SLOT(deleteConfiguration()));
    connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(modelChanged(QModelIndex,QModelIndex)) );
    connect( tree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
    QModelIndex idx = model->indexForConfig( Core::self()->runControllerInternal()->defaultLaunch() );
    kDebug() << "selecting index:" << idx;
    if( !idx.isValid() )
    {
        for( int i = 0; i < model->rowCount(); i++ )
        {
            if( model->rowCount( model->index( i, 0, QModelIndex() ) ) > 0 )
            {
                idx = model->index( 1, 0, model->index( i, 0, QModelIndex() ) );
                break;
            }
        }
        if( !idx.isValid() )
        {
            idx = model->index( 0, 0, QModelIndex() );
        }
    }
    tree->selectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    
    // Unfortunately tree->resizeColumnToContents() only looks at the top-level 
    // items, instead of all open ones. Hence we're calculating it ourselves like
    // this:
    // Take the selected index, check if it has childs, if so take the first child
    // Then count the level by going up, then let the tree calculate the width
    // for the selected or its first child index and add indentation*level
    //
    // If Qt Software ever fixes resizeColumnToContents, the following line
    // can be enabled and the rest be removed
    // tree->resizeColumnToContents( 0 );
    int level = 0;
    QModelIndex widthidx = idx;
    if( model->rowCount( idx ) > 0 )
    {
        widthidx = idx.child( 0, 0 );
    }
    QModelIndex parentidx = widthidx.parent();
    while( parentidx.isValid() )
    {
        level++;
        parentidx = parentidx.parent();
    }
    // make sure the base column width is honored, e.g. when no launch configs exist
    tree->resizeColumnToContents(0);
    int width = tree->columnWidth( 0 );
    while ( widthidx.isValid() )
    {
        width = qMax( width, level*tree->indentation() + tree->indentation() + tree->sizeHintForIndex( widthidx ).width() );
        widthidx = widthidx.parent();
    }
    tree->setColumnWidth( 0, width );

    QMenu* m = new QMenu(this);
    QList<LaunchConfigurationType*> types = Core::self()->runController()->launchConfigurationTypes();
    qSort(types.begin(), types.end(), launchConfigGreaterThan); //we want it in reverse order
    foreach(LaunchConfigurationType* type, types)
    {
        connect(type, SIGNAL(signalAddLaunchConfiguration(KDevelop::ILaunchConfiguration*)), SLOT(addConfiguration(KDevelop::ILaunchConfiguration*)));
        QMenu* suggestionsMenu = type->launcherSuggestions();
        
        if(suggestionsMenu) {
            m->addMenu(suggestionsMenu);
        }
    }
    // Simplify menu structure to get rid of 1-entry levels
    while (m->actions().count() == 1) {
        QMenu* subMenu = m->actions().first()->menu();
        if (subMenu && subMenu->isEnabled() && subMenu->actions().count()<5) {
            m = subMenu;
        } else {
            break;
        }
    }
    if(!m->isEmpty()) {
        QAction* separator = new QAction(m);
        separator->setSeparator(true);
        m->insertAction(m->actions().first(), separator);
    }
    
    foreach(LaunchConfigurationType* type, types) {
        QAction* action = new QAction(type->icon(), type->name(), m);
        action->setProperty("configtype", qVariantFromValue<QObject*>(type));
        connect(action, SIGNAL(triggered(bool)), SLOT(createEmptyLauncher()));
        
        if(!m->actions().isEmpty())
            m->insertAction(m->actions().first(), action);
        else
            m->addAction(action);
    }
    addConfig->setMenu(m);
    
    connect(debugger, SIGNAL(currentIndexChanged(int)), SLOT(launchModeChanged(int)));

    connect( this, SIGNAL(okClicked()), SLOT(saveConfig()) );
    connect( this, SIGNAL(applyClicked()), SLOT(saveConfig()) );
    
    setInitialSize( QSize(qMax(700, sizeHint().width()), qMax(500, sizeHint().height())) );
}

void LaunchConfigurationDialog::doTreeContextMenu(QPoint point)
{
    if ( ! tree->selectionModel()->selectedRows().isEmpty() ) {
        QModelIndex selected = tree->selectionModel()->selectedRows().first();
        if ( selected.parent().isValid() && ! selected.parent().parent().isValid() ) {
            // only display the menu if a launch config is clicked
            QMenu menu;
            QAction* rename = new QAction(KIcon("edit-rename"), i18n("Rename configuration"), &menu);
            QAction* delete_ = new QAction(KIcon("edit-delete"), i18n("Delete configuration"), &menu);
            connect(rename, SIGNAL(triggered(bool)), this, SLOT(renameSelected()));
            connect(delete_, SIGNAL(triggered(bool)), this, SLOT(deleteConfiguration()));
            menu.addAction(rename);
            menu.addAction(delete_);
            menu.exec(tree->mapToGlobal(point));
        }
    }
}

void LaunchConfigurationDialog::renameSelected()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        QModelIndex parent = tree->selectionModel()->selectedRows().first();
        if( parent.parent().isValid() )
        {
            parent = parent.parent();
        }
        QModelIndex index = model->index(tree->selectionModel()->selectedRows().first().row(), 0, parent);
        tree->edit( index );
    }
}

QSize LaunchConfigurationDialog::sizeHint() const
{
    QSize s = KDialog::sizeHint();
    return s.expandedTo(QSize(880, 520));
}

void LaunchConfigurationDialog::createEmptyLauncher()
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    
    LaunchConfigurationType* type = qobject_cast<LaunchConfigurationType*>(action->property("configtype").value<QObject*>());
    Q_ASSERT(type);
    
    IProject* p = model->projectForIndex(tree->currentIndex());
    QPair< QString, QString > launcher( type->launchers().at( 0 )->supportedModes().at(0), type->launchers().at( 0 )->id() );
    ILaunchConfiguration* l = ICore::self()->runController()->createLaunchConfiguration(type, launcher, p);
    addConfiguration(l);
}

void LaunchConfigurationDialog::selectionChanged(QItemSelection selected, QItemSelection deselected )
{
    if( !deselected.indexes().isEmpty() )
    {
        LaunchConfiguration* l = model->configForIndex( deselected.indexes().first() );
        if( l )
        {
            disconnect(l, SIGNAL(nameChanged(LaunchConfiguration*)), this,  SLOT(updateNameLabel(LaunchConfiguration*)));
            if( currentPageChanged )
            {
                if( KMessageBox::questionYesNo( this, i18n("Selected Launch Configuration has unsaved changes. Do you want to save it?"), i18n("Unsaved Changes") ) == KMessageBox::Yes )
                {
                    saveConfig( deselected.indexes().first() );
                } else {
                    LaunchConfigPagesContainer* tab = qobject_cast<LaunchConfigPagesContainer*>( stack->currentWidget() );
                    tab->setLaunchConfiguration( l );
                    button( KDialog::Apply )->setEnabled( false );
                    currentPageChanged = false;
                }
            }
        }
    }
    updateNameLabel(0);
    
    for( int i = 1; i < stack->count(); i++ )
    {
        QWidget* w = stack->widget(i);
        stack->removeWidget(w);
        delete w;
    }
    
    debugger->clear();
    if( !selected.indexes().isEmpty() )
    {
        QModelIndex idx = selected.indexes().first();
        LaunchConfiguration* l = model->configForIndex( idx );
        ILaunchMode* lm = model->modeForIndex( idx );
        
        if( l )
        {
            updateNameLabel( l );
            tree->expand( model->indexForConfig( l ) );
            connect( l, SIGNAL(nameChanged(LaunchConfiguration*)), SLOT(updateNameLabel(LaunchConfiguration*)) );
            if( lm )
            {
                bool b = debugger->blockSignals(true);
                QList<ILauncher*> launchers = l->type()->launchers();
                for( QList<ILauncher*>::const_iterator it = launchers.constBegin(); it != launchers.constEnd(); it++ )
                {
                    if( ((*it)->supportedModes().contains( lm->id() ) ) ) {
                        debugger->addItem( (*it)->name(), (*it)->id() );
                    }
                }
                debugger->blockSignals(b);
                
                debugger->setVisible(debugger->count()>0);
                debugLabel->setVisible(debugger->count()>0);
                
                QVariant currentLaunchMode = idx.sibling(idx.row(), 1).data(Qt::EditRole);
                debugger->setCurrentIndex(debugger->findData(currentLaunchMode));
                
                ILauncher* launcher = l->type()->launcherForId( currentLaunchMode.toString() );
                if( launcher )
                {
                    LaunchConfigPagesContainer* tab = launcherWidgets.value( launcher );
                    if(!tab)
                    {
                        QList<KDevelop::LaunchConfigurationPageFactory*> pages = launcher->configPages();
                        if(!pages.isEmpty()) {
                            tab = new LaunchConfigPagesContainer( launcher->configPages(), stack );
                            connect( tab, SIGNAL(changed()), SLOT(pageChanged()) );
                            stack->addWidget( tab );
                        }
                    }
                    
                    if(tab) {
                        tab->setLaunchConfiguration( l );
                        stack->setCurrentWidget( tab );
                    } else {
                        QLabel* label = new QLabel(i18nc("%1 is a launcher name",
                                                         "No configuration is needed for '%1'",
                                                         launcher->name()), stack);
                        label->setAlignment(Qt::AlignCenter);
                        QFont font = label->font();
                        font.setItalic(true);
                        label->setFont(font);
                        stack->addWidget(label);
                        stack->setCurrentWidget(label);
                    }
                    
                    updateNameLabel( l );
                    addConfig->setEnabled( false );
                    deleteConfig->setEnabled( false );
                } else
                {
                    addConfig->setEnabled( false );
                    deleteConfig->setEnabled( false );
                    stack->setCurrentIndex( 0 );
                }
            } else 
            {
                //TODO: enable removal button
                LaunchConfigurationType* type = l->type();
                LaunchConfigPagesContainer* tab = typeWidgets.value( type );
                if( !tab )
                {
                    tab = new LaunchConfigPagesContainer( type->configPages(), stack );
                    connect( tab, SIGNAL(changed()), SLOT(pageChanged()) );
                    stack->addWidget( tab );
                }
                kDebug() << "created pages, setting config up";
                tab->setLaunchConfiguration( l );
                stack->setCurrentWidget( tab );
                
                addConfig->setEnabled( true );
                deleteConfig->setEnabled( true );
                debugger->setVisible( false );
                debugLabel->setVisible( false );
            }
        } else 
        {
            addConfig->setEnabled( true );
            deleteConfig->setEnabled( false );
            stack->setCurrentIndex( 0 );
            QLabel* l = new QLabel(i18n("<i>Select a configuration to edit from the left,<br>"
                                        "or click the \"Add New\" button to add a new one.</i>"), stack);
            l->setAlignment(Qt::AlignCenter);
            stack->addWidget(l);
            stack->setCurrentWidget(l);
            debugger->setVisible( false );
            debugLabel->setVisible( false );
        }
    } else 
    {
        debugger->setVisible( false );
        debugLabel->setVisible( false );
        addConfig->setEnabled( false );
        deleteConfig->setEnabled( false );
        stack->setCurrentIndex( 0 );
    }
}

void LaunchConfigurationDialog::saveConfig( const QModelIndex& idx )
{
    Q_UNUSED( idx );
    LaunchConfigPagesContainer* tab = qobject_cast<LaunchConfigPagesContainer*>( stack->currentWidget() );
    if( tab )
    {
        tab->save();
        button( KDialog::Apply )->setEnabled( false );
        currentPageChanged = false;
    }
}

void LaunchConfigurationDialog::saveConfig()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        saveConfig( tree->selectionModel()->selectedRows().first() );
    }
}


void LaunchConfigurationDialog::pageChanged()
{
    currentPageChanged = true;
    button( KDialog::Apply )->setEnabled( true );
}

void LaunchConfigurationDialog::modelChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    if (tree->selectionModel())
    {
        QModelIndex index = tree->selectionModel()->selectedRows().first();
        if (index.row() >= topLeft.row() && index.row() <= bottomRight.row() && bottomRight.column() == 1)
            selectionChanged(tree->selectionModel()->selection(), tree->selectionModel()->selection());
    }
}

void LaunchConfigurationDialog::deleteConfiguration()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        model->deleteConfiguration( tree->selectionModel()->selectedRows().first() );
        tree->resizeColumnToContents( 0 );
    }
}

void LaunchConfigurationDialog::updateNameLabel( LaunchConfiguration* l )
{
    if( l )
    {
        configName->setText( i18n("Editing %2: <b>%1</b>", l->name(), l->type()->name() ) );
    } else
    {
        configName->clear();
    }
}

void LaunchConfigurationDialog::createConfiguration()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        QModelIndex idx = tree->selectionModel()->selectedRows().first();
        if( idx.parent().isValid() )
        {
            idx = idx.parent();
        }
        model->createConfiguration( idx );
        QModelIndex newindex = model->index( model->rowCount( idx ) - 1, 0, idx );
        tree->selectionModel()->select( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        tree->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        tree->edit( newindex );
        tree->resizeColumnToContents( 0 );
    }
}

void LaunchConfigurationDialog::addConfiguration(ILaunchConfiguration* _launch)
{
    LaunchConfiguration* launch = dynamic_cast<LaunchConfiguration*>(_launch);
    Q_ASSERT(launch);
    int row = launch->project() ? model->findItemForProject(launch->project())->row : 0;
    QModelIndex idx  = model->index(row, 0);
    
    model->addConfiguration(launch, idx);
    
    QModelIndex newindex = model->index( model->rowCount( idx ) - 1, 0, idx );
    tree->selectionModel()->select( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->edit( newindex );
    tree->resizeColumnToContents( 0 );
}

LaunchConfigurationsModel::LaunchConfigurationsModel(QObject* parent): QAbstractItemModel(parent)
{
    GenericPageItem* global = new GenericPageItem;
    global->text = i18n("Global");
    global->row = 0;
    topItems << global;
    foreach( IProject* p, Core::self()->projectController()->projects() )
    {
        ProjectItem* t = new ProjectItem;
        t->project = p;
        t->row = topItems.count();
        topItems << t;
    }
    foreach( LaunchConfiguration* l, Core::self()->runControllerInternal()->launchConfigurationsInternal() )
    {
        addItemForLaunchConfig( l );
    }    
}

void LaunchConfigurationsModel::addItemForLaunchConfig( LaunchConfiguration* l )
{
    LaunchItem* t = new LaunchItem;
    t->launch = l;
    TreeItem* parent;
    if( l->project() ) {
        parent = findItemForProject( l->project() );
    } else {
        parent = topItems.at(0);
    }
    t->parent = parent;
    t->row = parent->children.count();
    parent->children.append( t );
    addLaunchModeItemsForLaunchConfig ( t );
}

void LaunchConfigurationsModel::addLaunchModeItemsForLaunchConfig ( LaunchItem* t )
{
    QList<TreeItem*> items;
    QSet<QString> modes;
    foreach( ILauncher* launcher, t->launch->type()->launchers() )
    {
        foreach( const QString& mode, launcher->supportedModes() )
        {
            if( !modes.contains( mode ) && launcher->configPages().count() > 0 )
            {
                modes.insert( mode );
                LaunchModeItem* lmi = new LaunchModeItem;
                lmi->mode = Core::self()->runController()->launchModeForId( mode );
                lmi->parent = t;
                lmi->row = t->children.count();
                items.append( lmi );
            }
        }
    }
    if( !items.isEmpty() )
    {
        QModelIndex p = indexForConfig( t->launch );
        beginInsertRows( p, t->children.count(), t->children.count() + items.count() - 1  );
        t->children.append( items );
        endInsertRows();
    }
}

LaunchConfigurationsModel::ProjectItem* LaunchConfigurationsModel::findItemForProject( IProject* p )
{
    foreach( TreeItem* t, topItems )
    {
        ProjectItem* pi = dynamic_cast<ProjectItem*>( t );
        if( pi && pi->project == p ) 
        {
            return pi;
        }
    }
    Q_ASSERT(false);
    return 0;
}

int LaunchConfigurationsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED( parent );
    return 2;
}

QVariant LaunchConfigurationsModel::data(const QModelIndex& index, int role) const
{
    if( index.isValid() && index.column() >= 0 && index.column() < 2 )
    {
        TreeItem* t = static_cast<TreeItem*>( index.internalPointer() );
        switch( role ) 
        {
            case Qt::DisplayRole:
            {
                LaunchItem* li = dynamic_cast<LaunchItem*>( t );
                if( li ) 
                {
                    if( index.column() == 0 )
                    {
                        return li->launch->name();   
                    } else if( index.column() == 1 )
                    {
                        return li->launch->type()->name();
                    }
                }
                ProjectItem* pi = dynamic_cast<ProjectItem*>( t );
                if( pi && index.column() == 0 ) 
                {
                    return pi->project->name();
                }
                GenericPageItem* gpi = dynamic_cast<GenericPageItem*>( t );
                if( gpi && index.column() == 0 )
                {
                    return gpi->text;
                }
                LaunchModeItem* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi )
                {
                    if( index.column() == 0 )
                    {
                        return lmi->mode->name();
                    } else if( index.column() == 1 )
                    {
                        LaunchConfiguration* l = configForIndex( index );
                        return l->type()->launcherForId( l->launcherForMode( lmi->mode->id() ) )->name();
                    }
                }
                break;
            }
            case Qt::DecorationRole:
            {
                LaunchItem* li = dynamic_cast<LaunchItem*>( t );
                if( index.column() == 0 && li )
                {
                    return li->launch->type()->icon();
                }
                LaunchModeItem* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi && index.column() == 0 )
                {
                    return lmi->mode->icon();
                }
                if ( index.column() == 0 && !index.parent().isValid() ) {
                    if (index.row() == 0) {
                        // global item
                        return KIcon("folder");
                    } else {
                        // project item
                        return KIcon("folder-development");
                    }
                }
            }
            case Qt::EditRole:
            {
                LaunchItem* li = dynamic_cast<LaunchItem*>( t );
                if( li )
                {
                    if( index.column() == 0 )
                    {
                        return li->launch->name();
                    } else if ( index.column() == 1 )
                    {
                        return li->launch->type()->id();
                    }
                }
                LaunchModeItem* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi && index.column() == 1  )
                {
                    return configForIndex( index )->launcherForMode( lmi->mode->id() );
                }
                break;
            }
            default:
                break;
        }
    }
    return QVariant();
}

QModelIndex LaunchConfigurationsModel::index(int row, int column, const QModelIndex& parent) const
{
    if( !hasIndex( row, column, parent ) )
        return QModelIndex();
    TreeItem* tree;
    
    if( !parent.isValid() )
    {   
        tree = topItems.at( row );
    } else 
    {
        TreeItem* t = static_cast<TreeItem*>( parent.internalPointer() );
        tree = t->children.at( row );
    }
    if( tree )
    {
        return createIndex( row, column, tree );
    }
    return QModelIndex();
}

QModelIndex LaunchConfigurationsModel::parent(const QModelIndex& child) const
{
    if( child.isValid()  )
    {
        TreeItem* t = static_cast<TreeItem*>( child.internalPointer() );
        if( t->parent )
        {
            return createIndex( t->parent->row, 0, t->parent );
        }
    }
    return QModelIndex();
}

int LaunchConfigurationsModel::rowCount(const QModelIndex& parent) const
{
    if( parent.column() > 0 )
        return 0;
    if( parent.isValid() )
    {
        TreeItem* t = static_cast<TreeItem*>( parent.internalPointer() );
        return t->children.count();
    } else
    {
        return topItems.count();
    }
    return 0;
}

QVariant LaunchConfigurationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) 
    {
        if( section == 0 ) 
        {
            return i18nc("Name of the Launch Configurations", "Name");
        } else if( section == 1 ) 
        {
            return i18nc("The type of the Launch Configurations (i.e. Python Application, C++ Application)", "Type");
        }
    }
    return QVariant();
}

Qt::ItemFlags LaunchConfigurationsModel::flags(const QModelIndex& index) const
{
    if( index.isValid() && index.column() >= 0 
        && index.column() < columnCount( QModelIndex() ) ) 
    {
        TreeItem* t = static_cast<TreeItem*>( index.internalPointer() );
        if( t && ( dynamic_cast<LaunchItem*>( t ) || ( dynamic_cast<LaunchModeItem*>( t ) && index.column() == 1 ) ) )
        {
            return Qt::ItemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
        } else if( t )
        {
            return Qt::ItemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        }
    }
    return Qt::NoItemFlags;
}

bool LaunchConfigurationsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if( index.isValid() && index.parent().isValid() && role == Qt::EditRole ) 
    {
        if( index.row() >= 0 && index.row() < rowCount( index.parent() ) ) 
        {
            LaunchItem* t = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
            if( t )
            {
                if( index.column() == 0 )
                {
                    t->launch->setName( value.toString() );
                } else if( index.column() == 1 )
                {
                    if (t->launch->type()->id() != value.toString()) {
                        t->launch->setType( value.toString() );
                        QModelIndex p = indexForConfig(t->launch);
                        kDebug() << data(p);
                        beginRemoveRows( p, 0, t->children.count() );
                        qDeleteAll( t->children );
                        t->children.clear();
                        endRemoveRows();
                        addLaunchModeItemsForLaunchConfig( t );
                    }
                }
                emit dataChanged(index, index);
                return true;
            }
            LaunchModeItem* lmi = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
            if( lmi )
            {
                if( index.column() == 1 && index.data(Qt::EditRole)!=value)
                {
                    LaunchConfiguration* l = configForIndex( index );
                    l->setLauncherForMode( lmi->mode->id(), value.toString() );
                    emit dataChanged(index, index);
                    return true;
                }
            }
        }
    }
    return false;
}

ILaunchMode* LaunchConfigurationsModel::modeForIndex( const QModelIndex& idx ) const
{
    if( idx.isValid() )
    {
        LaunchModeItem* item = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( item )
        {
            return item->mode;
        }
    }
    return 0;
}

LaunchConfiguration* LaunchConfigurationsModel::configForIndex(const QModelIndex& idx ) const
{
    if( idx.isValid() )
    {
        LaunchItem* item = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( item )
        {
            return item->launch;
        } 
        LaunchModeItem* lmitem = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( lmitem ) 
        {
            return dynamic_cast<LaunchItem*>( lmitem->parent )->launch;
        }
    }
    return 0;
}

QModelIndex LaunchConfigurationsModel::indexForConfig( LaunchConfiguration* l ) const
{
    if( l )
    {
        TreeItem* tparent = topItems.at( 0 );
        if( l->project() )
        {
            foreach( TreeItem* t, topItems )
            {
                ProjectItem* pi = dynamic_cast<ProjectItem*>( t );
                if( pi && pi->project == l->project() )
                {
                    tparent = t;
                    break;
                }
            }
        }
        
        if( tparent )
        {
            foreach( TreeItem* c, tparent->children )
            {
                LaunchItem* li = dynamic_cast<LaunchItem*>( c );
                if( li->launch && li->launch == l )
                {
                    return index( c->row, 0, index( tparent->row, 0, QModelIndex() ) );
                }
            }
        }
    }
    return QModelIndex();
}


void LaunchConfigurationsModel::deleteConfiguration( const QModelIndex& index )
{
    LaunchItem* t = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
    if( !t )
        return;
    beginRemoveRows( parent( index ), index.row(), index.row() );
    t->parent->children.removeAll( t );
    Core::self()->runControllerInternal()->removeLaunchConfiguration( t->launch );
    endRemoveRows();
}

void LaunchConfigurationsModel::createConfiguration(const QModelIndex& parent )
{
    if(!Core::self()->runController()->launchConfigurationTypes().isEmpty())
    {
        TreeItem* t = static_cast<TreeItem*>( parent.internalPointer() );
        ProjectItem* ti = dynamic_cast<ProjectItem*>( t );
        
        LaunchConfigurationType* type = Core::self()->runController()->launchConfigurationTypes().at(0);
        QPair<QString,QString> launcher = qMakePair( type->launchers().at( 0 )->supportedModes().at(0), type->launchers().at( 0 )->id() );
        IProject* p = ( ti ? ti->project : 0 );
        ILaunchConfiguration* l = Core::self()->runController()->createLaunchConfiguration( type, launcher, p );
        
        addConfiguration(l, parent);
    }
}

void LaunchConfigurationsModel::addConfiguration(ILaunchConfiguration* l, const QModelIndex& parent)
{
    if( parent.isValid() )
    {
        beginInsertRows( parent, rowCount( parent ), rowCount( parent ) );
        addItemForLaunchConfig( dynamic_cast<LaunchConfiguration*>( l ) );
        endInsertRows();
    }
    else
    {
        delete l;
        Q_ASSERT(false && "could not add the configuration");
    }
}

IProject* LaunchConfigurationsModel::projectForIndex(const QModelIndex& idx)
{
    if(idx.parent().isValid()) {
        return projectForIndex(idx.parent());
    } else {
        const ProjectItem* item = dynamic_cast<const ProjectItem*>(topItems[idx.row()]);
        return item ? item->project : 0;
    }
}

LaunchConfigPagesContainer::LaunchConfigPagesContainer( const QList<LaunchConfigurationPageFactory*>& factories, QWidget* parent ) 
    : QWidget(parent)
{
    setLayout( new QVBoxLayout( this ) );
    layout()->setContentsMargins( 0, 0, 0, 0 );
    QWidget* parentwidget = this;
    KTabWidget* tab = 0;
    if( factories.count() > 1 )
    {
        tab = new KTabWidget( this );
        parentwidget = tab;
        layout()->addWidget( tab );
    }
    foreach( LaunchConfigurationPageFactory* fac, factories )
    {
        LaunchConfigurationPage* page = fac->createWidget( parentwidget );
        if ( page->layout() ) {
            page->layout()->setContentsMargins( 0, 0, 0, 0 );
        }
        pages.append( page );
        connect( page, SIGNAL(changed()), SIGNAL(changed()) );
        if( tab ) {
            tab->addTab( page, page->icon(), page->title() );
        } else 
        {
            layout()->addWidget( page );
        }
    }
}

void LaunchConfigPagesContainer::setLaunchConfiguration( KDevelop::LaunchConfiguration* l )
{
    config = l;
    foreach( LaunchConfigurationPage* p, pages )
    {
        p->loadFromConfiguration( config->config(), config->project() );
    }
}

void LaunchConfigPagesContainer::save()
{
    foreach( LaunchConfigurationPage* p, pages )
    {
        p->saveToConfiguration( config->config() );
    }
    config->config().sync();
}


QWidget* LaunchConfigurationModelDelegate::createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    const LaunchConfigurationsModel* model = dynamic_cast<const LaunchConfigurationsModel*>( index.model() );
    ILaunchMode* mode = model->modeForIndex( index );
    LaunchConfiguration* config = model->configForIndex( index );
    if( index.column() == 1 && mode && config ) 
    {
        KComboBox* box = new KComboBox( parent );
        QList<ILauncher*> launchers = config->type()->launchers();
        for( QList<ILauncher*>::const_iterator it = launchers.constBegin(); it != launchers.constEnd(); it++ )
        {
            if( ((*it)->supportedModes().contains( mode->id() ) ) )
            {
                box->addItem( (*it)->name(), (*it)->id() );
            }
        }
        return box;
    } else if( !mode && config && index.column() == 1 )
    {
        KComboBox* box = new KComboBox( parent );
        const QList<LaunchConfigurationType*> types = Core::self()->runController()->launchConfigurationTypes();
        for( QList<LaunchConfigurationType*>::const_iterator it = types.begin(); it != types.end(); it++ )
        {
            box->addItem( (*it)->name(), (*it)->id() );
        }
        return box;
    }
    return QStyledItemDelegate::createEditor ( parent, option, index );
}

LaunchConfigurationModelDelegate::LaunchConfigurationModelDelegate()
{
}

void LaunchConfigurationModelDelegate::setEditorData ( QWidget* editor, const QModelIndex& index ) const
{
    const LaunchConfigurationsModel* model = dynamic_cast<const LaunchConfigurationsModel*>( index.model() );
    LaunchConfiguration* config = model->configForIndex( index );
    if( index.column() == 1 && config ) 
    {
        KComboBox* box = qobject_cast<KComboBox*>( editor );
        box->setCurrentIndex( box->findData( index.data( Qt::EditRole ) ) );
    }
    else
    {
        QStyledItemDelegate::setEditorData ( editor, index );
    }
}

void LaunchConfigurationModelDelegate::setModelData ( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    LaunchConfigurationsModel* lmodel = dynamic_cast<LaunchConfigurationsModel*>( model );
    LaunchConfiguration* config = lmodel->configForIndex( index );
    if( index.column() == 1 && config ) 
    {
        KComboBox* box = qobject_cast<KComboBox*>( editor );
        lmodel->setData( index, box->itemData( box->currentIndex() ) );
    }
    else
    {
        QStyledItemDelegate::setModelData ( editor, model, index );
    }
}

void LaunchConfigurationDialog::launchModeChanged(int item)
{
    QModelIndex index = tree->currentIndex();
    if(debugger->isVisible() && item>=0)
        tree->model()->setData(index.sibling(index.row(), 1), debugger->itemData(item), Qt::EditRole);
}

}

#include "moc_launchconfigurationdialog.cpp"
