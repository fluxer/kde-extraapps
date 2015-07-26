/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_PROJECTPATHSWIDGET_H
#define KDEVELOP_PROJECTMANAGERS_CUSTOM_BUILDSYSTEM_PROJECTPATHSWIDGET_H

#include <QWidget>

#include <qabstractitemmodel.h>

#include "../compilerprovider/icompiler.h"
#include "../compilerprovider/settingsmanager.h"

class KUrlRequester;
namespace Ui
{
class ProjectPathsWidget;
}

namespace KDevelop
{
    class IProject;
}

class ProjectPathsModel;
class QItemSelection;

class ProjectPathsWidget : public QWidget
{
Q_OBJECT
public:
    ProjectPathsWidget( QWidget* parent = 0 );
    void setProject(KDevelop::IProject* w_project);
    void setPaths( const QList<ConfigEntry>& );
    QList<ConfigEntry> paths() const;
    void clear();

    void setCompilers(const QVector<CompilerPointer>& compilers);

    void setCurrentCompiler(const QString& name);

    CompilerPointer currentCompiler() const;

    QVector<CompilerPointer> compilers() const;

signals:
    void changed();
private slots:
    // Handling of project-path combobox, add and remove buttons
    void projectPathSelected( int index );
    void addProjectPath();
    void deleteProjectPath();
    void batchEdit();
    void configureCompilers();

    // Forward includes model changes into the pathsModel
    void includesChanged( const QStringList& includes );

    // Forward defines model changes into the pathsModel
    void definesChanged( const KDevelop::Defines& defines );
private:
    Ui::ProjectPathsWidget* ui;
    ProjectPathsModel* pathsModel;
    QVector<CompilerPointer> m_compilers;
    KDevelop::IProject* m_project;
    // Enables/Disables widgets based on UI state/selection
    void updateEnablements();
    void updatePathsModel( const QVariant& newData, int role );
};

#endif

