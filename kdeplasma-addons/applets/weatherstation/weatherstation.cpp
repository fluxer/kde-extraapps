/*
 * Copyright 2008-2009  Petri Damstén <damu@iki.fi>
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "weatherstation.h"

#include <math.h>

#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include <KConfigDialog>
#include <KConfigGroup>
#include <KToolInvocation>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <plasmaweather/weatherconfig.h>

#include "lcd.h"

WeatherStation::WeatherStation(QObject *parent, const QVariantList &args)
    : WeatherPopupApplet(parent, args)
    , m_declarativeWidget(0)
    , m_lcdPanel(0)
{
    resize(250, 350);
}

WeatherStation::~WeatherStation()
{
}

void WeatherStation::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    m_declarativeWidget = new Plasma::DeclarativeWidget(this);
    layout->addItem(m_declarativeWidget);

    m_declarativeWidget->engine()->rootContext()->setContextProperty("weatherStation", this);

    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package package(QString(), "org.kde.lcdweather", structure);
    m_declarativeWidget->setQmlPath(package.filePath("mainscript"));


    m_lcdPanel = new LCD(this);
    m_lcdPanel->setSvg("weatherstation/lcd_panel");
    m_lcdPanel->setLabel("temperature-label", i18n("OUTDOOR TEMP"));
    m_lcdPanel->hide();

    WeatherPopupApplet::init();
}

QGraphicsWidget* WeatherStation::graphicsWidget()
{
    return m_declarativeWidget;
}

void WeatherStation::createConfigurationInterface(KConfigDialog *parent)
{
    WeatherPopupApplet::createConfigurationInterface(parent);
    WeatherConfig* wc = weatherConfig();
    wc->setConfigurableUnits(WeatherConfig::Temperature | WeatherConfig::Speed |
                             WeatherConfig::Pressure);

    QWidget *w = new QWidget();
    m_appearanceConfig.setupUi(w);
    m_appearanceConfig.backgroundCheckBox->setChecked(m_useBackground);
    m_appearanceConfig.tooltipCheckBox->setChecked(m_showToolTip);
    parent->addPage(w, i18n("Appearance"), icon());
    connect(m_appearanceConfig.backgroundCheckBox, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
    connect(m_appearanceConfig.tooltipCheckBox, SIGNAL(clicked(bool)), parent, SLOT(settingsModified()));
}

bool WeatherStation::useBackground() const
{
    return m_useBackground;
}

void WeatherStation::setUseBackground(bool use)
{
    if (use == m_useBackground)
        return;

    m_useBackground = use;

    m_lcdPanel->clear();
    if (m_useBackground) {
        m_lcdPanel->setItemOn("lcd_background");
    }
    m_lcdPanel->setItemOn("background");

    emit useBackgroundChanged();
}

void WeatherStation::setLCDIcon()
{
    if (m_lcdPanel->size().toSize() != size().toSize()) {
        m_lcdPanel->resize(size());
    }
    setPopupIcon(QIcon(m_lcdPanel->toPixmap()));
}

void WeatherStation::configAccepted()
{
    setUseBackground(m_appearanceConfig.backgroundCheckBox->isChecked());
    m_showToolTip = m_appearanceConfig.tooltipCheckBox->isChecked();

    KConfigGroup cfg = config();
    cfg.writeEntry("background", m_useBackground);
    cfg.writeEntry("tooltip", m_showToolTip);

    WeatherPopupApplet::configAccepted();
}

void WeatherStation::configChanged()
{
    KConfigGroup cfg = config();
    setUseBackground(cfg.readEntry("background", true));
    m_showToolTip = cfg.readEntry("tooltip", true);

    if (!m_showToolTip) {
        emit weatherLabelChanged(i18n("CURRENT WEATHER"));
        Plasma::ToolTipManager::self()->clearContent(this);
    }

    setLCDIcon();
    WeatherPopupApplet::configChanged();
}

QString WeatherStation::tempValue(const QString& value, int unit)
{
    if (value.isEmpty() || value == "N/A") {
        return QString();
    }
    KTemperature temp(value.toDouble(), static_cast<KTemperature::KTempUnit>(unit));
    KTemperature totemp(0.0, temperatureUnit());
    const double tempvalue = temp.convertTo(totemp.unitEnum());
    return QString::number(KUnitConversion::round(tempvalue, 1));
}

QString WeatherStation::presValue(const QString& value, int unit)
{
    if (value.isEmpty() || value == "N/A") {
        return QString();
    }
    KPressure pres(value.toDouble(), static_cast<KPressure::KPresUnit>(unit));
    KPressure topres(0.0, pressureUnit());
    const double presvalue = pres.convertTo(topres.unitEnum());
    return QString::number(KUnitConversion::round(presvalue, 1));
}

QString WeatherStation::veloValue(const QString& value, int unit)
{
    if (value.isEmpty() || value == "N/A") {
        return QString();
    }
    KVelocity velo(value.toDouble(), static_cast<KVelocity::KVeloUnit>(unit));
    KVelocity tovelo(0.0, speedUnit());
    const double velovalue = velo.convertTo(tovelo.unitEnum());
    return QString::number(KUnitConversion::round(velovalue, 1));
}

void WeatherStation::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    WeatherPopupApplet::dataUpdated(source, data);

    if (!data.contains("Place"))
        return;

    QString v = data["Temperature"].toString();
    QString temp = tempValue(v, data["Temperature Unit"].toInt());
    setTemperature(temp);

    setPressure(conditionIcon(), presValue(data["Pressure"].toString(),
                data["Pressure Unit"].toInt()),
                data["Pressure Tendency"].toString());

    setHumidity(data["Humidity"].toString());

    setWind(veloValue(data["Wind Speed"].toString(), data["Wind Speed Unit"].toInt()),
            data["Wind Direction"].toString());

    emit providerLabelChanged(data["Credit"].toString());

    m_url = data["Credit Url"].toString();

    if (m_showToolTip)
        setToolTip(data["Place"].toString());
}

QString WeatherStation::fromCondition(const QString& rawCondition)
{
    QString::SplitBehavior skip = QString::SkipEmptyParts;
    const QString condition = rawCondition.split("weather-", skip).first();
    QString id;

    if (condition == "clear-night") {
        id = "moon";
    } else if (condition == "clear") {
        id = "sun";
    } else if (condition == "few-clouds-night"
               || condition == "clouds-night") {
        id = "cloud_nights";
    } else if (condition == "few-clouds"
               || condition == "clouds") {
        id = "cloud_days";
    } else if (condition == "hail") {
        id = "hail";
    } else if (condition == "many-clouds"
               || condition == "mist") {
        id = "clouds_mist";
    } else if (condition == "showers-night"
               || condition == "showers-day") {
        id = "half_showers";
    } else if (condition == "showers") {
        id = "showers";
    } else if (condition == "showers-scattered-night"
               || condition == "showers-scattered-day"
               || condition == "showers-scattered") {
        id = "showers_scattered";
    } else if (condition == "snow") {
        id = "snow";
    } else if (condition == "snow-rain") {
        id = "snow_rain";
    } else if (condition == "snow-scattered-night"
               || condition == "snow-scattered-day"
               || condition == "snow-scattered") {
        id = "snow_scattered";
    } else if (condition == "storm") {
        id = "snow_storm";
    }

    return id;
}

void WeatherStation::setPressure(const QString& condition, const QString& pressure,
                                 const QString& tendencyString)
{
    QString currentCondition = "weather:" + fromCondition(condition);

    qreal t;
    if (tendencyString.toLower() == "rising") {
        t = 1.0;
    } else if (tendencyString.toLower() == "falling") {
        t = -1.0;
    } else {
        t = tendencyString.toDouble();
    }

    QString direction;
    if (t > 0.0) {
        direction = "up";
    } else if (t < 0.0) {
        direction = "down";
    }

    emit pressureChanged(currentCondition, pressure, pressureUnit(), direction);
}

void WeatherStation::setTemperature(const QString& temperature)
{
    m_lcdPanel->setLabel("temperature-unit-label", temperatureUnit());
    m_lcdPanel->setNumber("temperature", temperature);
    setLCDIcon();

    emit temperatureChanged(temperature, temperatureUnit());
}

void WeatherStation::setHumidity(QString humidity)
{
    if (humidity != "N/A")
        humidity.remove('%');

    emit humidityChanged(humidity);
}

void WeatherStation::setToolTip(const QString& place)
{
    emit weatherLabelChanged(place.toUpper());

    QString t = KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(),
                                                  KLocale::FancyLongDate);
    Plasma::ToolTipContent ttc(place, i18n("Last updated: %1", t));
    Plasma::ToolTipManager::self()->setContent(this, ttc);
}

void WeatherStation::setWind(const QString& speed, const QString& dir)
{
    QString direction = dir;

    if (dir == "N/A")
        direction = "";

    emit windChanged(direction, speed, speedUnit());
}

void WeatherStation::clicked()
{
    if(!m_url.isEmpty())
        KToolInvocation::invokeBrowser(m_url);
}

#include "moc_weatherstation.cpp"
