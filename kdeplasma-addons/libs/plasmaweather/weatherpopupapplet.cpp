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

#include "weatherpopupapplet.h"
#include "weatheri18ncatalog.h"

#include <QTimer>

#include <KConfigGroup>
#include <KConfigDialog>

#include <kunitconversion.h>

#include "weatherconfig.h"
#include "weatherlocation.h"

class WeatherPopupApplet::Private
{
public:
    Private(WeatherPopupApplet *weatherapplet)
        : q(weatherapplet)
        , weatherConfig(0)
        , weatherEngine(0)
        , timeEngine(0)
        , updateInterval(0)
        , location(nullptr)
    {
        busyTimer = new QTimer(q);
        busyTimer->setInterval(2*60*1000);
        busyTimer->setSingleShot(true);
        QObject::connect(busyTimer, SIGNAL(timeout()), q, SLOT(giveUpBeingBusy()));
    }

    WeatherPopupApplet *q;
    WeatherConfig *weatherConfig;
    Plasma::DataEngine *weatherEngine;
    Plasma::DataEngine *timeEngine;
    QString temperatureUnit;
    QString speedUnit;
    QString pressureUnit;
    QString visibilityUnit;
    int updateInterval;
    QString source;
    WeatherLocation *location;
    QString defaultIon;

    QString conditionIcon;
    QString tend;
    double pressure;
    double temperature;
    double latitude;
    double longitude;
    QTimer *busyTimer;

    void locationReady(const QString &src)
    {
        // set the location to the first valid one (the most accurate one)
        if (source.isEmpty()) {
            source = src;
            KConfigGroup cfg = q->config();
            cfg.writeEntry("source", source);
            emit q->configNeedsSaving();
            q->connectToEngine();
            q->setConfigurationRequired(false);
        }
    }

    void locationFinished()
    {
        busyTimer->stop();
        q->setBusy(false);
        if (source.isEmpty()) {
            q->showMessage(QIcon(), QString(), Plasma::ButtonNone);
            q->setConfigurationRequired(true);
        }
        delete location;
        location = nullptr;
    }

    void giveUpBeingBusy()
    {
        q->setBusy(false);

        QStringList list = source.split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() < 3) {
            q->setConfigurationRequired(true);
        } else {
            q->showMessage(KIcon(QLatin1String( "dialog-error" )),
                           i18n("Weather information retrieval for %1 timed out.", list.value(2)),
                           Plasma::ButtonNone);
        }
    }

    qreal tendency(const double pressure, const QString& tendency)
    {
        qreal t;

        if (tendency.toLower() == QLatin1String( "rising" )) {
            t = 0.75;
        } else if (tendency.toLower() == QLatin1String( "falling" )) {
            t = -0.75;
        } else {
            t = KPressure(tendency.toDouble(), pressureUnit).convertTo(KPressure::Kilopascal);
        }
        return t;
    }

    QString conditionFromPressure()
    {
        QString result;
        if (KPressure(0.0, pressureUnit).unitEnum() == KPressure::Invalid) {
            return QLatin1String( "weather-none-available" );
        }
        qreal temp = KTemperature(temperature, temperatureUnit).convertTo(KTemperature::Celsius);
        qreal p = KPressure(pressure, pressureUnit).convertTo(KPressure::Kilopascal);
        qreal t = tendency(pressure, tend);

        // This is completely unscientific so if anyone have a better formula for this :-)
        p += t * 10;

        Plasma::DataEngine::Data data = timeEngine->query(
                QString(QLatin1String( "Local|Solar|Latitude=%1|Longitude=%2" )).arg(latitude).arg(longitude));
        bool day = (data[QLatin1String( "Corrected Elevation" )].toDouble() > 0.0);

        if (p > 103.0) {
            if (day) {
                result = QLatin1String( "weather-clear" );
            } else {
                result = QLatin1String( "weather-clear-night" );
            }
        } else if (p > 100.0) {
            if (day) {
                result = QLatin1String( "weather-clouds" );
            } else {
                result = QLatin1String( "weather-clouds-night" );
            }
        } else if (p > 99.0) {
            if (day) {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-day" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-day" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            } else {
                if (temp > 1.0) {
                    result = QLatin1String( "weather-showers-scattered-night" );
                } else if (temp < -1.0)  {
                    result = QLatin1String( "weather-snow-scattered-night" );
                } else {
                    result = QLatin1String( "weather-snow-rain" );
                }
            }
        } else {
            if (temp > 1.0) {
                result = QLatin1String( "weather-showers" );
            } else if (temp < -1.0)  {
                result = QLatin1String( "weather-snow" );
            } else {
                result = QLatin1String( "weather-snow-rain" );
            }
        }
        //kDebug() << result;
        return result;
    }
};

WeatherPopupApplet::WeatherPopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();
    setHasConfigurationInterface(true);
}

WeatherPopupApplet::~WeatherPopupApplet()
{
    delete d;
}

void WeatherPopupApplet::init()
{
    configChanged();
}

void WeatherPopupApplet::connectToEngine()
{
    emit newWeatherSource();
    const bool missingLocation = d->source.isEmpty();

    if (missingLocation) {
        if (!d->location) {
            d->location = new WeatherLocation(this);
            connect(d->location, SIGNAL(valid(QString)), this, SLOT(locationReady(QString)));
            connect(d->location, SIGNAL(finished()), this, SLOT(locationFinished()));
        }

        d->busyTimer->stop();
        setBusy(false);
        d->location->setDataEngines(dataEngine(QLatin1String("geolocation")), d->weatherEngine);
        d->location->getDefault(d->defaultIon);
    } else {
        d->busyTimer->start();
        setBusy(true);
        d->weatherEngine->connectSource(d->source, this, d->updateInterval * 60 * 1000);
    }
}

void WeatherPopupApplet::setDefaultIon(const QString &ion)
{
    d->defaultIon = ion;
}

void WeatherPopupApplet::createConfigurationInterface(KConfigDialog *parent)
{
    d->weatherConfig = new WeatherConfig(parent);
    d->weatherConfig->setDataEngine(d->weatherEngine);
    d->weatherConfig->setSource(d->source);
    d->weatherConfig->setIon(d->defaultIon);
    d->weatherConfig->setUpdateInterval(d->updateInterval);
    d->weatherConfig->setTemperatureUnit(d->temperatureUnit);
    d->weatherConfig->setSpeedUnit(d->speedUnit);
    d->weatherConfig->setPressureUnit(d->pressureUnit);
    d->weatherConfig->setVisibilityUnit(d->visibilityUnit);
    parent->addPage(d->weatherConfig, i18n("Weather"), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(d->weatherConfig, SIGNAL(configValueChanged()) , parent , SLOT(settingsModified()));
}

void WeatherPopupApplet::configAccepted()
{
    d->temperatureUnit = d->weatherConfig->temperatureUnit();
    d->speedUnit = d->weatherConfig->speedUnit();
    d->pressureUnit = d->weatherConfig->pressureUnit();
    d->visibilityUnit = d->weatherConfig->visibilityUnit();
    d->updateInterval = d->weatherConfig->updateInterval();
    d->source = d->weatherConfig->source();

    KConfigGroup cfg = config();
    cfg.writeEntry("temperatureUnit", d->temperatureUnit);
    cfg.writeEntry("speedUnit", d->speedUnit);
    cfg.writeEntry("pressureUnit", d->pressureUnit);
    cfg.writeEntry("visibilityUnit", d->visibilityUnit);
    cfg.writeEntry("updateInterval", d->updateInterval);
    cfg.writeEntry("source", d->source);

    emit configNeedsSaving();
}

void WeatherPopupApplet::configChanged()
{
    if (!d->source.isEmpty()) {
        d->weatherEngine->disconnectSource(d->source, this);
    }

    KConfigGroup cfg = config();

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        d->temperatureUnit = cfg.readEntry("temperatureUnit", "C");
        d->speedUnit = cfg.readEntry("speedUnit", "m/s");
        d->pressureUnit = cfg.readEntry("pressureUnit", "hPa");
        d->visibilityUnit = cfg.readEntry("visibilityUnit", "km");
    } else {
        d->temperatureUnit = cfg.readEntry("temperatureUnit", "F");
        d->speedUnit = cfg.readEntry("speedUnit", "mph");
        d->pressureUnit = cfg.readEntry("pressureUnit", "inHg");
        d->visibilityUnit = cfg.readEntry("visibilityUnit", "ml");
    }
    d->updateInterval = cfg.readEntry("updateInterval", 30);
    d->source = cfg.readEntry("source", "");
    setConfigurationRequired(d->source.isEmpty());
    d->weatherEngine = dataEngine(QLatin1String( "weather" ));
    d->timeEngine = dataEngine(QLatin1String( "time" ));

    connectToEngine();
}

void WeatherPopupApplet::dataUpdated(const QString& source,
                                     const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source)

    if (data.isEmpty()) {
        return;
    }

    d->conditionIcon = data[QLatin1String( "Condition Icon" )].toString();
    if (data[QLatin1String( "Pressure" )].toString() != QLatin1String( "N/A" )) {
        d->pressure = KPressure(data[QLatin1String( "Pressure" )].toDouble(), static_cast<KPressure::KPresUnit>(data[QLatin1String( "Pressure Unit" )].toInt())).number();
    } else {
        d->pressure = 0.0;
    }
    d->tend = data[QLatin1String( "Pressure Tendency" )].toString();
    d->temperature = KTemperature(data[QLatin1String( "Temperature" )].toDouble(), static_cast<KTemperature::KTempUnit>(data[QLatin1String( "Temperature Unit" )].toInt())).number();
    d->latitude = data[QLatin1String( "Latitude" )].toDouble();
    d->longitude = data[QLatin1String( "Longitude" )].toDouble();
    setAssociatedApplicationUrls(KUrl(data.value(QLatin1String( "Credit Url" )).toString()));

    d->busyTimer->stop();
    setBusy(false);
    showMessage(QIcon(), QString(), Plasma::ButtonNone);
}

QString WeatherPopupApplet::pressureUnit()
{
    return d->pressureUnit;
}

QString WeatherPopupApplet::temperatureUnit()
{
    return d->temperatureUnit;
}

QString WeatherPopupApplet::speedUnit()
{
    return d->speedUnit;
}

QString WeatherPopupApplet::visibilityUnit()
{
    return d->visibilityUnit;
}

QString WeatherPopupApplet::conditionIcon()
{
    if (d->conditionIcon.isEmpty() || d->conditionIcon == QLatin1String( "weather-none-available" )) {
        d->conditionIcon = d->conditionFromPressure();
    }
    return d->conditionIcon;
}

WeatherConfig* WeatherPopupApplet::weatherConfig()
{
    return d->weatherConfig;
}

#include "moc_weatherpopupapplet.cpp"
