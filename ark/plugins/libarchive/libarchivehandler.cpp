/*
 * Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>
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

#include "libarchivehandler.h"
#include "kerfuffle/kerfuffle_export.h"
#include "kerfuffle/queries.h"

#include <QFileInfo>
#include <QDir>
#include <KDebug>
#include <KLocale>
#include <KArchive>

#include <sys/stat.h>

LibArchiveInterface::LibArchiveInterface(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
}

LibArchiveInterface::~LibArchiveInterface()
{
}

bool LibArchiveInterface::list()
{
    KArchive karchive(filename());
    if (!karchive.isReadable()) {
        emit error(i18nc("@info", "Could not open the archive <filename>%1</filename>: %2.", filename(), karchive.errorString()));
        return false;
    }

    foreach (const KArchiveEntry &karchiveentry, karchive.list()) {
        ArchiveEntry archiveentry;
        archiveentry[FileName] = karchiveentry.pathname;
        archiveentry[InternalID] = karchiveentry.pathname;
        archiveentry[Size] = qlonglong(karchiveentry.size);
        archiveentry[IsDirectory] = S_ISDIR(karchiveentry.mode);
        archiveentry[Permissions] = ReadWriteArchiveInterface::permissionsString(karchiveentry.mode);
        archiveentry[Owner] = karchiveentry.username;
        archiveentry[Group] = karchiveentry.groupname;
        archiveentry[Timestamp] = QDateTime::fromTime_t(karchiveentry.mtime);
        archiveentry[IsPasswordProtected] = karchiveentry.encrypted;
        if (!karchiveentry.symlink.isEmpty()) {
            archiveentry[Link] = karchiveentry.symlink;
        }
        emit entry(archiveentry);
    }
    emit progress(1.0);

    return true;
}

bool LibArchiveInterface::copyFiles(const QVariantList& files, const QString &destinationDirectory, ExtractionOptions options)
{
    const bool extractAll = files.isEmpty();
    const bool preservePaths = options.value(QLatin1String("PreservePaths")).toBool();

    KArchive karchive(filename());
    if (!karchive.isReadable()) {
        emit error(i18nc("@info", "Could not open the archive <filename>%1</filename>: %2.", filename(), karchive.errorString()));
        return false;
    }

    QStringList fileslist;
    if (extractAll) {
        foreach (const KArchiveEntry &karchiveentry, karchive.list()) {
            fileslist.append(QFile::decodeName(karchiveentry.pathname));
        }
    } else {
        foreach (const QVariant &variant, files) {
            fileslist.append(variant.toString());
        }
    }
    if (karchive.extract(fileslist, destinationDirectory, preservePaths)) {
        emit progress(1.0);
        return true;
    }

    emit error(karchive.errorString());
    return false;
}

bool LibArchiveInterface::addFiles(const QStringList &files, const CompressionOptions &options)
{
    const QString globalWorkDir = options.value(QLatin1String("GlobalWorkDir")).toString();
    if (!globalWorkDir.isEmpty()) {
        kDebug() << "GlobalWorkDir is set, changing dir to " << globalWorkDir;
        QDir::setCurrent(globalWorkDir);
    }

    QString rootNode = options.value(QLatin1String("RootNode"), QVariant()).toString();
    if (!rootNode.isEmpty() && !rootNode.endsWith(QLatin1Char('/'))) {
        rootNode.append(QLatin1Char('/'));
    }

    KArchive karchive(filename());
    if (!karchive.isWritable()) {
        emit error(i18nc("@info", "Could not open the archive <filename>%1</filename>: %2.", filename(), karchive.errorString()));
        return false;
    }

    const QString strip = (QDir::cleanPath(globalWorkDir) + QDir::separator());
    if (karchive.add(files, QFile::encodeName(strip), QFile::encodeName(rootNode))) {
        emit progress(1.0);
        return true;
    }

    emit error(karchive.errorString());
    return false;
}

bool LibArchiveInterface::deleteFiles(const QVariantList &files)
{
    KArchive karchive(filename());
    if (!karchive.isWritable()) {
        emit error(i18nc("@info", "Could not open the archive <filename>%1</filename>: %2.", filename(), karchive.errorString()));
        return false;
    }

    QStringList fileslist;
    foreach (const QVariant &variant, files) {
        fileslist.append(variant.toString());
    }
    if (karchive.remove(fileslist)) {
        foreach (const QString &file, fileslist) {
            emit entryRemoved(file);
        }
        emit progress(1.0);
        return true;
    }

    emit error(karchive.errorString());
    return false;
}

KERFUFFLE_EXPORT_PLUGIN(LibArchiveInterface)

#include "moc_libarchivehandler.cpp"
