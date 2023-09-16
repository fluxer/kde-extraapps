/*
 *   Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>
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

#include "pexelsprovider.h"

#include <QImage>
#include <QJsonDocument>

#include <KDebug>
#include <KRandom>
#include <kio/job.h>

// for reference:
// https://www.pexels.com/api/documentation/#photos-curated

static const QString s_pexelsapikey = QString::fromLatin1("WeDUAEYr4oV9211fAie7Jcwat6tNlkrW0BTx7kaoiyITGWp7WMcxpSoM");
static const QString s_pexelsapiurl = QString::fromLatin1("https://api.pexels.com/v1/curated");

POTDPROVIDER_EXPORT_PLUGIN(PexelsProvider, "PexelsProvider", "")

class PexelsProvider::Private
{
public:
    Private(PexelsProvider *parent)
      : mParent(parent)
    {
    }

    void pageRequestFinished(KJob*);
    void imageRequestFinished(KJob*);
    void parsePage();

    PexelsProvider *mParent;
    QImage mImage;

private:
    QStringList m_photoList;
};

void PexelsProvider::Private::pageRequestFinished(KJob *kjob)
{
    KIO::StoredTransferJob* kstoredjob = qobject_cast<KIO::StoredTransferJob*>(kjob);
    if (kstoredjob->error()) {
        kWarning() << "request error" << kstoredjob->url();
        kstoredjob->deleteLater();
        emit mParent->error(mParent);
        return;
    }

    m_photoList.clear();

    const QByteArray jsondata = kstoredjob->data();
    kstoredjob->deleteLater();

    const QJsonDocument jsondoc = QJsonDocument::fromJson(jsondata);
    if (jsondoc.isNull()) {
        kWarning() << "JSON error" << jsondoc.errorString();
        return;
    }

    const QVariantMap jsonmap = jsondoc.toVariant().toMap();
    const QVariantList jsonphotoslist = jsonmap["photos"].toList();
    foreach (const QVariant &photo, jsonphotoslist) {
        const QVariantMap photomap = photo.toMap()["src"].toMap();
        const QString photourl = photomap["landscape"].toString();
        if (photourl.isEmpty()) {
            kDebug() << "skipping photo without landscape url";
            continue;
        }
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
    kstoredjob->addMetaData("Authorization", s_pexelsapikey);
    mParent->connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(imageRequestFinished(KJob*)));
}

void PexelsProvider::Private::imageRequestFinished(KJob *kjob)
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

PexelsProvider::PexelsProvider(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args),
    d(new Private(this))
{
    const KUrl queryurl(s_pexelsapiurl);
    kDebug() << "starting job for" << queryurl.prettyUrl();
    KIO::StoredTransferJob *kstoredjob = KIO::storedGet(queryurl, KIO::NoReload, KIO::HideProgressInfo);
    kstoredjob->setAutoDelete(false);
    kstoredjob->addMetaData("Authorization", s_pexelsapikey);
    connect(kstoredjob, SIGNAL(finished(KJob*)), SLOT(pageRequestFinished(KJob*)));
}

PexelsProvider::~PexelsProvider()
{
    delete d;
}

QImage PexelsProvider::image() const
{
    return d->mImage;
}

#include "moc_pexelsprovider.cpp"
