/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gradientwidget_impl.h"
#include "kalziumelementproperty.h"

#include <KLocalizedString>

#include "kalzium_debug.h"
#include <QIcon>
#include <QTimer>

#include <cmath>
#include <element.h>

#include "kalziumdataobject.h"
#include "prefs.h"

// used to convert the double variables to int's. (slider <-> spinbox)
#define MULTIPLIKATOR 1000

GradientWidgetImpl::GradientWidgetImpl(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
{
    setupUi(this);

    scheme_combo->addItems(KalziumElementProperty::instance()->schemeList());
    gradient_combo->addItems(KalziumElementProperty::instance()->gradientList());

    connect(gradient_spinbox, SIGNAL(valueChanged(double)), this, SLOT(doubleToSlider(double)));
    connect(gradient_slider, &QAbstractSlider::valueChanged, this, &GradientWidgetImpl::intToSpinbox);

    connect(Play, &QAbstractButton::clicked, this, &GradientWidgetImpl::play);
    connect(m_timer, &QTimer::timeout, this, &GradientWidgetImpl::tick);

    Play->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
}

GradientWidgetImpl::~GradientWidgetImpl()
{
    delete m_timer;
}

void GradientWidgetImpl::slotGradientChanged()
{
    if (!gradient_slider->isEnabled()) {
        gradient_spinbox->setEnabled(true);
        gradient_slider->setEnabled(true);
        Play->setEnabled(true);
    }

    KalziumElementProperty *elementProperty = KalziumElementProperty::instance();
    double dblMax = elementProperty->gradient()->maxValue();
    double dblMin = elementProperty->gradient()->minValue();

    // saving the decimals in the int
    const int intMax = dblMax * MULTIPLIKATOR;
    const int intMin = dblMin * MULTIPLIKATOR;

    // now we have the slider numbers, so put the speed to a adequate value.
    Speed->setMaximum(intMax / 100);
    Speed->setValue((intMax / 100) / 2);

    gradient_slider->setMaximum(intMax);
    gradient_slider->setMinimum(intMin);

    lblUnit->setText(elementProperty->gradient()->unit());

    gradient_spinbox->setMaximum(dblMax);
    gradient_spinbox->setMinimum(dblMin);
    gradient_spinbox->setDecimals(elementProperty->gradient()->decimals());

    switch (elementProperty->gradientId()) {
    case KalziumElementProperty::DISCOVERYDATE:
        gradient_spinbox->setValue(dblMax);
        break;

    case KalziumElementProperty::SOMGradientType:
        gradient_spinbox->setValue(dblMin + 293);
        break;

    default:
        gradient_spinbox->setValue(dblMin);
        break;
    }

    // Disable Gradient widgets if no gradient is selected.
    if (gradient_combo->currentIndex() == KalziumElementProperty::NOGRADIENT) {
        gradient_spinbox->setEnabled(false);
        gradient_slider->setEnabled(false);
        Play->setEnabled(false);
        text->clear();
    }
}

void GradientWidgetImpl::doubleToSlider(double doubleVar)
{
    // the signals need to be blocked as both will return to this slot. But no
    // matter which UI elements (slider oder spinbox) was changed, the other
    // has to be set to the same value

    gradient_slider->blockSignals(true);

    // setting the decimals in int
    int intvar = doubleVar * MULTIPLIKATOR;

    gradient_slider->setValue(intvar);

    gradient_slider->blockSignals(false);

    Q_EMIT gradientValueChanged(doubleVar);

    setNewValue(doubleVar);
}

void GradientWidgetImpl::intToSpinbox(int var)
{
    gradient_spinbox->blockSignals(true);

    // put int back to double with decimals
    double doublevar = var;
    doublevar = doublevar / MULTIPLIKATOR;

    gradient_spinbox->setValue(doublevar);

    gradient_spinbox->blockSignals(false);

    Q_EMIT gradientValueChanged(doublevar);

    setNewValue(doublevar);
}

void GradientWidgetImpl::setNewValue(double newValue)
{
    // Info text currently only for State of mater typ available.
    if (gradient_combo->currentIndex() != KalziumElementProperty::SOMGradientType) {
        text->clear();
        return;
    }

    static const int threshold = 25;

    const QString unitSymbol = lblUnit->text();

    QStringList listMeltingPoint;
    QStringList listBoilingPoint;
    QStringList listBoilingPointValue;
    QStringList listMeltingPointValue;

    foreach (Element *element, KalziumDataObject::instance()->ElementList) {
        double melting = element->dataAsVariant(ChemicalDataObject::meltingpoint, Prefs::temperatureUnit()).toDouble();
        if ((melting > 0.0) && fabs(melting - newValue) <= threshold) {
            listMeltingPoint << element->dataAsString(ChemicalDataObject::name);
            listMeltingPointValue << QString::number(melting);
        }

        double boiling = element->dataAsVariant(ChemicalDataObject::boilingpoint, Prefs::temperatureUnit()).toDouble();
        if ((boiling > 0.0) && fabs(boiling - newValue) <= threshold) {
            listBoilingPoint << element->dataAsString(ChemicalDataObject::name);
            listBoilingPointValue << QString::number(boiling);
        }
    }
    QString htmlcode;
    if (!listMeltingPoint.isEmpty()) {
        htmlcode += i18n("Elements with melting point around this temperature:") + '\n';
        for (int i = 0; i < listMeltingPoint.count(); ++i) {
            htmlcode += " - " + i18nc("For example: Carbon (300K)", "%1 (%2%3)", listMeltingPoint.at(i), listMeltingPointValue.at(i), unitSymbol) + '\n';
        }
        htmlcode += '\n';
    } else {
        htmlcode += i18n("No elements with a melting point around this temperature");
        htmlcode += QLatin1String("\n\n");
    }
    if (!listBoilingPoint.isEmpty()) {
        htmlcode += i18n("Elements with boiling point around this temperature:") + '\n';
        for (int i = 0; i < listBoilingPoint.count(); ++i) {
            htmlcode += " - " + i18nc("For example: Carbon (300K)", "%1 (%2%3)", listBoilingPoint.at(i), listBoilingPointValue.at(i), unitSymbol) + '\n';
        }
        htmlcode += '\n';
    } else {
        htmlcode += i18n("No elements with a boiling point around this temperature");
        htmlcode += '\n';
    }

    text->setText(/*m_htmlBegin +*/ htmlcode /*+ m_htmlEnd*/);
}

void GradientWidgetImpl::play()
{
    if (m_play) { // Currently playing
        // The Mode is 'Play' so stop
        stop();
        return;
    }

    // The mode is not 'play'
    // If the slider is at the maximum position bring it to the minimum
    if ((gradient_slider)->value() >= gradient_slider->maximum()) {
        gradient_slider->setValue(gradient_slider->minimum());
    }
    // start the timer at 200 millisecond time interval with single shot disabled
    m_timer->start(200);

    m_play = true; // start playing
    Play->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
}

void GradientWidgetImpl::stop()
{
    // Currently playing, stop the timer.
    m_timer->stop();
    Play->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_play = false; // Stop
}

void GradientWidgetImpl::tick()
{
    int increment = Speed->value();
    int temp = gradient_slider->value();
    int max = gradient_slider->maximum();
    if (temp + increment > max) {
        stop();
    }
    gradient_slider->setValue(temp + increment);
}

#include "moc_gradientwidget_impl.cpp"
