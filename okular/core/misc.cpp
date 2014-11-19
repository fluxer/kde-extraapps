/***************************************************************************
 *   Copyright (C) 2005   by Piotr Szymanski <niedakh@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "core/misc.h"

#include <kdebug.h>

#include "debug_p.h"

using namespace Okular;

class TextSelection::Private
{
    public:
        int direction;
        int it[2];
        NormalizedPoint cur[2];
};

TextSelection::TextSelection( const NormalizedPoint & a, const NormalizedPoint & b )
    : d( new Private )
{
    if (b.y-a.y<0 || (b.y-a.y==0 && b.x-a.x <0))
        d->direction = 1;
    else
        d->direction = 0;

    d->cur[0] = a;
    d->cur[1] = b;
    d->it[d->direction % 2] = -1;
    d->it[(d->direction + 1) % 2] = -1;
}

TextSelection::~TextSelection()
{
    delete d;
}

void TextSelection::end( const NormalizedPoint & p )
{
  // changing direction as in 2b , assuming the bool->int conversion is correct
  int dir1 = d->direction;
  d->direction = (p.y - d->cur[0].y < 0 || (p.y - d->cur[0].y == 0 && p.x - d->cur[0].x < 0));
  if (d->direction != dir1)
    kDebug(OkularDebug) << "changing direction in selection";

  d->cur[1] = p;
}

void TextSelection::itE( int p )
{
    d->it[(d->direction + 1) % 2] = p;
}

void TextSelection::itB( int p )
{
    d->it[(d->direction) % 2] = p;
}

int TextSelection::direction() const
{
    return d->direction;
}

NormalizedPoint TextSelection::start() const
{
    return d->cur[d->direction % 2];
}

NormalizedPoint TextSelection::end() const
{
    return d->cur[(d->direction + 1) % 2];
}

int TextSelection::itB() const
{
    return d->it[d->direction % 2];
}

int TextSelection::itE() const
{
    return d->it[(d->direction + 1) % 2];
}
