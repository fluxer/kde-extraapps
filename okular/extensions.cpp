/***************************************************************************
 *   Copyright (C) 2002 by Wilco Greven <greven@kde.org>                   *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "extensions.h"

// local includes
#include "part.h"

namespace Okular
{

/*
 * BrowserExtension class
 */
BrowserExtension::BrowserExtension(Part* parent)
    : KParts::BrowserExtension( parent ), m_part( parent )
{
    emit enableAction("print", true);
    setURLDropHandlingEnabled(true);
}


void BrowserExtension::print()
{
    m_part->slotPrint();
}

}

#include "moc_extensions.cpp"

/* kate: replace-tabs on; indent-width 4; */
