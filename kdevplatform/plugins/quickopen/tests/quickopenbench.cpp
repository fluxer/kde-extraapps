/*
 * Copyright <year> Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quickopenbench.h"

#include <QIcon>

#include <qtest_kde.h>

QTEST_KDEMAIN(QuickOpenBench, GUI);

using namespace KDevelop;

QuickOpenBench::QuickOpenBench(QObject* parent)
: QuickOpenTestBase(Core::NoUi, parent)
{
}

void QuickOpenBench::getData()
{
    QTest::addColumn<int>("files");
    QTest::addColumn<QString>("filter");

    QTest::newRow("0100-___") << 100  << "";
    QTest::newRow("0500-___") << 500  << "";
    QTest::newRow("0100-bar") << 100  << "bar";
    QTest::newRow("0500-bar") << 500  << "bar";
    QTest::newRow("0100-1__") << 100  << "1";
    QTest::newRow("0500-1__") << 500  << "1";
    QTest::newRow("0100-f/b") << 100  << "f/b";
    QTest::newRow("0500-f/b") << 500  << "f/b";
}

void QuickOpenBench::benchProjectFileFilter_addRemoveProject()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    QScopedPointer<TestProject> project(getProjectWithFiles(files));

    QBENCHMARK {
        projectController->addProject(project.data());
        projectController->takeProject(project.data());
    }
}

void QuickOpenBench::benchProjectFileFilter_addRemoveProject_data()
{
    getData();
}

void QuickOpenBench::benchProjectFileFilter_reset()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    provider.setFilterText(filter);

    projectController->addProject(project);
    QBENCHMARK {
        provider.reset();
    }
}

void QuickOpenBench::benchProjectFileFilter_reset_data()
{
    getData();
}

void QuickOpenBench::benchProjectFileFilter_setFilter()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);

    projectController->addProject(project);

    provider.reset();

    QBENCHMARK {
        provider.setFilterText(filter);
        provider.setFilterText(QString());
    }
}

void QuickOpenBench::benchProjectFileFilter_setFilter_data()
{
    getData();
}

void QuickOpenBench::benchProjectFileFilter_providerData()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    projectController->addProject(project);
    provider.reset();
    QCOMPARE(provider.itemCount(), uint(files));
    provider.setFilterText(filter);
    QVERIFY(provider.itemCount());
    const int itemIdx = provider.itemCount() - 1;
    QBENCHMARK {
        QuickOpenDataPointer data = provider.data(itemIdx);
        data->text();
    }
}

void QuickOpenBench::benchProjectFileFilter_providerData_data()
{
    getData();
}

void QuickOpenBench::benchProjectFileFilter_providerDataIcon()
{
    QFETCH(int, files);
    QFETCH(QString, filter);

    ProjectFileDataProvider provider;
    TestProject* project = getProjectWithFiles(files);
    projectController->addProject(project);
    provider.reset();
    QCOMPARE(provider.itemCount(), uint(files));
    provider.setFilterText(filter);
    QVERIFY(provider.itemCount());
    const int itemIdx = provider.itemCount() - 1;
    QBENCHMARK {
        QuickOpenDataPointer data = provider.data(itemIdx);
        data->icon();
    }
}

void QuickOpenBench::benchProjectFileFilter_providerDataIcon_data()
{
    getData();
}

#include "moc_quickopenbench.cpp"
