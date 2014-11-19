/***************************************************************************
 *   Copyright (C) 2006 by Pino Toscano <toscano.pino@tiscali.it>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "dlgdebug.h"

#include <qcheckbox.h>
#include <qlayout.h>

#define DEBUG_SIMPLE_BOOL( cfgname, layout ) \
{ \
    QCheckBox * foo = new QCheckBox( cfgname, this ); \
    foo->setObjectName( "kcfg_" cfgname ); \
    layout->addWidget( foo ); \
}

DlgDebug::DlgDebug( QWidget * parent )
    : QWidget( parent )
{
    QVBoxLayout * lay = new QVBoxLayout( this );
    lay->setMargin( 0 );

    DEBUG_SIMPLE_BOOL( "DebugDrawBoundaries", lay );
    DEBUG_SIMPLE_BOOL( "DebugDrawAnnotationRect", lay );
    DEBUG_SIMPLE_BOOL( "TocPageColumn", lay );

    lay->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding ) );
}
