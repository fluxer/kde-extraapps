/*
 * Copyright (c) 2009  Raphael Kubo da Costa <rakuco@FreeBSD.org>
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

#include "singlefileplugin.h"
#include "kerfuffle/kerfuffle_export.h"
#include "kerfuffle/queries.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include <KDebug>
#include <KCompressor>
#include <KDecompressor>
#include <KLocale>

LibSingleFileInterface::LibSingleFileInterface(QObject *parent, const QVariantList & args)
        : Kerfuffle::ReadWriteArchiveInterface(parent, args)
{
}

LibSingleFileInterface::~LibSingleFileInterface()
{
}

bool LibSingleFileInterface::copyFiles(const QList<QVariant> &files, const QString &destinationDirectory, Kerfuffle::ExtractionOptions options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)

    const QString inputfile = filename();
    QString outputFileName = destinationDirectory;
    if (!destinationDirectory.endsWith(QLatin1Char('/'))) {
        outputFileName += QLatin1Char('/');
    }
    outputFileName += uncompressedFileName();

    outputFileName = overwriteFileName(outputFileName);
    if (outputFileName.isEmpty()) {
        return true;
    }

    kDebug() << "Extracting to" << outputFileName;

    KDecompressor kdecompressor;
    if (!kdecompressor.setType(KDecompressor::typeForMime(m_mimeType))
        && !kdecompressor.setType(KDecompressor::typeForFile(inputfile))) {
        kDebug() << "Could not set KDecompressor type";
        emit error(i18nc("@info", "Ark could not open <filename>%1</filename> for extraction.", inputfile));

        return false;
    }

    QFile inputdevice(inputfile);
    if (!inputdevice.open(QFile::ReadOnly)) {
        kDebug() << "Could not open input file";
        emit error(i18n("Ark could not open <filename>%1</filename> for reading.", inputfile));
        return false;
    }

    if (!kdecompressor.process(inputdevice.readAll())) {
        kDebug() << "Could not process input";
        emit error(i18n("Ark could not process <filename>%1</filename>.", inputfile));
        return false;
    }

    QFile outputdevice(outputFileName);
    if (!outputdevice.open(QIODevice::WriteOnly)) {
        kDebug() << "Failed to open output file" << outputdevice.errorString();
        emit error(i18nc("@info", "Ark could not open <filename>%1</filename> for writing.", outputFileName));

        return false;
    }

    const QByteArray decompressed = kdecompressor.result();
    if (outputdevice.write(decompressed.constData(), decompressed.size()) != decompressed.size()) {
        kDebug() << "Could not write output file";
        emit error(i18n("Ark could not write <filename>%1</filename>.", outputFileName));
        QFile::remove(outputFileName); // in case of partial write
        return false;
    }

    return true;
}

bool LibSingleFileInterface::list()
{
    kDebug();

    const QString filename = uncompressedFileName();

    Kerfuffle::ArchiveEntry e;

    e[Kerfuffle::FileName] = filename;
    e[Kerfuffle::InternalID] = filename;

    emit entry(e);

    return true;
}

QString LibSingleFileInterface::overwriteFileName(QString& filename)
{
    QString newFileName(filename);

    while (QFile::exists(newFileName)) {
        Kerfuffle::OverwriteQuery query(newFileName);

        query.setMultiMode(false);
        emit userQuery(&query);
        query.waitForResponse();

        if ((query.responseCancelled()) || (query.responseSkip())) {
            return QString();
        } else if (query.responseOverwrite()) {
            break;
        } else if (query.responseRename()) {
            newFileName = query.newFilename();
        }
    }

    return newFileName;
}

const QString LibSingleFileInterface::uncompressedFileName() const
{
    QString uncompressedName(QFileInfo(filename()).fileName());

    foreach(const QString & extension, m_possibleExtensions) {
        kDebug() << extension;

        if (uncompressedName.endsWith(extension, Qt::CaseInsensitive)) {
            uncompressedName.chop(extension.size());
            return uncompressedName;
        }
    }

    return uncompressedName + QLatin1String( ".uncompressed" );
}

bool LibSingleFileInterface::addFiles(const QStringList & files, const Kerfuffle::CompressionOptions& options)
{
    Q_UNUSED(options);

    if (files.size() == 0) {
        emit error(i18n("No input files."));
        return false;
    } else if (files.size() > 1) {
        emit error(i18n("The archive format does not support multiple input files."));
        return false;
    }

    const QString inputfile = files.first();
    const QString outputfile = filename();

    KCompressor kcompressor;
    if (!kcompressor.setType(KCompressor::typeForFile(outputfile))) {
        kDebug() << "Could not set KCompressor type";
        emit error(i18n("Ark could not create filter."));
        return false;
    }

    QFile inputdevice(inputfile);
    if (!inputdevice.open(QFile::ReadOnly)) {
        kDebug() << "Could not open input file";
        emit error(i18n("Ark could not open <filename>%1</filename> for reading.", inputfile));
        return false;
    }

    if (!kcompressor.process(inputdevice.readAll())) {
        kDebug() << "Could not process input";
        emit error(i18n("Ark could not process <filename>%1</filename>.", inputfile));
        return false;
    }

    QFile outputdevice(outputfile);
    if (!outputdevice.open(QFile::WriteOnly)) {
        kDebug() << "Could not open output file";
        emit error(i18n("Ark could not open <filename>%1</filename> for writing.", outputfile));
        return false;
    }

    const QByteArray compressed = kcompressor.result();
    if (outputdevice.write(compressed.constData(), compressed.size()) != compressed.size()) {
        kDebug() << "Could not write output file";
        emit error(i18n("Ark could not write <filename>%1</filename>.", outputfile));
        QFile::remove(outputfile); // in case of partial write
        return false;
    }

    return true;
}

bool LibSingleFileInterface::deleteFiles(const QList<QVariant> & files)
{
    emit error(i18n("Not implemented."));
    return false;
}

#include "moc_singlefileplugin.cpp"
