/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef TRANSFER_KIO_H
#define TRANSFER_KIO_H

#include <kio/job.h>

#include "core/transfer.h"
#include "core/verifier.h"
#include "core/signature.h"
#include "core/filemodel.h"

/**
 * This transfer uses the KIO class to download files
 */

class TransferKio : public Transfer
{
    Q_OBJECT
public:
    TransferKio(TransferGroup * parent, TransferFactory * factory,
                Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                const QDomElement * e = 0);

    // --- Job virtual functions ---
    void start() final;
    void stop() final;

    // --- Transfer virtual functions ---
    void deinit(Transfer::DeleteOptions options) final;

    bool repair(const KUrl &file = KUrl()) final;

    Verifier *verifier(const KUrl &file = KUrl()) final;
    Signature *signature(const KUrl &file = KUrl()) final;

    FileModel *fileModel() final;

    void save(const QDomElement &element) final;
    void load(const QDomElement *element) final;

private slots:
    void slotResult( KJob * kioJob );
    void slotInfoMessage( KJob * kioJob, const QString & msg );
    void slotPercent( KJob * kioJob, unsigned long percent );
    void slotTotalSize( KJob * kioJob, qulonglong size );
    void slotProcessedSize( KJob * kioJob, qulonglong size );
    void slotSpeed( KJob * kioJob, unsigned long bytes_per_second );
    void slotVerified(bool isVerified);
    void slotChecksumFound(QString type, QString checksum);

private:
    void createJob();

    KIO::FileCopyJob * m_copyjob;
    Verifier *m_verifier;
    Signature *m_signature;
    FileModel *m_filemodel;
    QMap<QString, QString> m_checksums;
};

#endif
