// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
// Self
#include "moc_loadingdocumentimpl.cpp"

// STL
#include <memory>

// Qt
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QPointer>

// KDE
#include <KDebug>
#include <KIO/Job>
#include <KIO/JobClasses>
#include <KLocale>
#include <KMimeType>
#include <KProtocolInfo>
#include <KUrl>

// Local
#include "animateddocumentloadedimpl.h"
#include "document.h"
#include "documentjob.h"
#include "documentloadedimpl.h"
#include "emptydocumentimpl.h"
#include "gvdebug.h"
#include "imageutils.h"
#include "orientation.h"
#include "urlutils.h"
#include "gwenviewconfig.h"

namespace Gwenview
{

const int HEADER_SIZE = 256;

struct LoadingDocumentImplPrivate
{
    LoadingDocumentImpl* q;
    QPointer<KIO::TransferJob> mTransferJob;
    QScopedPointer<BoolThread> mMetaInfoFutureWatcher;
    QScopedPointer<VoidThread> mImageDataFutureWatcher;

    // If != 0, this means we need to load an image at zoom =
    // 1/mImageDataInvertedZoom
    int mImageDataInvertedZoom;

    bool mMetaInfoLoaded;
    bool mAnimated;
    bool mDownSampledImageLoaded;
    QByteArray mFormatHint;
    QByteArray mData;
    QByteArray mFormat;
    QSize mImageSize;
    QImage mImage;

    /**
     * Determine kind of document and switch to an implementation if it is not
     * necessary to download more data.
     * @return true if switched to another implementation.
     */
    bool determineKind()
    {
        QString mimeType;
        const KUrl& url = q->document()->url();
        if (KProtocolInfo::determineMimetypeFromExtension(url.protocol())) {
            mimeType = KMimeType::findByNameAndContent(url.fileName(), mData)->name();
        } else {
            mimeType = KMimeType::findByContent(mData)->name();
        }
        MimeTypeUtils::Kind kind = MimeTypeUtils::mimeTypeKind(mimeType);
        kDebug() << "mimeType:" << mimeType << ", kind:" << kind;
        q->setDocumentKind(kind);

        switch (kind) {
        case MimeTypeUtils::KIND_IMAGE:
            return false;

        default:
            q->setDocumentErrorString(
                i18nc("@info", "Gwenview cannot display documents of type %1.", mimeType)
            );
            emit q->loadingFailed();
            q->switchToImpl(new EmptyDocumentImpl(q->document()));
            return true;
        }
    }

    void startLoading()
    {
        Q_ASSERT(!mMetaInfoLoaded);

        switch (q->document()->kind()) {
        case MimeTypeUtils::KIND_IMAGE:
            // The hint is used to:
            // - Speed up loadMetaInfo(): QImageReader will try to decode the
            //   image using plugins matching this format first.
            mFormatHint = q->document()->url().fileName()
                .section('.', -1).toAscii().toLower();
            mMetaInfoFutureWatcher.reset(new BoolThread(q, std::bind(&LoadingDocumentImplPrivate::loadMetaInfo, this)));
            q->connect(
                mMetaInfoFutureWatcher.data(), SIGNAL(finished()),
                q, SLOT(slotMetaInfoLoaded())
            );
            mMetaInfoFutureWatcher->start();
            break;

        default:
            kWarning() << "We should not reach this point!";
            break;
        }
    }

    void startImageDataLoading()
    {
        kDebug() << "Starting image loading";
        Q_ASSERT(mMetaInfoLoaded);
        Q_ASSERT(mImageDataInvertedZoom != 0);
        Q_ASSERT(!mImageDataFutureWatcher->isRunning());
        mImageDataFutureWatcher.reset(new VoidThread(q, std::bind(&LoadingDocumentImplPrivate::loadImageData, this)));
        q->connect(
            mImageDataFutureWatcher.data(), SIGNAL(finished()),
            q, SLOT(slotImageLoaded())
        );
        mImageDataFutureWatcher->start();
    }

    bool loadMetaInfo()
    {
        kDebug() << "mFormatHint" << mFormatHint;
        QBuffer buffer;
        buffer.setBuffer(&mData);
        buffer.open(QIODevice::ReadOnly);

        QImageReader reader(&buffer, mFormatHint);
        mImageSize = reader.size();

        if (!reader.canRead()) {
            kWarning() << "QImageReader::read() using format hint" << mFormatHint << "failed:" << reader.errorString();
            if (buffer.pos() != 0) {
                kWarning() << "A bad Katie image decoder moved the buffer to" << buffer.pos() << "in a call to canRead()! Rewinding.";
                buffer.seek(0);
            }
            reader.setFormat(QByteArray());
            // Set buffer again, otherwise QImageReader won't restart from scratch
            reader.setDevice(&buffer);
            if (!reader.canRead()) {
                kWarning() << "QImageReader::read() without format hint failed:" << reader.errorString();
                return false;
            }
            kWarning() << "Image format is actually" << reader.format() << "not" << mFormatHint;
        }

        mFormat = reader.format();

        kDebug() << "mFormat" << mFormat;
        GV_RETURN_VALUE_IF_FAIL(!mFormat.isEmpty(), false);

        kDebug() << "mImageSize" << mImageSize;

        return true;
    }

    void loadImageData()
    {
        QBuffer buffer;
        buffer.setBuffer(&mData);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer, mFormat);

        kDebug() << "mImageDataInvertedZoom=" << mImageDataInvertedZoom;
        if (mImageSize.isValid()
                && mImageDataInvertedZoom != 1
                && reader.supportsOption(QImageIOHandler::ScaledSize)
           ) {
            // Do not use mImageSize here: QImageReader needs a non-transposed
            // image size
            QSize size = reader.size() / mImageDataInvertedZoom;
            if (!size.isEmpty()) {
                kDebug() << "Setting scaled size to" << size;
                reader.setScaledSize(size);
            } else {
                kDebug() << "Not setting scaled size as it is empty" << size;
            }
        }

        bool ok = reader.read(&mImage);
        if (!ok) {
            kDebug() << "QImageReader::read() failed";
            return;
        }

        if (reader.supportsAnimation()
                && reader.nextImageDelay() > 0 // Assume delay == 0 <=> only one frame
           ) {
            if (reader.imageCount() > 0) {
                kDebug() << "Really an animated image";
                mAnimated = true;
                return;
            }
            /*
             * QImageReader is not really helpful to detect animated images:
             * - QImageReader::imageCount() returns 0
             * - QImageReader::nextImageDelay() may return something > 0 if the
             *   image consists of only one frame but includes a "Graphic
             *   Control Extension" (usually only present if we have an
             *   animation) (Bug #185523)
             *
             * Decoding the next frame is the only reliable way I found to
             * detect an animated images
             */
            kDebug() << "May be an animated image. delay:" << reader.nextImageDelay();
            QImage nextImage;
            if (reader.read(&nextImage)) {
                kDebug() << "Really an animated image (more than one frame)";
                mAnimated = true;
            } else {
                kWarning() << q->document()->url() << "is not really an animated image (only one frame)";
            }
        }
    }
};

LoadingDocumentImpl::LoadingDocumentImpl(Document* document)
: AbstractDocumentImpl(document)
, d(new LoadingDocumentImplPrivate)
{
    d->q = this;
    d->mMetaInfoLoaded = false;
    d->mAnimated = false;
    d->mDownSampledImageLoaded = false;
    d->mImageDataInvertedZoom = 0;
}

LoadingDocumentImpl::~LoadingDocumentImpl()
{
    // Disconnect watchers to make sure they do not trigger further work
    if (d->mMetaInfoFutureWatcher) {
        d->mMetaInfoFutureWatcher->disconnect();
    }
    if (d->mImageDataFutureWatcher) {
        d->mImageDataFutureWatcher->disconnect();
    }

    if (d->mMetaInfoFutureWatcher) {
        d->mMetaInfoFutureWatcher->wait();
    }
    if (d->mImageDataFutureWatcher) {
        d->mImageDataFutureWatcher->wait();
    }

    if (d->mTransferJob) {
        d->mTransferJob->kill();
    }
    delete d;
}

void LoadingDocumentImpl::init()
{
    KUrl url = document()->url();

    if (UrlUtils::urlIsFastLocalFile(url)) {
        // Load file content directly
        QFile file(url.toLocalFile());
        if (!file.open(QIODevice::ReadOnly)) {
            setDocumentErrorString(i18nc("@info", "Could not open file %1", url.toLocalFile()));
            emit loadingFailed();
            switchToImpl(new EmptyDocumentImpl(document()));
            return;
        }
        d->mData = file.read(HEADER_SIZE);
        if (d->determineKind()) {
            return;
        }
        d->mData += file.readAll();
        d->startLoading();
    } else {
        // Transfer file via KIO
        d->mTransferJob = KIO::get(document()->url());
        connect(d->mTransferJob, SIGNAL(data(KIO::Job*,QByteArray)),
                SLOT(slotDataReceived(KIO::Job*,QByteArray)));
        connect(d->mTransferJob, SIGNAL(result(KJob*)),
                SLOT(slotTransferFinished(KJob*)));
        d->mTransferJob->start();
    }
}

void LoadingDocumentImpl::loadImage(int invertedZoom)
{
    if (d->mImageDataInvertedZoom == invertedZoom) {
        kDebug() << "Already loading an image at invertedZoom=" << invertedZoom;
        return;
    }
    if (d->mImageDataInvertedZoom == 1) {
        kDebug() << "Ignoring request: we are loading a full image";
        return;
    }
    if (d->mImageDataFutureWatcher) {
        d->mImageDataFutureWatcher->wait();
    }
    d->mImageDataInvertedZoom = invertedZoom;

    if (d->mMetaInfoLoaded) {
        // Do not test on mMetaInfoFuture.isRunning() here: it might not have
        // started if we are downloading the image from a remote url
        d->startImageDataLoading();
    }
}

void LoadingDocumentImpl::slotDataReceived(KIO::Job* job, const QByteArray& chunk)
{
    d->mData.append(chunk);
    if (document()->kind() == MimeTypeUtils::KIND_UNKNOWN && d->mData.length() >= HEADER_SIZE) {
        if (d->determineKind()) {
            job->kill();
            return;
        }
    }
}

void LoadingDocumentImpl::slotTransferFinished(KJob* job)
{
    if (job->error()) {
        setDocumentErrorString(job->errorString());
        emit loadingFailed();
        switchToImpl(new EmptyDocumentImpl(document()));
        return;
    }
    d->startLoading();
}

bool LoadingDocumentImpl::isEditable() const
{
    return d->mDownSampledImageLoaded;
}

Document::LoadingState LoadingDocumentImpl::loadingState() const
{
    if (!document()->image().isNull()) {
        return Document::Loaded;
    } else if (d->mMetaInfoLoaded) {
        return Document::MetaInfoLoaded;
    } else if (document()->kind() != MimeTypeUtils::KIND_UNKNOWN) {
        return Document::KindDetermined;
    } else {
        return Document::Loading;
    }
}

void LoadingDocumentImpl::slotMetaInfoLoaded()
{
    kDebug() << "Meta information loaded";
    Q_ASSERT(!d->mMetaInfoFutureWatcher->isRunning());
    if (!d->mMetaInfoFutureWatcher->result()) {
        setDocumentErrorString(
            i18nc("@info", "Loading meta information failed.")
        );
        emit loadingFailed();
        switchToImpl(new EmptyDocumentImpl(document()));
        d->mMetaInfoFutureWatcher.reset(nullptr);
        return;
    }
    d->mMetaInfoFutureWatcher.reset(nullptr);

    setDocumentFormat(d->mFormat);
    setDocumentImageSize(d->mImageSize);

    d->mMetaInfoLoaded = true;
    emit metaInfoLoaded();

    // Start image loading if necessary
    // We test if mImageDataFuture is not already running because code connected to
    // metaInfoLoaded() signal could have called loadImage()
    if ((!d->mImageDataFutureWatcher || !d->mImageDataFutureWatcher->isRunning()) && d->mImageDataInvertedZoom != 0) {
        d->startImageDataLoading();
    }
}

void LoadingDocumentImpl::slotImageLoaded()
{
    kDebug() << "Image loaded";
    if (d->mImage.isNull()) {
        setDocumentErrorString(
            i18nc("@info", "Loading image failed.")
        );
        emit loadingFailed();
        switchToImpl(new EmptyDocumentImpl(document()));
        d->mImageDataFutureWatcher.reset(nullptr);
        return;
    }
    d->mImageDataFutureWatcher.reset(nullptr);

    if (d->mAnimated) {
        if (d->mImage.size() == d->mImageSize) {
            // We already decoded the first frame at the right size, let's show
            // it
            setDocumentImage(d->mImage);
        }

        switchToImpl(new AnimatedDocumentLoadedImpl(
                         document(),
                         d->mData));

        return;
    }

    if (d->mImageDataInvertedZoom != 1 && d->mImage.size() != d->mImageSize) {
        kDebug() << "Loaded a down sampled image";
        d->mDownSampledImageLoaded = true;
        // We loaded a down sampled image
        setDocumentDownSampledImage(d->mImage, d->mImageDataInvertedZoom);
        return;
    }

    kDebug() << "Loaded a full image";
    setDocumentImage(d->mImage);
    DocumentLoadedImpl* impl;
    impl = new DocumentLoadedImpl(
        document(),
        d->mData);
    switchToImpl(impl);
}

} // namespace
