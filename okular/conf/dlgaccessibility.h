/***************************************************************************
 *   Copyright (C) 2006 by Pino Toscano <toscano.pino@tiscali.it>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _DLGACCESSIBILITY_H
#define _DLGACCESSIBILITY_H

#include <qlist.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE
class Ui_DlgAccessibilityBase;
QT_END_NAMESPACE

class DlgAccessibility : public QWidget
{
    Q_OBJECT

    public:
        DlgAccessibility( QWidget * parent = 0 );
        ~DlgAccessibility();

    private slots:
        void slotColorMode( int mode );

    private:
        Ui_DlgAccessibilityBase * m_dlg;
        QList< QWidget * > m_color_pages;
        int m_selected;
};

#endif
