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


#include <QtCore>

#ifndef KGPGMISC_H
#define KGPGMISC_H

/**
 * @brief copy-cat from KdepimLibs
 */
class KPIMUtils : public QObject
{
Q_OBJECT
public:
    KPIMUtils();

public slots:
    static bool isValidSimpleAddress( const QString &aStr );
}; // namespace

#endif // KGPGMISC_H

