/* This file is part of the KDE project
   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright 2004,2007 Alexander Dymo <adymo@kdevelop.org>
   Copyright 2006 Adam Treat <treat@kde.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPLATFORM_IPLUGIN_H
#define KDEVPLATFORM_IPLUGIN_H

#include <QtCore/QObject>
#include <kxmlguiclient.h>

#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QPair>
#include "interfacesexport.h"

class KIconLoader;
class QAction;

namespace Sublime {
    class MainWindow;
}

/**
 * This macro adds an extension interface to register with the extension manager
 * Call this macro for all interfaces your plugin implements in its constructor
 */

#define KDEV_USE_EXTENSION_INTERFACE( Extension ) \
    addExtension( qobject_interface_iid<Extension*>() );

namespace KDevelop
{

class ICore;
class Context;
class ContextMenuExtension;

/**
 * The base class for all KDevelop plugins.
 *
 * Plugin is a component which is loaded into KDevelop shell at startup or by
 * request. Each plugin should have corresponding .desktop file with a
 * description. The .desktop file template looks like:
 * @code
 * [Desktop Entry]
 * Type=Service
 * Exec=blubb
 * Name=
 * GenericName=
 * Comment=
 * Icon=
 * ServiceTypes=KDevelop/Plugin
 * X-KDE-Library=
 * X-KDE-PluginInfo-Name=
 * X-KDE-PluginInfo-Author=
 * X-KDE-PluginInfo-Version=
 * X-KDE-PluginInfo-License=
 * X-KDE-PluginInfo-Category=
 * X-KDevelop-Version=
 * X-KDevelop-Category=
 * X-KDevelop-Mode=GUI
 * X-KDevelop-LoadMode=
 * X-KDevelop-Language=
 * X-KDevelop-SupportedMimeTypes=
 * X-KDevelop-Interfaces=
 * X-KDevelop-IOptional=
 * X-KDevelop-IRequired=
 * @endcode
 * <b>Description of parameters in .desktop file:</b>
 * - <i>Name</i> is a translatable name of a plugin, it is used in the plugin 
 *   selection list (required);
 * - <i>GenericName</i> is a translatable generic name of a plugin, it should 
 *   describe in general what the plugin can do  (required);
 * - <i>Comment</i> is a short description about the plugin (optional);
 * - <i>Icon</i> is a plugin icon (preferred);
 *   <i>X-KDE-library</i>this is the name of the .so file to load for this plugin (required);
 * - <i>X-KDE-PluginInfo-Name</i> is a non-translatable user-readable plugin
 *   identifier used in KTrader queries (required);
 * - <i>X-KDE-PluginInfo-Author</i> is a non-translateable name of the plugin
 *   author (optional);
 * - <i>X-KDE-PluginInfo-Version</i> is version number of the plugin (optional);
 * - <i>X-KDE-PluginInfo-License</i> is a license (optional). can be: GPL,
 * LGPL, BSD, Artistic, QPL or Custom. If this property is not set, license is
 * considered as unknown;
 * - <i>X-KDE-PluginInfo-Category</i> is used to categorize plugins (optional). can be:
 *    Core, Project Management, Version Control, Utilities, Documentation,
 *    Language Support, Debugging, Other
 *   If this property is not set, "Other" is assumed
 * - <i>X-KDevelop-Version</i> is the KDevPlatform API version this plugin
 *   works with (required);
 * - <i>X-KDevelop-Interfaces</i> is a list of extension interfaces that this
 * plugin implements (optional);
 * - <i>X-KDevelop-IRequired</i> is a list of extension interfaces that this
 * plugin depends on (optional); A list entry can also be of the form @c interface@pluginname,
 * in which case a plugin of the given name is required which implements the interface.
 * - <i>X-KDevelop-IOptional</i> is a list of extension interfaces that this
 * plugin will use if they are available (optional);
 * - <i>X-KDevelop-Language</i> is the name of the language the plugin provides 
 *   support for (optional);
 * - <i>X-KDevelop-SupportedMimeTypes</i> is a list of mimetypes that the 
 *   language-parser in this plugin supports (optional);
 * - <i>X-KDevelop-Mode</i> is either GUI or NoGUI to indicate whether a plugin can run
 *   with the GUI components loaded or not (required);
 * - <i>X-KDevelop-Category</i> is a scope of a plugin (see below for
 * explanation) (required);
 * - <i>X-KDevelop-LoadMode</i> can be set to AlwaysOn in which case the plugin will
 *   never be unloaded even if requested via the API. (optional);
 *
 * Plugin scope can be either:
 * - Global
 * - Project
 * .
 * Global plugins are plugins which require only the shell to be loaded and do not operate on
 * the Project interface and/or do not use project wide information.\n
 * Core plugins are global plugins which offer some important "core" functionality and thus
 * are not selectable by user in plugin configuration pages.\n
 * Project plugins require a project to be loaded and are usually loaded/unloaded along with
 * the project.
 * If your plugin uses the Project interface and/or operates on project-related
 * information then this is a project plugin.
 *
 *
 * @sa Core class documentation for information about features available to
 * plugins from shell applications.
 */
class KDEVPLATFORMINTERFACES_EXPORT IPlugin: public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    /**Constructs a plugin.
     * @param instance The instance for this plugin.
     * @param parent The parent object for the plugin.
     */
    IPlugin(const KComponentData &instance, QObject *parent);

    /**Destructs a plugin.*/
    virtual ~IPlugin();

    /**
     * Signal the plugin that it should cleanup since it will be unloaded soon.
     */
    Q_SCRIPTABLE virtual void unload();

    /**
     * Provides access to the global icon loader
     * @return the plugin's icon loader
     */
    Q_SCRIPTABLE KIconLoader* iconLoader() const;

    /**
     * Provides access to the ICore implementation
     */
    Q_SCRIPTABLE ICore *core() const;

    Q_SCRIPTABLE QStringList extensions() const;

    template<class Extension> Extension* extension()
    {
        if( extensions().contains( qobject_interface_iid<Extension*>() ) ) {
            return qobject_cast<Extension*>( this );
        }
        return 0;
    }
    
    /**
     * Ask the plugin for a ContextActionContainer, which contains actions
     * that will be merged into the context menu.
     * @param context the context describing where the context menu was requested
     * @returns a container describing which actions to merge into which context menu part
     */
    virtual ContextMenuExtension contextMenuExtension( KDevelop::Context* context );

    /**
     * Can create a new KXMLGUIClient, and set it up correctly with all the plugins per-window GUI actions.
     *
     * The caller owns the created object, including all contained actions. The object is destroyed as soon as
     * the mainwindow is closed.
     *
     * The default implementation calls the convenience function @ref createActionsForMainWindow and uses it to fill a custom KXMLGUIClient.
     *
     * Only override this version if you need more specific features of KXMLGUIClient, or other special per-window handling.
     *
     * @param window The main window the actions are created for
     */
    virtual KXMLGUIClient* createGUIForMainWindow( Sublime::MainWindow* window );

    /**
     * This function allows allows setting up plugin actions conveniently. Unless createGUIForMainWindow was overridden,
     * this is called for each new mainwindow, and the plugin should add its actions to @p actions, and write its KXMLGui xml file
     * into @p xmlFile.
     *
     * @param window The main window the actions are created for
     * @param xmlFile Change this to the xml file that needed for merging the actions into the GUI
     * @param actions Add your actions here. A new set of actions has to be created for each mainwindow.
     */
    virtual void createActionsForMainWindow( Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions );

    /**
     * This function is necessary because there is no proper way to signal errors from plugin constructor.
     * @returns True if the plugin has encountered an error, false otherwise.
     */
    virtual bool hasError() const;

    /**
     * Description of the last encountered error, of an empty string if none.
     */
    virtual QString errorDescription() const;

private Q_SLOTS:
    /**
     * Re-initialize the global icon loader
     */
    void newIconLoader() const;

protected:
    void addExtension( const QString& );

    /**
     * Initialize the XML GUI State.
     */
    virtual void initializeGuiState();

private:
    Q_PRIVATE_SLOT(d, void _k_guiClientAdded(KXMLGUIClient *))
    Q_PRIVATE_SLOT(d, void _k_updateState())

    friend class IPluginPrivate;
    class IPluginPrivate* const d;
};

}
#endif

