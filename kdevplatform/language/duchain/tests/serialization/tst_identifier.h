/*
 * This file is part of KDevelop
 * Copyright 2012-2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TESTIDENTIFIER_H
#define KDEVPLATFORM_TESTIDENTIFIER_H

#include <QObject>

class TestIdentifier : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void testIdentifier();
  void testIdentifier_data();

  void testQualifiedIdentifier();
  void testQualifiedIdentifier_data();

  void benchIdentifierCopyConstant();
  void benchIdentifierCopyDynamic();
  void benchQidCopyPush();
};


#endif // KDEVPLATFORM_TESTIDENTIFIER_H
