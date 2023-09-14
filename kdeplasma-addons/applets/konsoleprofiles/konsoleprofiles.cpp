/*  This file is part of the KDE project
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "konsoleprofiles.h"

#include <QTimer>
#include <QFileInfo>
#include <QGraphicsLinearLayout>
#include <Plasma/IconWidget>
#include <Plasma/Separator>
#include <Plasma/ToolButton>
#include <KDirWatch>
#include <KStandardDirs>
#include <KIcon>
#include <KToolInvocation>
#include <KDebug>

// standard issue margin/spacing
static const int s_spacing = 4;

class KonsoleProfilesWidget : public QGraphicsWidget
{
    Q_OBJECT
public:
    KonsoleProfilesWidget(KonsoleProfilesApplet* konsoleprofiles);

private Q_SLOTS:
    void slotUpdateLayout();
    void slotProfileClicked();

private:
    void addSpacer();

    KonsoleProfilesApplet* m_konsoleprofiles;
    QGraphicsLinearLayout* m_layout;
    Plasma::IconWidget* m_iconwidget;
    Plasma::Separator* m_separator;
    QGraphicsWidget* m_spacer;
    QList<Plasma::ToolButton*> m_profilebuttons;
    KDirWatch* m_dirwatch;
};

KonsoleProfilesWidget::KonsoleProfilesWidget(KonsoleProfilesApplet* konsoleprofiles)
    : QGraphicsWidget(konsoleprofiles),
    m_konsoleprofiles(konsoleprofiles),
    m_layout(nullptr),
    m_iconwidget(nullptr),
    m_separator(nullptr),
    m_spacer(nullptr),
    m_dirwatch(nullptr)
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(s_spacing);

    m_iconwidget = new Plasma::IconWidget(this);
    m_iconwidget->setOrientation(Qt::Horizontal);
    m_iconwidget->setAcceptHoverEvents(false);
    m_iconwidget->setAcceptedMouseButtons(Qt::NoButton);
    m_iconwidget->setText(i18n("Konsole Profiles"));
    m_iconwidget->setIcon("utilities-terminal");
    m_layout->addItem(m_iconwidget);

    m_separator = new Plasma::Separator(this);
    m_layout->addItem(m_separator);

    addSpacer();

    setLayout(m_layout);

    m_dirwatch = new KDirWatch(this);
    const QStringList konsoleprofiledirs = KGlobal::dirs()->findDirs("data", "konsole/");
    foreach (const QString &konsoleprofiledir, konsoleprofiledirs) {
        m_dirwatch->addDir(konsoleprofiledir);
    }
    connect(
        m_dirwatch, SIGNAL(dirty(QString)),
        this, SLOT(slotUpdateLayout())
    );
}

void KonsoleProfilesWidget::slotUpdateLayout()
{
    foreach (Plasma::ToolButton* profilebutton, m_profilebuttons) {
        m_layout->removeItem(profilebutton);
    }
    qDeleteAll(m_profilebuttons);
    m_profilebuttons.clear();
    if (m_spacer) {
        m_layout->removeItem(m_spacer);
        delete m_spacer;
        m_spacer = nullptr;
    }

    bool hasprofiles = false;
    const QStringList konsoleprofiles = KGlobal::dirs()->findAllResources(
        "data", "konsole/*.profile",
        KStandardDirs::NoDuplicates
    );
    foreach (const QString &konsoleprofile, konsoleprofiles) {
        KConfig kconfig(konsoleprofile, KConfig::SimpleConfig);
        KConfigGroup kconfiggroup(&kconfig, "General");
        const QString profilename = QFileInfo(konsoleprofile).baseName();
        if (profilename.isEmpty()) {
            kWarning() << "empty profile name for" << konsoleprofile;
            continue;
        }
        hasprofiles = true;
        QString profileprettyname = kconfiggroup.readEntry("Name");
        if (profileprettyname.isEmpty()) {
            profileprettyname = profilename;
        }
        Plasma::ToolButton* profilebutton = new Plasma::ToolButton(this);
        profilebutton->setText(profileprettyname);
        profilebutton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        profilebutton->setProperty("_k_profile", profilename);
        connect(
            profilebutton, SIGNAL(clicked()),
            this, SLOT(slotProfileClicked())
        );
        m_profilebuttons.append(profilebutton);
        m_layout->addItem(profilebutton);
    }

    addSpacer();
    adjustSize();
}

void KonsoleProfilesWidget::addSpacer()
{
    Q_ASSERT(!m_spacer);
    m_spacer = new QGraphicsWidget(this);
    m_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_spacer->setMinimumSize(1, 1);
    m_layout->addItem(m_spacer);
}

void KonsoleProfilesWidget::slotProfileClicked()
{
    Plasma::ToolButton* profilebutton = qobject_cast<Plasma::ToolButton*>(sender());
    const QString profilename = profilebutton->property("_k_profile").toString();
    Q_ASSERT(!profilename.isEmpty());
    QStringList konsoleargs;
    konsoleargs << "--profile" << profilename;
    QString invocationerror;
    const int invocationresult = KToolInvocation::kdeinitExec("konsole", konsoleargs, &invocationerror);
    if (invocationresult != 0) {
        m_konsoleprofiles->showMessage(KIcon("dialog-error"), invocationerror, Plasma::ButtonOk);
    }
}

KonsoleProfilesApplet::KonsoleProfilesApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
    m_konsoleprofileswidget(nullptr)
{
    KGlobal::locale()->insertCatalog("konsoleprofiles");
    setAspectRatioMode(Plasma::AspectRatioMode::IgnoreAspectRatio);
    setPopupIcon("utilities-terminal");
    setPreferredSize(290, 340);
    m_konsoleprofileswidget = new KonsoleProfilesWidget(this);
}

KonsoleProfilesApplet::~KonsoleProfilesApplet()
{
    delete m_konsoleprofileswidget;
}

void KonsoleProfilesApplet::init()
{
    QTimer::singleShot(500, m_konsoleprofileswidget, SLOT(slotUpdateLayout()));
}

QGraphicsWidget* KonsoleProfilesApplet::graphicsWidget()
{
    return m_konsoleprofileswidget;
}

#include "moc_konsoleprofiles.cpp"
#include "konsoleprofiles.moc"
