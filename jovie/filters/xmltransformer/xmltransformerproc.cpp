/***************************************************** vim:set ts=4 sw=4 sts=4:
  Generic XML Transformation Filter Processing class.
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

// XmlTransformer includes.
#include "xmltransformerproc.h"
#include "moc_xmltransformerproc.cpp"

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/qstring.h>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

// KDE includes.
#include <kdeversion.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// KTTS includes.
#include "filterproc.h"

/**
 * Constructor.
 */
XmlTransformerProc::XmlTransformerProc( QObject *parent, const QVariantList& args) :
    KttsFilterProc(parent, args)
{
    m_xsltProc = 0;
}

/**
 * Destructor.
 */
/*virtual*/ XmlTransformerProc::~XmlTransformerProc()
{
    delete m_xsltProc;
    if (!m_inFilename.isEmpty()) QFile::remove(m_inFilename);
    if (!m_outFilename.isEmpty()) QFile::remove(m_outFilename);
}

bool XmlTransformerProc::init(KConfig* c, const QString& configGroup)
{
    // kDebug() << "XmlTransformerProc::init: Running.";
    KConfigGroup config( c, configGroup );
    m_UserFilterName = config.readEntry( "UserFilterName" );
    m_xsltFilePath = config.readEntry( "XsltFilePath" );
    m_xsltprocPath = config.readEntry( "XsltprocPath" );
    m_rootElementList = config.readEntry( "RootElement", QStringList() );
    m_doctypeList = config.readEntry( "DocType", QStringList() );
    m_appIdList = config.readEntry( "AppID", QStringList() );
    kDebug() << "XmlTransformerProc::init: m_xsltprocPath = " << m_xsltprocPath;
    kDebug() << "XmlTransformerProc::init: m_xsltFilePath = " << m_xsltFilePath;
    return ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() );
}

/**
 * Returns True if the plugin supports asynchronous processing,
 * i.e., supports asyncConvert method.
 * @return                        True if this plugin supports asynchronous processing.
 *
 * If the plugin returns True, it must also implement @ref getState .
 * It must also emit @ref filteringFinished when filtering is completed.
 * If the plugin returns True, it must also implement @ref stopFiltering .
 * It must also emit @ref filteringStopped when filtering has been stopped.
 */
/*virtual*/ bool XmlTransformerProc::supportsAsync() { return true; }

/**
 * Convert input, returning output.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString XmlTransformerProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QString& appId)
{
    // kDebug() << "XmlTransformerProc::convert: Running.";
    // If not properly configured, do nothing.
    if ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() )
    {
        kDebug() << "XmlTransformerProc::convert: not properly configured";
        return inputText;
    }
    // Asynchronously convert and wait for completion.
    if (asyncConvert(inputText, talkerCode, appId))
    {
        waitForFinished();
        m_state = fsIdle;
        return m_text;
    } else
        return inputText;
}

bool XmlTransformerProc::asyncConvert(const QString& inputText, TalkerCode* talkerCode,
    const QString& appId)
{
    Q_UNUSED(talkerCode);
    m_wasModified = false;

    // kDebug() << "XmlTransformerProc::asyncConvert: Running.";
    m_text = inputText;
    // If not properly configured, do nothing.
    if ( m_xsltFilePath.isEmpty() || m_xsltprocPath.isEmpty() )
    {
        kDebug() << "XmlTransformerProc::asyncConvert: not properly configured.";
        return false;
    }

    bool found = false;
    // If not correct XML type, or DOCTYPE, do nothing.
    if ( !m_rootElementList.isEmpty() )
    {
        // kDebug() << "XmlTransformerProc::asyncConvert:: searching for root elements " << m_rootElementList;
        for ( int ndx=0; ndx < m_rootElementList.count(); ++ndx )
        {
            if ( hasRootElement( inputText, m_rootElementList[ndx] ) )
            {
                found = true;
                break;
            }
        }
        if ( !found && m_doctypeList.isEmpty() )
        {
            kDebug() << "XmlTransformerProc::asyncConvert: Did not find root element(s)" << m_rootElementList;
            return false;
        }
    }
    if ( !found && !m_doctypeList.isEmpty() )
    {
        for ( int ndx=0; ndx < m_doctypeList.count(); ++ndx )
        {
            if ( hasDoctype( inputText, m_doctypeList[ndx] ) )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            // kDebug() << "XmlTransformerProc::asyncConvert: Did not find doctype(s)" << m_doctypeList;
            return false;
        }
    }

    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        QString appIdStr = appId;
        // kDebug() << "XmlTransformrProc::convert: converting " << inputText << " if appId "
        //     << appId << " matches " << m_appIdList << endl;
        found = false;
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
            // kDebug() << "XmlTransformerProc::asyncConvert: Did not find appId(s)" << m_appIdList;
            return false;
        }
    }

    /// Write @param text to a temporary file.
    KTemporaryFile inFile;
    inFile.setPrefix(QLatin1String( "kttsd-" ));
    inFile.setSuffix(QLatin1String( ".xml" ));
    inFile.setAutoRemove(false);
    inFile.open();
    m_inFilename = inFile.fileName();
    QTextStream wstream (&inFile);
    // TODO: Is encoding an issue here?
    // If input does not have xml processing instruction, add it.
    if (!inputText.startsWith(QLatin1String("<?xml"))) wstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    // FIXME: Temporary Fix until Konqi returns properly formatted xhtml with & coded as &amp;
    // This will change & inside a CDATA section, which is not good, and also within comments and
    // processing instructions, which is OK because we don't speak those anyway.
    QString text = inputText;
    text.replace(QRegExp(QLatin1String( "&(?!amp;)") ),QLatin1String( "&amp;" ));
    wstream << text;
    inFile.flush();

    // Get a temporary output file name.
    KTemporaryFile outFile;
    outFile.setPrefix(QLatin1String( "kttsd-" ));
    outFile.setSuffix(QLatin1String( ".output" ));
    outFile.setAutoRemove(false);
    outFile.open();
    m_outFilename = outFile.fileName();

    /// Spawn an xsltproc process to apply our stylesheet to input file.
    m_xsltProc = new QProcess;
    m_xsltProc->setProcessChannelMode(QProcess::SeparateChannels);
    const QStringList xsltProcArgs = QStringList()
        << QLatin1String("-o") << m_outFilename  << QLatin1String("--novalid")
        << m_xsltFilePath << m_inFilename;
    // Warning: This won't compile under KDE 3.2.  See FreeTTS::argsToStringList().
    // kDebug() << "SSMLConvert::transform: executing command: " <<
    //     m_xsltProc->args() << endl;

    m_state = fsFiltering;
    connect(m_xsltProc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
    connect(m_xsltProc, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReceivedStdout()));
    connect(m_xsltProc, SIGNAL(readyReadStandardError()),
            this, SLOT(slotReceivedStderr()));
    m_xsltProc->start(m_xsltprocPath, xsltProcArgs);
    if (!m_xsltProc->waitForStarted())
    {
        kWarning() << "XmlTransformerProc::convert: Error starting xsltproc";
        m_state = fsIdle;
        return false;
    }
    return true;
}

// Process output when xsltproc exits.
void XmlTransformerProc::processOutput()
{
    QFile::remove(m_inFilename);

    int exitStatus = 11;
    if (m_xsltProc->exitStatus() == QProcess::NormalExit)
        exitStatus = m_xsltProc->exitCode();
    else
        kDebug() << "XmlTransformerProc::processOutput: xsltproc was killed.";

    delete m_xsltProc;
    m_xsltProc = 0;

    if (exitStatus != 0)
    {
        kDebug() << "XmlTransformerProc::processOutput: xsltproc abnormal exit.  Status = " << exitStatus;
        m_state = fsFinished;
        QFile::remove(m_outFilename);
        emit filteringFinished();
        return;
    }

    /// Read back the data that was written to /tmp/fileName.output.
    QFile readfile(m_outFilename);
    if(!readfile.open(QIODevice::ReadOnly)) {
        /// uhh yeah... Issues writing to the output file.
        kDebug() << "XmlTransformerProc::processOutput: Could not read file " << m_outFilename;
        m_state = fsFinished;
        emit filteringFinished();
    }
    QTextStream rstream(&readfile);
    m_text = rstream.readAll();
    readfile.close();

    kDebug() << QLatin1String( "XmlTransformerProc::processOutput: Read file at " ) + m_inFilename + QLatin1String( " and created " ) + m_outFilename + QLatin1String( " based on the stylesheet at " ) << m_xsltFilePath;

    // Clean up.
    QFile::remove(m_outFilename);

    m_state = fsFinished;
    m_wasModified = true;
    emit filteringFinished();
}

/**
 * Waits for a previous call to asyncConvert to finish.
 */
/*virtual*/ void XmlTransformerProc::waitForFinished()
{
    if (m_xsltProc)
    {
        if (m_xsltProc->state() != QProcess::NotRunning)
        {
            if ( !m_xsltProc->waitForFinished( 15 ) )
            {
                m_xsltProc->kill();
                kDebug() << "XmlTransformerProc::waitForFinished: After waiting 15 seconds, xsltproc process seems to hung.  Killing it.";
                processOutput();
            }
        }
    }
}

/**
 * Returns the state of the Filter.
 */
/*virtual*/ int XmlTransformerProc::getState() { return m_state; }

/**
 * Returns the filtered output.
 */
/*virtual*/ QString XmlTransformerProc::getOutput() { return m_text; }

/**
 * Acknowledges the finished filtering.
 */
/*virtual*/ void XmlTransformerProc::ackFinished()
{
    m_state = fsIdle;
    m_text.clear();
}

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped and state returns to fsIdle;
 */
/*virtual*/ void XmlTransformerProc::stopFiltering()
{
    m_state = fsStopping;
    m_xsltProc->kill();
}

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool XmlTransformerProc::wasModified() { return m_wasModified; }

void XmlTransformerProc::slotProcessExited(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    // kDebug() << "XmlTransformerProc::slotProcessExited: xsltproc has exited.";
    processOutput();
}

void XmlTransformerProc::slotReceivedStdout()
{
    // QString buf = QString::fromLatin1(buffer, buflen);
    // kDebug() << "XmlTransformerProc::slotReceivedStdout: Received from xsltproc: " << buf;
}

void XmlTransformerProc::slotReceivedStderr()
{
    // QString buf = QString::fromLatin1(buffer, buflen);
    // kDebug() << "XmlTransformerProc::slotReceivedStderr: Received error from xsltproc: " << buf;
}

/**
 * Check if an XML document has a certain root element.
 * @param xmldoc             The document to check for the element.
 * @param elementName        The element to check for in the document.
 * @returns                  True if the root element exists in the document, false otherwise.
*/
bool XmlTransformerProc::hasRootElement(const QString &xmldoc, const QString &elementName) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.simplified();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith(QLatin1String("<?xml"))) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.indexOf(QLatin1String( "?>" ));
        if(xmlStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
    }
    // Take off leading comments, if they exist.
    while(doc.startsWith(QLatin1String("<!--")) || doc.startsWith(QLatin1String(" <!--"))) {
        int commentStatementEnd = doc.indexOf(QLatin1String( "-->" ));
        if(commentStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        commentStatementEnd += 3; // len '>' == 2
        doc = doc.right(doc.length() - commentStatementEnd);
    }
    // Take off the doctype statement if it exists.
    while(doc.startsWith(QLatin1String("<!DOCTYPE")) || doc.startsWith(QLatin1String(" <!DOCTYPE"))) {
        int doctypeStatementEnd = doc.indexOf(QLatin1Char( '>' ));
        if(doctypeStatementEnd == -1) {
            kDebug() << "KttsUtils::hasRootElement: Bad XML file syntax\n";
            return false;
        }
        doctypeStatementEnd += 1; // len '>' == 2
        doc = doc.right(doc.length() - doctypeStatementEnd);
    }
    // We should (hopefully) be left with the root element.
    return (doc.startsWith(QString(QLatin1Char( '<' ) + elementName)) || doc.startsWith(QString(QLatin1String( " <" ) + elementName)));
}

bool XmlTransformerProc::hasDoctype(const QString &xmldoc, const QString &name) {
    // Strip all whitespace and go from there.
    QString doc = xmldoc.trimmed();
    // Take off the <?xml...?> if it exists
    if(doc.startsWith(QLatin1String("<?xml"))) {
        // Look for ?> and strip everything off from there to the start - effectively removing
        // <?xml...?>
        int xmlStatementEnd = doc.indexOf(QLatin1String( "?>" ));
        if(xmlStatementEnd == -1) {
            kDebug() << "XmlTransformerProc::hasDoctype: Bad XML file syntax\n";
            return false;
        }
        xmlStatementEnd += 2;  // len '?>' == 2
        doc = doc.right(doc.length() - xmlStatementEnd);
        doc = doc.trimmed();
    }
    // Take off leading comments, if they exist.
    while(doc.startsWith(QLatin1String("<!--"))) {
        int commentStatementEnd = doc.indexOf(QLatin1String( "-->" ));
        if(commentStatementEnd == -1) {
            kDebug() << "XmlTransformerProc::hasDoctype: Bad XML file syntax\n";
            return false;
        }
        commentStatementEnd += 3; // len '>' == 2
        doc = doc.right(doc.length() - commentStatementEnd);
        doc = doc.trimmed();
    }
    // Match the doctype statement if it exists.
    // kDebug() << "XmlTransformerProc::hasDoctype: searching " << doc.left(20) << "... for " << "<!DOCTYPE " << name;
    return (doc.startsWith(QString(QLatin1String( "<!DOCTYPE " ) + name)));
}
