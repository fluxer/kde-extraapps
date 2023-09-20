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

#include "weatherconfig.h"

#include <KDebug>
#include <KDialog>
#include <KInputDialog>
#include <KPixmapSequence>
#include <KPixmapSequenceWidget>
#include <KUnitConversion>

#include "weatherlocation.h"
#include "weathervalidator.h"
#include "weatheri18ncatalog.h"
#include "ui_weatherconfig.h"

class WeatherConfig::Private
{
public:
    Private(WeatherConfig *weatherconfig)
        : q(weatherconfig),
        location(nullptr),
        validator(nullptr),
        ion("wettercom"),
        dlg(nullptr),
        busyWidget(nullptr)
    {
    }

    void setSource(int index)
    {
        // do not emit the signals when searching, valid source may not even be found
        if (!searchLocation.isEmpty()) {
            return;
        }

        QString text = ui.locationCombo->itemData(index).toString();
        if (!text.isEmpty()) {
            source = text;
            emit q->settingsChanged();
            emit q->configValueChanged();
        }
    }

    void changePressed()
    {
        if (!validator) {
            kWarning() << "No validator";
            return;
        }

        QString text = ui.locationCombo->currentText();

        if (text.isEmpty()) {
            return;
        }

        if (location) {
            delete location;
            location = nullptr;
        }

        searchLocation = text;
        ui.locationCombo->clear();

        if (!busyWidget) {
            busyWidget = new KPixmapSequenceWidget(q);
            KPixmapSequence seq(QLatin1String("process-working"), 22);
            busyWidget->setSequence(seq);
            ui.locationSearchLayout->insertWidget(1, busyWidget);
        }

        validator->validate(text, true);
    }

    void enableOK()
    {
        if (dlg) {
            dlg->enableButton(KDialog::Ok, !source.isEmpty());
        }
    }

    void addSource(const QString &sources);
    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);

    WeatherConfig* q;
    QString searchLocation;
    WeatherLocation* location;
    WeatherValidator* validator;
    QString ion;
    QString source;
    Ui::WeatherConfig ui;
    KDialog *dlg;
    KPixmapSequenceWidget *busyWidget;
};

WeatherConfig::WeatherConfig(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    Weatheri18nCatalog::loadCatalog();

    d->dlg = qobject_cast<KDialog*>(parent);
    d->ui.setupUi(this);
    for (int i = 0; i < KTemperature::UnitCount; i++) {
        KTemperature::KTempUnit unit = static_cast<KTemperature::KTempUnit>(i);
        d->ui.temperatureComboBox->addItem(KTemperature::unitDescription(unit), unit);
    }
    for (int i = 0; i < KPressure::UnitCount; i++) {
        KPressure::KPresUnit unit = static_cast<KPressure::KPresUnit>(i);
        d->ui.pressureComboBox->addItem(KPressure::unitDescription(unit), unit);
    }
    for (int i = 0; i < KVelocity::UnitCount; i++) {
        KVelocity::KVeloUnit unit = static_cast<KVelocity::KVeloUnit>(i);
        d->ui.speedComboBox->addItem(KVelocity::unitDescription(unit), unit);
    }
    for (int i = 0; i < KLength::UnitCount; i++) {
        KLength::KLengUnit unit = static_cast<KLength::KLengUnit>(i);
        d->ui.visibilityComboBox->addItem(KLength::unitDescription(unit), unit);
    }

    d->ui.locationMessage->hide();
    d->ui.locationMessage->setMessageType(KMessageWidget::Error);

    connect(d->ui.locationCombo, SIGNAL(returnPressed()), this, SLOT(changePressed()));
    connect(d->ui.changeButton, SIGNAL(clicked()), this, SLOT(changePressed()));

    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setUpdateInterval(int)));

    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(settingsChanged()));
    connect(d->ui.temperatureComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingsChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingsChanged()));
    connect(d->ui.speedComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingsChanged()));
    connect(d->ui.visibilityComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingsChanged()));
    connect(d->ui.locationCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setSource(int)));

    connect(d->ui.updateIntervalSpinBox, SIGNAL(valueChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.temperatureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.speedComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.visibilityComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
}

WeatherConfig::~WeatherConfig()
{
    delete d;
}

void WeatherConfig::setDataEngines(Plasma::DataEngine *location, Plasma::DataEngine *weather)
{
    delete d->validator;
    d->validator = nullptr;
    if (weather) {
        d->validator = new WeatherValidator(this);
        connect(d->validator, SIGNAL(error(QString)), this, SLOT(validatorError(QString)));
        connect(d->validator, SIGNAL(finished(QMap<QString,QString>)), this, SLOT(addSources(QMap<QString,QString>)));
        d->validator->setDataEngine(weather);
        d->validator->setIon(d->ion);
    }
    if (location && weather) {
        d->location = new WeatherLocation(this);
        connect(d->location, SIGNAL(valid(QString)), this, SLOT(addSource(QString)));
        d->location->setDataEngines(location, weather);
    }
}

void WeatherConfig::Private::validatorError(const QString &error)
{
    kDebug() << error;
}

void WeatherConfig::Private::addSource(const QString &source)
{
    QStringList list = source.split(QLatin1Char('|'), QString::SkipEmptyParts);
    if (list.count() > 2) {
        // kDebug() << list;
        QString result = i18nc("A weather station location and the weather service it comes from",
                                "%1 (%2)", list[2], list[0]); // the names are too looong ions.value(list[0]));
        const int resultIndex = ui.locationCombo->findText(result);
        if (resultIndex < 0) {
            ui.locationCombo->addItem(result, source);
        }
    }
}

void WeatherConfig::Private::addSources(const QMap<QString, QString> &sources)
{
    QMapIterator<QString, QString> it(sources);

    while (it.hasNext()) {
        it.next();
        addSource(it.value());
    }

    delete busyWidget;
    busyWidget = nullptr;
    kDebug() << ui.locationCombo->count();
    if (ui.locationCombo->count() == 0) {
        ui.locationMessage->setText(i18n("No weather stations found for '%1'", searchLocation));
        ui.locationMessage->animatedShow();
    } else {
        ui.locationCombo->showPopup();
    }
    searchLocation.clear();
}

void WeatherConfig::setUpdateInterval(int interval)
{
    d->ui.updateIntervalSpinBox->setValue(interval);
    d->ui.updateIntervalSpinBox->setSuffix(ki18np(" minute", " minutes"));
}

void WeatherConfig::setTemperatureUnit(const QString &unit)
{
    KTemperature temp(0.0, unit);
    if (temp.unitEnum() != KTemperature::Invalid) {
        d->ui.temperatureComboBox->setCurrentIndex(static_cast<int>(temp.unitEnum()));
    }
}

void WeatherConfig::setPressureUnit(const QString &unit)
{
    KPressure pres(0.0, unit);
    if (pres.unitEnum() != KPressure::Invalid) {
        d->ui.pressureComboBox->setCurrentIndex(static_cast<int>(pres.unitEnum()));
    }
}

void WeatherConfig::setSpeedUnit(const QString &unit)
{
    KVelocity velo(0.0, unit);
    if (velo.unitEnum() != KVelocity::Invalid) {
        d->ui.speedComboBox->setCurrentIndex(static_cast<int>(velo.unitEnum()));
    }
}

void WeatherConfig::setVisibilityUnit(const QString &unit)
{
    KLength leng(0.0, unit);
    if (leng.unitEnum() != KLength::Invalid) {
        d->ui.visibilityComboBox->setCurrentIndex(static_cast<int>(leng.unitEnum()));
    }
}

void WeatherConfig::setSource(const QString &source)
{
    // kDebug() << "source set to" << source;
    d->addSource(source);
    d->source = source;
}

QString WeatherConfig::source() const
{
    return d->source;
}

int WeatherConfig::updateInterval() const
{
    return d->ui.updateIntervalSpinBox->value();
}

QString WeatherConfig::temperatureUnit() const
{
    KTemperature temp(0.0, static_cast<KTemperature::KTempUnit>(d->ui.temperatureComboBox->currentIndex()));
    return temp.unit();
}

QString WeatherConfig::pressureUnit() const
{
    KPressure pres(0.0, static_cast<KPressure::KPresUnit>(d->ui.pressureComboBox->currentIndex()));
    return pres.unit();
}

QString WeatherConfig::speedUnit() const
{
    KVelocity velo(0.0, static_cast<KVelocity::KVeloUnit>(d->ui.speedComboBox->currentIndex()));
    return velo.unit();
}

QString WeatherConfig::visibilityUnit() const
{
    KLength leng(0.0, static_cast<KLength::KLengUnit>(d->ui.visibilityComboBox->currentIndex()));
    return leng.unit();
}

void WeatherConfig::setConfigurableUnits(const ConfigurableUnits units)
{
    d->ui.unitsLabel->setVisible(units != None);
    d->ui.temperatureLabel->setVisible(units & Temperature);
    d->ui.temperatureComboBox->setVisible(units & Temperature);
    d->ui.pressureLabel->setVisible(units & Pressure);
    d->ui.pressureComboBox->setVisible(units & Pressure);
    d->ui.speedLabel->setVisible(units & Speed);
    d->ui.speedComboBox->setVisible(units & Speed);
    d->ui.visibilityLabel->setVisible(units & Visibility);
    d->ui.visibilityComboBox->setVisible(units & Visibility);
}

void WeatherConfig::setHeadersVisible(bool visible)
{
    d->ui.locationLabel->setVisible(visible);
    d->ui.unitsLabel->setVisible(visible);
}

void WeatherConfig::setIon(const QString &ion)
{
    d->ion = ion;
    if (d->validator) {
        d->validator->setIon(ion);
    }
    if (d->location) {
        d->location->getDefault(ion);
    }
}

#include "moc_weatherconfig.cpp"
