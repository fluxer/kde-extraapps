/***************************************************************************
 *   Copyright (C) 2007 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "converter.h"

#include <QtCore/QDate>
#include <QtCore/QUrl>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextFrame>
#include <QtXml/QDomElement>
#include <QtXml/QDomText>

#include <kglobal.h>
#include <klocale.h>

#include <core/action.h>
#include <core/document.h>

#include "document.h"

using namespace FictionBook;

class Converter::TitleInfo
{
    public:
        QStringList mGenres;
        QString mAuthor;
        QString mTitle;
        QStringList mKeywords;
        QDate mDate;
        QDomElement mCoverPage;
        QString mLanguage;
};

class Converter::DocumentInfo
{
    public:
        QString mAuthor;
        QString mProducer;
        QDate mDate;
        QString mId;
        QString mVersion;
};

Converter::Converter()
    : mTextDocument( 0 ), mCursor( 0 ),
      mTitleInfo( 0 ), mDocumentInfo( 0 )
{
}

Converter::~Converter()
{
    delete mTitleInfo;
    delete mDocumentInfo;
}

QTextDocument* Converter::convert( const QString &fileName )
{
    Document fbDocument( fileName );
    if ( !fbDocument.open() ) {
        emit error( fbDocument.lastErrorString(), -1 );
        return 0;
    }

    mTextDocument = new QTextDocument;
    mCursor = new QTextCursor( mTextDocument );
    mSectionCounter = 0;
    mLocalLinks.clear();
    mSectionMap.clear();

    const QDomDocument document = fbDocument.content();

    /**
     * Set the correct page size
     */
    mTextDocument->setPageSize( QSizeF( 600, 800 ) );

    QTextFrameFormat frameFormat;
    frameFormat.setMargin( 20 );

    QTextFrame *rootFrame = mTextDocument->rootFrame();
    rootFrame->setFrameFormat( frameFormat );

    /**
     * Parse the content of the document
     */
    const QDomElement documentElement = document.documentElement();

    if ( documentElement.tagName() != QLatin1String( "FictionBook" ) ) {
        emit error( i18n( "Document is not a valid FictionBook" ), -1 );
        delete mCursor;
        return 0;
    }

    /**
     * First we read all images, so we can calculate the size later.
     */
    QDomElement element = documentElement.firstChildElement();
    while ( !element.isNull() ) {
        if ( element.tagName() == QLatin1String( "binary" ) ) {
            if ( !convertBinary( element ) ) {
                delete mCursor;
                return 0;
            }
        }

        element = element.nextSiblingElement();
    }

    /**
     * Read the rest...
     */
    element = documentElement.firstChildElement();
    while ( !element.isNull() ) {
        if ( element.tagName() == QLatin1String( "description" ) ) {
            if ( !convertDescription( element ) ) {
                delete mCursor;
                return 0;
            }
        } else if ( element.tagName() == QLatin1String( "body" ) ) {
            if ( !mTitleInfo->mCoverPage.isNull() ) {
                convertCover( mTitleInfo->mCoverPage );
                mCursor->insertBlock();
            }

            QTextFrame *topFrame = mCursor->currentFrame();

            QTextFrameFormat frameFormat;
            frameFormat.setBorder( 2 );
            frameFormat.setPadding( 8 );
            frameFormat.setBackground( Qt::lightGray );

            if ( !mTitleInfo->mTitle.isEmpty() ) {
                mCursor->insertFrame( frameFormat );

                QTextCharFormat charFormat;
                charFormat.setFontPointSize( 22 );
                charFormat.setFontWeight( QFont::Bold );
                mCursor->insertText( mTitleInfo->mTitle, charFormat );

                mCursor->setPosition( topFrame->lastPosition() );
            }

            if ( !mTitleInfo->mAuthor.isEmpty() ) {
                frameFormat.setBorder( 1 );
                mCursor->insertFrame( frameFormat );

                QTextCharFormat charFormat;
                charFormat.setFontPointSize( 10 );
                mCursor->insertText( mTitleInfo->mAuthor, charFormat );

                mCursor->setPosition( topFrame->lastPosition() );
                mCursor->insertBlock();
            }

            mCursor->insertBlock();

            if ( !convertBody( element ) ) {
                delete mCursor;
                return 0;
            }
        }

        element = element.nextSiblingElement();
    }

    /**
     * Add document info.
     */
    if ( mTitleInfo ) {
        if ( !mTitleInfo->mTitle.isEmpty() )
            emit addMetaData( Okular::DocumentInfo::Title, mTitleInfo->mTitle );

        if ( !mTitleInfo->mAuthor.isEmpty() )
            emit addMetaData( Okular::DocumentInfo::Author, mTitleInfo->mAuthor );
    }

    if ( mDocumentInfo ) {
        if ( !mDocumentInfo->mProducer.isEmpty() )
            emit addMetaData( Okular::DocumentInfo::Producer, mDocumentInfo->mProducer );

        if ( mDocumentInfo->mDate.isValid() )
            emit addMetaData( Okular::DocumentInfo::CreationDate,
                      KGlobal::locale()->formatDate( mDocumentInfo->mDate, KLocale::ShortDate ) );
    }

    QMapIterator<QString, QPair<int, int> > it( mLocalLinks );
    while ( it.hasNext() ) {
        it.next();

        const QTextBlock block = mSectionMap[ it.key() ];
        if ( !block.isValid() ) // local link without existing target
          continue;

        Okular::DocumentViewport viewport = calculateViewport( mTextDocument, block );

        Okular::GotoAction *action = new Okular::GotoAction( QString(), viewport );

        emit addAction( action, it.value().first, it.value().second );
    }

    delete mCursor;

    return mTextDocument;
}

bool Converter::convertBody( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "section" ) ) {
            mCursor->insertBlock();
            if ( !convertSection( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "image" ) ) {
            if ( !convertImage( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "title" ) ) {
            if ( !convertTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "epigraph" ) ) {
            if ( !convertEpigraph( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertDescription( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "title-info" ) ) {
            if ( !convertTitleInfo( child ) )
                return false;
        } if ( child.tagName() == QLatin1String( "document-info" ) ) {
            if ( !convertDocumentInfo( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertTitleInfo( const QDomElement &element )
{
    delete mTitleInfo;
    mTitleInfo = new TitleInfo;

    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "genre" ) ) {
            QString genre;
            if ( !convertTextNode( child, genre ) )
                return false;

            if ( !genre.isEmpty() )
                mTitleInfo->mGenres.append( genre );
        } else if ( child.tagName() == QLatin1String( "author" ) ) {
            QString firstName, middleName, lastName, dummy;

            if ( !convertAuthor( child, firstName, middleName, lastName, dummy, dummy ) )
                return false;

            mTitleInfo->mAuthor = QString( "%1 %2 %3" ).arg( firstName, middleName, lastName );
        } else if ( child.tagName() == QLatin1String( "book-title" ) ) {
            if ( !convertTextNode( child, mTitleInfo->mTitle ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "keywords" ) ) {
            QString keywords;
            if ( !convertTextNode( child, keywords ) )
                return false;

            mTitleInfo->mKeywords = keywords.split( ' ', QString::SkipEmptyParts );
        } else if ( child.tagName() == QLatin1String( "date" ) ) {
            if ( !convertDate( child, mTitleInfo->mDate ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "coverpage" ) ) {
            mTitleInfo->mCoverPage = child;
        } else if ( child.tagName() == QLatin1String( "lang" ) ) {
            if ( !convertTextNode( child, mTitleInfo->mLanguage ) )
                return false;
        }
        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertDocumentInfo( const QDomElement &element )
{
    delete mDocumentInfo;
    mDocumentInfo = new DocumentInfo;

    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "author" ) ) {
            QString firstName, middleName, lastName, email, nickname;

            if ( !convertAuthor( child, firstName, middleName, lastName, email, nickname ) )
                return false;

            mDocumentInfo->mAuthor = QString( "%1 %2 %3 <%4> (%5)" )
                                      .arg( firstName ).arg( middleName ).arg( lastName )
                                      .arg( email ).arg( nickname );
        } else if ( child.tagName() == QLatin1String( "program-used" ) ) {
            if ( !convertTextNode( child, mDocumentInfo->mProducer ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "date" ) ) {
            if ( !convertDate( child, mDocumentInfo->mDate ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "id" ) ) {
            if ( !convertTextNode( child, mDocumentInfo->mId ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "version" ) ) {
            if ( !convertTextNode( child, mDocumentInfo->mVersion ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}
bool Converter::convertAuthor( const QDomElement &element, QString &firstName, QString &middleName, QString &lastName,
                               QString &email, QString &nickname )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "first-name" ) ) {
            if ( !convertTextNode( child, firstName ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "middle-name" ) ) {
            if ( !convertTextNode( child, middleName ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "last-name" ) ) {
            if ( !convertTextNode( child, lastName ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "email" ) ) {
            if ( !convertTextNode( child, email ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "nickname" ) ) {
            if ( !convertTextNode( child, nickname ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertTextNode( const QDomElement &element, QString &data )
{
    QDomNode child = element.firstChild();
    while ( !child.isNull() ) {
        QDomText text = child.toText();
        if ( !text.isNull() )
            data = text.data();

        child = child.nextSibling();
    }

    return true;
}

bool Converter::convertDate( const QDomElement &element, QDate &date )
{
    if ( element.hasAttribute( "value" ) )
        date = QDate::fromString( element.attribute( "value" ), Qt::ISODate );

    return true;
}

bool Converter::convertSection( const QDomElement &element )
{
    if ( element.hasAttribute( "id" ) )
        mSectionMap.insert( element.attribute( "id" ), mCursor->block() );

    mSectionCounter++;

    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "title" ) ) {
            if ( !convertTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "epigraph" ) ) {
            if ( !convertEpigraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "image" ) ) {
            if ( !convertImage( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "section" ) ) {
            if ( !convertSection( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "p" ) ) {
            QTextBlockFormat format;
            format.setTextIndent( 10 );
            mCursor->insertBlock( format );
            if ( !convertParagraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "poem" ) ) {
            if ( !convertPoem( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "subtitle" ) ) {
            if ( !convertSubTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "cite" ) ) {
            if ( !convertCite( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "empty-line" ) ) {
            if ( !convertEmptyLine( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    mSectionCounter--;

    return true;
}

bool Converter::convertTitle( const QDomElement &element )
{
    QTextFrame *topFrame = mCursor->currentFrame();

    QTextFrameFormat frameFormat;
    frameFormat.setBorder( 1 );
    frameFormat.setPadding( 8 );
    frameFormat.setBackground( Qt::lightGray );

    mCursor->insertFrame( frameFormat );

    QDomElement child = element.firstChildElement();

    bool firstParagraph = true;
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "p" ) ) {
            if ( firstParagraph )
                firstParagraph = false;
            else
                mCursor->insertBlock();

            QTextCharFormat origFormat = mCursor->charFormat();

            QTextCharFormat titleFormat( origFormat );
            titleFormat.setFontPointSize( 22 - ( mSectionCounter*2 ) );
            titleFormat.setFontWeight( QFont::Bold );
            mCursor->setCharFormat( titleFormat );

            if ( !convertParagraph( child ) )
                return false;

            mCursor->setCharFormat( origFormat );

            emit addTitle( mSectionCounter, child.text(), mCursor->block() );

        } else if ( child.tagName() == QLatin1String( "empty-line" ) ) {
            if ( !convertEmptyLine( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    mCursor->setPosition( topFrame->lastPosition() );

    return true;
}


bool Converter::convertParagraph( const QDomElement &element )
{
    QDomNode child = element.firstChild();
    while ( !child.isNull() ) {
        if ( child.isElement() ) {
            const QDomElement childElement = child.toElement();
            if ( childElement.tagName() == QLatin1String( "emphasis" ) ) {
                if ( !convertEmphasis( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "strong" ) ) {
                if ( !convertStrong( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "style" ) ) {
                if ( !convertStyle( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "a" ) ) {
                if ( !convertLink( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "image" ) ) {
                if ( !convertImage( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "strikethrough" ) ) {
                if ( !convertStrikethrough( childElement ) )
                    return false;
            }
        } else if ( child.isText() ) {
            const QDomText childText = child.toText();
            mCursor->insertText( childText.data() );
        }

        child = child.nextSibling();
    }

    return true;
}

bool Converter::convertEmphasis( const QDomElement &element )
{
    QTextCharFormat origFormat = mCursor->charFormat();

    QTextCharFormat italicFormat( origFormat );
    italicFormat.setFontItalic( true );
    mCursor->setCharFormat( italicFormat );

    if ( !convertParagraph( element ) )
        return false;

    mCursor->setCharFormat( origFormat );

    return true;
}

bool Converter::convertStrikethrough( const QDomElement &element )
{
    QTextCharFormat origFormat = mCursor->charFormat();

    QTextCharFormat strikeoutFormat( origFormat );
    strikeoutFormat.setFontStrikeOut( true );
    mCursor->setCharFormat( strikeoutFormat );

    if ( !convertParagraph( element ) )
        return false;

    mCursor->setCharFormat( origFormat );

    return true;
}

bool Converter::convertStrong( const QDomElement &element )
{
    QTextCharFormat origFormat = mCursor->charFormat();

    QTextCharFormat boldFormat( origFormat );
    boldFormat.setFontWeight( QFont::Bold );
    mCursor->setCharFormat( boldFormat );

    if ( !convertParagraph( element ) )
        return false;

    mCursor->setCharFormat( origFormat );

    return true;
}

bool Converter::convertStyle( const QDomElement &element )
{
    if ( !convertParagraph( element ) )
        return false;

    return true;
}

bool Converter::convertBinary( const QDomElement &element )
{
    const QString id = element.attribute( "id" );

    const QDomText textNode = element.firstChild().toText();
    QByteArray data = textNode.data().toLatin1();
    data = QByteArray::fromBase64( data );

    mTextDocument->addResource( QTextDocument::ImageResource, QUrl( id ), QImage::fromData( data ) );

    return true;
}

bool Converter::convertCover( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "image" ) ) {
            if ( !convertImage( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertImage( const QDomElement &element )
{
    QString href = element.attributeNS( "http://www.w3.org/1999/xlink", "href" );

    if ( href.startsWith( '#' ) )
        href = href.mid( 1 );

    const QImage img = qVariantValue<QImage>( mTextDocument->resource( QTextDocument::ImageResource, QUrl( href ) ) );

    QTextImageFormat format;
    format.setName( href );

    if ( img.width() > 560 )
        format.setWidth( 560 );

    format.setHeight( img.height() );

    mCursor->insertImage( format );

    return true;
}

bool Converter::convertEpigraph( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "p" ) ) {
            QTextBlockFormat format;
            format.setTextIndent( 10 );
            mCursor->insertBlock( format );
            if ( !convertParagraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "poem" ) ) {
            if ( !convertPoem( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "cite" ) ) {
            if ( !convertCite( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "empty-line" ) ) {
            if ( !convertEmptyLine( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertPoem( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "title" ) ) {
            if ( !convertTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "epigraph" ) ) {
            if ( !convertEpigraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "empty-line" ) ) {
            if ( !convertEmptyLine( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "stanza" ) ) {
            if ( !convertStanza( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertSubTitle( const QDomElement &element )
{
    Q_UNUSED( element )
    return true;
}

bool Converter::convertCite( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "p" ) ) {
            QTextBlockFormat format;
            format.setTextIndent( 10 );
            mCursor->insertBlock( format );
            if ( !convertParagraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "poem" ) ) {
            if ( !convertParagraph( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "empty-line" ) ) {
            if ( !convertEmptyLine( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}

bool Converter::convertEmptyLine( const QDomElement& )
{
    mCursor->insertText( "\n\n" );
    return true;
}

bool Converter::convertLink( const QDomElement &element )
{
    QString href = element.attributeNS( "http://www.w3.org/1999/xlink", "href" );
    QString type = element.attributeNS( "http://www.w3.org/1999/xlink", "type" );

    if ( type == "note" )
        mCursor->insertText( "[" );

    int startPosition = mCursor->position();

    QTextCharFormat origFormat( mCursor->charFormat() );

    QTextCharFormat format( mCursor->charFormat() );
    format.setForeground( Qt::blue );
    format.setFontUnderline( true );
    mCursor->setCharFormat( format );

    QDomNode child = element.firstChild();
    while ( !child.isNull() ) {
        if ( child.isElement() ) {
            const QDomElement childElement = child.toElement();
            if ( childElement.tagName() == QLatin1String( "emphasis" ) ) {
                if ( !convertEmphasis( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "strong" ) ) {
                if ( !convertStrong( childElement ) )
                    return false;
            } else if ( childElement.tagName() == QLatin1String( "style" ) ) {
                if ( !convertStyle( childElement ) )
                    return false;
            }
        } else if ( child.isText() ) {
            const QDomText text = child.toText();
            if ( !text.isNull() ) {
                mCursor->insertText( text.data() );
            }
        }

        child = child.nextSibling();
    }
    mCursor->setCharFormat( origFormat );

    int endPosition = mCursor->position();

    if ( type == "note" )
        mCursor->insertText( "]" );

    if ( href.startsWith( '#' ) ) { // local link
        mLocalLinks.insert( href.mid( 1 ), QPair<int, int>( startPosition, endPosition ) );
    } else {
        // external link
        Okular::BrowseAction *action = new Okular::BrowseAction( href );
        emit addAction( action, startPosition, endPosition );
    }

    return true;
}

bool Converter::convertStanza( const QDomElement &element )
{
    QDomElement child = element.firstChildElement();
    while ( !child.isNull() ) {
        if ( child.tagName() == QLatin1String( "title" ) ) {
            if ( !convertTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "subtitle" ) ) {
            if ( !convertSubTitle( child ) )
                return false;
        } else if ( child.tagName() == QLatin1String( "v" ) ) {
            QTextBlockFormat format;
            format.setTextIndent( 50 );
            mCursor->insertBlock( format );
            if ( !convertParagraph( child ) )
                return false;
        }

        child = child.nextSiblingElement();
    }

    return true;
}
