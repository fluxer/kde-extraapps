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

static const char* flickrapiurl = "https://api.flickr.com/services/rest/?api_key=31f4917c363e2f76b9fc944790dcc338&format=json&method=flickr.interestingness.getList&date=";

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
    KIO::StoredTransferJob *kstoredjob = static_cast<KIO::StoredTransferJob*>(kjob);
    if (kstoredjob->error()) {
        emit mParent->error(mParent);
        kWarning() << "pageRequestFinished error";
        return;
    }

    m_photoList.clear();

    // HACK: fix the data to be valid JSON
    QByteArray jsondata = kstoredjob->data();
    if (jsondata.startsWith("jsonFlickrApi(") && jsondata.endsWith(')')) {
        jsondata = jsondata.mid(14, jsondata.size() - 15);
    }

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

        const KUrl queryurl(QString::fromLatin1(flickrapiurl) + mActualDate.toString(Qt::ISODate));
        kDebug() << "stat fail, retrying with" << queryurl.prettyUrl();
        kstoredjob = KIO::storedGet(queryurl, KIO::NoReload, KIO::HideProgressInfo);
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
        const QString photofarm = photomap["farm"].toString();
        const QString photoserver = photomap["server"].toString();
        const QString photoid = photomap["id"].toString();
        const QString photosecret = photomap["secret"].toString();
        const QString photourl = QString::fromLatin1("https://farm%1.static.flickr.com/%2/%3_%4.jpg").arg(photofarm).arg(photoserver).arg(photoid).arg(photosecret);
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
    mParent->connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)));
}

void FlickrProvider::Private::imageRequestFinished(KJob *kjob)
{
    KIO::StoredTransferJob *kstoredjob = static_cast<KIO::StoredTransferJob*>(kjob);
    if (kstoredjob->error()) {
        emit mParent->error(mParent);
        return;
    }

    mImage = QImage::fromData(kstoredjob->data());
    emit mParent->finished(mParent);
}

FlickrProvider::FlickrProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args),
    d(new Private(this))
{
    d->mActualDate = date();
    const KUrl queryurl(QString::fromLatin1(flickrapiurl) + date().toString(Qt::ISODate));
    KIO::StoredTransferJob *kstoredjob = KIO::storedGet(queryurl, KIO::NoReload, KIO::HideProgressInfo);
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
