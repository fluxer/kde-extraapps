/*  This file is part of the KDE project
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "gscreator.h"

#include <QImage>
#include <QFileInfo>
#include <QProcess>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kdemacros.h>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new GSCreator();
    }
}

bool GSCreator::create(const QString &path, int, int, QImage &img)
{
    const QString gsexe = KStandardDirs::findExe("gs");
    if (gsexe.isEmpty()) {
        kWarning() << "Ghostscript is not available";
        return false;
    }

    const QStringList gsargs = QStringList()
        << QLatin1String("-q")
        << QLatin1String("-dNOPAUSE")
        << QLatin1String("-dSAFER")
        << QLatin1String("-dBATCH")
        << QLatin1String("-dFirstPage=1")
        << QLatin1String("-dLastPage=1")
        << QLatin1String("-sDEVICE=png16m")
        << QLatin1String("-sOutputFile=-")
        << path;
    QProcess gsprocess;
    gsprocess.start(gsexe, gsargs);
    if (gsprocess.waitForFinished() == false || gsprocess.exitCode() != 0) {
        kWarning() << gsprocess.readAllStandardError();
        return false;
    }
    const QByteArray gsimage = gsprocess.readAllStandardOutput();

    if (img.loadFromData(gsimage, "PNG") == false) {
        kWarning() << "Could not load ghostscript image";
        return false;
    }

    return true;
}

ThumbCreator::Flags GSCreator::flags() const
{
    return ThumbCreator::None;
}
