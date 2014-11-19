/***************************************************************************
 *   Copyright (C) 2006 by Pino Toscano <toscano.pino@tiscali.it>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "dlgperformance.h"

#include <qfont.h>
#include <kiconloader.h>

#include "ui_dlgperformancebase.h"

DlgPerformance::DlgPerformance( QWidget * parent )
    : QWidget( parent )
{
    m_dlg = new Ui_DlgPerformanceBase();
    m_dlg->setupUi( this );

    QFont labelFont = m_dlg->descLabel->font();
    labelFont.setBold( true );
    m_dlg->descLabel->setFont( labelFont );

    m_dlg->cpuLabel->setPixmap( BarIcon( "cpu", 32 ) );
//     m_dlg->memoryLabel->setPixmap( BarIcon( "kcmmemory", 32 ) ); // TODO: enable again when proper icon is available

    connect( m_dlg->kcfg_MemoryLevel, SIGNAL(changed(int)), this, SLOT(radioGroup_changed(int)) );
}

DlgPerformance::~DlgPerformance()
{
    delete m_dlg;
}

void DlgPerformance::radioGroup_changed( int which )
{
    switch ( which )
    {
        case 0:
            m_dlg->descLabel->setText( i18n("Keeps used memory as low as possible. Do not reuse anything. (For systems with low memory.)") );
            break;
        case 1:
            m_dlg->descLabel->setText( i18n("A good compromise between memory usage and speed gain. Preload next page and boost searches. (For systems with 256MB of memory, typically.)") );
            break;
        case 2:
            m_dlg->descLabel->setText( i18n("Keeps everything in memory. Preload next pages. Boost searches. (For systems with more than 512MB of memory.)") );
            break;
        case 3:
	    // xgettext: no-c-format
            m_dlg->descLabel->setText( i18n("Loads and keeps everything in memory. Preload all pages. (Will use at maximum 50% of your total memory or your free memory, whatever is bigger.)"));
            break;
    }
}

#include "dlgperformance.moc"
