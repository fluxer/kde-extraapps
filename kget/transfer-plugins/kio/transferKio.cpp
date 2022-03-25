/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferKio.h"
#include "core/verifier.h"
#include "core/signature.h"
#include "settings.h"

#include <utime.h>

#include <kiconloader.h>
#include <kio/scheduler.h>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>
#include <KIO/NetAccess>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>

#include <QtCore/QFile>

TransferKio::TransferKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(nullptr),
      m_verifier(nullptr),
      m_signature(nullptr),
      m_filemodel(nullptr)
{
    // TODO: check if it really can resume
    setCapabilities(Transfer::Cap_Resuming);
}

void TransferKio::start()
{
    if (status() != Job::Finished) {
        kDebug(5001) << "TransferKio::start";
        setStatus(Job::Running);
        setTransferChange(Transfer::Tc_Status, true);

        createJob();
    }
}

void TransferKio::stop()
{
    if (status() == Job::Stopped || status() == Job::Finished) {
        return;
    }

    if (m_copyjob) {
        m_copyjob->kill(KJob::EmitResult);
        m_copyjob = nullptr;
    }

    kDebug(5001) << "Stop";
    setStatus(Job::Stopped);
    m_downloadSpeed = 0;
    setTransferChange(Transfer::Tc_Status | Transfer::Tc_DownloadSpeed, true);
}

void TransferKio::deinit(Transfer::DeleteOptions options)
{
    // if the transfer is not finished, we delete the *.part-file
    if (options & Transfer::DeleteFiles) {
        KIO::Job *del = KIO::del(m_dest, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }
    // TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
}

bool TransferKio::repair(const KUrl &file)
{
    Q_UNUSED(file);

    if (verifier()->status() == Verifier::NotVerified) {
        m_downloadedSize = 0;
        m_percent = 0;
        if (m_copyjob) {
            m_copyjob->kill(KJob::Quietly);
            m_copyjob = nullptr;
        }
        setTransferChange(Transfer::Tc_DownloadedSize | Transfer::Tc_Percent, true);

        start();

        return true;
    }

    return false;
}

Verifier *TransferKio::verifier(const KUrl &file)
{
    Q_UNUSED(file);

    if (!m_verifier) {
        m_verifier = new Verifier(m_dest, this);
        connect(m_verifier, SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    }

    return m_verifier;
}

Signature *TransferKio::signature(const KUrl &file)
{
    Q_UNUSED(file);

    if (!m_signature) {
        m_signature = new Signature(m_dest, this);
    }

    return m_signature;
}

FileModel *TransferKio::fileModel()
{
    if (!m_filemodel) {
        m_filemodel = new FileModel(files(), directory(), this);
    }

    const KUrl fileurl = m_dest;

    QModelIndex fileindex = m_filemodel->index(fileurl, FileItem::File);
    m_filemodel->setData(fileindex, Qt::Checked, Qt::CheckStateRole);

    QModelIndex statusindex = m_filemodel->index(fileurl, FileItem::Status);
    m_filemodel->setData(statusindex, status());

    QModelIndex sizeindex = m_filemodel->index(fileurl, FileItem::Size);
    m_filemodel->setData(sizeindex, m_totalSize);

    QModelIndex checksumindex = m_filemodel->index(fileurl, FileItem::ChecksumVerified);
    m_filemodel->setData(checksumindex, verifier()->status());

    QModelIndex signatureindex = m_filemodel->index(fileurl, FileItem::SignatureVerified);
    m_filemodel->setData(signatureindex, signature()->status());

    return m_filemodel;
}

// NOTE: INTERNAL METHODS
void TransferKio::createJob()
{
    if (!m_copyjob) {
        KIO::Scheduler::checkSlaveOnHold(true);
        m_copyjob = KIO::file_copy(m_source, m_dest, -1, KIO::HideProgressInfo);

        connect(m_copyjob, SIGNAL(result(KJob*)), 
                this, SLOT(slotResult(KJob*)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob*,QString)), 
                this, SLOT(slotInfoMessage(KJob*,QString)));
        connect(m_copyjob, SIGNAL(percent(KJob*,ulong)), 
                this, SLOT(slotPercent(KJob*,ulong)));
        connect(m_copyjob, SIGNAL(totalSize(KJob*,qulonglong)), 
                this, SLOT(slotTotalSize(KJob*,qulonglong)));
        connect(m_copyjob, SIGNAL(processedSize(KJob*,qulonglong)), 
                this, SLOT(slotProcessedSize(KJob*,qulonglong)));
        connect(m_copyjob, SIGNAL(speed(KJob*,ulong)), 
                this, SLOT(slotSpeed(KJob*,ulong)));
    }
}

void TransferKio::slotResult( KJob * kioJob )
{
    kDebug(5001) << "slotResult" << kioJob->error() << kioJob->errorString();

    switch (kioJob->error()) {
        case 0:                            // The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST: { // The file has already been downloaded.
            setStatus(Job::Finished);
            m_percent = 100;
            m_downloadSpeed = 0;
            m_downloadedSize = m_totalSize;
            setTransferChange(Transfer::Tc_Status | Transfer::Tc_Percent | Transfer::Tc_DownloadSpeed, true);
            break;
        }
        default: {
            // There has been an error
            setError(kioJob->errorString(), SmallIcon("dialog-error"), Job::ManualSolve);
            setStatus(Job::Aborted);
            setLog(kioJob->errorString(), Transfer::Log_Error);
            setTransferChange(Transfer::Tc_Status | Transfer::Tc_Log, true);
            break;
        }
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob = nullptr;

    if (status() == Job::Finished) {
        if (m_verifier && Settings::checksumAutomaticVerification()) {
            m_verifier->verify();
        }
        if (m_signature && Settings::signatureAutomaticVerification()) {
            m_signature->verify();
        }
    }
}

void TransferKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
    Q_UNUSED(kioJob);

    setLog(msg, Transfer::Log_Info);
    setTransferChange(Transfer::Tc_Log, true);
}

void TransferKio::slotPercent( KJob * kioJob, unsigned long percent )
{
    kDebug(5001) << "slotPercent";
    Q_UNUSED(kioJob);

    m_percent = percent;
    setTransferChange(Transfer::Tc_Percent, true);
}

void TransferKio::slotTotalSize( KJob * kioJob, qulonglong size )
{
    kDebug(5001) << "slotTotalSize";
    Q_UNUSED(kioJob);

    m_totalSize = size;
    setTransferChange(Transfer::Tc_TotalSize, true);
}

void TransferKio::slotProcessedSize( KJob * kioJob, qulonglong size )
{
    kDebug(5001) << "slotProcessedSize";
    Q_UNUSED(kioJob);

    m_downloadedSize = size;
    setTransferChange(Transfer::Tc_DownloadedSize, true);
}

void TransferKio::slotSpeed( KJob * kioJob, unsigned long bytes_per_second )
{
    kDebug(5001) << "slotSpeed";
    Q_UNUSED(kioJob)

    m_downloadSpeed = bytes_per_second;
    setTransferChange(Transfer::Tc_DownloadSpeed, true);
}

void TransferKio::slotVerified(bool isVerified)
{
    if (!isVerified) {
        QString text = i18n("The download (%1) could not be verified. Do you want to repair it?", m_dest.fileName());

        if (!verifier()->partialChunkLength()) {
            text = i18n("The download (%1) could not be verified. Do you want to redownload it?", m_dest.fileName());
        }
        if (KMessageBox::warningYesNo(0,
                                      text,
                                      i18n("Verification failed.")) == KMessageBox::Yes) {
            repair();
        }
    }
}

#include "moc_transferKio.cpp"
