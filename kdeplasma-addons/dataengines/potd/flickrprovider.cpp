/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>
 *   Copyright  2008 by Georges Toth <gtoth@trypill.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "flickrprovider.h"

#include <QImage>
#include <QJsonDocument>

#include <KDebug>
#include <KRandom>
#include <kio/job.h>

// for reference:
// https://www.flickr.com/services/api/misc.urls.html
// https://www.flickr.com/services/api/explore/flickr.interestingness.getList

static const QString s_flickrapiurl = QString::fromLatin1(
    "https://api.flickr.com/services/rest/?api_key=31f4917c363e2f76b9fc944790dcc338&format=json&method=flickr.interestingness.getList&date="
);
static const QString s_flickrimageurl = QString::fromLatin1(
    "https://live.staticflickr.com/%1/%2_%3_b.jpg"
);

POTDPROVIDER_EXPORT_PLUGIN(FlickrProvider, "FlickrProvider", "")

class FlickrProvider::Private
{
  public:
    Private(FlickrProvider *parent)
      : mParent(parent)
    {
    }

    void pageRequestFinished(KJob*);
    void imageRequestFinished(KJob*);
    void parsePage();

    FlickrProvider *mParent;
    QDate mActualDate;
    QImage mImage;

  private:
    QStringList m_photoList;
};

void FlickrProvider::Private::pageRequestFinished(KJob *kjob)
{
    KIO::StoredTransferJob* kstoredjob = qobject_cast<KIO::StoredTransferJob*>(kjob);
    if (kstoredjob->error()) {
        kWarning() << "request error" << kstoredjob->url();
        kstoredjob->deleteLater();
        emit mParent->error(mParent);
        return;
    }

    m_photoList.clear();

    // HACK: fix the data to be valid JSON
    QByteArray jsondata = kstoredjob->data();
    if (jsondata.startsWith("jsonFlickrApi(") && jsondata.endsWith(')')) {
        jsondata = jsondata.mid(14, jsondata.size() - 15);
    }
    kstoredjob->deleteLater();

    const QJsonDocument jsondoc = QJsonDocument::fromJson(jsondata);
    if (jsondoc.isNull()) {
        kWarning() << "JSON error" << jsondoc.errorString();
        return;
    }

    const QVariantMap jsonmap = jsondoc.toVariant().toMap();
    const QString apistat = jsonmap["stat"].toString();
    if (apistat == QLatin1String("fail")) {
        // No pictures available for the specified parameters, decrement the date to two days earlier...
        mActualDate = mActualDate.addDays(-2);

        const KUrl queryurl(s_flickrapiurl + mActualDate.toString(Qt::ISODate));
        kDebug() << "stat fail, retrying with" << queryurl.prettyUrl();
        kstoredjob->deleteLater();
        kstoredjob = KIO::storedGet(queryurl, KIO::NoReload, KIO::HideProgressInfo);
        kstoredjob->setAutoDelete(false);
        mParent->connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)));
        return;
    }

    foreach (const QVariant &photo, jsonmap["photos"].toMap()["photo"].toList()) {
        const QVariantMap photomap = photo.toMap();
        const QString photoispublic = photomap["ispublic"].toString();
        if (photoispublic !=  QLatin1String("1")) {
            kDebug() << "skipping non-public photo";
            continue;
        }
        const QString photoserver = photomap["server"].toString();
        const QString photoid = photomap["id"].toString();
        const QString photosecret = photomap["secret"].toString();
        const QString photourl = s_flickrimageurl.arg(photoserver, photoid, photosecret);
        kDebug() << "photo" << photourl;
        m_photoList.append(photourl);
    }

    if (m_photoList.isEmpty()) {
        kWarning() << "empty photo list";
        return;
    }

    const KUrl photourl(m_photoList.at(KRandom::randomMax(m_photoList.size())));
    kDebug() << "chosen photo" << photourl.prettyUrl();
    kstoredjob = KIO::storedGet(photourl, KIO::NoReload, KIO::HideProgressInfo);
    kstoredjob->setAutoDelete(false);
    mParent->connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)));
}

void FlickrProvider::Private::imageRequestFinished(KJob *kjob)
{
    KIO::StoredTransferJob* kstoredjob = qobject_cast<KIO::StoredTransferJob*>(kjob);
    if (kstoredjob->error()) {
        kWarning() << "image job error" << kstoredjob->url();
        kstoredjob->deleteLater();
        emit mParent->error(mParent);
        return;
    }

    mImage = QImage::fromData(kstoredjob->data());
    if (mImage.isNull()) {
        kWarning() << "null image for" << kstoredjob->url();
    }
    kstoredjob->deleteLater();
    emit mParent->finished(mParent);
}

FlickrProvider::FlickrProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args),
    d(new Private(this))
{
    d->mActualDate = date();
    const KUrl queryurl(s_flickrapiurl + d->mActualDate.toString(Qt::ISODate));
    kDebug() << "starting job for" << queryurl.prettyUrl();
    KIO::StoredTransferJob *kstoredjob = KIO::storedGet(queryurl, KIO::NoReload, KIO::HideProgressInfo);
    kstoredjob->setAutoDelete(false);
    connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)));
}

FlickrProvider::~FlickrProvider()
{
    delete d;
}

QImage FlickrProvider::image() const
{
    return d->mImage;
}

#include "moc_flickrprovider.cpp"
