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

#ifndef LIBARCHIVEHANDLER_H
#define LIBARCHIVEHANDLER_H

#include "kerfuffle/archiveinterface.h"

#include <QStringList>

using namespace Kerfuffle;

class LibArchiveInterface: public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibArchiveInterface(QObject *parent, const QVariantList &args);
    ~LibArchiveInterface();

    bool list() final;
    bool copyFiles(const QVariantList& files, const QString &destinationDirectory, ExtractionOptions options) final;
    bool addFiles(const QStringList& files, const CompressionOptions &options) final;
    bool deleteFiles(const QVariantList& files) final;

private Q_SLOTS:
    void emitProgress(const qreal value);
};

#endif // LIBARCHIVEHANDLER_H
