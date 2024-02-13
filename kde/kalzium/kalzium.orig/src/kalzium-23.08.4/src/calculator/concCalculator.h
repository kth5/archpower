/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONCCALCULATOR_H
#define CONCCALCULATOR_H

#include "kalzium_debug.h"

#include <KUnitConversion/UnitCategory>

#include <kalziumdataobject.h>
#include <prefs.h>

#include "ui_concCalculator.h"

// This is required for the unit conversion library
using namespace KUnitConversion;

// Enumeration for type of error used in the error() function
enum ERROR_TYPE_CONC {
    RESET_CONC_MESSAGE = 0,
    PERCENTAGE,
    DENSITY_ZERO,
    MASS_ZERO,
    VOLUME_ZERO,
    MOLES_ZERO,
    MOLAR_MASS_ZERO,
    EQT_MASS_ZERO,
    MOLAR_SOLVENT_ZERO,
    EQTS_ZERO,
    CONC_ZERO,
    SOLVENT_VOLUME_ZERO,
    SOLVENT_MASS_ZERO,
    SOLVENT_MOLES_ZERO,
    INSUFFICIENT_DATA_EQT,
    INSUFFICIENT_DATA_MOLE,
    INSUFFICIENT_DATA_MOLES,
    INSUFFICIENT_DATA_SOLVENT
};

// enumeration for the mode of calculation in the setMode(int) function
enum MODE_CALCULATION_CONC { AMT_SOLUTE = 0, MOLAR_MASS, EQT_MASS, AMT_SOLVENT, MOLAR_MASS_SOLVENT, CONCENTRATION };

/*
 * This class implements the concentration calculator. This widget performs basic
 * calculations like calculation of molarity, mass percentages etc.
 *
 * @author Kashyap R Puranik
 */
class concCalculator : public QFrame
{
    Q_OBJECT

public:
    /*
     * The constructor and destructor for the class
     */
    explicit concCalculator(QWidget *parent = nullptr);
    ~concCalculator() override;

public Q_SLOTS:
    // Sub-routines involved in calculations of the unit

    /// Calculates the amount of solute
    void calculateAmtSolute();

    /// Calculates the amount of solvent
    void calculateAmtSolvent();

    /// Calculates the molar mass
    void calculateMolarMass();

    /// Calculates the equivalent mass
    void calculateEqtMass();

    /// Calculates the calculate molar mass of the solvent
    void calculateMolarMassSolvent();

    /// calculates the concentration
    void calculateConcentration();

    // Functions (slots) that occur on changing a value
    // Sub routines which act as quantity change event handlers

    /// occurs when the amount of solute is changed
    void amtSoluteChanged();

    /// occurs when the type in which amount of solute is specified is changed
    void amtSoluteTypeChanged();

    /// occurs when the amount of solvent is changed
    void amtSolventChanged();

    /// occurs when the type in which amount of solvent is specified is changed
    void amtSolventTypeChanged();

    /// occurs when the molar mass of solute is changed
    void molarMassChanged(double);

    /// occurs when the equivalent mass of solute is changed
    void eqtMassChanged(double);

    /// occurs when the molar mass of solvent is changed
    void molarMassSolventChanged(double);

    /// occurs when the number of moles is changed
    void densitySoluteChanged();

    /// occurs when the density of solvent is changed
    void densitySolventChanged();

    /// occurs when the concentration is changed
    void concentrationChanged();

    /// occurs when any quantity is changed
    void calculate();

    /// returns volume of solvent in liters
    double volumeSolvent();

    /// returns mass of solvent in grams
    double massSolvent();

    /// returns number of moles of solvent
    double molesSolvent();

    /// returns density of solvent in grams per liter
    double densitySolvent();

    /// returns volume of solute in liters
    double volumeSolute();

    /// returns mass of solute in grams
    double massSolute();

    /// returns the number of moles of solute
    double molesSolute();

    /// returns the number of equivalents of solute
    double eqtsSolute();

    /// returns density of solute in grams per liter
    double densitySolute();

    /// Fills a Combobox with vulumina units
    void volumeUnitCombobox(QComboBox *comboBox);

    /// Fills a Combobox with mass units
    void massUnitCombobox(QComboBox *comboBox);

    /// Performs initialisation of the class.
    void init();

    /*
     * This function is called when an error occurs
     * @param mode indicates the mode of error
     * Refer ERROR_MODE_CONC for various modes
     */
    void error(int);

    /*
     * This function is called when the mode is changed
     * @param indicates the mode of calculation.
     * Refer MODE_CALCULATION_CONC for various modes
     */
    void setMode(int);

private:
    Ui::concCalculator ui; // The user interface

    Value m_amtSolute; // amount of solute
    Value m_amtSolvent; // amount of solvent
    double m_molesSolute; // amount of solute in moles
    double m_molesSolvent; // amount of solvent in moles
    double m_molarMass; // molar mass of solute
    double m_eqtMass; // equivalent mass of solute
    double m_molarMassSolvent; // molar mass of solvent
    Value m_densitySolute; // density of solute
    Value m_densitySolvent; // density of the solvent
    double m_concentration; // concentration of the solution

    int m_mode; // specifies the mode of calculation
};

#endif // CONCCALCULATOR_H
