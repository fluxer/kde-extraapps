/**
  Natural order sorting of strings which contains numbers.

  Copyright 2007 Tobias Koenig <tokoe@kde.org>
  based on the natural order code by Martin Pool <mbp sourcefrog net>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 or at your option version 3 as published by
  the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "qnatsort.h"

static int compare_right( const QString &leftStr, int left, const QString &rightStr, int right )
{
  int bias = 0;
     
  /**
   * The longest run of digits wins.  That aside, the greatest
	 * value wins, but we can't know that it will until we've scanned
	 * both numbers to know that they have the same magnitude, so we
	 * remember it in BIAS.
   */
  for ( ;; left++, right++ ) {
    if ( left >= leftStr.length() && right < rightStr.length() )
      return -1;
    else if ( right >= rightStr.length() && left < leftStr.length() )
      return +1;
    else if ( right >= rightStr.length() && left >= leftStr.length() )
      return bias;
    else if ( !leftStr[ left ].isDigit() && !rightStr[ right ].isDigit() )
      return bias;
    else if ( !leftStr[ left ].isDigit() )
      return -1;
    else if ( !rightStr[ right ].isDigit() )
      return +1;
    else if ( leftStr[ left ] < rightStr[ right ] ) {
      if ( !bias )
        bias = -1;
    } else if ( leftStr[ left ] > rightStr[ right ] ) {
      if ( !bias )
        bias = +1;
    } else if ( leftStr[ left ].isNull() && rightStr[ right ].isNull() )
      return bias;
  }

  return 0;
}

static int compare_left( const QString &leftStr, int left, const QString &rightStr, int right )
{
  /**
   * Compare two left-aligned numbers: the first to have a
   * different value wins.
   */
  for ( ;; left++, right++ ) {
    if ( left >= leftStr.length() && right < rightStr.length() )
      return -1;
    else if ( right >= rightStr.length() && left < leftStr.length() )
      return +1;
    else if ( right >= rightStr.length() && left >= leftStr.length() )
      return 0;
    else if ( !leftStr[ left ].isDigit() && !rightStr[ right ].isDigit() )
      return 0;
    else if ( !leftStr[ left ].isDigit() )
      return -1;
    else if ( !rightStr[ right ].isDigit() )
      return +1;
    else if ( leftStr[ left ] < rightStr[ right ] )
      return -1;
    else if ( leftStr[ left ] > rightStr[ right ] )
      return +1;
  }

  return 0;
}

static int natural_order_compare( const QString &leftStr, const QString &rightStr, bool fold_case )
{
  if ( leftStr.isEmpty() && rightStr.isEmpty() )
    return 0;

  int ai, bi;
  QChar ca, cb;
  int fractional, result;
     
  ai = bi = 0;
  const int aSize = leftStr.size();
  const int bSize = rightStr.size();

  while ( true ) {
    ca = leftStr[ ai ]; cb = rightStr[ bi ];

    /* skip over leading spaces or zeros */
    while ( ca.isSpace() && ++ai < aSize )
      ca = leftStr[ ai ];

    while ( cb.isSpace() && ++bi < bSize )
      cb = rightStr[ bi ];

    /* process run of digits */
    if ( ca.isDigit() && cb.isDigit() ) {
      fractional = (ca == '0' || cb == '0');

      if ( fractional ) {
        if ( (result = compare_left( leftStr, ai, rightStr, bi )) != 0 )
          return result;
      } else {
        if ( (result = compare_right( leftStr, ai, rightStr, bi )) != 0 )
          return result;
      }
    }

    if ( ca.isNull() && cb.isNull() ) {
      /* The strings compare the same.  Perhaps the caller
         will want to call strcmp to break the tie. */
      return 0;
    }

    if ( fold_case ) {
      ca = ca.toUpper();
      cb = cb.toUpper();
    }
	  
    if ( ca < cb )
      return -1;
    else if ( ca > cb )
      return +1;

    ++ai; ++bi;
    if( aSize == ai ) {
      return aSize <= bSize ? -1 : 1;
    }
    if ( bSize == bi ) {
      return bSize <= aSize ? 1 : -1;
    }
  }
}

bool caseSensitiveNaturalOrderLessThen( const QString &left, const QString &right )
{
  return (natural_order_compare( left, right, false ) < 0);
}

bool caseInsensitiveNaturalOrderLessThen( const QString &left, const QString &right )
{
  return (natural_order_compare( left, right, true ) < 0);
}

