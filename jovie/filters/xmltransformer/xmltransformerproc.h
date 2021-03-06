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

#ifndef XMLTRANSFORMERPROC_H
#define XMLTRANSFORMERPROC_H

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QProcess>

// KTTS includes.
#include "filterproc.h"
#include "talkercode.h"

class XmlTransformerProc : public KttsFilterProc
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit XmlTransformerProc( QObject *parent, const QVariantList &args);

    /**
     * Destructor.
     */
    virtual ~XmlTransformerProc();

    /**
     * Initialize the filter.
     * @param c               Settings object.
     * @param configGroup     Settings Group.
     * @return                False if filter is not ready to filter.
     *
     * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
     * separate configuration files of their own.
     */
    virtual bool init(KConfig *c, const QString &configGroup);

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
    virtual bool supportsAsync();

    /**
     * Convert input, returning output.
     * @param inputText         Input text.
     * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
     *                          use for synthing the text.  Useful for extracting hints about
     *                          how to filter the text.  For example, languageCode.
     * @param appId             The DCOP appId of the application that queued the text.
     *                          Also useful for hints about how to do the filtering.
     */
    virtual QString convert(const QString& inputText, TalkerCode* talkerCode, const QString& appId);

    /**
     * Convert input.  Runs asynchronously.
     * @param inputText         Input text.
     * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
     *                          use for synthing the text.  Useful for extracting hints about
     *                          how to filter the text.  For example, languageCode.
     * @param appId             The DCOP appId of the application that queued the text.
     *                          Also useful for hints about how to do the filtering.
     * @return                  False if the filter cannot perform the conversion.
     *
     * When conversion is completed, emits signal @ref filteringFinished.  Calling
     * program may then call @ref getOutput to retrieve converted text.  Calling
     * program must call @ref ackFinished to acknowledge the conversion.
     */
    virtual bool asyncConvert(const QString& inputText, TalkerCode* talkerCode, const QString& appId);

    /**
     * Waits for a previous call to asyncConvert to finish.
     */
    virtual void waitForFinished();

    /**
     * Returns the state of the Filter.
     */
    virtual int getState();

    /**
     * Returns the filtered output.
     */
    virtual QString getOutput();

    /**
     * Acknowledges the finished filtering.
     */
    virtual void ackFinished();

    /**
     * Stops filtering.  The filteringStopped signal will emit when filtering
     * has in fact stopped and state returns to fsIdle;
     */
    virtual void stopFiltering();

    /**
     * Did this filter do anything?  If the filter returns the input as output
     * unmolested, it should return False when this method is called.
     */
    virtual bool wasModified();

private slots:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReceivedStdout();
    void slotReceivedStderr();

private:
    // Process output when xsltproc exits.
    void processOutput();

    /**
     * Check if an XML document has a certain root element.
     * @param xmldoc                 The document to check for the element.
     * @param elementName      The element to check for in the document.
     * @returns                             true if the root element exists in the document, false otherwise.
     */
    bool hasRootElement(const QString &xmldoc, const QString &elementName);
  
    /**
     * Check if an XML document has a certain DOCTYPE.
     * @param xmldoc             The document to check for the doctype.
     * @param name                The doctype name to check for. Pass QString() to not check the name.
     * @returns                         true if the parameters match the doctype, false otherwise.
     */
    bool hasDoctype(const QString &xmldoc, const QString &name);
  
    // If not empty, only apply to text queued by an applications containing one of these strings.
    QStringList m_appIdList;
    // If not empty, only apply to XML that has the specified root element.
    QStringList m_rootElementList;
    // If not empty, only apply to XML that has the specified DOCTYPE spec.
    QStringList m_doctypeList;
    // The text that is being filtered.
    QString m_text;
    // Processing state.
    int m_state;
    // xsltproc process.
    QProcess* m_xsltProc;
    // Input and Output filenames.
    QString m_inFilename;
    QString m_outFilename;
    // User's name for the filter.
    QString m_UserFilterName;
    // XSLT file.
    QString m_xsltFilePath;
    // Path to xsltproc processor.
    QString m_xsltprocPath;
    // Did this filter modify the text?
    bool m_wasModified;
};

#endif      // XMLTRANSFORMERPROC_H
