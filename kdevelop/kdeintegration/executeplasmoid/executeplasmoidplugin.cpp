/*
  * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#include "executeplasmoidplugin.h"
#include "plasmoidexecutionconfig.h"
#include "plasmoidexecutionjob.h"
#include <kpluginfactory.h>
#include <KDebug>
#include <KAboutData>
#include <interfaces/icore.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevExecutePlasmoidFactory, registerPlugin<ExecutePlasmoidPlugin>(); )
K_EXPORT_PLUGIN(KDevExecutePlasmoidFactory(KAboutData("kdevexecuteplasmoid", "kdevexecuteplasmoid", ki18n("Execute plasmoid support"), "55.2",
                                                      ki18n("Allows running of plasmoids"), KAboutData::License_GPL)
    .addAuthor(ki18n("Aleix Pol Gonzalez"), ki18n("Author"), "aleixpol@kde.org", "http://kdevelop.org")
))

ExecutePlasmoidPlugin::ExecutePlasmoidPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevExecutePlasmoidFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( IExecutePlugin )
    m_configType = new PlasmoidExecutionConfigType();
    m_configType->addLauncher( new PlasmoidLauncher( this ) );
    kDebug() << "adding plasmoid launch config";
    core()->runController()->addConfigurationType( m_configType );
}

ExecutePlasmoidPlugin::~ExecutePlasmoidPlugin()
{}

void ExecutePlasmoidPlugin::unload()
{
    core()->runController()->removeConfigurationType( m_configType );
    delete m_configType;
    m_configType = 0;
}

KUrl ExecutePlasmoidPlugin::executable(ILaunchConfiguration* config, QString& /*error*/) const
{
    return PlasmoidExecutionJob::executable(config);
}

QStringList ExecutePlasmoidPlugin::arguments(ILaunchConfiguration* config, QString& /*error*/) const
{
    return PlasmoidExecutionJob::arguments(config);
}

KJob* ExecutePlasmoidPlugin::dependecyJob(ILaunchConfiguration* config) const
{
    return PlasmoidLauncher::calculateDependencies(config);
}

KUrl ExecutePlasmoidPlugin::workingDirectory(ILaunchConfiguration* config) const
{
    return PlasmoidExecutionJob::workingDirectory(config);
}

QString ExecutePlasmoidPlugin::environmentGroup(ILaunchConfiguration* /*config*/) const
{
    return QString();
}

QString ExecutePlasmoidPlugin::nativeAppConfigTypeId() const
{
    return PlasmoidExecutionConfigType::typeId();
}

bool ExecutePlasmoidPlugin::useTerminal(ILaunchConfiguration* /*config*/) const
{
    return false;
}

QString ExecutePlasmoidPlugin::terminal(ILaunchConfiguration* /*config*/) const
{
    return QString();
}
