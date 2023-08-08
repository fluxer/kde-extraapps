/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kolourpicker.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qgraphicslinearlayout.h>
#include <qicon.h>
#include <qimage.h>
#include <qmimedata.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtoolbutton.h>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kcolormimedata.h>

#include <plasma/widgets/toolbutton.h>

#if defined(Q_WS_X11)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <fixx11h.h>
#  include <QX11Info>
#endif

static KMenu* buildMenuForColor(const QColor &color)
{
    KMenu *menu = new KMenu();
    const QVariant colorData = qVariantFromValue(color);
    QAction *act = menu->addAction(KIcon("draw-text"), QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    act->setData(colorData);
    QString htmlName = color.name();
    QString htmlNameUp = htmlName.toUpper();
    KIcon mimeIcon("text-html");
    act = menu->addAction(mimeIcon, htmlName);
    act->setData(colorData);
    act = menu->addAction(mimeIcon, htmlName.mid(1));
    act->setData(colorData);

    if (htmlNameUp != htmlName) {
        act = menu->addAction(mimeIcon, htmlNameUp);
        act->setData(colorData);
        act = menu->addAction(mimeIcon, htmlNameUp.mid(1));
        act->setData(colorData);
    }

    menu->addSeparator();
    act = menu->addAction(mimeIcon, "Latex Color");
    act->setData(colorData);

    act = menu->addAction(mimeIcon, htmlName + QString::fromLatin1("ff"));
    act->setData(colorData);
    return menu;
}

static QColor pickColor(const QPoint &point)
{
#if defined(Q_WS_X11)
    // use the X11 API directly for performance reasons
    Window root = RootWindow(QX11Info::display(), QX11Info::appScreen());
    XImage *ximg = XGetImage(QX11Info::display(), root, point.x(), point.y(), 1, 1, -1, ZPixmap);
    unsigned long xpixel = XGetPixel(ximg, 0, 0);
    XDestroyImage(ximg);
    XColor xcol;
    xcol.pixel = xpixel;
    xcol.flags = DoRed | DoGreen | DoBlue;
    XQueryColor(QX11Info::display(), DefaultColormap(QX11Info::display(), QX11Info::appScreen()), &xcol);
    return QColor::fromRgbF(xcol.red / 65535.0, xcol.green / 65535.0, xcol.blue / 65535.0);
#else
    QDesktopWidget *desktop = QApplication::desktop();
    QPixmap pix = QPixmap::grabWindow(desktop->winId(), point.x(), point.y(), 1, 1);
    QImage img = pix.toImage();
    return QColor(img.pixel(0, 0));
#endif
}

class ColorButton : public Plasma::ToolButton
{
    Q_OBJECT
public:
    ColorButton(QGraphicsWidget *parent);

    void setColor(const QColor &color);
    QIcon colorIcon() const;

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event) final;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) final;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) final;
    void dropEvent(QGraphicsSceneDragDropEvent *event) final;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) final;

private:
    void updateColorPixmap();

    QColor m_color;
    QPixmap m_colorpix;
    QPointF m_dragstartpos;
};

ColorButton::ColorButton(QGraphicsWidget *parent)
    : Plasma::ToolButton(parent)
{
    Plasma::ToolButton::setAcceptDrops(true);
}

void ColorButton::setColor(const QColor &color)
{
    m_color = color;
    updateColorPixmap();
    Plasma::ToolButton::setIcon(colorIcon());
}

QIcon ColorButton::colorIcon() const
{
    return QIcon(m_colorpix);
}

void ColorButton::updateColorPixmap()
{
    const QSizeF sizef = Plasma::ToolButton::size();
    const int minsize = qRound(qMin(sizef.width(), sizef.height())) - 4;
    m_colorpix = QPixmap(QSize(minsize, minsize));
    m_colorpix.fill(QColor(Qt::transparent));
    QPainter p(&m_colorpix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(m_color);
    p.drawEllipse(m_colorpix.rect());
    p.end();
}

void ColorButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Plasma::ToolButton::resizeEvent(event);
    updateColorPixmap();
    Plasma::ToolButton::nativeWidget()->setIconSize(m_colorpix.size());
    Plasma::ToolButton::setIcon(colorIcon());
}

void ColorButton::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    kDebug() << event << KColorMimeData::canDecode(event->mimeData());
    if (KColorMimeData::canDecode(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void ColorButton::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (KColorMimeData::canDecode(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void ColorButton::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    // just in case
    if (!KColorMimeData::canDecode(event->mimeData())) {
        event->ignore();
        return;
    }
    const QColor color = KColorMimeData::fromMimeData(event->mimeData());
    Kolourpicker* picker = qobject_cast<Kolourpicker*>(parentWidget());
    kDebug() << event << color << picker;
    picker->addColor(color);
}

void ColorButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_dragstartpos = event->pos();
    Plasma::ToolButton::mousePressEvent(event);
}

void ColorButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton &&
        (event->pos() - m_dragstartpos).manhattanLength() > KGlobalSettings::dndEventDelay())
    {
        QDrag* drag = KColorMimeData::createDrag(m_color, nativeWidget());
        drag->start();
        setDown(false);
    }
}


Kolourpicker::Kolourpicker(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_grabWidget(0)
{
    resize(40, 80);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    QGraphicsLinearLayout *mainlay = new QGraphicsLinearLayout(Qt::Vertical);
    setLayout(mainlay);
    mainlay->setSpacing(4);
    mainlay->setContentsMargins(0.0, 0.0, 0.0, 0.0);

    m_grabWidget = new QWidget( 0,  Qt::X11BypassWindowManagerHint );
    m_grabWidget->move( -1000, -1000 );

    m_grabButton = new Plasma::ToolButton(this);
    m_grabButton->setMinimumSize(20, 20);
    mainlay->addItem(m_grabButton);
    m_grabButton->nativeWidget()->setIcon(KIcon("color-picker"));
    m_grabButton->nativeWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_grabButton, SIGNAL(clicked()), this, SLOT(grabClicked()));

    m_configAndHistory = new ColorButton(this);
    m_configAndHistory->setMinimumSize(20, 20);
    mainlay->addItem(m_configAndHistory);

    m_configAndHistory->setColor(QColor(Qt::gray));
    m_configAndHistory->nativeWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_configAndHistory, SIGNAL(clicked()), this, SLOT(historyClicked()));

    KMenu *menu = new KMenu();
    menu->addTitle(i18n("Color Options"));
    m_configAndHistoryMenu = menu;
    QAction *act = m_configAndHistoryMenu->addAction(KIcon("edit-clear-history"), i18n("Clear History"));
    connect(act, SIGNAL(triggered(bool)), this, SLOT(clearHistory()));
    m_configAndHistoryMenu->addSeparator();

    // building the menu for default color string format.
    KMenu *m_colors_menu = new KMenu();
    m_colors_menu->addTitle(i18n("Default Format"));
    m_colors_format << "r, g, b" << "#RRGGBB" << "RRGGBB" << "#rrggbb" << "rrggbb";
    foreach (const QString& s, m_colors_format) {
      act = m_colors_menu->addAction(KIcon("draw-text"), s);
      act->setData(s);
    }

    m_colors_menu->addSeparator();
    act = m_colors_menu->addAction(KIcon("draw-text"), "Latex");
    act->setData("Latex");

    connect(m_colors_menu, SIGNAL(triggered(QAction*)), this, SLOT(setDefaultColorFormat(QAction*)));
    act = m_colors_menu->menuAction();
    act->setText(i18n("Default Color Format"));
    m_configAndHistoryMenu->addMenu(m_colors_menu);
}

void Kolourpicker::setDefaultColorFormat(QAction *act)
{
    if (!act) {
      return;
    }

    m_color_format = qvariant_cast<QString>(act->data());
}

Kolourpicker::~Kolourpicker()
{
    clearHistory(false);
    delete m_grabWidget;
    delete m_configAndHistoryMenu;
}

void Kolourpicker::init()
{
    configChanged();
}

void Kolourpicker::configChanged()
{
    // remove old entries, we are only interested in ones from the config now
    clearHistory(false);

    KConfigGroup cg = config();

    QList<QString> colorList = cg.readEntry("Colors", QList<QString>());
    m_color_format = cg.readEntry("ColorFormat", QString());

    Q_FOREACH (const QString &color, colorList) {
        addColor(QColor(color), false);
    }
}

void Kolourpicker::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar) {
            setBackgroundHints(Plasma::Applet::StandardBackground);
        } else {
            setBackgroundHints(Plasma::Applet::NoBackground);
        }
    }

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::SizeConstraint) {
        QGraphicsLinearLayout *l = dynamic_cast<QGraphicsLinearLayout *>(layout());
        if (formFactor() == Plasma::Horizontal && size().height() < 40) {
            l->setOrientation(Qt::Horizontal);
        } else {
            l->setOrientation(Qt::Vertical);
        }
    }
}

bool Kolourpicker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_grabWidget && event->type() == QEvent::MouseButtonRelease) {
        m_grabWidget->removeEventFilter(this);
        m_grabWidget->hide();
        m_grabWidget->releaseMouse();
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        const QColor color = pickColor(me->globalPos());
        kDebug() << event->type() << me->globalPos() << color;
        addColor(color);
        colorActionTriggered(color);
    }
    return Plasma::Applet::eventFilter(watched, event);
}

QVariant Kolourpicker::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange) {
        QMetaObject::invokeMethod(this, "installFilter", Qt::QueuedConnection);
    }
    return Plasma::Applet::itemChange(change, value);
}

void Kolourpicker::grabClicked()
{
    if (m_grabWidget) {
        m_grabWidget->show();
        m_grabWidget->installEventFilter( this );
        m_grabWidget->grabMouse(Qt::CrossCursor);
    }
}

void Kolourpicker::historyClicked()
{
    m_configAndHistoryMenu->popup(QCursor::pos());
}

void Kolourpicker::colorActionTriggered(const QColor &color)
{
    if (!color.isValid()) {
        return;
    }

    QMimeData *mime = new QMimeData();
    mime->setColorData(color);

    QString text;
    /*
    converts the color according to the color format choosen by the user
    */
    if (m_color_format == "r, g, b") {
       text = QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue());
    } else if(m_color_format == "#RRGGBB") {
      text = color.name().toUpper();
    } else if(m_color_format == "RRGGBB") {
      text = color.name().toUpper().mid(1);
    } else if(m_color_format == "#rrggbb") {
      text = color.name();
    } else if(m_color_format == "rrggbb") {
      text = color.name().mid(1);
    } else if(m_color_format == "Latex") {
      text = toLatex(color);
    } else {
      text = QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue());
    }

    mime->setText(text);
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

QString Kolourpicker::toLatex(const QColor& color)
{
      qreal r = (qreal)color.red()/255;
      qreal g = (qreal)color.green()/255;
      qreal b = (qreal)color.blue()/255;

      return QString("\\definecolor{ColorName}{rgb}{%1,%2,%3}").arg(r,0,'f',2).arg(g,0,'f',2).arg(b,0,'f',2);
}

void Kolourpicker::colorActionTriggered(QAction *act)
{
    if (!act) {
        return;
    }

    QColor color = qvariant_cast<QColor>(act->data());
    QString text =  act->text().remove('&');

    if(text == i18n("Latex Color")) {
      text = toLatex(color);
    }

    QMimeData *mime = new QMimeData();
    mime->setColorData(color);
    mime->setText(text);
    QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
}

void Kolourpicker::clearHistory(bool save)
{
    m_configAndHistory->setColor(QColor(Qt::gray));
    QHash<QColor, QAction *>::ConstIterator it = m_menus.constBegin(), itEnd = m_menus.constEnd();
    for (; it != itEnd; ++it ) {
        m_configAndHistoryMenu->removeAction(*it);
        delete *it;
    }
    m_menus.clear();
    m_colors.clear();

    if (save) {
        KConfigGroup cg = config();
        saveData(cg);
    }
}

void Kolourpicker::installFilter()
{
    m_grabButton->installSceneEventFilter(this);
}

void Kolourpicker::addColor(const QColor &color, bool save)
{
    m_configAndHistory->setColor(color);

    QHash<QColor, QAction *>::ConstIterator it = m_menus.constFind(color);
    if (it != m_menus.constEnd()) {
        return;
    }
    KMenu *newmenu = buildMenuForColor(color);
    QAction *act = newmenu->menuAction();
    act->setIcon(m_configAndHistory->colorIcon());
    act->setText(QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()));
    connect(newmenu, SIGNAL(triggered(QAction*)), this, SLOT(colorActionTriggered(QAction*)));
    m_configAndHistoryMenu->insertMenu(m_configAndHistoryMenu->actions().at(1), newmenu);
    m_menus.insert(color, act);
    m_colors.append(color.name());
    m_configAndHistory->setEnabled(true);
    if (save) {
        KConfigGroup cg = config();
        saveData(cg);
    }
}

void Kolourpicker::saveData(KConfigGroup &cg)
{
    cg.writeEntry("Colors", m_colors);
    cg.writeEntry("ColorFormat", m_color_format);
    emit configNeedsSaving();
}

#include "moc_kolourpicker.cpp"
#include "kolourpicker.moc"
