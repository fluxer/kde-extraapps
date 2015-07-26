/* This file is part of KDevelop
    Copyright 2013 Milian Wolff <mail@milianw.de>
    Copyright 2008 Alexander Dymo <adymo@kdevelop.org>

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
#ifndef KDEVPLATFORM_PLUGIN_PROJECTFILTERKCM_H
#define KDEVPLATFORM_PLUGIN_PROJECTFILTERKCM_H

#include <project/projectkcmodule.h>

#include "projectfiltersettings.h"

namespace Ui
{
class ProjectFilterSettings;
}

namespace KDevelop
{

class FilterModel;

class ProjectFilterKCM : public ProjectKCModule<ProjectFilterSettings>
{
    Q_OBJECT
public:
    ProjectFilterKCM(QWidget* parent, const QVariantList& args);
    virtual ~ProjectFilterKCM();

    virtual void save();
    virtual void load();
    virtual void defaults();

protected:
    virtual bool eventFilter(QObject* object, QEvent* event);

private slots:
    void add();
    void remove();
    void moveUp();
    void moveDown();
    void selectionChanged();
    void emitChanged();

private:
    FilterModel *m_model;
    QScopedPointer<Ui::ProjectFilterSettings> m_ui;
};

}
#endif
