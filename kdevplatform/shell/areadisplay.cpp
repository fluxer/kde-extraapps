/***************************************************************************
 *   Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "areadisplay.h"

#include "mainwindow.h"
#include "workingsetcontroller.h"
#include "core.h"

#include <sublime/area.h>
#include <interfaces/iuicontroller.h>

#include <KLocalizedString>
#include <KMenuBar>

#include <QMenu>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>

using namespace KDevelop;

AreaDisplay::AreaDisplay(KDevelop::MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent)
{
    setLayout(new QHBoxLayout);
    m_separator = new QLabel("|", this);
    m_separator->setEnabled(false);
    m_separator->setVisible(false);
    layout()->addWidget(m_separator);

    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(Core::self()->workingSetControllerInternal()->createSetManagerWidget(m_mainWindow));

    m_button = new QToolButton(this);
    m_button->setToolTip(i18n(
        "Execute actions to change the area.<br />"
        "An area is a toolview configuration for a specific use case. "
        "From here you can also navigate back to the default code area."));
    m_button->setAutoRaise(true);
    m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_button->setPopupMode(QToolButton::InstantPopup);
    layout()->addWidget(m_button);

    connect(parent, SIGNAL(areaChanged(Sublime::Area*)), SLOT(newArea(Sublime::Area*)));
}

void AreaDisplay::newArea(Sublime::Area* area)
{
    if(m_button->menu())
        m_button->menu()->deleteLater();

    Sublime::Area* currentArea = m_mainWindow->area();

    m_button->setText(currentArea->title());
    m_button->setIcon(KIcon(currentArea->iconName()));

    QMenu* m = new QMenu(m_button);
    m->addActions(area->actions());
    if(currentArea->objectName() != "code") {
        if(!m->actions().isEmpty())
            m->addSeparator();
        m->addAction(KIcon("document-edit"), i18n("Back to code"), this, SLOT(backToCode()), QKeySequence(Qt::AltModifier | Qt::Key_Backspace));
    }
    m_button->setMenu(m);

    //remove the additional widgets we might have added for the last area
    QBoxLayout* l = qobject_cast<QBoxLayout*>(layout());
    if(l->count()>=4) {
        QLayoutItem* item = l->takeAt(0);
        delete item->widget();
        delete item;
    }
    QWidget* w = Core::self()->workingSetControllerInternal()->createSetManagerWidget(m_mainWindow, area);
    w->installEventFilter(this);
    m_separator->setVisible(w->isVisible());
    l->insertWidget(0, w);
}

bool AreaDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Show) {
        m_separator->setVisible(true);
    } else if (event->type() == QEvent::Hide) {
        m_separator->setVisible(false);
    }
    return QObject::eventFilter(obj, event);
}

void AreaDisplay::backToCode()
{
    ICore::self()->uiController()->switchToArea("code", IUiController::ThisWindow);
}

QSize AreaDisplay::minimumSizeHint() const
{
    QSize hint = QWidget::minimumSizeHint();
    hint = hint.boundedTo(QSize(hint.width(), m_mainWindow->menuBar()->height()-1));
    return hint;
}

QSize AreaDisplay::sizeHint() const
{
    QSize hint = QWidget::sizeHint();
    hint = hint.boundedTo(QSize(hint.width(), m_mainWindow->menuBar()->height()-1));
    return hint;
}
