/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef CTESTUTILS_H
#define CTESTUTILS_H

#include <QtCore/QStringList>
#include <cmaketypes.h>

namespace KDevelop
{
class ProjectFolderItem;
}

namespace CTestUtils
{
    void createTestSuites(const QVector<Test>& testSuites, KDevelop::ProjectFolderItem* folder);
}

#endif // CTESTUTILS_H
