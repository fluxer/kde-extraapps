/*  This file is part of the KDE libraries
    Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KRUNNER_KEMU_H
#define KRUNNER_KEMU_H

#include <plasma/abstractrunner.h>

class KEmuControlRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    KEmuControlRunner(QObject *parent, const QVariantList& args);
    ~KEmuControlRunner();

    void match(Plasma::RunnerContext &context);
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match);

private slots:
    void prep();
};

K_EXPORT_PLASMA_RUNNER(kemucontrol, KEmuControlRunner)

#endif
