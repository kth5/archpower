/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gasCalculator.h"

#include <KLocalizedString>

#include "kalziumunitcombobox.h"
#include "kalziumutils.h"
#include "prefs.h"

using namespace KUnitConversion;

gasCalculator::gasCalculator(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    setupUnitComboboxes();

    init();

    connect(ui.temp, SIGNAL(valueChanged(double)), this, SLOT(tempChanged()));
    connect(ui.temp_unit, SIGNAL(activated(int)), this, SLOT(tempChanged()));
    connect(ui.volume, SIGNAL(valueChanged(double)), this, SLOT(volChanged()));
    connect(ui.volume_unit, SIGNAL(activated(int)), this, SLOT(volChanged()));
    connect(ui.pressure, SIGNAL(valueChanged(double)), this, SLOT(pressureChanged()));
    connect(ui.pressure_unit, SIGNAL(activated(int)), this, SLOT(pressureChanged()));
    connect(ui.mass, SIGNAL(valueChanged(double)), this, SLOT(massChanged()));
    connect(ui.mass_unit, SIGNAL(activated(int)), this, SLOT(massChanged()));
    connect(ui.moles, SIGNAL(valueChanged(double)), this, SLOT(molesChanged(double)));
    connect(ui.molarMass, SIGNAL(valueChanged(double)), this, SLOT(molarMassChanged(double)));
    connect(ui.a, SIGNAL(valueChanged(double)), this, SLOT(Vand_aChanged()));
    connect(ui.b, SIGNAL(valueChanged(double)), this, SLOT(Vand_bChanged()));
    connect(ui.b_unit, SIGNAL(activated(int)), this, SLOT(Vand_bChanged()));
    connect(ui.mode, SIGNAL(activated(int)), this, SLOT(setMode(int)));
    connect(ui.reset, &QAbstractButton::clicked, this, &gasCalculator::init);
}

gasCalculator::~gasCalculator() = default;

void gasCalculator::init()
{
    error(RESET_GAS_MESSAGE);

    ui.molarMass->setValue(2.008);
    ui.temp->setValue(273.0);
    ui.volume->setValue(22.400);
    ui.pressure->setValue(1.0);
    ui.a->setValue(0.0);
    ui.b->setValue(0.0);
    ui.mass->setValue(2.016);
    ui.moles->setValue(1.0);

    ui.mass_unit->setCurrentIndex(0);
    ui.pressure_unit->setCurrentIndex(0);
    ui.temp_unit->setCurrentIndex(0);
    ui.volume_unit->setCurrentIndex(0);
    ui.b_unit->setCurrentIndex(0);

    m_temp = Value(273.0, KUnitConversion::Kelvin);
    m_molarMass = 2.016;
    m_pressure = Value(1.0, KUnitConversion::Atmosphere);
    m_mass = Value(2.016, KUnitConversion::Gram);
    m_moles = 1.0;
    m_Vand_a = 0.0;
    m_Vand_b = Value(0.0, KUnitConversion::Liter);
    m_vol = Value(22.4, KUnitConversion::Liter);

    if (Prefs::ideal()) {
        ui.non_ideal->hide();
    }

    setMode(VOLUME);
}

void gasCalculator::setupUnitComboboxes()
{
    QList<int> units;
    units << Gram << Milligram << Kilogram << Ton;
    KalziumUtils::populateUnitCombobox(ui.mass_unit, units);

    units.clear();
    units << Atmosphere << Pascal << Bar << Millibar << Torr;
    KalziumUtils::populateUnitCombobox(ui.pressure_unit, units);

    units.clear();
    units << Kelvin << Celsius << Fahrenheit;
    KalziumUtils::populateUnitCombobox(ui.temp_unit, units);

    units.clear();
    units << Liter << Milliliter << CubicMeter << KUnitConversion::GallonUS;
    KalziumUtils::populateUnitCombobox(ui.volume_unit, units);

    units.clear();
    units << Liter << Milliliter << CubicMeter << KUnitConversion::GallonUS;
    KalziumUtils::populateUnitCombobox(ui.b_unit, units);
}

int gasCalculator::getCurrentUnitId(QComboBox *comboBox)
{
    return comboBox->itemData(comboBox->currentIndex()).toInt();
}

void gasCalculator::calculatePressure()
{
    double volume = m_vol.convertTo(KUnitConversion::Liter).number();
    double temp = m_temp.convertTo(KUnitConversion::Kelvin).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    double pressure = m_moles * R * temp / (volume - m_moles * b) - m_moles * m_moles * m_Vand_a / volume / volume;

    m_pressure = Value(pressure, KUnitConversion::Atmosphere);
    m_pressure = m_pressure.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.pressure_unit)));
    ui.pressure->setValue(m_pressure.number());
}

void gasCalculator::calculateMolarMass()
{
    double mass = m_mass.convertTo(KUnitConversion::Gram).number();
    double volume = m_vol.convertTo(KUnitConversion::Liter).number();
    double pressure = m_pressure.convertTo(KUnitConversion::Atmosphere).number();
    double temp = m_temp.convertTo(KUnitConversion::Kelvin).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    m_molarMass = mass * R * temp / (pressure + m_moles * m_moles * m_Vand_a / volume / volume) / (volume - m_moles * b);
    ui.molarMass->setValue(m_molarMass);
}

void gasCalculator::calculateVol()
{
    double pressure = m_pressure.convertTo(KUnitConversion::Atmosphere).number();
    double temp = m_temp.convertTo(KUnitConversion::Kelvin).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    double volume = m_moles * R * temp / pressure + (m_moles * b);
    m_vol = Value(volume, KUnitConversion::Liter);
    m_vol = m_vol.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.volume_unit)));
    ui.volume->setValue(m_vol.number());
}

void gasCalculator::calculateTemp()
{
    double volume = m_vol.convertTo(KUnitConversion::Liter).number();
    double pressure = m_pressure.convertTo(KUnitConversion::Atmosphere).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    double temp = (pressure + (m_moles * m_moles * m_Vand_a / volume / volume)) * (volume - m_moles * b) / m_moles / R;
    m_temp = Value(temp, KUnitConversion::Kelvin);
    m_temp = m_temp.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.temp_unit)));
    ui.temp->setValue(m_temp.number());
}

void gasCalculator::calculateMoles()
{
    double volume = m_vol.convertTo(KUnitConversion::Liter).number();
    double pressure = m_pressure.convertTo(KUnitConversion::Atmosphere).number();
    double temp = m_temp.convertTo(KUnitConversion::Kelvin).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    m_moles = (pressure + m_moles * m_moles * m_Vand_a / volume / volume) * (volume - m_moles * b) / R / temp;
    ui.moles->setValue(m_moles);
}

void gasCalculator::calculateMass()
{
    double volume = m_vol.convertTo(KUnitConversion::Liter).number();
    double pressure = m_pressure.convertTo(KUnitConversion::Atmosphere).number();
    double temp = m_temp.convertTo(KUnitConversion::Kelvin).number();
    double b = m_Vand_b.convertTo(KUnitConversion::Liter).number();

    double mass = (pressure + m_moles * m_moles * m_Vand_a / volume / volume) * (volume - m_moles * b) * m_molarMass / R / temp;
    m_mass = Value(mass, KUnitConversion::Gram);
    m_mass = m_mass.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.mass_unit)));
    ui.mass->setValue(m_mass.number());
}

void gasCalculator::volChanged()
{
    m_vol = Value(ui.volume->value(), KUnitConversion::UnitId(getCurrentUnitId(ui.volume_unit)));
    calculate();
}

void gasCalculator::tempChanged()
{
    m_temp = Value(ui.temp->value(), KUnitConversion::UnitId(getCurrentUnitId(ui.temp_unit)));
    calculate();
}

void gasCalculator::pressureChanged()
{
    m_pressure = Value(ui.pressure->value(), KUnitConversion::UnitId(getCurrentUnitId(ui.pressure_unit)));
    calculate();
}

void gasCalculator::massChanged()
{
    m_mass = Value(ui.mass->value(), KUnitConversion::UnitId(getCurrentUnitId(ui.mass_unit)));
    m_moles = m_mass.convertTo(KUnitConversion::Gram).number() / m_molarMass;
    ui.moles->setValue(m_moles);
    calculate();
}

void gasCalculator::molesChanged(double value)
{
    m_moles = value;
    m_mass = Value(m_moles * m_molarMass, KUnitConversion::Gram);
    m_mass = m_mass.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.mass_unit)));
    ui.mass->setValue(m_mass.number());
    calculate();
}

void gasCalculator::molarMassChanged(double value)
{
    if (value == 0.0) {
        error(GAS_MOLAR_MASS_ZERO);
        return;
    }
    m_molarMass = value;
    m_mass = Value(m_molarMass * m_moles, KUnitConversion::Gram);
    m_mass = m_mass.convertTo(KUnitConversion::UnitId(getCurrentUnitId(ui.mass_unit)));
    ui.mass->setValue(m_mass.number());
    calculate();
}

void gasCalculator::Vand_aChanged()
{
    m_Vand_a = ui.a->value();
    calculate();
}

void gasCalculator::Vand_bChanged()
{
    m_Vand_b = Value(ui.b->value(), KUnitConversion::UnitId(getCurrentUnitId(ui.b_unit)));
    calculate();
}

void gasCalculator::setMode(int mode)
{
    m_mode = mode;

    ui.moles->setReadOnly(false);
    ui.mass->setReadOnly(false);
    ui.pressure->setReadOnly(false);
    ui.temp->setReadOnly(false);
    ui.volume->setReadOnly(false);

    switch (mode) {
    case MOLES:
        ui.moles->setReadOnly(true);
        ui.mass->setReadOnly(true);
        break;
    case PRESSURE:
        ui.pressure->setReadOnly(true);
        break;
    case TEMPERATURE:
        ui.temp->setReadOnly(true);
        break;
    case VOLUME:
        ui.volume->setReadOnly(true);
        break;
    }

    calculate();
}

void gasCalculator::calculate()
{
    error(RESET_GAS_MESSAGE);

    switch (m_mode) {
    case MOLES:
        calculateMoles();
        break;
    case PRESSURE:
        calculatePressure();
        break;
    case TEMPERATURE:
        calculateTemp();
        break;
    case VOLUME:
        calculateVol();
        break;
    }
}

void gasCalculator::error(int mode)
{
    switch (mode) { // Depending on the mode, set the error messages.
    case RESET_GAS_MESSAGE:
        ui.error->setText(QString());
        break;
    case VOL_ZERO:
        ui.error->setText(i18n("Volume cannot be zero, please enter a valid value."));
        break;
    case GAS_MOLAR_MASS_ZERO:
        ui.error->setText(i18n("Molar mass cannot be zero, please enter a non-zero value."));
    default:
        break;
    }
}

#include "moc_gasCalculator.cpp"
