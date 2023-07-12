/*
 *   Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>
 *
 *   based on work by:
 *   Copyright (C) 2007 Thomas Georgiou <TAGeorgiou@gmail.com> and Jeff Cooper <weirdsox11@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dict.h"

#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KDebug>
#include <KIcon>
#include <QTreeView>
#include <QStringListModel>

#include <KColorScheme>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KLineEdit>

#include <Plasma/Animator>
#include <Plasma/IconWidget>
#include <Plasma/LineEdit>
#include <Plasma/TextBrowser>
#include <Plasma/Theme>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>
#include <plasma/animations/animation.h>

#define AUTO_DEFINE_TIMEOUT 500

DictApplet::DictApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , m_graphicsWidget(0)
    , m_wordEdit(0)
{
    setPopupIcon(QLatin1String( "accessories-dictionary" ));
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void DictApplet::init()
{
    m_dataEngine = QLatin1String("dict");

    // tooltip stuff
    Plasma::ToolTipContent toolTipData;
    toolTipData.setAutohide(true);
    toolTipData.setMainText(name());
    toolTipData.setImage(KIcon(QLatin1String("accessories-dictionary")));

    Plasma::ToolTipManager::self()->setContent(this, toolTipData);

    // Only register the tooltip in panels
    switch (formFactor()) {
        case Plasma::Horizontal:
        case Plasma::Vertical:
            Plasma::ToolTipManager::self()->registerWidget(this);
            break;
        default:
            Plasma::ToolTipManager::self()->unregisterWidget(this);
            break;
    }
}

DictApplet::~DictApplet()
{
    m_defBrowser->deleteLater();
}

QGraphicsWidget *DictApplet::graphicsWidget()
{
    if (m_graphicsWidget) {
        return m_graphicsWidget;
    }

    m_wordEdit = new Plasma::LineEdit(this);
    m_wordEdit->nativeWidget()->setClearButtonShown( true );
    m_wordEdit->nativeWidget()->setClickMessage(i18n("Enter word to define here"));
    m_wordEdit->show();

    m_defBrowser = new Plasma::TextBrowser();
    m_defBrowser->nativeWidget()->setOpenLinks(true);
    connect(m_defBrowser->nativeWidget(),SIGNAL(anchorClicked(QUrl)),this,SLOT(linkDefine(QUrl)));
    m_defBrowser->hide();

    //  Icon in upper-left corner
    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon(QLatin1String("accessories-dictionary"));

    //  Position lineedits
    m_icon->setPos(12,3);

    //  Timer for auto-define
    m_timer = new QTimer(this);
    m_timer->setInterval(AUTO_DEFINE_TIMEOUT);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(define()));

    m_horLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_horLayout->addItem(m_icon);
    m_horLayout->addItem(m_wordEdit);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_horLayout);
    m_layout->addItem(m_defBrowser);

    m_source.clear();
    dataEngine(m_dataEngine)->connectSource(m_source, this);
    connect(m_wordEdit, SIGNAL(editingFinished()), this, SLOT(define()));
    connect(m_wordEdit->nativeWidget(), SIGNAL(textChanged(QString)), this, SLOT(autoDefine(QString)));

    dataEngine(m_dataEngine)->connectSource(QLatin1String("list-dictionaries"), this);

    m_graphicsWidget = new QGraphicsWidget(this);
    m_graphicsWidget->setLayout(m_layout);
    m_graphicsWidget->setPreferredSize(500, 200);

    Plasma::Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
    zoomAnim->setTargetWidget(m_wordEdit);
    zoomAnim->setProperty("zoom", 1.0);
    zoomAnim->setProperty("duration", 350);
    zoomAnim->start(QAbstractAnimation::DeleteWhenStopped);

    return m_graphicsWidget;
}

void DictApplet::linkDefine(const QUrl &url)
{
    //kDebug() <<"ACTIVATED";
    m_wordEdit->setText(url.toString());
    define();
}

void DictApplet::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    if (source == QLatin1String("list-dictionaries")) {
        define();
    }

    if (!m_source.isEmpty()) {
        m_defBrowser->show();
    }

    if (data.contains(QLatin1String("text"))) {
        m_defBrowser->nativeWidget()->setHtml(data[QLatin1String("text")].toString());
    }

    updateGeometry();
}

void DictApplet::define()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    QString newSource = m_wordEdit->text();
    if (newSource == m_source) {
        return;
    }

    dataEngine(m_dataEngine)->disconnectSource(m_source, this);

    if (!newSource.isEmpty()) {
        m_source = newSource;
        dataEngine(m_dataEngine)->connectSource(m_source, this);
    } else {
        // make the definition box disappear
        m_defBrowser->hide();
    }

    updateConstraints();
}

void DictApplet::autoDefine(const QString &word)
{
    Q_UNUSED(word)
    m_timer->start();
}

void DictApplet::popupEvent(bool shown)
{
    // kDebug() << shown;
    if (shown && m_wordEdit) {
        focusEditor();
    }
}

void DictApplet::focusEditor()
{
    m_wordEdit->clearFocus();
    m_wordEdit->setFocus();
    m_wordEdit->nativeWidget()->clearFocus();
    m_wordEdit->nativeWidget()->setFocus();
}

#include "moc_dict.cpp"
