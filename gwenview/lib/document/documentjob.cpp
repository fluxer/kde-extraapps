// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2010 Aurélien Gâteau <agateau@kde.org>

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
#include "documentjob.h"
// KDE
#include <KApplication>
#include <KDebug>
#include <kdialogjobuidelegate.h>
#include <KLocale>

#include <chrono>

namespace Gwenview
{

VoidFuture::VoidFuture(QObject *parent, std::future<void> *voidfuture)
    : QThread(parent),
    mFuturePtr(voidfuture)
{
}

void VoidFuture::run()
{
    if (!mFuturePtr || !mFuturePtr->valid()) {
        kWarning() << "Invalid future";
        return;
    }
    // qDebug() << Q_FUNC_INFO;
    try {
        mFuturePtr->get();
    } catch (const std::system_error &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
    }
}


BoolFuture::BoolFuture(QObject *parent, std::future<bool> *boolfuture)
    : QThread(parent),
    mFuturePtr(boolfuture),
    mResult(false)
{
}

void BoolFuture::run()
{
    if (!mFuturePtr || !mFuturePtr->valid()) {
        kWarning() << "Invalid future";
        return;
    }
    // qDebug() << Q_FUNC_INFO;
    try {
#if 0
        while (mFuturePtr->valid()) {
            qApp->processEvents();
            mFuturePtr->wait_for(std::chrono::seconds(1));
            qDebug() << "Waiting";
        }
#endif
        mResult = mFuturePtr->get();
    } catch (const std::system_error &err) {
        kWarning() << err.what();
    } catch (...) {
        kWarning() << "Exception raised";
    }
}

bool BoolFuture::result() const
{
    return mResult;
}

struct DocumentJobPrivate
{
    Document::Ptr mDoc;
};

DocumentJob::DocumentJob()
: KCompositeJob(0)
, d(new DocumentJobPrivate)
{
    KDialogJobUiDelegate* delegate = new KDialogJobUiDelegate;
    delegate->setWindow(KApplication::kApplication()->activeWindow());
    delegate->setAutoErrorHandlingEnabled(true);
    setUiDelegate(delegate);
}

DocumentJob::~DocumentJob()
{
    delete d;
}

Document::Ptr DocumentJob::document() const
{
    return d->mDoc;
}

void DocumentJob::setDocument(const Document::Ptr& doc)
{
    d->mDoc = doc;
}

void DocumentJob::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

bool DocumentJob::checkDocumentEditor()
{
    if (!document()->editor()) {
        setError(NoDocumentEditorError);
        setErrorText(i18nc("@info", "Gwenview cannot edit this kind of image."));
        return false;
    }
    return true;
}

ThreadedDocumentJob::ThreadedDocumentJob()
    : DocumentJob(),
    mThreadedJob(nullptr)
{
    mFuture = std::async(std::launch::deferred, &ThreadedDocumentJob::threadedStart, this);
    mThreadedJob = new VoidFuture(this, &mFuture);
    connect(mThreadedJob, SIGNAL(finished()), this, SLOT(slotFinished()));
}

ThreadedDocumentJob::~ThreadedDocumentJob()
{
    if (!mThreadedJob->isFinished()) {
        mThreadedJob->wait();
    }
}

void ThreadedDocumentJob::doStart()
{
    mThreadedJob->start();
}

void ThreadedDocumentJob::slotFinished()
{
    emitResult();
}

} // namespace

#include "moc_documentjob.cpp"
