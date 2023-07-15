/*
 *   Copyright (C) 2009 Petri Damstén <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#include "weatherlocation.h"
#include "weathervalidator.h"
#include "weatheri18ncatalog.h"

#include <kdebug.h>

class WeatherLocation::Private
{
public:
    Private(WeatherLocation *location)
        : q(location),
          locationEngine(nullptr),
          weatherEngine(nullptr),
          foundsource(false)
    {
    }

    void validatorFinished(const QMap<QString, QString> &results)
    {
        WeatherValidator* validator = qobject_cast<WeatherValidator*>(q->sender());
        QString source;
        if (!results.isEmpty()) {
            source = results.begin().value();
        }

        validators.remove(validator);
        if (!source.isEmpty() && !foundsource) {
            foundsource = true;
            emit q->finished(source);
        }
        if (!foundsource && validators.isEmpty()) {
            emit q->finished(QString());
        }
    }

    WeatherLocation *q;
    Plasma::DataEngine *locationEngine;
    Plasma::DataEngine *weatherEngine;
    bool foundsource;
    QMap<WeatherValidator*,QString> validators;
};

WeatherLocation::WeatherLocation(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();
}

WeatherLocation::~WeatherLocation()
{
    Q_ASSERT(d->validators.size() == 0);
    delete d;
}

void WeatherLocation::setDataEngines(Plasma::DataEngine* location, Plasma::DataEngine* weather)
{
    d->locationEngine = location;
    d->weatherEngine = weather;
}

void WeatherLocation::getDefault()
{
    if (d->locationEngine && d->locationEngine->isValid()) {
        d->locationEngine->connectSource(QLatin1String("location"), this);
    } else {
        emit finished(QString());
    }
}

void WeatherLocation::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (!d->locationEngine) {
        return;
    }

    d->locationEngine->disconnectSource(source, this);
    foreach (const QString &datakey, data.keys()) {
        const QVariantHash datahash = data[datakey].toHash();
        QString city = datahash[QLatin1String("city")].toString();
        if (city.contains(QLatin1Char(','))) {
            city.truncate(city.indexOf(QLatin1Char(',')) - 1);
        }
        if (!city.isEmpty()) {
            WeatherValidator* validator = new WeatherValidator(this);
            validator->setDataEngine(d->weatherEngine);
            connect(
                validator, SIGNAL(finished(QMap<QString,QString>)),
                this, SLOT(validatorFinished(QMap<QString,QString>))
            );
            d->validators.insert(validator, city);
            kDebug() <<  "validating" << city;
        }
    }

    if (d->validators.isEmpty()) {
        emit finished(QString());
    } else {
        foreach (WeatherValidator* validator, d->validators.keys()) {
            validator->validate(QLatin1String("wettercom"), d->validators.value(validator), true);
        }
    }
}

#include "moc_weatherlocation.cpp"
