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

#include <QLineEdit>

#include <KDebug>
#include <KDialog>
#include <KInputDialog>
#include <KPixmapSequence>
#include <kpixmapsequencewidget.h>
//#include <KPixmapSequenceWidget>

#include <kunitconversion.h>

#include "weathervalidator.h"
#include "weatheri18ncatalog.h"
#include "ui_weatherconfig.h"

class WeatherConfig::Private
{
public:
    Private(WeatherConfig *weatherconfig)
        : q(weatherconfig),
          dataengine(0),
          dlg(0),
          busyWidget(0),
          checkedInCount(0)
    {
    }

    void setSource(int index)
    {
        QString text = ui.locationCombo->itemData(index).toString();
        if (text.isEmpty()) {
            ui.locationCombo->lineEdit()->setText(QString());
        } else {
            source = text;
            emit q->settingsChanged();
        }
    }

    void changePressed()
    {
        checkedInCount = 0;
        QString text = ui.locationCombo->currentText();

        if (text.isEmpty()) {
            return;
        }

        ui.locationCombo->clear();
        ui.locationCombo->lineEdit()->setText(text);

        if (!busyWidget) {
            busyWidget = new KPixmapSequenceWidget(q);
            KPixmapSequence seq(QLatin1String( "process-working" ), 22);
            busyWidget->setSequence(seq);
            ui.locationSearchLayout->insertWidget(1, busyWidget);
        }

        foreach (WeatherValidator *validator, validators) {
            validator->validate(text, true);
        }
    }

    void enableOK()
    {
        if (dlg) {
            dlg->enableButton(KDialog::Ok, !source.isEmpty());
        }
    }

    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);

    WeatherConfig *q;
    //QHash<QString, QString> ions;
    QList<WeatherValidator *> validators;
    Plasma::DataEngine *dataengine;
    QString source;
    Ui::WeatherConfig ui;
    KDialog *dlg;
    KPixmapSequenceWidget *busyWidget;
    int checkedInCount;
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
    
    connect(d->ui.locationCombo, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
    connect(d->ui.pressureComboBox, SIGNAL(currentIndexChanged(int)) , this , SIGNAL(configValueChanged()));
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

void WeatherConfig::setDataEngine(Plasma::DataEngine* dataengine)
{
    d->dataengine = dataengine;
    //d->ions.clear();
    qDeleteAll(d->validators);
    d->validators.clear();
    if (d->dataengine) {
        const QVariantList plugins = d->dataengine->query(QLatin1String( "ions" )).values();
        foreach (const QVariant& plugin, plugins) {
            const QStringList pluginInfo = plugin.toString().split(QLatin1Char( '|' ));
            if (pluginInfo.count() > 1) {
                //kDebug() << "ion: " << pluginInfo[0] << pluginInfo[1];
                //d->ions.insert(pluginInfo[1], pluginInfo[0]);
                WeatherValidator *validator = new WeatherValidator(this);
                connect(validator, SIGNAL(error(QString)), this, SLOT(validatorError(QString)));
                connect(validator, SIGNAL(finished(QMap<QString,QString>)), this, SLOT(addSources(QMap<QString,QString>)));
                validator->setDataEngine(dataengine);
                validator->setIon(pluginInfo[1]);
                d->validators.append(validator);
            }
        }
    }
}

void WeatherConfig::Private::validatorError(const QString &error)
{
    kDebug() << error;
}

void WeatherConfig::Private::addSources(const QMap<QString, QString> &sources)
{
    QMapIterator<QString, QString> it(sources);

    while (it.hasNext()) {
        it.next();
        QStringList list = it.value().split(QLatin1Char( '|' ), QString::SkipEmptyParts);
        if (list.count() > 2) {
            //kDebug() << list;
            QString result = i18nc("A weather station location and the weather service it comes from",
                                   "%1 (%2)", list[2], list[0]); // the names are too looong ions.value(list[0]));
            ui.locationCombo->addItem(result, it.value());
        }
    }

    ++checkedInCount;
    if (checkedInCount >= validators.count()) {
        delete busyWidget;
        busyWidget = 0;
        kDebug() << ui.locationCombo->count();
        if (ui.locationCombo->count() == 0 && ui.locationCombo->itemText(0).isEmpty()) {
            const QString current = ui.locationCombo->currentText();
            ui.locationCombo->addItem(i18n("No weather stations found for '%1'", current));
            ui.locationCombo->lineEdit()->setText(current);
        }
        ui.locationCombo->showPopup();
    }
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
        d->ui.pressureComboBox->setCurrentIndex(static_cast<int>(velo.unitEnum()));
    }
}

void WeatherConfig::setVisibilityUnit(const QString &unit)
{
    KLength leng(0.0, unit);
    if (leng.unitEnum() != KLength::Invalid) {
        d->ui.pressureComboBox->setCurrentIndex(static_cast<int>(leng.unitEnum()));
    }
}

void WeatherConfig::setSource(const QString &source)
{
    //kDebug() << "source set to" << source;
    const QStringList list = source.split(QLatin1Char( '|' ));
    if (list.count() > 2) {
        QString result = i18nc("A weather station location and the weather service it comes from",
                               "%1 (%2)", list[2], list[0]);
        d->ui.locationCombo->lineEdit()->setText(result);
    }
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

#include "moc_weatherconfig.cpp"
