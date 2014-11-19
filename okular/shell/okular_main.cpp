/***************************************************************************
 *   Copyright (C) 2002 by Wilco Greven <greven@kde.org>                   *
 *   Copyright (C) 2003 by Christophe Devriese                             *
 *                         <Christophe.Devriese@student.kuleuven.ac.be>    *
 *   Copyright (C) 2003 by Laurent Montel <montel@kde.org>                 *
 *   Copyright (C) 2003-2007 by Albert Astals Cid <aacid@kde.org>          *
 *   Copyright (C) 2004 by Andy Goossens <andygoossens@telenet.be>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "okular_main.h"

#include "shell.h"
#include <kapplication.h>
#include <klocale.h>
#include <QtDBus/qdbusinterface.h>
#include <QTextStream>
#include <kwindowsystem.h>
#include "aboutdata.h"
#include "shellutils.h"

static bool attachUniqueInstance(const QStringList &paths, const QString &serializedOptions)
{
    if (!ShellUtils::unique(serializedOptions) || paths.count() != 1)
        return false;

    QDBusInterface iface("org.kde.okular", "/okularshell", "org.kde.okular");
    if (!iface.isValid())
        return false;

    const QString page = ShellUtils::page(serializedOptions);
    iface.call("openDocument", ShellUtils::urlFromArg(paths[0], ShellUtils::qfileExistFunc(), page).url(), serializedOptions);
    if (!ShellUtils::noRaise(serializedOptions)) {
        iface.call("tryRaise");
    }

    return true;
}

// Ask an existing non-unique instance to open new tabs
static bool attachExistingInstance(const QStringList &paths, const QString &serializedOptions)
{
    if ( paths.count() < 1 )
        return false;

    const QStringList services = QDBusConnection::sessionBus().interface()->registeredServiceNames().value();

    // Don't match the service without trailing "-" (unique instance)
    const QString pattern = "org.kde.okular-";
    const QString myPid = QString::number( kapp->applicationPid() );
    QScopedPointer<QDBusInterface> bestService;
    const int desktop = KWindowSystem::currentDesktop();

    // Select the first instance that isn't us (metric may change in future)
    foreach ( const QString& service, services )
    {
        if ( service.startsWith(pattern) && !service.endsWith( myPid ) )
        {
            bestService.reset( new QDBusInterface(service, "/okularshell", "org.kde.okular") );

            // Find a window that can handle our documents
            const QDBusReply<bool> reply = bestService->call( "canOpenDocs", paths.count(), desktop );
            if( reply.isValid() && reply.value() )
                break;

            bestService.reset();
        }
    }

    if ( !bestService )
        return false;

    foreach( const QString &arg, paths )
    {
        // Copy stdin to temporary file which can be opened by the existing
        // window. The temp file is automatically deleted after it has been
        // opened. Not sure if this behavior is safe on all platforms.
        QScopedPointer<QTemporaryFile> tempFile;
        QString path;
        if( arg == "-" )
        {
            tempFile.reset( new QTemporaryFile );
            QFile stdinFile;
            if( !tempFile->open() || !stdinFile.open(stdin,QIODevice::ReadOnly) )
                return false;

            const size_t bufSize = 1024*1024;
            QScopedPointer<char,QScopedPointerArrayDeleter<char> > buf( new char[bufSize] );
            size_t bytes;
            do
            {
                bytes = stdinFile.read( buf.data(), bufSize );
                tempFile->write( buf.data(), bytes );
            } while( bytes != 0 );

            path = tempFile->fileName();
        }
        else
        {
            // Page only makes sense if we are opening one file
            const QString page = paths.count() == 1 ? ShellUtils::page(serializedOptions) : QString();
            path = ShellUtils::urlFromArg(arg, ShellUtils::qfileExistFunc(), page).url();
        }

        // Returns false if it can't fit another document
        const QDBusReply<bool> reply = bestService->call( "openDocument", path, serializedOptions );
        if( !reply.isValid() || !reply.value() )
        return false;
    }

    bestService->call( "tryRaise" );

    return true;
}

namespace Okular {

Status main(const QStringList &paths, const QString &serializedOptions)
{
    if (ShellUtils::unique(serializedOptions) && paths.count() > 1)
    {
        QTextStream stream(stderr);
        stream << i18n( "Error: Can't open more than one document with the --unique switch" ) << endl;
        return Error;
    }

    // try to attach to existing session, unique or not
    if (attachUniqueInstance(paths, serializedOptions) || attachExistingInstance(paths, serializedOptions))
    {
        return AttachedOtherProcess;
    }

    Shell* shell = new Shell( serializedOptions );
    if ( !shell->isValid() )
    {
        return Error;
    }

    shell->show();
    for ( int i = 0; i < paths.count(); )
    {
        // Page only makes sense if we are opening one file
        const QString page = paths.count() == 1 ? ShellUtils::page(serializedOptions) : QString();
        if ( shell->openDocument( ShellUtils::urlFromArg(paths[i], ShellUtils::qfileExistFunc(), page).url(), serializedOptions) )
        {
            ++i;
        }
        else
        {
            shell = new Shell( serializedOptions );
            if ( !shell->isValid() )
            {
                return Error;
            }
            shell->show();
        }
    }

    return Success;
}

}

/* kate: replace-tabs on; indent-width 4; */
