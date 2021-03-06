/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic String Replacement Filter Processing class.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// StringReplacer includes.
#include "stringreplacerproc.h"
#include "moc_stringreplacerproc.cpp"

// Qt includes.
#include <QtXml/qdom.h>
#include <QtCore/QFile>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "filterproc.h"
#include "talkercode.h"
#include "cdataescaper.h"

/**
 * Constructor.
 */
StringReplacerProc::StringReplacerProc( QObject *parent, QVariantList list) :
    KttsFilterProc(parent, list)
{
}

/**
 * Destructor.
 */
/*virtual*/ StringReplacerProc::~StringReplacerProc()
{
    m_matchList.clear();
    m_substList.clear();
}

bool StringReplacerProc::init(KConfig* c, const QString& configGroup){
    //kDebug() << "StringReplacerProc::init: Running";
    QString wordsFilename =
        KGlobal::dirs()->saveLocation( "data" ,QLatin1String( "kttsd/stringreplacer/" ), false );
    if ( wordsFilename.isEmpty() ) return false;
    wordsFilename += configGroup;
    KConfigGroup config( c, configGroup );
    wordsFilename = config.readEntry( "WordListFile", wordsFilename );

    // Open existing word list.
    QFile file( wordsFilename );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        //kDebug() << "StringReplacerProc::init: couldn't open file " << wordsFilename;
        return false;
    }
    QDomDocument doc( QLatin1String( "" ) );
    if ( !doc.setContent( &file ) ) {
        //kDebug() << "StringReplacerProc::init: couldn't get xml from file " << wordsFilename;
        file.close();
        return false;
    }
    file.close();

    // Clear list.
    m_matchList.clear();
    m_substList.clear();

    // Name setting.
    // QDomNodeList nameList = doc.elementsByTagName( "name" );
    // QDomNode nameNode = nameList.item( 0 );
    // m_widget->nameLineEdit->setText( nameNode.toElement().text() );

    // Language Codes setting.  List may be single element of comma-separated values,
    // or multiple elements.
    m_languageCodeList.clear();
    QDomNodeList languageList = doc.elementsByTagName( QLatin1String( "language-code" ) );
    for ( int ndx=0; ndx < languageList.count(); ++ndx )
    {
        QDomNode languageNode = languageList.item( ndx );
        m_languageCodeList += languageNode.toElement().text().split( QLatin1Char(','), QString::SkipEmptyParts);
    }

    // AppId.  Apply this filter only if DCOP appId of application that queued
    // the text contains this string.  List may be single element of comma-separated values,
    // or multiple elements.
    m_appIdList.clear();
    QDomNodeList appIdList = doc.elementsByTagName( QLatin1String( "appid" ) );
    for ( int ndx=0; ndx < appIdList.count(); ++ndx )
    {
        QDomNode appIdNode = appIdList.item( ndx );
        m_appIdList += appIdNode.toElement().text().split( QLatin1Char( ',' ), QString::SkipEmptyParts);
    }

    // Word list.
    QDomNodeList wordList = doc.elementsByTagName(QLatin1String( "word" ) );
    const int wordListCount = wordList.count();
    for (int wordIndex = 0; wordIndex < wordListCount; ++wordIndex)
    {
        QDomNode wordNode = wordList.item(wordIndex);
        QDomNodeList propList = wordNode.childNodes();
        QString wordType;
        QString matchCase = QLatin1String( "No" ); // Default for old (v<=3.5.3) config files with no <case/>.
        QString match;
        QString subst;
        const int propListCount = propList.count();
        for (int propIndex = 0; propIndex < propListCount; ++propIndex)
        {
            QDomNode propNode = propList.item(propIndex);
            QDomElement prop = propNode.toElement();
            if (prop.tagName() == QLatin1String( "type" )) wordType = prop.text();
            if (prop.tagName() == QLatin1String( "case" )) matchCase = prop.text();
            if (prop.tagName() == QLatin1String( "match" ))
            {
                match = prop.text();
                cdataUnescape( &match );
            }
            if (prop.tagName() == QLatin1String( "subst" ))
            {
                subst = prop.text();
                cdataUnescape( &subst );
            }
        }
        // Build Regular Expression for each word's match string.
        QRegExp rx;
        rx.setCaseSensitivity(matchCase == QLatin1String( "Yes" )?Qt::CaseInsensitive:Qt::CaseSensitive);
        if ( wordType == QLatin1String( "Word" ) )
        {
                // TODO: Does \b honor strange non-Latin1 encodings?
            rx.setPattern( QLatin1String( "\\b" ) + match + QLatin1String( "\\b" ) );
        }
        else
        {
            rx.setPattern( match );
        }
            // Add Regular Expression to list (if valid).
        if ( rx.isValid() )
        {
            m_matchList.append( rx );
            m_substList.append( subst );
        }
    }
    return true;
}

/**
 * Convert input, returning output.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString StringReplacerProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QString& appId)
{
    Q_UNUSED(talkerCode);
    m_wasModified = false;
    // If language doesn't match, return input unmolested.
    //if ( !m_languageCodeList.isEmpty() )
    //{
    //    QString languageCode = talkerCode->languageCode();
    //    //kDebug() << "StringReplacerProc::convert: converting " << inputText <<
    //    //    " if language code " << languageCode << " matches " << m_languageCodeList << endl;
    //    if ( !m_languageCodeList.contains( languageCode ) )
    //    {
    //        if ( !talkerCode->countryCode().isEmpty() )
    //        {
    //            languageCode += '_' + talkerCode->countryCode();
    //             //kDebug() << "StringReplacerProc::convert: converting " << inputText <<
    //             //   " if language code " << languageCode << " matches " << m_languageCodeList << endl;
    //            if ( !m_languageCodeList.contains( languageCode ) ) return inputText;
    //        } else return inputText;
    //    }
    //}
    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
         //kDebug() << "StringReplacerProc::convert: converting " << inputText << " if appId "
         //    << appId << " matches " << m_appIdList << endl;
        bool found = false;
        QString appIdStr = appId;
        for ( int ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.contains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
             //kDebug() << "StringReplacerProc::convert: appId not found";
            return inputText;
        }
    }
    QString newText = inputText;
    const int listCount = m_matchList.count();
    for ( int index = 0; index < listCount; ++index )
    {
        //kDebug() << "newtext = " << newText << " matching " << m_matchList[index].pattern() << " replacing with " << m_substList[index];
        newText.replace( m_matchList[index], m_substList[index] );
    }
    m_wasModified = true;
    return newText;
}

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool StringReplacerProc::wasModified() { return m_wasModified; }

