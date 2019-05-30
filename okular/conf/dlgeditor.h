/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef DLGEDITOR_H
#define DLGEDITOR_H

#include <qhash.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE
class Ui_DlgEditorBase;
QT_END_NAMESPACE

class DlgEditor : public QWidget
{
    Q_OBJECT

    public:
        DlgEditor( QWidget * parent = 0 );
        virtual ~DlgEditor();

    private slots:
        void editorChanged( int which );

    private:
        Ui_DlgEditorBase * m_dlg;
        QHash< int, QString > m_editors;
};

#endif
