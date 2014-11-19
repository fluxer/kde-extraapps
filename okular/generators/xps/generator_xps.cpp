/*
  Copyright (C)  2006, 2009  Brad Hards <bradh@frogmouth.net>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "generator_xps.h"

#include <qdatetime.h>
#include <qfile.h>
#include <qlist.h>
#include <qpainter.h>
#include <qprinter.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurl.h>
#include <QBuffer>
#include <QImageReader>
#include <QMutex>

#include <core/document.h>
#include <core/page.h>
#include <core/area.h>
#include <core/fileprinter.h>

const int XpsDebug = 4712;

static KAboutData createAboutData()
{
    KAboutData aboutData(
         "okular_xps",
         "okular_xps",
         ki18n( "XPS Backend" ),
         "0.3.3",
         ki18n( "An XPS backend" ),
         KAboutData::License_GPL,
         ki18n( "© 2006-2007 Brad Hards\n"
                "© 2007 Jiri Klement\n"
                "© 2008 Pino Toscano" )
    );
    aboutData.addAuthor( ki18n( "Brad Hards" ), KLocalizedString(), "bradh@frogmouth.net" );
    aboutData.addAuthor( ki18n( "Jiri Klement" ), KLocalizedString(), "jiri.klement@gmail.com" );
    aboutData.addAuthor( ki18n( "Pino Toscano" ), KLocalizedString(), "pino@kde.org" );
    return aboutData;
}

OKULAR_EXPORT_PLUGIN( XpsGenerator, createAboutData() )

Q_DECLARE_METATYPE( QGradient* )
Q_DECLARE_METATYPE( XpsPathFigure* )
Q_DECLARE_METATYPE( XpsPathGeometry* )

// From Qt4
static int hex2int(char hex)
{
    QChar hexchar = QLatin1Char(hex);
    int v;
    if (hexchar.isDigit())
        v = hexchar.digitValue();
    else if (hexchar >= QLatin1Char('A') && hexchar <= QLatin1Char('F'))
        v = hexchar.cell() - 'A' + 10;
    else if (hexchar >= QLatin1Char('a') && hexchar <= QLatin1Char('f'))
        v = hexchar.cell() - 'a' + 10;
    else
        v = -1;
    return v;
}

// Modified from Qt4
static QColor hexToRgba(const char *name)
{
    if(name[0] != '#')
        return QColor();
    name++; // eat the leading '#'
    int len = qstrlen(name);
    int r, g, b;
    int a = 255;
    if (len == 6) {
        r = (hex2int(name[0]) << 4) + hex2int(name[1]);
        g = (hex2int(name[2]) << 4) + hex2int(name[3]);
        b = (hex2int(name[4]) << 4) + hex2int(name[5]);
    } else if (len == 8) {
        a = (hex2int(name[0]) << 4) + hex2int(name[1]);
        r = (hex2int(name[2]) << 4) + hex2int(name[3]);
        g = (hex2int(name[4]) << 4) + hex2int(name[5]);
        b = (hex2int(name[6]) << 4) + hex2int(name[7]);
    } else {
        r = g = b = -1;
    }
    if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255) {
        return QColor();
    }
    return QColor(r,g,b,a);
}

static QRectF stringToRectF( const QString &data )
{
    QStringList numbers = data.split(',');
    QPointF origin( numbers.at(0).toDouble(), numbers.at(1).toDouble() );
    QSizeF size( numbers.at(2).toDouble(), numbers.at(3).toDouble() );
    return QRectF( origin, size );
}

static bool parseGUID( const QString &guidString, unsigned short guid[16]) {

    if (guidString.length() <= 35) {
        return false;
    }

    // Maps bytes to positions in guidString
    const static int indexes[] = {6, 4, 2, 0, 11, 9, 16, 14, 19, 21, 24, 26, 28, 30, 32, 34};

    for (int i = 0; i < 16; i++) {
        int hex1 = hex2int(guidString[indexes[i]].cell());
        int hex2 = hex2int(guidString[indexes[i]+1].cell());

        if ((hex1 < 0) || (hex2 < 0))
        {
            return false;
        }

        guid[i] = hex1 * 16 + hex2;
    }

    return true;

}


// Read next token of abbreviated path data
static bool nextAbbPathToken(AbbPathToken *token)
{
    int *curPos = &token->curPos;
    QString data = token->data;

    while ((*curPos < data.length()) && (data.at(*curPos).isSpace()))
    {
        (*curPos)++;
    }

    if (*curPos == data.length())
    {
        token->type = abtEOF;
        return true;
    }

    QChar ch = data.at(*curPos);

    if (ch.isNumber() || (ch == '+') || (ch == '-'))
    {
        int start = *curPos;
        while ((*curPos < data.length()) && (!data.at(*curPos).isSpace()) && (data.at(*curPos) != ',') && (!data.at(*curPos).isLetter() || data.at(*curPos) == 'e'))
    {
        (*curPos)++;
    }
    token->number = data.mid(start, *curPos - start).toDouble();
    token->type = abtNumber;

    } else if (ch == ',')
    {
        token->type = abtComma;
        (*curPos)++;
    } else if (ch.isLetter())
    {
        token->type = abtCommand;
        token->command = data.at(*curPos).cell();
        (*curPos)++;
    } else
    {
        (*curPos)++;
        return false;
    }

    return true;
}

/**
    Read point (two reals delimited by comma) from abbreviated path data
*/
static QPointF getPointFromString(AbbPathToken *token, bool relative, const QPointF &currentPosition) {
    //TODO Check grammar

    QPointF result;
    result.rx() = token->number;
    nextAbbPathToken(token);
    nextAbbPathToken(token); // ,
    result.ry() = token->number;
    nextAbbPathToken(token);

    if (relative)
    {
        result += currentPosition;
    }

    return result;
}

/**
    Read point (two reals delimited by comma) from string
*/
static QPointF getPointFromString(const QString &string)
{
    const int commaPos = string.indexOf(QLatin1Char(','));
    if (commaPos == -1 || string.indexOf(QLatin1Char(','), commaPos + 1) != -1)
        return QPointF();

    QPointF result;
    bool ok = false;
    QStringRef ref = string.midRef(0, commaPos);
    result.setX(QString::fromRawData(ref.constData(), ref.count()).toDouble(&ok));
    if (!ok)
        return QPointF();

    ref = string.midRef(commaPos + 1);
    result.setY(QString::fromRawData(ref.constData(), ref.count()).toDouble(&ok));
    if (!ok)
        return QPointF();

    return result;
}

static Qt::FillRule fillRuleFromString( const QString &data, Qt::FillRule def = Qt::OddEvenFill )
{
    if ( data == QLatin1String( "EvenOdd" ) ) {
        return Qt::OddEvenFill;
    } else if ( data == QLatin1String( "NonZero" ) ) {
        return Qt::WindingFill;
    }
    return def;
}

/**
    Parse an abbreviated path "Data" description
    \param data the string containing the whitespace separated values

    \see XPS specification 4.2.3 and Appendix G
*/
static QPainterPath parseAbbreviatedPathData( const QString &data)
{
    QPainterPath path;

    AbbPathToken token;

    token.data = data;
    token.curPos = 0;

    nextAbbPathToken(&token);

    // Used by Smooth cubic curve (command s)
    char lastCommand = ' ';
    QPointF lastSecondControlPoint;

    while (true)
    {
        if (token.type != abtCommand)
        {
            if (token.type != abtEOF)
            {
                kDebug(XpsDebug).nospace() << "Error in parsing abbreviated path data (" << token.type << "@" << token.curPos << "): " << data;
            }
            return path;
        }

        char command = QChar(token.command).toLower().cell();
        bool isRelative = QChar(token.command).isLower();
        QPointF currPos = path.currentPosition();
        nextAbbPathToken(&token);

        switch (command) {
            case 'f':
                int rule;
                rule = (int)token.number;
                if (rule == 0)
                {
                    path.setFillRule(Qt::OddEvenFill);
                }
                else if (rule == 1)
                {
                    // In xps specs rule 1 means NonZero fill. I think it's equivalent to WindingFill but I'm not sure
                    path.setFillRule(Qt::WindingFill);
                }
                nextAbbPathToken(&token);
                break;
            case 'm': // Move
                while (token.type == abtNumber)
                {
                    QPointF point = getPointFromString(&token, isRelative, currPos);
                    path.moveTo(point);
                }
                break;
            case 'l': // Line
                while (token.type == abtNumber)
                {
                    QPointF point = getPointFromString(&token, isRelative, currPos);
                    path.lineTo(point);
                }
                break;
            case 'h': // Horizontal line
                while (token.type == abtNumber)
                {
                    double x = token.number;
                    if ( isRelative )
                        x += path.currentPosition().x();
                    path.lineTo(x, path.currentPosition().y());
                    nextAbbPathToken(&token);
                }
                break;
            case 'v': // Vertical line
                while (token.type == abtNumber)
                {
                    double y = token.number;
                    if ( isRelative )
                        y += path.currentPosition().y();
                    path.lineTo(path.currentPosition().x(), y);
                    nextAbbPathToken(&token);
                }
                break;
            case 'c': // Cubic bezier curve
                while (token.type == abtNumber)
                {
                    QPointF firstControl = getPointFromString(&token, isRelative, currPos);
                    QPointF secondControl = getPointFromString(&token, isRelative, currPos);
                    QPointF endPoint = getPointFromString(&token, isRelative, currPos);
                    path.cubicTo(firstControl, secondControl, endPoint);

                    lastSecondControlPoint = secondControl;
                }
                break;
            case 'q': // Quadratic bezier curve
                while (token.type == abtNumber)
                {
                    QPointF point1 = getPointFromString(&token, isRelative, currPos);
                    QPointF point2 = getPointFromString(&token, isRelative, currPos);
                    path.quadTo(point1, point2);
                }
                break;
            case 's': // Smooth cubic bezier curve
                while (token.type == abtNumber)
                {
                    QPointF firstControl;
                    if ((lastCommand == 's') || (lastCommand == 'c'))
                    {
                        firstControl = lastSecondControlPoint + (lastSecondControlPoint + path.currentPosition());
                    }
                    else
                    {
                        firstControl = path.currentPosition();
                    }
                    QPointF secondControl = getPointFromString(&token, isRelative, currPos);
                    QPointF endPoint = getPointFromString(&token, isRelative, currPos);
                    path.cubicTo(firstControl, secondControl, endPoint);
                }
                break;
            case 'a': // Arc
                //TODO Implement Arc drawing
                while (token.type == abtNumber)
                {
                    /*QPointF rp =*/ getPointFromString(&token, isRelative, currPos);
                    /*double r = token.number;*/
                    nextAbbPathToken(&token);
                    /*double fArc = token.number; */
                    nextAbbPathToken(&token);
                    /*double fSweep = token.number; */
                    nextAbbPathToken(&token);
                    /*QPointF point = */getPointFromString(&token, isRelative, currPos);
                }
                break;
            case 'z': // Close path
                path.closeSubpath();
                break;
        }

        lastCommand = command;
    }

    return path;
}

/**
   Parse a "Matrix" attribute string
   \param csv the comma separated list of values
   \return the QTransform corresponding to the affine transform
   given in the attribute

   \see XPS specification 7.4.1
*/
static QTransform attsToMatrix( const QString &csv )
{
    QStringList values = csv.split( ',' );
    if ( values.count() != 6 ) {
        return QTransform(); // that is an identity matrix - no effect
    }
    return QTransform( values.at(0).toDouble(), values.at(1).toDouble(),
                    values.at(2).toDouble(), values.at(3).toDouble(),
                    values.at(4).toDouble(), values.at(5).toDouble() );
}

/**
   \return Brush with given color or brush specified by reference to resource
*/
static QBrush parseRscRefColorForBrush( const QString &data )
{
    if (data[0] == '{') {
        //TODO
        kDebug(XpsDebug) << "Reference" << data;
        return QBrush();
    } else {
        return QBrush( hexToRgba( data.toLatin1() ) );
    }
}

/**
   \return Pen with given color or Pen specified by reference to resource
*/
static QPen parseRscRefColorForPen( const QString &data )
{
    if (data[0] == '{') {
        //TODO
        kDebug(XpsDebug) << "Reference" << data;
        return QPen();
    } else {
        return QPen( hexToRgba( data.toLatin1() ) );
    }
}

/**
   \return Matrix specified by given data or by referenced dictionary
*/
static QTransform parseRscRefMatrix( const QString &data )
{
    if (data[0] == '{') {
        //TODO
        kDebug(XpsDebug) << "Reference" << data;
        return QTransform();
    } else {
        return attsToMatrix( data );
    }
}

/**
   \return Path specified by given data or by referenced dictionary
*/
static QPainterPath parseRscRefPath( const QString &data )
{
    if (data[0] == '{') {
        //TODO
        kDebug(XpsDebug) << "Reference" << data;
        return QPainterPath();
    } else {
        return parseAbbreviatedPathData( data );
    }
}

/**
   \return The path of the entry
*/
static QString entryPath( const QString &entry )
{
    const int index = entry.lastIndexOf( QChar::fromLatin1( '/' ) );
    QString ret = QString::fromLatin1( "/" ) + entry.mid( 0, index );
    if ( index > 0 ) {
        ret.append( QChar::fromLatin1( '/' ) );
    }
    return ret;
}

/**
   \return The path of the entry
*/
static QString entryPath( const KZipFileEntry* entry )
{
    return entryPath( entry->path() );
}

/**
   \return The absolute path of the \p location, according to \p path if it's non-absolute
*/
static QString absolutePath( const QString &path, const QString &location )
{
    QString retPath;
    if ( location.at( 0 ) == QLatin1Char( '/' ) ) {
        // already absolute
        retPath = location;
    } else {
        KUrl url = KUrl::fromPath( path );
        url.setFileName( location );
        retPath = url.toLocalFile();
    }
    // it seems paths & file names can also be percent-encoded
    // (XPS won't ever finish surprising me)
    if ( retPath.contains( QLatin1Char( '%' ) ) ) {
        retPath = QUrl::fromPercentEncoding( retPath.toUtf8() );
    }
    return retPath;
}

/**
   Read the content of an archive entry in both the cases:
   a) single file
      + foobar
   b) directory
      + foobar/
        + [0].piece
        + [1].piece
        + ...
        + [x].last.piece

   \see XPS specification 10.1.2
*/
static QByteArray readFileOrDirectoryParts( const KArchiveEntry *entry, QString *pathOfFile = 0 )
{
    QByteArray data;
    if ( entry->isDirectory() ) {
        const KArchiveDirectory* relDir = static_cast<const KArchiveDirectory *>( entry );
        QStringList entries = relDir->entries();
        qSort( entries );
        Q_FOREACH ( const QString &entry, entries ) {
            const KArchiveEntry* relSubEntry = relDir->entry( entry );
            if ( !relSubEntry->isFile() )
                continue;

            const KZipFileEntry* relSubFile = static_cast<const KZipFileEntry *>( relSubEntry );
            data.append( relSubFile->data() );
        }
    } else {
        const KZipFileEntry* relFile = static_cast<const KZipFileEntry *>( entry );
        data.append( relFile->data() );
        if ( pathOfFile ) {
            *pathOfFile = entryPath( relFile );
        }
    }
    return data;
}

/**
   Load the resource \p fileName from the specified \p archive using the case sensitivity \p cs
*/
static const KArchiveEntry* loadEntry( KZip *archive, const QString &fileName, Qt::CaseSensitivity cs )
{
    // first attempt: loading the entry straight as requested
    const KArchiveEntry* entry = archive->directory()->entry( fileName );
    // in case sensitive mode, or if we actually found something, return what we found
    if ( cs == Qt::CaseSensitive || entry ) {
        return entry;
    }

    QString path;
    QString entryName;
    const int index = fileName.lastIndexOf( QChar::fromLatin1( '/' ) );
    QString ret;
    if ( index > 0 ) {
        path = fileName.left( index );
        entryName = fileName.mid( index + 1 );
    } else {
        path = '/';
        entryName = fileName;
    }
    const KArchiveEntry * newEntry = archive->directory()->entry( path );
    if ( newEntry->isDirectory() ) {
        const KArchiveDirectory* relDir = static_cast< const KArchiveDirectory * >( newEntry );
        QStringList relEntries = relDir->entries();
        qSort( relEntries );
        Q_FOREACH ( const QString &relEntry, relEntries ) {
            if ( relEntry.compare( entryName, Qt::CaseInsensitive ) == 0 ) {
                return relDir->entry( relEntry );
            }
        }
    }
    return 0;
}

static const KZipFileEntry* loadFile( KZip *archive, const QString &fileName, Qt::CaseSensitivity cs )
{
    const KArchiveEntry *entry = loadEntry( archive, fileName, cs );
    return entry->isFile() ? static_cast< const KZipFileEntry * >( entry ) : 0;
}

/**
   \return The name of a resource from the \p fileName
*/
static QString resourceName( const QString &fileName )
{
    QString resource = fileName;
    const int slashPos = fileName.lastIndexOf( QLatin1Char( '/' ) );
    const int dotPos = fileName.lastIndexOf( QLatin1Char( '.' ) );
    if ( slashPos > -1 ) {
        if ( dotPos > -1 && dotPos > slashPos ) {
            resource = fileName.mid( slashPos + 1, dotPos - slashPos - 1 );
        } else {
            resource = fileName.mid( slashPos + 1 );
        }
    }
    return resource;
}

static QColor interpolatedColor( const QColor &c1, const QColor &c2 )
{
    QColor res;
    res.setAlpha( ( c1.alpha() + c2.alpha() ) / 2 );
    res.setRed( ( c1.red() + c2.red() ) / 2 );
    res.setGreen( ( c1.green() + c2.green() ) / 2 );
    res.setBlue( ( c1.blue() + c2.blue() ) / 2 );
    return res;
}

static bool xpsGradientLessThan( const XpsGradient &g1, const XpsGradient &g2 )
{
    return qFuzzyCompare( g1.offset, g2.offset )
        ? g1.color.name() < g2.color.name()
        : g1.offset < g2.offset;
}

static int xpsGradientWithOffset( const QList<XpsGradient> &gradients, double offset )
{
    int i = 0;
    Q_FOREACH ( const XpsGradient &grad, gradients ) {
        if ( grad.offset == offset ) {
            return i;
        }
        ++i;
    }
    return -1;
}

/**
   Preprocess a list of gradients.

   \see XPS specification 11.3.1.1
*/
static void preprocessXpsGradients( QList<XpsGradient> &gradients )
{
    if ( gradients.isEmpty() )
        return;

    // sort the gradients (case 1.)
    qStableSort( gradients.begin(), gradients.end(), xpsGradientLessThan );

    // no gradient with stop 0.0 (case 2.)
    if ( xpsGradientWithOffset( gradients, 0.0 ) == -1 ) {
        int firstGreaterThanZero = 0;
        while ( firstGreaterThanZero < gradients.count() && gradients.at( firstGreaterThanZero ).offset < 0.0 )
            ++firstGreaterThanZero;
        // case 2.a: no gradients with stop less than 0.0
        if ( firstGreaterThanZero == 0 ) {
            gradients.prepend( XpsGradient( 0.0, gradients.first().color ) );
        }
        // case 2.b: some gradients with stop more than 0.0
        else if ( firstGreaterThanZero != gradients.count() ) {
            QColor col1 = gradients.at( firstGreaterThanZero - 1 ).color;
            QColor col2 = gradients.at( firstGreaterThanZero ).color;
            for ( int i = 0; i < firstGreaterThanZero; ++i ) {
                gradients.removeFirst();
            }
            gradients.prepend( XpsGradient( 0.0, interpolatedColor( col1, col2 ) ) );
        }
        // case 2.c: no gradients with stop more than 0.0
        else {
            XpsGradient newGrad( 0.0, gradients.last().color );
            gradients.clear();
            gradients.append( newGrad );
        }
    }

    if ( gradients.isEmpty() )
        return;

    // no gradient with stop 1.0 (case 3.)
    if ( xpsGradientWithOffset( gradients, 1.0 ) == -1 ) {
        int firstLessThanOne = gradients.count() - 1;
        while ( firstLessThanOne >= 0 && gradients.at( firstLessThanOne ).offset > 1.0 )
            --firstLessThanOne;
        // case 2.a: no gradients with stop greater than 1.0
        if ( firstLessThanOne == gradients.count() - 1 ) {
            gradients.append( XpsGradient( 1.0, gradients.last().color ) );
        }
        // case 2.b: some gradients with stop more than 1.0
        else if ( firstLessThanOne != -1 ) {
            QColor col1 = gradients.at( firstLessThanOne ).color;
            QColor col2 = gradients.at( firstLessThanOne + 1 ).color;
            for ( int i = firstLessThanOne + 1; i < gradients.count(); ++i ) {
                gradients.removeLast();
            }
            gradients.append( XpsGradient( 1.0, interpolatedColor( col1, col2 ) ) );
        }
        // case 2.c: no gradients with stop less than 1.0
        else {
            XpsGradient newGrad( 1.0, gradients.first().color );
            gradients.clear();
            gradients.append( newGrad );
        }
    }
}

static void addXpsGradientsToQGradient( const QList<XpsGradient> &gradients, QGradient *qgrad )
{
    Q_FOREACH ( const XpsGradient &grad, gradients ) {
        qgrad->setColorAt( grad.offset, grad.color );
    }
}

static void applySpreadStyleToQGradient( const QString &style, QGradient *qgrad )
{
    if ( style.isEmpty() )
        return;

    if ( style == QLatin1String( "Pad" ) ) {
        qgrad->setSpread( QGradient::PadSpread );
    } else if ( style == QLatin1String( "Reflect" ) ) {
        qgrad->setSpread( QGradient::ReflectSpread );
    } else if ( style == QLatin1String( "Repeat" ) ) {
        qgrad->setSpread( QGradient::RepeatSpread );
    }
}

/**
    Read an UnicodeString
    \param string the raw value of UnicodeString

    \see XPS specification 5.1.4
*/
static QString unicodeString( const QString &raw )
{
    QString ret;
    if ( raw.startsWith( QLatin1String( "{}" ) ) ) {
        ret = raw.mid( 2 );
    } else {
        ret = raw;
    }
    return ret;
}


XpsHandler::XpsHandler(XpsPage *page): m_page(page)
{
    m_painter = NULL;
}

XpsHandler::~XpsHandler()
{
}

bool XpsHandler::startDocument()
{
    kDebug(XpsDebug) << "start document" << m_page->m_fileName ;

    XpsRenderNode node;
    node.name = "document";
    m_nodes.push(node);

    return true;
}

bool XpsHandler::startElement( const QString &nameSpace,
                               const QString &localName,
                               const QString &qname,
                               const QXmlAttributes & atts )
{
    Q_UNUSED( nameSpace )
    Q_UNUSED( qname )

    XpsRenderNode node;
    node.name = localName;
    node.attributes = atts;
    processStartElement( node );
    m_nodes.push(node);


    return true;
}

bool XpsHandler::endElement( const QString &nameSpace,
                             const QString &localName,
                             const QString &qname)
{
    Q_UNUSED( nameSpace )
    Q_UNUSED( qname )


    XpsRenderNode node = m_nodes.pop();
    if (node.name != localName) {
        kDebug(XpsDebug) << "Name doesn't match";
    }
    processEndElement( node );
    node.children.clear();
    m_nodes.top().children.append(node);

    return true;
}

void XpsHandler::processGlyph( XpsRenderNode &node )
{
    //TODO Currently ignored attributes: CaretStops, DeviceFontName, IsSideways, OpacityMask, Name, FixedPage.NavigateURI, xml:lang, x:key
    //TODO Indices is only partially implemented
    //TODO Currently ignored child elements: Clip, OpacityMask
    //Handled separately: RenderTransform

    QString att;

    m_painter->save();

    // Get font (doesn't work well because qt doesn't allow to load font from file)
    // This works despite the fact that font size isn't specified in points as required by qt. It's because I set point size to be equal to drawing unit.
    float fontSize = node.attributes.value("FontRenderingEmSize").toFloat();
    // kDebug(XpsDebug) << "Font Rendering EmSize:" << fontSize;
    // a value of 0.0 means the text is not visible (see XPS specs, chapter 12, "Glyphs")
    if ( fontSize < 0.1 ) {
        m_painter->restore();
        return;
    }
    QFont font = m_page->m_file->getFontByName( node.attributes.value("FontUri"), fontSize );
    att = node.attributes.value( "StyleSimulations" );
    if  ( !att.isEmpty() ) {
        if ( att == QLatin1String( "ItalicSimulation" ) ) {
            font.setItalic( true );
        } else if ( att == QLatin1String( "BoldSimulation" ) ) {
            font.setBold( true );
        } else if ( att == QLatin1String( "BoldItalicSimulation" ) ) {
            font.setItalic( true );
            font.setBold( true );
        }
    }
    m_painter->setFont(font);

    //Origin
    QPointF origin( node.attributes.value("OriginX").toDouble(), node.attributes.value("OriginY").toDouble() );

    //Fill
    QBrush brush;
    att = node.attributes.value("Fill");
    if (att.isEmpty()) {
        QVariant data = node.getChildData( "Glyphs.Fill" );
        if (data.canConvert<QBrush>()) {
            brush = data.value<QBrush>();
        } else {
            // no "Fill" attribute and no "Glyphs.Fill" child, so show nothing
            // (see XPS specs, 5.10)
            m_painter->restore();
            return;
        }
    } else {
        brush = parseRscRefColorForBrush( att );
        if ( brush.style() > Qt::NoBrush && brush.style() < Qt::LinearGradientPattern
             && brush.color().alpha() == 0 ) {
            m_painter->restore();
            return;
        }
    }
    m_painter->setBrush( brush );
    m_painter->setPen( QPen( brush, 0 ) );

    // Opacity
    att = node.attributes.value("Opacity");
    if (! att.isEmpty()) {
        bool ok = true;
        double value = att.toDouble( &ok );
        if ( ok && value >= 0.1 ) {
            m_painter->setOpacity( value );
        } else {
            m_painter->restore();
            return;
        }
    }

    //RenderTransform
    att = node.attributes.value("RenderTransform");
    if (!att.isEmpty()) {
        m_painter->setWorldTransform( parseRscRefMatrix( att ), true);
    }

    // Clip
    att = node.attributes.value( "Clip" );
    if ( !att.isEmpty() ) {
        QPainterPath clipPath = parseRscRefPath( att );
        if ( !clipPath.isEmpty() ) {
            m_painter->setClipPath( clipPath );
        }
    }

    // BiDiLevel - default Left-to-Right
    m_painter->setLayoutDirection( Qt::LeftToRight );
    att = node.attributes.value( "BiDiLevel" );
    if ( !att.isEmpty() ) {
        if ( (att.toInt() % 2) == 1 ) {
            // odd BiDiLevel, so Right-to-Left
            m_painter->setLayoutDirection( Qt::RightToLeft );
        }
    }

    // Indices - partial handling only
    att = node.attributes.value( "Indices" );
    QList<qreal> advanceWidths;
    if ( ! att.isEmpty() ) {
        QStringList indicesElements = att.split( ';' );
        for( int i = 0; i < indicesElements.size(); ++i ) {
            if ( indicesElements.at(i).contains( "," ) ) {
                QStringList parts = indicesElements.at(i).split( ',' );
                if (parts.size() == 2 ) {
                    // regular advance case, no offsets
                    advanceWidths.append( parts.at(1).toDouble() * fontSize / 100.0 );
                } else if (parts.size() == 3 ) {
                    // regular advance case, with uOffset
                    qreal AdvanceWidth = parts.at(1).toDouble() * fontSize / 100.0 ;
                    qreal uOffset = parts.at(2).toDouble() / 100.0;
                    advanceWidths.append( AdvanceWidth + uOffset );
                } else {
                    // has vertical offset, but don't know how to handle that yet
                    kDebug(XpsDebug) << "Unhandled Indices element: " << indicesElements.at(i);
                    advanceWidths.append( -1.0 );
                }
            } else {
                // no special advance case
                advanceWidths.append( -1.0 );
            }
        }
    }

    // UnicodeString
    QString stringToDraw( unicodeString( node.attributes.value( "UnicodeString" ) ) );
    QPointF originAdvance(0, 0);
    QFontMetrics metrics = m_painter->fontMetrics();
    for ( int i = 0; i < stringToDraw.size(); ++i ) {
        QChar thisChar = stringToDraw.at( i );
        m_painter->drawText( origin + originAdvance, QString( thisChar ) );
	const qreal advanceWidth = advanceWidths.value( i, qreal(-1.0) );
        if ( advanceWidth > 0.0 ) {
            originAdvance.rx() += advanceWidth;
        } else {
            originAdvance.rx() += metrics.width( thisChar );
        }
    }
    // kDebug(XpsDebug) << "Glyphs: " << atts.value("Fill") << ", " << atts.value("FontUri");
    // kDebug(XpsDebug) << "    Origin: " << atts.value("OriginX") << "," << atts.value("OriginY");
    // kDebug(XpsDebug) << "    Unicode: " << atts.value("UnicodeString");

    m_painter->restore();
}

void XpsHandler::processFill( XpsRenderNode &node )
{
    //TODO Ignored child elements: VirtualBrush

    if (node.children.size() != 1) {
        kDebug(XpsDebug) << "Fill element should have exactly one child";
    } else {
        node.data = node.children[0].data;
    }
}

void XpsHandler::processStroke( XpsRenderNode &node )
{
    //TODO Ignored child elements: VirtualBrush

    if (node.children.size() != 1) {
        kDebug(XpsDebug) << "Stroke element should have exactly one child";
    } else {
        node.data = node.children[0].data;
    }
}

void XpsHandler::processImageBrush( XpsRenderNode &node )
{
    //TODO Ignored attributes: Opacity, x:key, TileMode, ViewBoxUnits, ViewPortUnits
    //TODO Check whether transformation works for non standard situations (viewbox different that whole image, Transform different that simple move & scale, Viewport different than [0, 0, 1, 1]

    QString att;
    QBrush brush;

    QRectF viewport = stringToRectF( node.attributes.value( "Viewport" ) );
    QRectF viewbox = stringToRectF( node.attributes.value( "Viewbox" ) );
    QImage image = m_page->loadImageFromFile( node.attributes.value( "ImageSource" ) );

    // Matrix which can transform [0, 0, 1, 1] rectangle to given viewbox
    QTransform viewboxMatrix = QTransform( viewbox.width() * image.physicalDpiX() / 96, 0, 0, viewbox.height() * image.physicalDpiY() / 96, viewbox.x(), viewbox.y() );

    // Matrix which can transform [0, 0, 1, 1] rectangle to given viewport
    //TODO Take ViewPort into account
    QTransform viewportMatrix;
    att = node.attributes.value( "Transform" );
    if ( att.isEmpty() ) {
        QVariant data = node.getChildData( "ImageBrush.Transform" );
        if (data.canConvert<QTransform>()) {
            viewportMatrix = data.value<QTransform>();
        } else {
            viewportMatrix = QTransform();
        }
    } else {
        viewportMatrix = parseRscRefMatrix( att );
    }
    viewportMatrix = viewportMatrix * QTransform( viewport.width(), 0, 0, viewport.height(), viewport.x(), viewport.y() );


    brush = QBrush( image );
    brush.setTransform( viewboxMatrix.inverted() * viewportMatrix );

    node.data = qVariantFromValue( brush );
}

void XpsHandler::processPath( XpsRenderNode &node )
{
    //TODO Ignored attributes: Clip, OpacityMask, StrokeEndLineCap, StorkeStartLineCap, Name, FixedPage.NavigateURI, xml:lang, x:key, AutomationProperties.Name, AutomationProperties.HelpText, SnapsToDevicePixels
    //TODO Ignored child elements: RenderTransform, Clip, OpacityMask
    // Handled separately: RenderTransform
    m_painter->save();

    QString att;
    QVariant data;

    // Get path
    XpsPathGeometry * pathdata = node.getChildData( "Path.Data" ).value< XpsPathGeometry * >();
    att = node.attributes.value( "Data" );
    if (! att.isEmpty() ) {
        QPainterPath path = parseRscRefPath( att );
        delete pathdata;
        pathdata = new XpsPathGeometry();
        pathdata->paths.append( new XpsPathFigure( path, true ) );
    }
    if ( !pathdata ) {
        // nothing to draw
        m_painter->restore();
        return;
    }

    // Set Fill
    att = node.attributes.value( "Fill" );
    QBrush brush;
    if (! att.isEmpty() ) {
        brush = parseRscRefColorForBrush( att );
    } else {
        data = node.getChildData( "Path.Fill" );
        if (data.canConvert<QBrush>()) {
            brush = data.value<QBrush>();
        }
    }
    m_painter->setBrush( brush );

    // Stroke (pen)
    att = node.attributes.value( "Stroke" );
    QPen pen( Qt::transparent );
    if  (! att.isEmpty() ) {
        pen = parseRscRefColorForPen( att );
    } else {
        data = node.getChildData( "Path.Stroke" );
        if (data.canConvert<QBrush>()) {
            pen.setBrush( data.value<QBrush>() );
        }
    }
    att = node.attributes.value( "StrokeThickness" );
    if  (! att.isEmpty() ) {
        bool ok = false;
        int thickness = att.toInt( &ok );
        if (ok)
            pen.setWidth( thickness );
    }
    att = node.attributes.value( "StrokeDashArray" );
    if  ( !att.isEmpty() ) {
        const QStringList pieces = att.split( QLatin1Char( ' ' ), QString::SkipEmptyParts );
        QVector<qreal> dashPattern( pieces.count() );
        bool ok = false;
        for ( int i = 0; i < pieces.count(); ++i ) {
            qreal value = pieces.at( i ).toInt( &ok );
            if ( ok ) {
                dashPattern[i] = value;
            } else {
                break;
            }
        }
        if ( ok ) {
            pen.setDashPattern( dashPattern );
        }
    }
    att = node.attributes.value( "StrokeDashOffset" );
    if  ( !att.isEmpty() ) {
        bool ok = false;
        int offset = att.toInt( &ok );
        if ( ok )
            pen.setDashOffset( offset );
    }
    att = node.attributes.value( "StrokeDashCap" );
    if  ( !att.isEmpty() ) {
        Qt::PenCapStyle cap = Qt::FlatCap;
        if ( att == QLatin1String( "Flat" ) ) {
            cap = Qt::FlatCap;
        } else if ( att == QLatin1String( "Round" ) ) {
            cap = Qt::RoundCap;
        } else if ( att == QLatin1String( "Square" ) ) {
            cap = Qt::SquareCap;
        }
        // ### missing "Triangle"
        pen.setCapStyle( cap );
    }
    att = node.attributes.value( "StrokeLineJoin" );
    if  ( !att.isEmpty() ) {
        Qt::PenJoinStyle joinStyle = Qt::MiterJoin;
        if ( att == QLatin1String( "Miter" ) ) {
            joinStyle = Qt::MiterJoin;
        } else if ( att == QLatin1String( "Bevel" ) ) {
            joinStyle = Qt::BevelJoin;
        } else if ( att == QLatin1String( "Round" ) ) {
            joinStyle = Qt::RoundJoin;
        }
        pen.setJoinStyle( joinStyle );
    }
    att = node.attributes.value( "StrokeMiterLimit" );
    if  ( !att.isEmpty() ) {
        bool ok = false;
        double limit = att.toDouble( &ok );
        if ( ok ) {
            // we have to divide it by two, as XPS consider half of the stroke width,
            // while Qt the whole of it
            pen.setMiterLimit( limit / 2 );
        }
    }
    m_painter->setPen( pen );

    // Opacity
    att = node.attributes.value("Opacity");
    if (! att.isEmpty()) {
        m_painter->setOpacity(att.toDouble());
    }

    // RenderTransform
    att = node.attributes.value( "RenderTransform" );
    if (! att.isEmpty() ) {
        m_painter->setWorldTransform( parseRscRefMatrix( att ), true );
    }
    if ( !pathdata->transform.isIdentity() ) {
        m_painter->setWorldTransform( pathdata->transform, true );
    }

    Q_FOREACH ( XpsPathFigure *figure, pathdata->paths ) {
        m_painter->setBrush( figure->isFilled ? brush : QBrush() );
        m_painter->drawPath( figure->path );
    }

    delete pathdata;

    m_painter->restore();
}

void XpsHandler::processPathData( XpsRenderNode &node )
{
    if (node.children.size() != 1) {
        kDebug(XpsDebug) << "Path.Data element should have exactly one child";
    } else {
        node.data = node.children[0].data;
    }
}

void XpsHandler::processPathGeometry( XpsRenderNode &node )
{
    XpsPathGeometry * geom = new XpsPathGeometry();

    Q_FOREACH ( const XpsRenderNode &child, node.children ) {
        if ( child.data.canConvert<XpsPathFigure *>() ) {
            XpsPathFigure *figure = child.data.value<XpsPathFigure *>();
            geom->paths.append( figure );
        }
    }

    QString att;

    att = node.attributes.value( "Figures" );
    if ( !att.isEmpty() ) {
        QPainterPath path = parseRscRefPath( att );
        qDeleteAll( geom->paths );
        geom->paths.clear();
        geom->paths.append( new XpsPathFigure( path, true ) );
    }

    att = node.attributes.value( "FillRule" );
    if ( !att.isEmpty() ) {
        geom->fillRule = fillRuleFromString( att );
    }

    // Transform
    att = node.attributes.value( "Transform" );
    if ( !att.isEmpty() ) {
        geom->transform = parseRscRefMatrix( att );
    }

    if ( !geom->paths.isEmpty() ) {
        node.data = qVariantFromValue( geom );
    } else {
        delete geom;
    }
}

void XpsHandler::processPathFigure( XpsRenderNode &node )
{
    //TODO Ignored child elements: ArcSegment

    QString att;
    QPainterPath path;

    att = node.attributes.value( "StartPoint" );
    if ( !att.isEmpty() ) {
        QPointF point = getPointFromString( att );
        path.moveTo( point );
    } else {
        return;
    }

    Q_FOREACH ( const XpsRenderNode &child, node.children ) {
        bool isStroked = true;
        att = node.attributes.value( "IsStroked" );
        if ( !att.isEmpty() ) {
            isStroked = att == QLatin1String( "true" );
        }
        if ( !isStroked ) {
            continue;
        }

        // PolyLineSegment
        if ( child.name == QLatin1String( "PolyLineSegment" ) ) {
            att = child.attributes.value( "Points" );
            if ( !att.isEmpty() ) {
                const QStringList points = att.split( QLatin1Char( ' ' ), QString::SkipEmptyParts );
                Q_FOREACH ( const QString &p, points ) {
                    QPointF point = getPointFromString( p );
                    path.lineTo( point );
                }
            }
        }
        // PolyBezierSegment
        else if ( child.name == QLatin1String( "PolyBezierSegment" ) ) {
            att = child.attributes.value( "Points" );
            if ( !att.isEmpty() ) {
                const QStringList points = att.split( QLatin1Char( ' ' ), QString::SkipEmptyParts );
                if ( points.count() % 3 == 0 ) {
                    for ( int i = 0; i < points.count(); ) {
                        QPointF firstControl = getPointFromString( points.at( i++ ) );
                        QPointF secondControl = getPointFromString( points.at( i++ ) );
                        QPointF endPoint = getPointFromString( points.at( i++ ) );
                        path.cubicTo(firstControl, secondControl, endPoint);
                    }
                }
            }
        }
        // PolyQuadraticBezierSegment
        else if ( child.name == QLatin1String( "PolyQuadraticBezierSegment" ) ) {
            att = child.attributes.value( "Points" );
            if ( !att.isEmpty() ) {
                const QStringList points = att.split( QLatin1Char( ' ' ), QString::SkipEmptyParts );
                if ( points.count() % 2 == 0 ) {
                    for ( int i = 0; i < points.count(); ) {
                        QPointF point1 = getPointFromString( points.at( i++ ) );
                        QPointF point2 = getPointFromString( points.at( i++ ) );
                        path.quadTo( point1, point2 );
                    }
                }
            }
        }
    }

    bool closePath = false;
    att = node.attributes.value( "IsClosed" );
    if ( !att.isEmpty() ) {
        closePath = att == QLatin1String( "true" );
    }
    if ( closePath ) {
        path.closeSubpath();
    }

    bool isFilled = true;
    att = node.attributes.value( "IsFilled" );
    if ( !att.isEmpty() ) {
        isFilled = att == QLatin1String( "true" );
    }

    if ( !path.isEmpty() ) {
        node.data = qVariantFromValue( new XpsPathFigure( path, isFilled ) );
    }
}

void XpsHandler::processStartElement( XpsRenderNode &node )
{
    if (node.name == "Canvas") {
        m_painter->save();
        QString att = node.attributes.value( "RenderTransform" );
        if ( !att.isEmpty() ) {
            m_painter->setWorldTransform( parseRscRefMatrix( att ), true );
        }
        att = node.attributes.value( "Opacity" );
        if ( !att.isEmpty() ) {
            double value = att.toDouble();
            if ( value > 0.0 && value <= 1.0 ) {
                m_painter->setOpacity( m_painter->opacity() * value );
            } else {
                // setting manually to 0 is necessary to "disable"
                // all the stuff inside
                m_painter->setOpacity( 0.0 );
            }
        }
    }
}

void XpsHandler::processEndElement( XpsRenderNode &node )
{
    if (node.name == "Glyphs") {
        processGlyph( node );
    } else if (node.name == "Path") {
        processPath( node );
    } else if (node.name == "MatrixTransform") {
        //TODO Ignoring x:key
        node.data = qVariantFromValue( QTransform( attsToMatrix( node.attributes.value( "Matrix" ) ) ) );
    } else if ((node.name == "Canvas.RenderTransform") || (node.name == "Glyphs.RenderTransform") || (node.name == "Path.RenderTransform"))  {
        QVariant data = node.getRequiredChildData( "MatrixTransform" );
        if (data.canConvert<QTransform>()) {
            m_painter->setWorldTransform( data.value<QTransform>(), true );
        }
    } else if (node.name == "Canvas") {
        m_painter->restore();
    } else if ((node.name == "Path.Fill") || (node.name == "Glyphs.Fill")) {
        processFill( node );
    } else if (node.name == "Path.Stroke") {
        processStroke( node );
    } else if (node.name == "SolidColorBrush") {
        //TODO Ignoring opacity, x:key
        node.data = qVariantFromValue( QBrush( QColor( hexToRgba( node.attributes.value( "Color" ).toLatin1() ) ) ) );
    } else if (node.name == "ImageBrush") {
        processImageBrush( node );
    } else if (node.name == "ImageBrush.Transform") {
        node.data = node.getRequiredChildData( "MatrixTransform" );
    } else if (node.name == "LinearGradientBrush") {
        XpsRenderNode * gradients = node.findChild( "LinearGradientBrush.GradientStops" );
        if ( gradients && gradients->data.canConvert< QGradient * >() ) {
            QPointF start = getPointFromString( node.attributes.value( "StartPoint" ) );
            QPointF end = getPointFromString( node.attributes.value( "EndPoint" ) );
            QLinearGradient * qgrad = static_cast< QLinearGradient * >( gradients->data.value< QGradient * >() );
            qgrad->setStart( start );
            qgrad->setFinalStop( end );
            applySpreadStyleToQGradient( node.attributes.value( "SpreadMethod" ), qgrad );
            node.data = qVariantFromValue( QBrush( *qgrad ) );
            delete qgrad;
        }
    } else if (node.name == "RadialGradientBrush") {
        XpsRenderNode * gradients = node.findChild( "RadialGradientBrush.GradientStops" );
        if ( gradients && gradients->data.canConvert< QGradient * >() ) {
            QPointF center = getPointFromString( node.attributes.value( "Center" ) );
            QPointF origin = getPointFromString( node.attributes.value( "GradientOrigin" ) );
            double radiusX = node.attributes.value( "RadiusX" ).toDouble();
            double radiusY = node.attributes.value( "RadiusY" ).toDouble();
            QRadialGradient * qgrad = static_cast< QRadialGradient * >( gradients->data.value< QGradient * >() );
            qgrad->setCenter( center );
            qgrad->setFocalPoint( origin );
            // TODO what in case of different radii?
            qgrad->setRadius( qMin( radiusX, radiusY ) );
            applySpreadStyleToQGradient( node.attributes.value( "SpreadMethod" ), qgrad );
            node.data = qVariantFromValue( QBrush( *qgrad ) );
            delete qgrad;
        }
    } else if (node.name == "LinearGradientBrush.GradientStops") {
        QList<XpsGradient> gradients;
        Q_FOREACH ( const XpsRenderNode &child, node.children ) {
            double offset = child.attributes.value( "Offset" ).toDouble();
            QColor color = hexToRgba( child.attributes.value( "Color" ).toLatin1() );
            gradients.append( XpsGradient( offset, color ) );
        }
        preprocessXpsGradients( gradients );
        if ( !gradients.isEmpty() ) {
            QLinearGradient * qgrad = new QLinearGradient();
            addXpsGradientsToQGradient( gradients, qgrad );
            node.data = qVariantFromValue< QGradient * >( qgrad );
        }
    } else if (node.name == "RadialGradientBrush.GradientStops") {
        QList<XpsGradient> gradients;
        Q_FOREACH ( const XpsRenderNode &child, node.children ) {
            double offset = child.attributes.value( "Offset" ).toDouble();
            QColor color = hexToRgba( child.attributes.value( "Color" ).toLatin1() );
            gradients.append( XpsGradient( offset, color ) );
        }
        preprocessXpsGradients( gradients );
        if ( !gradients.isEmpty() ) {
            QRadialGradient * qgrad = new QRadialGradient();
            addXpsGradientsToQGradient( gradients, qgrad );
            node.data = qVariantFromValue< QGradient * >( qgrad );
        }
    } else if (node.name == "PathFigure") {
        processPathFigure( node );
    } else if (node.name == "PathGeometry") {
        processPathGeometry( node );
    } else if (node.name == "Path.Data") {
        processPathData( node );
    } else {
        //kDebug(XpsDebug) << "Unknown element: " << node->name;
    }
}

XpsPage::XpsPage(XpsFile *file, const QString &fileName): m_file( file ),
    m_fileName( fileName ), m_pageIsRendered(false)
{
    m_pageImage = NULL;

    // kDebug(XpsDebug) << "page file name: " << fileName;

    const KZipFileEntry* pageFile = static_cast<const KZipFileEntry *>(m_file->xpsArchive()->directory()->entry( fileName ));

    QXmlStreamReader xml;
    xml.addData( readFileOrDirectoryParts( pageFile ) );
    while ( !xml.atEnd() )
    {
        xml.readNext();
        if ( xml.isStartElement() && ( xml.name() == "FixedPage" ) )
        {
            QXmlStreamAttributes attributes = xml.attributes();
            m_pageSize.setWidth( attributes.value( "Width" ).toString().toDouble() );
            m_pageSize.setHeight( attributes.value( "Height" ).toString().toDouble() );
            break;
        }
    }
    if ( xml.error() )
    {
        kDebug(XpsDebug) << "Could not parse XPS page:" << xml.errorString();
    }
}

XpsPage::~XpsPage()
{
    delete m_pageImage;
}

bool XpsPage::renderToImage( QImage *p )
{

    if ((m_pageImage == NULL) || (m_pageImage->size() != p->size())) {
        delete m_pageImage;
        m_pageImage = new QImage( p->size(), QImage::Format_ARGB32 );
        // Set one point = one drawing unit. Useful for fonts, because xps specifies font size using drawing units, not points as usual
        m_pageImage->setDotsPerMeterX( 2835 );
        m_pageImage->setDotsPerMeterY( 2835 );

        m_pageIsRendered = false;
    }
    if (! m_pageIsRendered) {
        m_pageImage->fill( qRgba( 255, 255, 255, 255 ) );
        QPainter painter( m_pageImage );
        renderToPainter( &painter );
        m_pageIsRendered = true;
    }

    *p = *m_pageImage;

    return true;
}

bool XpsPage::renderToPainter( QPainter *painter )
{
    XpsHandler handler( this );
    handler.m_painter = painter;
    handler.m_painter->setWorldTransform(QTransform().scale((qreal)painter->device()->width() / size().width(), (qreal)painter->device()->height() / size().height()));
    QXmlSimpleReader parser;
    parser.setContentHandler( &handler );
    parser.setErrorHandler( &handler );
    const KZipFileEntry* pageFile = static_cast<const KZipFileEntry *>(m_file->xpsArchive()->directory()->entry( m_fileName ));
    QByteArray data = readFileOrDirectoryParts( pageFile );
    QBuffer buffer( &data );
    QXmlInputSource source( &buffer );
    bool ok = parser.parse( source );
    kDebug(XpsDebug) << "Parse result: " << ok;

    return true;
}

QSizeF XpsPage::size() const
{
    return m_pageSize;
}

QFont XpsFile::getFontByName( const QString &fileName, float size )
{
    // kDebug(XpsDebug) << "trying to get font: " << fileName << ", size: " << size;

    int index = m_fontCache.value(fileName, -1);
    if (index == -1)
    {
        index = loadFontByName(fileName);
        m_fontCache[fileName] = index;
    }
    if ( index == -1 ) {
        kWarning(XpsDebug) << "Requesting uknown font:" << fileName;
        return QFont();
    }

    const QStringList fontFamilies = m_fontDatabase.applicationFontFamilies( index );
    if ( fontFamilies.isEmpty() ) {
      kWarning(XpsDebug) << "The unexpected has happened. No font family for a known font:" << fileName << index;
      return QFont();
    }
    const QString fontFamily = fontFamilies[0];
    const QStringList fontStyles = m_fontDatabase.styles( fontFamily );
    if ( fontStyles.isEmpty() ) {
      kWarning(XpsDebug) << "The unexpected has happened. No font style for a known font family:" << fileName << index << fontFamily ;
      return QFont();
    }
    const QString fontStyle =  fontStyles[0];
    return m_fontDatabase.font(fontFamily, fontStyle, qRound(size));
}

int XpsFile::loadFontByName( const QString &fileName )
{
    // kDebug(XpsDebug) << "font file name: " << fileName;

    const KArchiveEntry* fontFile = loadEntry( m_xpsArchive, fileName, Qt::CaseInsensitive );
    if ( !fontFile ) {
        return -1;
    }

    QByteArray fontData = readFileOrDirectoryParts( fontFile ); // once per file, according to the docs

    int result = m_fontDatabase.addApplicationFontFromData( fontData );
    if (-1 == result) {
        // Try to deobfuscate font
       // TODO Use deobfuscation depending on font content type, don't do it always when standard loading fails

        const QString baseName = resourceName( fileName );

        unsigned short guid[16];
        if (!parseGUID(baseName, guid))
        {
            kDebug(XpsDebug) << "File to load font - file name isn't a GUID";
        }
        else
        {
        if (fontData.length() < 32)
            {
                kDebug(XpsDebug) << "Font file is too small";
            } else {
                // Obfuscation - xor bytes in font binary with bytes from guid (font's filename)
                const static int mapping[] = {15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 4, 5, 0, 1, 2, 3};
                for (int i = 0; i < 16; i++) {
                    fontData[i] = fontData[i] ^ guid[mapping[i]];
                    fontData[i+16] = fontData[i+16] ^ guid[mapping[i]];
                }
                result = m_fontDatabase.addApplicationFontFromData( fontData );
            }
        }
    }


    // kDebug(XpsDebug) << "Loaded font: " << m_fontDatabase.applicationFontFamilies( result );

    return result; // a font ID
}

KZip * XpsFile::xpsArchive() {
    return m_xpsArchive;
}

QImage XpsPage::loadImageFromFile( const QString &fileName )
{
    // kDebug(XpsDebug) << "image file name: " << fileName;

    if ( fileName.at( 0 ) == QLatin1Char( '{' ) ) {
        // for example: '{ColorConvertedBitmap /Resources/bla.wdp /Resources/foobar.icc}'
        // TODO: properly read a ColorConvertedBitmap
        return QImage();
    }

    QString absoluteFileName = absolutePath( entryPath( m_fileName ), fileName );
    const KZipFileEntry* imageFile = loadFile( m_file->xpsArchive(), absoluteFileName, Qt::CaseInsensitive );
    if ( !imageFile ) {
        // image not found
        return QImage();
    }

    /* WORKAROUND:
        XPS standard requires to use 96dpi for images which doesn't have dpi specified (in file). When Qt loads such an image,
        it sets its dpi to qt_defaultDpi and doesn't allow to find out that it happend.

        To workaround this I used this procedure: load image, set its dpi to 96, load image again. When dpi isn't set in file,
        dpi set by me stays unchanged.

        Trolltech task ID: 159527.

    */

    QImage image;
    QByteArray data = imageFile->data();

    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadOnly);

    QImageReader reader(&buffer);
    image = reader.read();

    image.setDotsPerMeterX(qRound(96 / 0.0254));
    image.setDotsPerMeterY(qRound(96 / 0.0254));
  
    buffer.seek(0);
    reader.setDevice(&buffer);
    reader.read(&image);

    return image;
}

Okular::TextPage* XpsPage::textPage()
{
    // kDebug(XpsDebug) << "Parsing XpsPage, text extraction";

    Okular::TextPage* textPage = new Okular::TextPage();

    const KZipFileEntry* pageFile = static_cast<const KZipFileEntry *>(m_file->xpsArchive()->directory()->entry( m_fileName ));
    QXmlStreamReader xml;
    xml.addData( readFileOrDirectoryParts( pageFile ) );

    QTransform matrix = QTransform();
    QStack<QTransform> matrices;
    matrices.push( QTransform() );
    bool useMatrix = false;
    QXmlStreamAttributes glyphsAtts;

    while ( ! xml.atEnd() ) {
        xml.readNext();
        if ( xml.isStartElement() ) {
            if ( xml.name() == "Canvas") {
                matrices.push(matrix);

                QString att = xml.attributes().value( "RenderTransform" ).toString();
                if (!att.isEmpty()) {
                    matrix = parseRscRefMatrix( att ) * matrix;
                }
            } else if ((xml.name() == "Canvas.RenderTransform") || (xml.name() == "Glyphs.RenderTransform")) {
                useMatrix = true;
            } else if (xml.name() == "MatrixTransform") {
                if (useMatrix) {
                    matrix = attsToMatrix( xml.attributes().value("Matrix").toString() ) * matrix;
                }
            } else if (xml.name() == "Glyphs") {
                matrices.push( matrix );
                glyphsAtts = xml.attributes();
            } else if ( (xml.name() == "Path") || (xml.name() == "Path.Fill") || (xml.name() == "SolidColorBrush")
                        || (xml.name() == "ImageBrush") ||  (xml.name() == "ImageBrush.Transform")
                        || (xml.name() == "Path.OpacityMask") || (xml.name() == "Path.Data")
                        || (xml.name() == "PathGeometry") || (xml.name() == "PathFigure")
                        || (xml.name() == "PolyLineSegment") ) {
                // those are only graphical - no use in text handling
            } else if ( (xml.name() == "FixedPage") || (xml.name() == "FixedPage.Resources") ) {
                // not useful for text extraction
            } else {
                kDebug(XpsDebug) << "Unhandled element in Text Extraction start: " << xml.name().toString();
            }
        } else if (xml.isEndElement() ) {
            if (xml.name() == "Canvas") {
                matrix = matrices.pop();
            } else if ((xml.name() == "Canvas.RenderTransform") || (xml.name() == "Glyphs.RenderTransform")) {
                useMatrix = false;
            } else if (xml.name() == "MatrixTransform") {
                // not clear if we need to do anything here yet.
            } else if (xml.name() == "Glyphs") {
                QString att = glyphsAtts.value( "RenderTransform" ).toString();
                if (!att.isEmpty()) {
                    matrix = parseRscRefMatrix( att ) * matrix;
                }
                QString text = unicodeString( glyphsAtts.value( "UnicodeString" ).toString() );

                // Get font (doesn't work well because qt doesn't allow to load font from file)
                QFont font = m_file->getFontByName( glyphsAtts.value( "FontUri" ).toString(),
                                                    glyphsAtts.value("FontRenderingEmSize").toString().toFloat() * 72 / 96 );
                QFontMetrics metrics = QFontMetrics( font );
                // Origin
                QPointF origin( glyphsAtts.value("OriginX").toString().toDouble(),
                                glyphsAtts.value("OriginY").toString().toDouble() );


                int lastWidth = 0;
                for (int i = 0; i < text.length(); i++) {
                    int width = metrics.width( text, i + 1 );

                    Okular::NormalizedRect * rect = new Okular::NormalizedRect( (origin.x() + lastWidth) / m_pageSize.width(),
                                                                                (origin.y() - metrics.height()) / m_pageSize.height(),
                                                                                (origin.x() + width) / m_pageSize.width(),
                                                                                origin.y() / m_pageSize.height() );
                    rect->transform( matrix );
                    textPage->append( text.mid(i, 1), rect );

                    lastWidth = width;
                }

                matrix = matrices.pop();
            } else if ( (xml.name() == "Path") || (xml.name() == "Path.Fill") || (xml.name() == "SolidColorBrush")
                        || (xml.name() == "ImageBrush") ||  (xml.name() == "ImageBrush.Transform")
                        || (xml.name() == "Path.OpacityMask") || (xml.name() == "Path.Data")
                        || (xml.name() == "PathGeometry") || (xml.name() == "PathFigure")
                        || (xml.name() == "PolyLineSegment") ) {
                // those are only graphical - no use in text handling
            } else if ( (xml.name() == "FixedPage") || (xml.name() == "FixedPage.Resources") ) {
                // not useful for text extraction
            } else {
                kDebug(XpsDebug) << "Unhandled element in Text Extraction end: " << xml.name().toString();
            }
        }
    }
    if ( xml.error() ) {
        kDebug(XpsDebug) << "Error parsing XpsPage text: " << xml.errorString();
    }
    return textPage;
}

void XpsDocument::parseDocumentStructure( const QString &documentStructureFileName )
{
    kDebug(XpsDebug) << "document structure file name: " << documentStructureFileName;
    m_haveDocumentStructure = false;

    const KZipFileEntry* documentStructureFile = static_cast<const KZipFileEntry *>(m_file->xpsArchive()->directory()->entry( documentStructureFileName ));

    QXmlStreamReader xml;
    xml.addData( documentStructureFile->data() );

    while ( !xml.atEnd() )
    {
        xml.readNext();

        if ( xml.isStartElement() ) {
            if ( xml.name() == "DocumentStructure" ) {
                // just a container for optional outline and story elements - nothing to do here
            } else if ( xml.name() == "DocumentStructure.Outline" ) {
                kDebug(XpsDebug) << "found DocumentStructure.Outline";
            } else if ( xml.name() == "DocumentOutline" ) {
                kDebug(XpsDebug) << "found DocumentOutline";
                m_docStructure = new Okular::DocumentSynopsis;
            } else if ( xml.name() == "OutlineEntry" ) {
                m_haveDocumentStructure = true;
                QXmlStreamAttributes attributes = xml.attributes();
                int outlineLevel = attributes.value( "OutlineLevel").toString().toInt();
                QString description = attributes.value("Description").toString();
                QDomElement synopsisElement = m_docStructure->createElement( description );
                synopsisElement.setAttribute( "OutlineLevel",  outlineLevel );
                QString target = attributes.value("OutlineTarget").toString();
                int hashPosition = target.lastIndexOf( '#' );
                target = target.mid( hashPosition + 1 );
                // kDebug(XpsDebug) << "target: " << target;
                Okular::DocumentViewport viewport;
                viewport.pageNumber = m_docStructurePageMap.value( target );
                synopsisElement.setAttribute( "Viewport",  viewport.toString() );
                if ( outlineLevel == 1 ) {
                    // kDebug(XpsDebug) << "Description: "
                    // << outlineEntryElement.attribute( "Description" ) << endl;
                    m_docStructure->appendChild( synopsisElement );
                } else {
                    // find the last next highest element (so it this is level 3, we need
                    // to find the most recent level 2 node)
                    QDomNode maybeParentNode = m_docStructure->lastChild();
                    while ( !maybeParentNode.isNull() )
                    {
                        if ( maybeParentNode.toElement().attribute( "OutlineLevel" ).toInt() == ( outlineLevel - 1 ) )  {
                            // we have the right parent
                            maybeParentNode.appendChild( synopsisElement );
                            break;
                        }
                        maybeParentNode = maybeParentNode.lastChild();
                    }
                }
            } else if ( xml.name() == "Story" ) {
                // we need to handle Story here, but I have no idea what to do with it.
            } else if ( xml.name() == "StoryFragment" ) {
                // we need to handle StoryFragment here, but I have no idea what to do with it.
            } else if ( xml.name() == "StoryFragmentReference" ) {
                // we need to handle StoryFragmentReference here, but I have no idea what to do with it.
            } else {
                kDebug(XpsDebug) << "Unhandled entry in DocumentStructure: " << xml.name().toString();
            }
        }
    }
}

const Okular::DocumentSynopsis * XpsDocument::documentStructure()
{
    return m_docStructure;
}

bool XpsDocument::hasDocumentStructure()
{
    return m_haveDocumentStructure;
}

XpsDocument::XpsDocument(XpsFile *file, const QString &fileName): m_file(file), m_haveDocumentStructure( false ), m_docStructure( 0 )
{
    kDebug(XpsDebug) << "document file name: " << fileName;

    const KArchiveEntry* documentEntry = file->xpsArchive()->directory()->entry( fileName );
    QString documentFilePath = fileName;
    const QString documentEntryPath = entryPath( fileName );

    QXmlStreamReader docXml;
    docXml.addData( readFileOrDirectoryParts( documentEntry, &documentFilePath ) );
    while( !docXml.atEnd() ) {
        docXml.readNext();
        if ( docXml.isStartElement() ) {
            if ( docXml.name() == "PageContent" ) {
                QString pagePath = docXml.attributes().value("Source").toString();
                kDebug(XpsDebug) << "Page Path: " << pagePath;
                XpsPage *page = new XpsPage( file, absolutePath( documentFilePath, pagePath ) );
                m_pages.append(page);
            } else if ( docXml.name() == "PageContent.LinkTargets" ) {
                // do nothing - wait for the real LinkTarget elements
            } else if ( docXml.name() == "LinkTarget" ) {
                QString targetName = docXml.attributes().value( "Name" ).toString();
                if ( ! targetName.isEmpty() ) {
                    m_docStructurePageMap[ targetName ] = m_pages.count() - 1;
                }
            } else if ( docXml.name() == "FixedDocument" ) {
                // we just ignore this - it is just a container
            } else {
                kDebug(XpsDebug) << "Unhandled entry in FixedDocument: " << docXml.name().toString();
            }
        }
    }
    if ( docXml.error() ) {
        kDebug(XpsDebug) << "Could not parse main XPS document file: " << docXml.errorString();
    }

    // There might be a relationships entry for this document - typically used to tell us where to find the
    // content structure description

    // We should be able to find this using a reference from some other part of the document, but I can't see it.
    const int slashPosition = fileName.lastIndexOf( '/' );
    const QString documentRelationshipFile = absolutePath( documentEntryPath, "_rels/" + fileName.mid( slashPosition + 1 ) + ".rels" );

    const KZipFileEntry* relFile = static_cast<const KZipFileEntry *>(file->xpsArchive()->directory()->entry(documentRelationshipFile));

    QString documentStructureFile;
    if ( relFile ) {
        QXmlStreamReader xml;
        xml.addData( readFileOrDirectoryParts( relFile ) );
        while ( !xml.atEnd() )
        {
            xml.readNext();
            if ( xml.isStartElement() && ( xml.name() == "Relationship" ) ) {
                QXmlStreamAttributes attributes = xml.attributes();
                if ( attributes.value( "Type" ).toString() == "http://schemas.microsoft.com/xps/2005/06/documentstructure" ) {
                    documentStructureFile  = attributes.value( "Target" ).toString();
                } else {
                    kDebug(XpsDebug) << "Unknown document relationships element: "
                                     << attributes.value( "Type" ).toString() << " : "
                                     << attributes.value( "Target" ).toString() << endl;
                }
            }
        }
        if ( xml.error() ) {
            kDebug(XpsDebug) << "Could not parse XPS page relationships file ( "
                             << documentRelationshipFile
                             << " ) - " << xml.errorString() << endl;
        }
    } else { // the page relationship file didn't exist in the zipfile
        // this isn't fatal
        kDebug(XpsDebug) << "Could not open Document relationship file from " << documentRelationshipFile;
    }

    if ( ! documentStructureFile.isEmpty() )
    {
        // kDebug(XpsDebug) << "Document structure filename: " << documentStructureFile;
        // make the document path absolute
        documentStructureFile = absolutePath( documentEntryPath, documentStructureFile );
        // kDebug(XpsDebug) << "Document structure absolute path: " << documentStructureFile;
        parseDocumentStructure( documentStructureFile );
    }

}

XpsDocument::~XpsDocument()
{
    for (int i = 0; i < m_pages.size(); i++) {
        delete m_pages.at( i );
    }
    m_pages.clear();

    if ( m_docStructure )
        delete m_docStructure;
}

int XpsDocument::numPages() const
{
    return m_pages.size();
}

XpsPage* XpsDocument::page(int pageNum) const
{
    return m_pages.at(pageNum);
}

XpsFile::XpsFile() : m_docInfo( 0 )
{
}


XpsFile::~XpsFile()
{
    m_fontCache.clear();
    m_fontDatabase.removeAllApplicationFonts();
}


bool XpsFile::loadDocument(const QString &filename)
{
    m_xpsArchive = new KZip( filename );
    if ( m_xpsArchive->open( QIODevice::ReadOnly ) == true ) {
        kDebug(XpsDebug) << "Successful open of " << m_xpsArchive->fileName();
    } else {
        kDebug(XpsDebug) << "Could not open XPS archive: " << m_xpsArchive->fileName();
        delete m_xpsArchive;
        return false;
    }

    // The only fixed entry in XPS is /_rels/.rels
    const KArchiveEntry* relEntry = m_xpsArchive->directory()->entry("_rels/.rels");
    if ( !relEntry ) {
        // this might occur if we can't read the zip directory, or it doesn't have the relationships entry
        return false;
    }

    QXmlStreamReader relXml;
    relXml.addData( readFileOrDirectoryParts( relEntry ) );

    QString fixedRepresentationFileName;
    // We work through the relationships document and pull out each element.
    while ( !relXml.atEnd() )
    {
        relXml.readNext();
        if ( relXml.isStartElement() ) {
            if ( relXml.name() == "Relationship" ) {
                QXmlStreamAttributes attributes = relXml.attributes();
                QString type = attributes.value( "Type" ).toString();
                QString target = attributes.value( "Target" ).toString();
                if ( "http://schemas.openxmlformats.org/package/2006/relationships/metadata/thumbnail" == type ) {
                    m_thumbnailFileName = target;
                } else if ( "http://schemas.microsoft.com/xps/2005/06/fixedrepresentation" == type ) {
                    fixedRepresentationFileName = target;
                } else if ("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" == type) {
                    m_corePropertiesFileName = target;
                } else if ("http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin" == type) {
                    m_signatureOrigin = target;
                } else {
                    kDebug(XpsDebug) << "Unknown relationships element: " << type << " : " << target;
                }
            } else if ( relXml.name() == "Relationships" ) {
                // nothing to do here - this is just the container level
            } else {
                kDebug(XpsDebug) << "unexpected element in _rels/.rels: " << relXml.name().toString();
            }
        }
    }
    if ( relXml.error() ) {
        kDebug(XpsDebug) << "Could not parse _rels/.rels: " << relXml.errorString();
        return false;
    }

    if ( fixedRepresentationFileName.isEmpty() ) {
        // FixedRepresentation is a required part of the XPS document
        return false;
    }

    const KArchiveEntry* fixedRepEntry = m_xpsArchive->directory()->entry( fixedRepresentationFileName );
    QString fixedRepresentationFilePath = fixedRepresentationFileName;

    QXmlStreamReader fixedRepXml;
    fixedRepXml.addData( readFileOrDirectoryParts( fixedRepEntry, &fixedRepresentationFileName ) );

    while ( !fixedRepXml.atEnd() )
    {
        fixedRepXml.readNext();
        if ( fixedRepXml.isStartElement() ) {
            if ( fixedRepXml.name() == "DocumentReference" ) {
                const QString source = fixedRepXml.attributes().value("Source").toString();
                XpsDocument *doc = new XpsDocument( this, absolutePath( fixedRepresentationFilePath, source ) );
                for (int lv = 0; lv < doc->numPages(); ++lv) {
                    // our own copy of the pages list
                    m_pages.append( doc->page( lv ) );
                }
                m_documents.append(doc);
            } else if ( fixedRepXml.name() == "FixedDocumentSequence") {
                // we don't do anything here - this is just a container for one or more DocumentReference elements
            } else {
                kDebug(XpsDebug) << "Unhandled entry in FixedDocumentSequence: " << fixedRepXml.name().toString();
            }
        }
    }
    if ( fixedRepXml.error() ) {
        kDebug(XpsDebug) << "Could not parse FixedRepresentation file:" << fixedRepXml.errorString();
        return false;
    }

    return true;
}

const Okular::DocumentInfo * XpsFile::generateDocumentInfo()
{
    if ( m_docInfo )
        return m_docInfo;

    m_docInfo = new Okular::DocumentInfo();

    m_docInfo->set( Okular::DocumentInfo::MimeType, "application/oxps" );

    if ( ! m_corePropertiesFileName.isEmpty() ) {
        const KZipFileEntry* corepropsFile = static_cast<const KZipFileEntry *>(m_xpsArchive->directory()->entry(m_corePropertiesFileName));

        QXmlStreamReader xml;
        xml.addData( corepropsFile->data() );
        while ( !xml.atEnd() )
        {
            xml.readNext();
            if ( xml.isEndElement() )
                break;
            if ( xml.isStartElement() )
            {
                if (xml.name() == "title") {
                    m_docInfo->set( Okular::DocumentInfo::Title, xml.readElementText() );
                } else if (xml.name() == "subject") {
                    m_docInfo->set( Okular::DocumentInfo::Subject, xml.readElementText() );
                } else if (xml.name() == "description") {
                    m_docInfo->set( Okular::DocumentInfo::Description, xml.readElementText() );
                } else if (xml.name() == "creator") {
                    m_docInfo->set( Okular::DocumentInfo::Creator, xml.readElementText() );
                } else if (xml.name() == "category") {
                    m_docInfo->set( Okular::DocumentInfo::Category, xml.readElementText() );
                } else if (xml.name() == "created") {
                    QDateTime createdDate = QDateTime::fromString( xml.readElementText(), "yyyy-MM-ddThh:mm:ssZ" );
                    m_docInfo->set( Okular::DocumentInfo::CreationDate, KGlobal::locale()->formatDateTime( createdDate, KLocale::LongDate, true ) );
                } else if (xml.name() == "modified") {
                    QDateTime modifiedDate = QDateTime::fromString( xml.readElementText(), "yyyy-MM-ddThh:mm:ssZ" );
                    m_docInfo->set( Okular::DocumentInfo::ModificationDate, KGlobal::locale()->formatDateTime( modifiedDate, KLocale::LongDate, true ) );
                } else if (xml.name() == "keywords") {
                    m_docInfo->set( Okular::DocumentInfo::Keywords, xml.readElementText() );
                } else if (xml.name() == "revision") {
                    m_docInfo->set( "revision", xml.readElementText(), i18n( "Revision" ) );
                }
            }
        }
        if ( xml.error() )
        {
            kDebug(XpsDebug) << "Could not parse XPS core properties:" << xml.errorString();
        }
    } else {
        kDebug(XpsDebug) << "No core properties filename";
    }

    m_docInfo->set( Okular::DocumentInfo::Pages, QString::number(numPages()) );

    return m_docInfo;
}

bool XpsFile::closeDocument()
{

    if ( m_docInfo )
        delete m_docInfo;

    m_docInfo = 0;

    qDeleteAll( m_documents );
    m_documents.clear();

    delete m_xpsArchive;

    return true;
}

int XpsFile::numPages() const
{
    return m_pages.size();
}

int XpsFile::numDocuments() const
{
    return m_documents.size();
}

XpsDocument* XpsFile::document(int documentNum) const
{
    return m_documents.at( documentNum );
}

XpsPage* XpsFile::page(int pageNum) const
{
    return m_pages.at( pageNum );
}

XpsGenerator::XpsGenerator( QObject *parent, const QVariantList &args )
  : Okular::Generator( parent, args ), m_xpsFile( 0 )
{
    setFeature( TextExtraction );
    setFeature( PrintNative );
    setFeature( PrintToFile );
    // activate the threaded rendering iif:
    // 1) QFontDatabase says so
    // 2) Qt >= 4.4.0 (see Trolltech task ID: 169502)
    // 3) Qt >= 4.4.2 (see Trolltech task ID: 215090)
#if QT_VERSION >= 0x040402
    if ( QFontDatabase::supportsThreadedFontRendering() )
        setFeature( Threaded );
#endif
    userMutex();
}

XpsGenerator::~XpsGenerator()
{
}

bool XpsGenerator::loadDocument( const QString & fileName, QVector<Okular::Page*> & pagesVector )
{
    m_xpsFile = new XpsFile();

    m_xpsFile->loadDocument( fileName );
    pagesVector.resize( m_xpsFile->numPages() );

    int pagesVectorOffset = 0;

    for (int docNum = 0; docNum < m_xpsFile->numDocuments(); ++docNum )
    {
        XpsDocument *doc = m_xpsFile->document( docNum );
        for (int pageNum = 0; pageNum < doc->numPages(); ++pageNum )
        {
            QSizeF pageSize = doc->page( pageNum )->size();
            pagesVector[pagesVectorOffset] = new Okular::Page( pagesVectorOffset, pageSize.width(), pageSize.height(), Okular::Rotation0 );
            ++pagesVectorOffset;
        }
    }

    return true;
}

bool XpsGenerator::doCloseDocument()
{
    m_xpsFile->closeDocument();
    delete m_xpsFile;
    m_xpsFile = 0;

    return true;
}

QImage XpsGenerator::image( Okular::PixmapRequest * request )
{
    QMutexLocker lock( userMutex() );
    QSize size( (int)request->width(), (int)request->height() );
    QImage image( size, QImage::Format_RGB32 );
    XpsPage *pageToRender = m_xpsFile->page( request->page()->number() );
    pageToRender->renderToImage( &image );
    return image;
}

Okular::TextPage* XpsGenerator::textPage( Okular::Page * page )
{
    QMutexLocker lock( userMutex() );
    XpsPage * xpsPage = m_xpsFile->page( page->number() );
    return xpsPage->textPage();
}

const Okular::DocumentInfo * XpsGenerator::generateDocumentInfo()
{
    kDebug(XpsDebug) << "generating document metadata";

    return m_xpsFile->generateDocumentInfo();
}

const Okular::DocumentSynopsis * XpsGenerator::generateDocumentSynopsis()
{
    kDebug(XpsDebug) << "generating document synopsis";

    // we only generate the synopsis for the first file.
    if ( !m_xpsFile || !m_xpsFile->document( 0 ) )
        return NULL;

    if ( m_xpsFile->document( 0 )->hasDocumentStructure() )
        return m_xpsFile->document( 0 )->documentStructure();


    return NULL;
}

Okular::ExportFormat::List XpsGenerator::exportFormats() const
{
    static Okular::ExportFormat::List formats;
    if ( formats.isEmpty() ) {
        formats.append( Okular::ExportFormat::standardFormat( Okular::ExportFormat::PlainText ) );
    }
    return formats;
}

bool XpsGenerator::exportTo( const QString &fileName, const Okular::ExportFormat &format )
{
    if ( format.mimeType()->name() == QLatin1String( "text/plain" ) ) {
        QFile f( fileName );
        if ( !f.open( QIODevice::WriteOnly ) )
            return false;

        QTextStream ts( &f );
        for ( int i = 0; i < m_xpsFile->numPages(); ++i )
        {
            Okular::TextPage* textPage = m_xpsFile->page(i)->textPage();
            QString text = textPage->text();
            ts << text;
            ts << QChar('\n');
            delete textPage;
        }
        f.close();

        return true;
    }

    return false;
}

bool XpsGenerator::print( QPrinter &printer )
{
    QList<int> pageList = Okular::FilePrinter::pageList( printer, document()->pages(),
                                                         document()->currentPage() + 1,
                                                         document()->bookmarkedPageList() );

    QPainter painter( &printer );

    for ( int i = 0; i < pageList.count(); ++i )
    {
        if ( i != 0 )
            printer.newPage();

        const int page = pageList.at( i ) - 1;
        XpsPage *pageToRender = m_xpsFile->page( page );
        pageToRender->renderToPainter( &painter );
    }

    return true;
}

XpsRenderNode * XpsRenderNode::findChild( const QString &name )
{
    for (int i = 0; i < children.size(); i++) {
        if (children[i].name == name) {
            return &children[i];
        }
    }

    return NULL;
}

QVariant XpsRenderNode::getRequiredChildData( const QString &name )
{
    XpsRenderNode * child = findChild( name );
    if (child == NULL) {
        kDebug(XpsDebug) << "Required element " << name << " is missing in " << this->name;
        return QVariant();
    }

    return child->data;
}

QVariant XpsRenderNode::getChildData( const QString &name )
{
    XpsRenderNode * child = findChild( name );
    if (child == NULL) {
        return QVariant();
    } else {
        return child->data;
    }
}

#include "generator_xps.moc"

