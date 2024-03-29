/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2008 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "addtoarchive.h"
#include "adddialog.h"
#include "archive.h"
#include "jobs.h"

#include <KConfig>
#include <kdebug.h>
#include <kjobtrackerinterface.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kio/job.h>

#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QtCore/qsharedpointer.h>

namespace Kerfuffle
{
AddToArchive::AddToArchive(QObject *parent)
        : KJob(parent), m_changeToFirstPath(false)
{
}

AddToArchive::~AddToArchive()
{
}

void AddToArchive::setAutoFilenameSuffix(const QString& suffix)
{
    m_autoFilenameSuffix = suffix;
}

void AddToArchive::setChangeToFirstPath(bool value)
{
    m_changeToFirstPath = value;
}

void AddToArchive::setFilename(const KUrl& path)
{
    m_filename = path.pathOrUrl();
}

void AddToArchive::setMimeType(const QString & mimeType)
{
    m_mimeType = mimeType;
}

bool AddToArchive::showAddDialog(void)
{
    QWeakPointer<Kerfuffle::AddDialog> dialog = new Kerfuffle::AddDialog(
        m_inputs, // itemsToAdd
        KUrl(m_firstPath), // startDir
        QLatin1String( "" ), // filter
        NULL, // parent
        NULL); // widget

    bool ret = dialog.data()->exec();

    if (ret) {
        kDebug() << "Returned URL:" << dialog.data()->selectedUrl();
        kDebug() << "Returned mime:" << dialog.data()->currentMimeFilter();
        setFilename(dialog.data()->selectedUrl());
        setMimeType(dialog.data()->currentMimeFilter());
    }

    delete dialog.data();

    return ret;
}

bool AddToArchive::addInput(const KUrl& url)
{
    m_inputs << url.pathOrUrl(
        QFileInfo(url.pathOrUrl()).isDir() ?
        KUrl::AddTrailingSlash :
        KUrl::RemoveTrailingSlash
    );

    if (m_firstPath.isEmpty()) {
        QString firstEntry = url.pathOrUrl(KUrl::RemoveTrailingSlash);
        m_firstPath = QFileInfo(firstEntry).dir().absolutePath();
    }

    return true;
}

void AddToArchive::start()
{
    QTimer::singleShot(0, this, SLOT(slotStartJob()));
}

void AddToArchive::slotStartJob(void)
{
    kDebug();

    Kerfuffle::CompressionOptions options;

    if (!m_inputs.size()) {
        KMessageBox::error(NULL, i18n("No input files were given."));
        return;
    }

    Kerfuffle::Archive *archive;
    if (!m_filename.isEmpty()) {
        archive = Kerfuffle::Archive::create(m_filename, m_mimeType, this);
        kDebug() << "Set filename to " << m_filename;
    } else {
        if (m_autoFilenameSuffix.isEmpty()) {
            KMessageBox::error(NULL, i18n("You need to either supply a filename for the archive or a suffix (such as tar.gz) with the <command>--autofilename</command> argument."));
            emitResult();
            return;
        }

        if (m_firstPath.isEmpty()) {
            kDebug() << "Weird, this should not happen. no firstpath defined. aborting";
            emitResult();
            return;
        }

        QString base = QFileInfo(m_inputs.first()).absoluteFilePath();
        if (base.endsWith(QLatin1Char('/'))) {
            base.chop(1);
        }

        QString finalName = base + QLatin1Char( '.' ) + m_autoFilenameSuffix;

        //if file already exists, append a number to the base until it doesn't
        //exist
        int appendNumber = 0;
        while (QFileInfo(finalName).exists()) {
            ++appendNumber;
            finalName = base + QLatin1Char( '_' ) + QString::number(appendNumber) + QLatin1Char( '.' ) + m_autoFilenameSuffix;
        }

        kDebug() << "Autoset filename to "<< finalName;
        archive = Kerfuffle::Archive::create(finalName, m_mimeType, this);
    }

    if (archive == NULL) {
        KMessageBox::error(NULL, i18n("Failed to create the new archive. Permissions might not be sufficient."));
        emitResult();
        return;
    } else if (archive->isReadOnly()) {
        KMessageBox::error(NULL, i18n("It is not possible to create archives of this type."));
        emitResult();
        return;
    }

    if (m_changeToFirstPath) {
        if (m_firstPath.isEmpty()) {
            kDebug() << "Weird, this should not happen. no firstpath defined. aborting";
            emitResult();
            return;
        }

        const QDir stripDir(m_firstPath);

        for (int i = 0; i < m_inputs.size(); ++i) {
            m_inputs[i] = stripDir.absoluteFilePath(m_inputs.at(i));
        }

        options[QLatin1String( "GlobalWorkDir" )] = stripDir.path();
        kDebug() << "Setting GlobalWorkDir to " << stripDir.path();
    }

    Kerfuffle::AddJob *job =
        archive->addFiles(m_inputs, options);

    KIO::getJobTracker()->registerJob(job);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotFinished(KJob*)));

    job->start();
}

void AddToArchive::slotFinished(KJob *job)
{
    kDebug();

    if (job->error()) {
        KMessageBox::error(NULL, job->errorText());
    }

    emitResult();
}
}
