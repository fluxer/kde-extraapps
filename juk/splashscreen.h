/**
 * Copyright (C) 2002-2004 Scott Wheeler <wheeler@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QLabel>

/**
 * Well, all of this session restoration sure is fun, but it's starting to take
 * a while, especially say, if you're building KDE and indexing your file system
 * in the background.  ;-)  So, despite my general hate of splashscreens I
 * thought on appropriate here.
 *
 * As in other places, this is a singleton.  That makes it relatively easy to
 * handle the updates from whichever class seems appropriate through static
 * methods.
 */

class SplashScreen : public QLabel
{
public:
    static SplashScreen *instance();
    static void finishedLoading();
    static void increment();
    static void update();

protected:
    SplashScreen();
    virtual ~SplashScreen();

private:
    void processEvents();

    static SplashScreen *splash;
    static bool done;
    static int count;
};

#endif

// vim: set et sw=4 tw=0 sta:
