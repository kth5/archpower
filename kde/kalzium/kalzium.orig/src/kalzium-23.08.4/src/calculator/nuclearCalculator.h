/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NUCLEARCALCULATOR_H
#define NUCLEARCALCULATOR_H

#include "kalzium_debug.h"

#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>

#include "ui_nuclearCalculator.h"
#include <element.h>
#include <isotope.h>
#include <kalziumdataobject.h>
#include <prefs.h>

// This is required for the unit conversion
using namespace KUnitConversion;

// This is the enumeration for the error type required in the error(int mode) function
enum ERROR_MODE_NUKE { RESET_NUKE_MESSAGE = 0, INIT_AMT_ZERO, FINAL_AMT_ZERO, HALFLIFE_ZERO, FINAL_AMT_GREATER };

// This is the enumeration for the mode of calculation in the nuclear calculator
enum MODE_CALCULATION_NUKE { INIT_AMT = 0, FINAL_AMT, TIME };

/*
 * This class implements the nuclear calculator which calculates the amount of substance,
 * remaining after a given time and given initial amount and so on after a radio-active decay.
 *
 * @author Kashyap R Puranik
 */
class nuclearCalculator : public QFrame
{
    Q_OBJECT

public:
    explicit nuclearCalculator(QWidget *parent = nullptr);
    ~nuclearCalculator() override;

public Q_SLOTS:
    /// Calculates the initial amount and updates the UI
    void calculateInitAmount();

    /// Calculates the final amount and updates the UI
    void calculateFinalAmount();

    /// Calculates the time required and updates the UI
    void calculateTime();

    /// This function is called when the element is changed
    void elementChanged(int index);

    /// This function is called when the isotope is changed
    void isotopeChanged(int index);

    /// This function is called when the halfLife is changed
    void halfLifeChanged();

    /// This function is called when any quantity is changed
    void calculate();

    /// This function is called when the initial amount is changed in the UI
    void initAmtChanged();

    /// This function is called when the final amount is changed in the UI
    void finalAmtChanged();

    /// This function is called when the time is changed in the UI
    void timeChanged();

    /// Fills a Combobox with vulumina units
    void timeUnitCombobox(QComboBox *comboBox);

    /// Fills a Combobox with mass units
    void massUnitCombobox(QComboBox *comboBox);

    /// Fetch the active unit id (KUnitConversion) from the combobox
    KUnitConversion::UnitId getUnitIdFromCombobox(QComboBox *comboBox);

    /*
     * This function is called when the slider in the ui is moved
     * @param x, is 10 times the number of halfLives
     * The slider is used to change the halfLife
     */
    void sliderMoved(int x);

    /*
     * This function is called when the mode is changed
     * @param indicates the mode of calculation.
     * Refer MODE_CALCULATION_NUKE for various modes
     */
    void setMode(int mode);

    /// This function is called during initialisation
    void init();

    /*
     * This function is called when an error occurs
     * @param mode indicates the mode of error
     * Refer ERROR_MODE_NUKE for various modes
     */
    void error(int mode);

private:
    Ui::nuclearCalculator ui; // The user interface

    Element m_element; // Current element
    Isotope m_isotope; // current isotope

    Value m_halfLife; // The halfLife
    Value m_initAmount; // initial amount present
    Value m_finalAmount; // amount after time
    Value m_time; // the time involved in calculation
    double m_mass; // the atomic mass of the isotope

    int m_mode; // the mode of calculation
};

#endif // NUCLEARCALCULATOR_H
