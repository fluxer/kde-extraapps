/***************************************************************************
 *   Copyright (C) 2002 by Wilco Greven <greven@kde.org>                   *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _EXTENSIONS_H_
#define _EXTENSIONS_H_

#include <kparts/browserextension.h>

namespace Okular
{

class Part;

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    public:
        BrowserExtension(Part*);

    public slots:
        // Automatically detected by the host.
        void print();

    private:
        Part *m_part;
};

class OkularLiveConnectExtension : public KParts::LiveConnectExtension
{
    Q_OBJECT

    public:
        OkularLiveConnectExtension( Part *parent );

        // from LiveConnectExtension
        virtual bool get( const unsigned long objid, const QString &field, Type &type,
                          unsigned long &retobjid, QString &value );
        virtual bool put( const unsigned long objid, const QString &field, const QString &value );
        virtual bool call( const unsigned long objid, const QString &func, const QStringList &args,
                           Type &type, unsigned long &retobjid, QString &value );

    private:
        QString eval( const QString &script );
        void postMessage( const QStringList &args );

        bool m_inEval;
        QString m_evalRes;
};

}

#endif

/* kate: replace-tabs on; indent-width 4; */
