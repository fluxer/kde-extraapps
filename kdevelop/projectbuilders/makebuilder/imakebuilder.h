/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef IMAKEBUILDER_H
#define IMAKEBUILDER_H

#include <project/interfaces/iprojectbuilder.h>

#include <QList>
#include <QStringList>
#include <QPair>

class KJob;

/**
@author Andreas Pakulat
*/

/**
 * Used to create make variables of the form KEY=VALUE.
 */
typedef QList< QPair<QString, QString> > MakeVariables;

class IMakeBuilder : public KDevelop::IProjectBuilder
{
public:

    virtual ~IMakeBuilder() {}

    /**
     * Return a build job for the given target.
     *
     * E.g. if @a targetnames is \c 'myModule', the returned job
     * will execute the command:
     *
     * \code make myModule \endcode
     *
     * The command is executed inside the directory of @a item and uses the
     * configuration of its project.
     *
     * @param item Item of the project to build.
     * @param targetname Command-line target name to pass to make.
     */
    virtual KJob* executeMakeTarget(KDevelop::ProjectBaseItem* item,
                                   const QString& targetname ) = 0;

    /**
     * Return a build job for the given targets with optional make variables defined.
     *
     * E.g. if @a targetnames is \code {'all', 'modules'} \endcode and @a variables is
     * \code { 'CFLAGS' : '-Wall', 'CC' : 'gcc-3.4' } \endcode, the returned job should
     * execute this command:
     *
     * \code make CFLAGS=-Wall CC=gcc-3.4 all modules \endcode
     *
     * The command is executed inside the directory of @a item and uses the
     * configuration of its project.
     *
     * @param item Item of the project to build.
     * @param targetnames Optional command-line targets names to pass to make.
     * @param variables Optional list of command-line variables to pass to make.
     */
    virtual KJob* executeMakeTargets(KDevelop::ProjectBaseItem* item,
                                     const QStringList& targetnames = QStringList(),
                                     const MakeVariables& variables = MakeVariables() ) = 0;

signals:
    /**
     * Emitted every time a target is finished being built for a project item.
     */
    void makeTargetBuilt( KDevelop::ProjectBaseItem* item, const QString& targetname );
};

Q_DECLARE_INTERFACE( IMakeBuilder, "org.kdevelop.IMakeBuilder" )

#endif

