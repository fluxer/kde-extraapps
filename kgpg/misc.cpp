/*
 * Copyright (C) 2014 <xakepa10@gmail.com>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "misc.h"

#include <QString>
#include <QRegExp>

/* these functions are copy from KdepimLibs */
bool KPIMUtils::isValidSimpleAddress( const QString &aStr )
{
  // If we are passed an empty string bail right away no need to process further
  // and waste resources
  if ( aStr.isEmpty() ) {
    return false;
  }

  int atChar = aStr.lastIndexOf( '@' );
  QString domainPart = aStr.mid( atChar + 1 );
  QString localPart = aStr.left( atChar );
  bool tooManyAtsFlag = false;
  bool inQuotedString = false;
  int atCount = localPart.count( '@' );

  unsigned int strlen = localPart.length();
  for ( unsigned int index=0; index < strlen; index++ ) {
    switch( localPart[ index ].toLatin1() ) {
    case '"' :
      inQuotedString = !inQuotedString;
      break;
    case '@' :
      if ( inQuotedString ) {
        --atCount;
        if ( atCount == 0 ) {
          tooManyAtsFlag = false;
        }
      }
      break;
    }
  }

  QString addrRx =
    "[a-zA-Z]*[~|{}`\\^?=/+*'&%$#!_\\w.-]*[~|{}`\\^?=/+*'&%$#!_a-zA-Z0-9-]@";

  if ( localPart[ 0 ] == '\"' || localPart[ localPart.length()-1 ] == '\"' ) {
    addrRx = "\"[a-zA-Z@]*[\\w.@-]*[a-zA-Z0-9@]\"@";
  }
  if ( domainPart[ 0 ] == '[' || domainPart[ domainPart.length()-1 ] == ']' ) {
    addrRx += "\\[[0-9]{,3}(\\.[0-9]{,3}){3}\\]";
  } else {
    addrRx += "[\\w-]+(\\.[\\w-]+)*";
  }
  QRegExp rx( addrRx );
  return  rx.exactMatch( aStr ) && !tooManyAtsFlag;
} 
