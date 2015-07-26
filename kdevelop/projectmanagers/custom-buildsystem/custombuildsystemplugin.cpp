/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
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

#include "custombuildsystemplugin.h"

#include <KPluginFactory>
#include <KLocale>
#include <KAboutData>
#include <KComponentData>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include "configconstants.h"

#include "config.h"

#include "custombuildjob.h"

using KDevelop::ProjectTargetItem;
using KDevelop::ProjectFolderItem;
using KDevelop::ProjectBuildFolderItem;
using KDevelop::ProjectBaseItem;
using KDevelop::ProjectFileItem;
using KDevelop::IPlugin;
using KDevelop::ICore;
using KDevelop::IOutputView;
using KDevelop::IGenericProjectManager;
using KDevelop::IProjectFileManager;
using KDevelop::IProjectBuilder;
using KDevelop::IProject;
using KDevelop::Path;

K_PLUGIN_FACTORY(CustomBuildSystemFactory, registerPlugin<CustomBuildSystem>(); )
K_EXPORT_PLUGIN(CustomBuildSystemFactory(KAboutData("kdevcustombuildsystem","kdevcustombuildsystem", ki18n("Custom Build System"), VERSION, ki18n("Support for building and managing custom build systems"), KAboutData::License_GPL, ki18n("Copyright 2010 Andreas Pakulat <apaku@gmx.de>"), KLocalizedString(), "", "apaku@gmx.de" )))


CustomBuildSystem::CustomBuildSystem( QObject *parent, const QVariantList & )
    : AbstractFileManagerPlugin( CustomBuildSystemFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
}

CustomBuildSystem::~CustomBuildSystem()
{
}

bool CustomBuildSystem::addFilesToTarget( const QList<ProjectFileItem*>&, ProjectTargetItem* )
{
    return false;
}

KJob* CustomBuildSystem::build( ProjectBaseItem* dom )
{
    return new CustomBuildJob( this, dom, CustomBuildSystemTool::Build );
}

Path CustomBuildSystem::buildDirectory( ProjectBaseItem*  item ) const
{
    Path p;
    if( item->folder() ) {
        p = item->path();
    } else {
        ProjectBaseItem* parent = item;
        while( !parent->folder() ) {
            parent = parent->parent();
        }
        p = parent->path();
    }
    const QString relative = item->project()->path().relativePath(p);
    KConfigGroup grp = configuration( item->project() );
    if(!grp.isValid()) {
        return Path();
    }

    Path builddir(grp.readEntry( ConfigConstants::buildDirKey, KUrl() ));
    if(!builddir.isValid() )  // set builddir to default if project contains a buildDirKey that does not have a value
    {
        builddir = item->project()->path();
    }
    builddir.addPath( relative );
    return builddir;
}

IProjectBuilder* CustomBuildSystem::builder() const
{
    return const_cast<IProjectBuilder*>(dynamic_cast<const IProjectBuilder*>(this));
}

KJob* CustomBuildSystem::clean( ProjectBaseItem* dom )
{
    return new CustomBuildJob( this, dom, CustomBuildSystemTool::Clean );
}

KJob* CustomBuildSystem::configure( IProject* project )
{
    return new CustomBuildJob( this, project->projectItem(), CustomBuildSystemTool::Configure );
}

ProjectTargetItem* CustomBuildSystem::createTarget( const QString&, ProjectFolderItem* )
{
    return 0;
}

QHash<QString, QString> CustomBuildSystem::defines( ProjectBaseItem* ) const
{
    return {};
}

IProjectFileManager::Features CustomBuildSystem::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders;
}

ProjectFolderItem* CustomBuildSystem::createFolderItem( IProject* project,
                    const Path& path, ProjectBaseItem* parent )
{
    return new ProjectBuildFolderItem( project, path, parent );
}

Path::List CustomBuildSystem::includeDirectories( ProjectBaseItem* ) const
{
    return {};
}

KJob* CustomBuildSystem::install( ProjectBaseItem* item )
{
    return new CustomBuildJob( this, item, CustomBuildSystemTool::Install );
}

KJob* CustomBuildSystem::prune( IProject* project )
{
    return new CustomBuildJob( this, project->projectItem(), CustomBuildSystemTool::Prune );
}

bool CustomBuildSystem::removeFilesFromTargets( const QList<ProjectFileItem*>& )
{
    return false;
}

bool CustomBuildSystem::removeTarget( ProjectTargetItem* )
{
    return false;
}

QList<ProjectTargetItem*> CustomBuildSystem::targets( ProjectFolderItem* ) const
{
    return QList<ProjectTargetItem*>();
}

KConfigGroup CustomBuildSystem::configuration( IProject* project ) const
{
    KConfigGroup grp = project->projectConfiguration()->group( ConfigConstants::customBuildSystemGroup );
    if(grp.isValid() && grp.hasKey(ConfigConstants::currentConfigKey))
        return grp.group( grp.readEntry( ConfigConstants::currentConfigKey ) );
    else
        return KConfigGroup();
}

#include "moc_custombuildsystemplugin.cpp"
