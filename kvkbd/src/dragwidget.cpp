#include "dragwidget.h"

#include <QPainter>
#include <QStyleOption>
#include <QStylePainter>
#include <QX11Info>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>


DragWidget::DragWidget(QWidget *parent) :
    QWidget(parent), dragged(false), moved(false), locked(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
}

DragWidget::~DragWidget()
{
}

void DragWidget::blurBackground(bool blurEnabled)
{
    setProperty("blurBackground", QVariant(blurEnabled));
    repaint();
}

void DragWidget::setLocked(bool locked)
{
    this->locked = locked;
}
bool DragWidget::isLocked()
{
    return locked;
}

void DragWidget::mousePressEvent(QMouseEvent *e)
{
    if (locked) {
        return;
    }
    
    gpress = e->globalPos();
    dragged = true;
    dragPoint = e->pos();
}

void DragWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!dragged) {
        return;
    }
    
    moved = true;
    
    QPoint curr(ev->globalPos().x() - dragPoint.x(), ev->globalPos().y() - dragPoint.y());
    move(curr);

}

void DragWidget::mouseReleaseEvent(QMouseEvent *)
{
    dragged = false;
    moved = false;
}

void DragWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, opt);
}

void DragWidget::toggleVisibility()
{
    if (isMinimized()) {
        showNormal();
        show();
        raise();
    } else {
        showMinimized();
        hide();
        lower();
    }

}
