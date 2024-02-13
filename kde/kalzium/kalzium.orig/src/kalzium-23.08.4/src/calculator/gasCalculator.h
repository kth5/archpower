/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GASCALCULATOR_H
#define GASCALCULATOR_H

#include "kalzium_debug.h"

#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>

#include <kalziumdataobject.h>

#include "ui_gasCalculator.h"

// The universal Gas constant is defined here.
const double R = 0.08206;

using namespace KUnitConversion;

/// This is the enumeration for the error type required in the error(int mode) function
enum ERROR_TYPE_GAS { RESET_GAS_MESSAGE = 0, VOL_ZERO, GAS_MOLAR_MASS_ZERO };

/// This is the enumeration for the mode of calculation for the gas calculator
enum MODE_CALCULATION_GAS { MOLES = 0, PRESSURE, TEMPERATURE, VOLUME };

/**
 * This class implements the gas calculator. It performs basic calculations like
 * calculation of volume given pressure, temperature, amount etc. and so on.
 *
 *   Van der Val's gas equation
 *   ( P + n^2 a / V^2) ( V - nb ) = nRT
 *
 *   where P - pressure
 *        V - Volume
 *        n - number of moles
 *        R - Universal gas constant
 *        T - temperature
 *
 *        a,b - Van der Val's constants
 *
 * @author Kashyap R Puranik
 **/
class gasCalculator : public QWidget
{
    Q_OBJECT

public:
    explicit gasCalculator(QWidget *parent = nullptr);
    ~gasCalculator() override;

public Q_SLOTS:
    /// Calculates the Pressure and updates the UI
    void calculatePressure();

    /// Calculates the Volume and updates the UI
    void calculateVol();

    /// Calculates the Temperature and updates the UI
    void calculateTemp();

    /// Calculates the number of moles and updates the UI
    void calculateMoles();

    /// Calculates the mass of substance and updates the UI
    void calculateMass();

    /// Calculates the molar mass of the substance and updates the UI
    void calculateMolarMass();

    /// Functions (slots) that occur on changing a value
    /// This function is called when the volume is changed
    void volChanged();

    /// This function is called when the temperature is changed
    void tempChanged();

    /// This function is called when the pressure is changed
    void pressureChanged();

    /// This function is called when the mass is changed
    void massChanged();

    /**
     * This function is called when the number of moles is changed
     * @param value is the number of moles
     **/
    void molesChanged(double value);

    /**
     * This function is called when the molar mass is changed
     * @param value is the molar mass
     **/
    void molarMassChanged(double value);

    /// This function is called when Vander Val's constant a is changed
    void Vand_aChanged();

    /// This function is called when Vander Val's constant b is changed
    void Vand_bChanged();

    /// This function is called when any quantity is changed
    void calculate();

    /**
     * This function is called when an error occurs
     * @param mode indicates the mode of error
     * Refer ERROR_MODE_GAS for various modes
     **/
    void error(int);

    /**
     * This function is called when the mode is changed
     * @param indicates the mode of calculation.
     * Refer MODE_CALCULATION_GAS for various modes
     **/
    void setMode(int);

    void init();

private:
    void setupUnitComboboxes();
    void populateUnitCombobox(QComboBox *comboBox, const QList<int> &unitList);

    int getCurrentUnitId(QComboBox *comboBox);

    Ui::gasCalculator ui;

    double m_moles;
    double m_molarMass;
    Value m_mass;
    Value m_temp;
    Value m_pressure;
    Value m_vol;
    Value m_Vand_b;
    double m_Vand_a;

    int m_mode;
};

#endif // GASCALCULATOR_H
