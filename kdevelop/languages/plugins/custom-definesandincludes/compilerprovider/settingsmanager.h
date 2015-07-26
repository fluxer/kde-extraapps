/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <language/interfaces/idefinesandincludesmanager.h>

#include "compilerprovider.h"
#include "icompiler.h"

class KConfig;

namespace KDevelop {
class IProject;
}

struct KDEVCOMPILERPROVIDER_EXPORT ConfigEntry
{
    QString path;
    QStringList includes;
    KDevelop::Defines defines;

    ConfigEntry( const QString& path = QString() ) : path( path ) {}

    // FIXME: get rid of this but stay backwards compatible
    void setDefines(const QHash<QString, QVariant>& defines);
};

class KDEVCOMPILERPROVIDER_EXPORT SettingsManager
{
public:
    SettingsManager(bool globalInstance = false);
    ~SettingsManager();

    QList<ConfigEntry> readPaths(KConfig* cfg) const;
    void writePaths(KConfig* cfg, const QList<ConfigEntry>& paths);

    /**
     * @param defaultCompiler the compiler that will be returned, if the @c CompilerProvider
     * doesn't have a factory to create the needed type of compiler.
     *
     * @return stored compiler or nullptr if the project is opened for the first time.
     */
    CompilerPointer currentCompiler(KConfig* cfg, const CompilerPointer& defaultCompiler = {}) const;
    void writeCurrentCompiler(KConfig* cfg, const CompilerPointer& compiler);

    QVector<CompilerPointer> userDefinedCompilers() const;
    void writeUserDefinedCompilers(const QVector<CompilerPointer>& compilers);

    bool needToReparseCurrentProject( KConfig* cfg ) const;

    CompilerProvider* provider();
    const CompilerProvider* provider() const;

    static SettingsManager* globalInstance();

private:
    CompilerProvider m_provider;
    static SettingsManager* s_globalInstance;
};

#endif // SETTINGSMANAGER_H
