/*****************************************************************************
 *   Copyright (C) 2008 by Gerhard Gappmeier <gerhard.gappmeier@ascolab.com> *
 *   Copyright (C) 2009 by Petri Damstén <damu@iki.fi>                       *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.                *
 *****************************************************************************/

#include "unitconverter.h"

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QRegExp>
#include <KComboBox>
#include <KLineEdit>
#include <Plasma/Label>
#include <Plasma/Frame>
#include <kunitconversion.h>

ComboBox::ComboBox(QGraphicsWidget* parent)
    : Plasma::ComboBox(parent)
{
}

void ComboBox::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    emit mousePressed();
    Plasma::ComboBox::mousePressEvent(event);
}

UnitConverter::UnitConverter(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args)
    , m_widget(0)
    , m_bCalculateReverse(false)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("accessories-calculator");
    resize(400, 300);
}

UnitConverter::~UnitConverter()
{
    KConfigGroup cg = config();
    cg.writeEntry("category", m_pCmbCategory->nativeWidget()->currentIndex());
    cg.writeEntry("unit1", m_pCmbUnit1->nativeWidget()->currentIndex());
    cg.writeEntry("unit2", m_pCmbUnit2->nativeWidget()->currentIndex());
    cg.writeEntry("value", m_pTxtValue1->text());
}

void UnitConverter::init()
{
}

void UnitConverter::sltCategoryChanged(int index)
{
    m_pCmbUnit1->clear();
    m_pCmbUnit2->clear();
    switch (index) {
        case 0: {
            for (int i = 0; i < KTemperature::UnitCount; i++) {
                const QString description = KTemperature::unitDescription(static_cast<KTemperature::KTempUnit>(i));
                m_pCmbUnit1->nativeWidget()->addItem(description, QVariant::fromValue(i));
                m_pCmbUnit2->nativeWidget()->addItem(description, QVariant::fromValue(i));
            }
            break;
        }
        case 1: {
            for (int i = 0; i < KVelocity::UnitCount; i++) {
                const QString description = KVelocity::unitDescription(static_cast<KVelocity::KVeloUnit>(i));
                m_pCmbUnit1->nativeWidget()->addItem(description, QVariant::fromValue(i));
                m_pCmbUnit2->nativeWidget()->addItem(description, QVariant::fromValue(i));
            }
            break;
        }
        case 2: {
            for (int i = 0; i < KPressure::UnitCount; i++) {
                const QString description = KPressure::unitDescription(static_cast<KPressure::KPresUnit>(i));
                m_pCmbUnit1->nativeWidget()->addItem(description, QVariant::fromValue(i));
                m_pCmbUnit2->nativeWidget()->addItem(description, QVariant::fromValue(i));
            }
            break;
        }
        case 3: {
            for (int i = 0; i < KLength::UnitCount; i++) {
                const QString description = KLength::unitDescription(static_cast<KLength::KLengUnit>(i));
                m_pCmbUnit1->nativeWidget()->addItem(description, QVariant::fromValue(i));
                m_pCmbUnit2->nativeWidget()->addItem(description, QVariant::fromValue(i));
            }
            break;
        }
    }

    m_pCmbUnit1->nativeWidget()->setCurrentIndex(0);
    m_pCmbUnit2->nativeWidget()->setCurrentIndex(0);
    calculate();
}

void UnitConverter::sltUnitChanged(int index)
{
    Q_UNUSED(index);
    if ( m_bCalculateReverse ) {
        calculateReverse();
    } else {
        calculate();
    }
}

void UnitConverter::sltValueChanged(const QString &sNewValue)
{
    Q_UNUSED(sNewValue);
    m_bCalculateReverse = false; // store calculation direction
    calculate();
}

void UnitConverter::sltValueChangedReverse(const QString &sNewValue)
{
    Q_UNUSED(sNewValue);
    m_bCalculateReverse = true; // store calculation direction
    calculateReverse();
}

/// Calculates from left to right
void UnitConverter::calculate()
{
    const int fromindex = m_pCmbUnit1->nativeWidget()->currentIndex();
    const int toindex = m_pCmbUnit2->nativeWidget()->currentIndex();
    switch (m_pCmbCategory->nativeWidget()->currentIndex()) {
        case 0: {
            KTemperature temp(m_pTxtValue1->text().toDouble(), static_cast<KTemperature::KTempUnit>(fromindex));
            KTemperature totemp(0.0, static_cast<KTemperature::KTempUnit>(toindex));
            m_pTxtValue2->setText(QString::number(temp.convertTo(totemp.unitEnum())));
            break;
        }
        case 1: {
            KVelocity velo(m_pTxtValue1->text().toDouble(), static_cast<KVelocity::KVeloUnit>(fromindex));
            KVelocity tovelo(0.0, static_cast<KVelocity::KVeloUnit>(toindex));
            m_pTxtValue2->setText(QString::number(velo.convertTo(tovelo.unitEnum())));
            break;
        }
        case 2: {
            KPressure pres(m_pTxtValue1->text().toDouble(), static_cast<KPressure::KPresUnit>(fromindex));
            KPressure topres(0.0, static_cast<KPressure::KPresUnit>(toindex));
            m_pTxtValue2->setText(QString::number(pres.convertTo(topres.unitEnum())));
            break;
        }
        case 3: {
            KLength leng(m_pTxtValue1->text().toDouble(), static_cast<KLength::KLengUnit>(fromindex));
            KLength toleng(0.0, static_cast<KLength::KLengUnit>(toindex));
            m_pTxtValue2->setText(QString::number(leng.convertTo(toleng.unitEnum())));
            break;
        }
    }
}

/// Calculates from right to left
void UnitConverter::calculateReverse()
{
    const int fromindex = m_pCmbUnit2->nativeWidget()->currentIndex();
    const int toindex = m_pCmbUnit1->nativeWidget()->currentIndex();
    switch (m_pCmbCategory->nativeWidget()->currentIndex()) {
        case 0: {
            KTemperature temp(m_pTxtValue2->text().toDouble(), static_cast<KTemperature::KTempUnit>(fromindex));
            KTemperature totemp(0.0, static_cast<KTemperature::KTempUnit>(toindex));
            m_pTxtValue1->setText(QString::number(temp.convertTo(totemp.unitEnum())));
            break;
        }
        case 1: {
            KVelocity velo(m_pTxtValue2->text().toDouble(), static_cast<KVelocity::KVeloUnit>(fromindex));
            KVelocity tovelo(0.0, static_cast<KVelocity::KVeloUnit>(toindex));
            m_pTxtValue1->setText(QString::number(velo.convertTo(tovelo.unitEnum())));
            break;
        }
        case 2: {
            KPressure pres(m_pTxtValue2->text().toDouble(), static_cast<KPressure::KPresUnit>(fromindex));
            KPressure topres(0.0, static_cast<KPressure::KPresUnit>(toindex));
            m_pTxtValue1->setText(QString::number(pres.convertTo(topres.unitEnum())));
            break;
        }
        case 3: {
            KLength leng(m_pTxtValue2->text().toDouble(), static_cast<KLength::KLengUnit>(fromindex));
            KLength toleng(0.0, static_cast<KLength::KLengUnit>(toindex));
            m_pTxtValue1->setText(QString::number(leng.convertTo(toleng.unitEnum())));
            break;
        }
    }
}

QGraphicsWidget *UnitConverter::graphicsWidget()
{
    if (!m_widget) {
        m_widget = new QGraphicsWidget(this);
        Plasma::Frame *pHeader = new Plasma::Frame(this);
        pHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        pHeader->setText(i18n("Unit Converter"));

        Plasma::Label *pLabel = new Plasma::Label(this);
        pLabel->nativeWidget()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pLabel->setText(i18n("Convert:"));
        m_pCmbCategory = new ComboBox(this);
        connect(m_pCmbCategory, SIGNAL(mousePressed()), this, SLOT(raise()));
        m_pCmbCategory->setZValue(2);

        m_pCmbUnit1 = new ComboBox(this);
        m_pCmbUnit2 = new ComboBox(this);
        connect(m_pCmbUnit1, SIGNAL(mousePressed()), this, SLOT(raise()));
        connect(m_pCmbUnit2, SIGNAL(mousePressed()), this, SLOT(raise()));
        m_pCmbUnit1->setZValue(1);
        m_pCmbUnit2->setZValue(1);
        m_pTxtValue1 = new Plasma::LineEdit(this);
        m_pTxtValue2 = new Plasma::LineEdit(this);

        QGraphicsGridLayout *pGridLayout = new QGraphicsGridLayout(m_widget);
        pGridLayout->addItem(pHeader, 0, 0, 1, 2);
        pGridLayout->addItem(pLabel, 1, 0);
        pGridLayout->addItem(m_pCmbCategory, 1, 1);
        pGridLayout->addItem(m_pCmbUnit1, 2, 0);
        pGridLayout->addItem(m_pCmbUnit2, 2, 1);
        pGridLayout->addItem(m_pTxtValue1, 3, 0);
        pGridLayout->addItem(m_pTxtValue2, 3, 1);
        pGridLayout->setRowStretchFactor(4, 1);

        m_pCmbCategory->nativeWidget()->addItem(KTemperature::description(), QVariant::fromValue(0));
        m_pCmbCategory->nativeWidget()->addItem(KVelocity::description(), QVariant::fromValue(1));
        m_pCmbCategory->nativeWidget()->addItem(KPressure::description(), QVariant::fromValue(2));
        m_pCmbCategory->nativeWidget()->addItem(KLength::description(), QVariant::fromValue(3));

        connect(m_pTxtValue1->nativeWidget(), SIGNAL(textEdited(QString)),
                this, SLOT(sltValueChanged(QString)));
        connect(m_pTxtValue2->nativeWidget(), SIGNAL(textEdited(QString)),
                this, SLOT(sltValueChangedReverse(QString)));
        connect(m_pCmbCategory->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltCategoryChanged(int)));
        connect(m_pCmbUnit1->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltUnitChanged(int)));
        connect(m_pCmbUnit2->nativeWidget(), SIGNAL(currentIndexChanged(int)),
                this, SLOT(sltUnitChanged(int)));

        configChanged();
    }
    return m_widget;
}

void UnitConverter::configChanged()
{
    // Load previous values
    KConfigGroup cg = config();
    int category = cg.readEntry("category", 0);
    m_pCmbCategory->nativeWidget()->setCurrentIndex(category);
    sltCategoryChanged(category);
    int unit1 = cg.readEntry("unit1", -1);
    if (unit1 >= 0) {
        m_pCmbUnit1->nativeWidget()->setCurrentIndex(unit1);
    }
    int unit2 = cg.readEntry("unit2", -1);
    if (unit2 >= 0) {
        m_pCmbUnit2->nativeWidget()->setCurrentIndex(unit2);
    }
    m_pTxtValue1->setText(cg.readEntry("value", "1"));
    calculate();
}

#include "moc_unitconverter.cpp"

