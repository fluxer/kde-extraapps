/***************************************************************************
 *   Copyright (C) 2002 by Wilco Greven <greven@kde.org>                   *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "extensions.h"

// local includes
#include "part.h"

namespace Okular
{

/*
 * BrowserExtension class
 */
BrowserExtension::BrowserExtension(Part* parent)
    : KParts::BrowserExtension( parent ), m_part( parent )
{
    emit enableAction("print", true);
    setURLDropHandlingEnabled(true);
}


void BrowserExtension::print()
{
    m_part->slotPrint();
}


/*
 * OkularLiveConnectExtension class
 */
#define OKULAR_EVAL_RES_OBJ_NAME "__okular_object"
#define OKULAR_EVAL_RES_OBJ "this." OKULAR_EVAL_RES_OBJ_NAME

OkularLiveConnectExtension::OkularLiveConnectExtension( Part *parent )
    : KParts::LiveConnectExtension( parent ), m_inEval( false )
{
}


bool OkularLiveConnectExtension::get( const unsigned long objid, const QString &field,
                                      KParts::LiveConnectExtension::Type &type,
                                      unsigned long &retobjid, QString &value )
{
    Q_UNUSED( value )
    retobjid = objid;
    bool result = false;
    if ( field == QLatin1String( "postMessage" ) )
    {
         type = KParts::LiveConnectExtension::TypeFunction;
         result = true;
    }
    return result;
}


bool OkularLiveConnectExtension::put( const unsigned long objid, const QString &field, const QString &value )
{
    Q_UNUSED( objid )
    if ( m_inEval )
    {
        if ( field == QLatin1String( OKULAR_EVAL_RES_OBJ_NAME ) )
           m_evalRes = value;
        return true;
    }

    return false;
}


bool OkularLiveConnectExtension::call( const unsigned long objid, const QString &func, const QStringList &args,
                                       KParts::LiveConnectExtension::Type &type, unsigned long &retobjid, QString &value )
{
    retobjid = objid;
    bool result = false;
    if ( func == QLatin1String( "postMessage" ) )
    {
        type = KParts::LiveConnectExtension::TypeVoid;
        postMessage( args );
        value = QString();
        result = true;
    }
    return result;
}


QString OkularLiveConnectExtension::eval( const QString &script )
{
    KParts::LiveConnectExtension::ArgList args;
    args.append( qMakePair( KParts::LiveConnectExtension::TypeString, script ) );
    m_evalRes.clear();
    m_inEval = true;
    emit partEvent( 0, "eval", args );
    m_inEval = false;
    return m_evalRes;
}


void OkularLiveConnectExtension::postMessage( const QStringList &args )
{
    QStringList arrayargs;
    Q_FOREACH ( const QString &arg, args )
    {
        QString newarg = arg;
        newarg.replace( '\'', "\\'" );
        arrayargs.append( "\"" + newarg + "\"" );
    }
    const QString arrayarg = '[' + arrayargs.join( ", " ) + ']';
    eval( "if (this.messageHandler && typeof this.messageHandler.onMessage == 'function') "
          "{ this.messageHandler.onMessage(" + arrayarg + ") }" );
}

}

#include "extensions.moc"

/* kate: replace-tabs on; indent-width 4; */
