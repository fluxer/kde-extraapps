/***************************************************************************
 *   Copyright (C) 2005-2014 by the Quassel Project                        *
 *   devel@quassel-irc.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) version 3.                                           *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "coreapplication.h"

#include "core.h"

CoreApplicationInternal::CoreApplicationInternal()
    : _coreCreated(false)
{
}


CoreApplicationInternal::~CoreApplicationInternal()
{
    if (_coreCreated) {
        Core::saveState();
        Core::destroy();
    }
}


bool CoreApplicationInternal::init()
{
    Core::instance(); // create and init the core
    _coreCreated = true;

    if (!Quassel::isOptionSet("norestore"))
        Core::restoreState();

    return true;
}


/*****************************************************************************/

CoreApplication::CoreApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv), Quassel()
{

    setRunMode(Quassel::CoreOnly);
    _internal = new CoreApplicationInternal();
}


CoreApplication::~CoreApplication()
{
    delete _internal;
}


bool CoreApplication::init()
{
    if (Quassel::init() && _internal->init()) {
        return true;
    }
    return false;
}
