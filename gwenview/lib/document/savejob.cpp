// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "savejob.h"

// Qt
#include <QFuture>
#include <QFutureWatcher>
#include <QScopedPointer>
#include <QtConcurrentRun>

// KDE
#include <KApplication>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
#include <KLocale>
#include <KSaveFile>
#include <KTemporaryFile>
#include <KUrl>

// Local
#include "documentloadedimpl.h"

namespace Gwenview
{

struct SaveJobPrivate
{
    DocumentLoadedImpl* mImpl;
    KUrl mOldUrl;
    KUrl mNewUrl;
    QByteArray mFormat;
    QString mTemporaryFile;
    QScopedPointer<QFutureWatcher<void> > mInternalSaveWatcher;

    bool mKillReceived;
};

SaveJob::SaveJob(DocumentLoadedImpl* impl, const KUrl& url, const QByteArray& format)
: d(new SaveJobPrivate)
{
    d->mImpl = impl;
    d->mOldUrl = impl->document()->url();
    d->mNewUrl = url;
    d->mFormat = format;
    d->mKillReceived = false;
    setCapabilities(Killable);
}

SaveJob::~SaveJob()
{
    delete d;
}

void SaveJob::saveInternal()
{
    if (!d->mImpl->saveInternal(d->mTemporaryFile, d->mFormat)) {
        QFile::remove(d->mTemporaryFile);
        setError(UserDefinedError + 2);
        setErrorText(d->mImpl->document()->errorString());
    }
}

void SaveJob::doStart()
{
    if (d->mKillReceived) {
        return;
    }

    {
        QScopedPointer<KTemporaryFile> temporaryFile(new KTemporaryFile());
        temporaryFile->setAutoRemove(true);
        temporaryFile->setSuffix(QString::fromLatin1(".%1").arg(d->mFormat.constData()));

        if (!temporaryFile->open()) {
            KUrl dirUrl = d->mNewUrl;
            dirUrl.setFileName(QString());
            setError(UserDefinedError + 1);
            setErrorText(i18nc("@info", "Could not open file for writing, check that you have the necessary rights in <filename>%1</filename>.", dirUrl.pathOrUrl()));
            emitResult();
            return;
        }
        d->mTemporaryFile = temporaryFile->fileName();
    }

    QFuture<void> future = QtConcurrent::run(this, &SaveJob::saveInternal);
    d->mInternalSaveWatcher.reset(new QFutureWatcher<void>(this));
    connect(d->mInternalSaveWatcher.data(), SIGNAL(finished()), SLOT(finishSave()));
    d->mInternalSaveWatcher->setFuture(future);
}

void SaveJob::finishSave()
{
    d->mInternalSaveWatcher.reset(0);
    if (d->mKillReceived) {
        return;
    }

    if (error()) {
        emitResult();
        return;
    }

    // whether to overwite has already been asked for
    KIO::Job* job = KIO::move(KUrl::fromPath(d->mTemporaryFile), d->mNewUrl, KIO::Overwrite);
    job->ui()->setWindow(KApplication::kApplication()->activeWindow());
    addSubjob(job);
}

void SaveJob::slotResult(KJob* job)
{
    DocumentJob::slotResult(job);
    if (!error()) {
        emitResult();
    }
}

KUrl SaveJob::oldUrl() const
{
    return d->mOldUrl;
}

KUrl SaveJob::newUrl() const
{
    return d->mNewUrl;
}

bool SaveJob::doKill()
{
    d->mKillReceived = true;
    if (d->mInternalSaveWatcher) {
        d->mInternalSaveWatcher->waitForFinished();
    }
    return true;
}

} // namespace
