/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "concCalculator.h"


#include <KLocalizedString>

#include "kalziumutils.h"
#include "prefs.h"

using namespace KUnitConversion;

concCalculator::concCalculator(QWidget *parent)
    : QFrame(parent)
{
    ui.setupUi(this);

    /**************************************************************************/
    //                       concentration Calculator set up
    /**************************************************************************/

    // initialise the initially selected values
    init();

    // Connect signals with slots (when a change of selection in the UI takes place,
    // corresponding quantity should be updated in the class.)

    // Amount of solute changed
    connect(ui.amtSolute, SIGNAL(valueChanged(double)), this, SLOT(amtSoluteChanged()));
    connect(ui.amtSltType, SIGNAL(activated(int)), this, SLOT(amtSoluteTypeChanged()));
    connect(ui.amtSlt_unit, SIGNAL(activated(int)), this, SLOT(amtSoluteChanged()));
    // Molar mass and equivalent mass change for solvent
    connect(ui.molarMass, SIGNAL(valueChanged(double)), this, SLOT(molarMassChanged(double)));
    connect(ui.eqtMass, SIGNAL(valueChanged(double)), this, SLOT(eqtMassChanged(double)));
    // Density change for solute
    connect(ui.densitySolute, SIGNAL(valueChanged(double)), this, SLOT(densitySoluteChanged()));
    connect(ui.densSlt_unit, SIGNAL(activated(int)), this, SLOT(densitySoluteChanged()));
    // Amount of solvent changed
    connect(ui.amtSolvent, SIGNAL(valueChanged(double)), this, SLOT(amtSolventChanged()));
    connect(ui.amtSlvtType, SIGNAL(activated(int)), this, SLOT(amtSolventTypeChanged()));
    connect(ui.amtSlvt_unit, SIGNAL(activated(int)), this, SLOT(amtSolventChanged()));
    // Molar mass change for solvent
    connect(ui.molarMassSolvent, SIGNAL(valueChanged(double)), this, SLOT(molarMassSolventChanged(double)));
    // Density changed
    connect(ui.densitySolvent, SIGNAL(valueChanged(double)), this, SLOT(densitySolventChanged()));
    connect(ui.densSlvt_unit, SIGNAL(activated(int)), this, SLOT(densitySolventChanged()));
    // concentration change
    connect(ui.concentration, SIGNAL(valueChanged(double)), this, SLOT(concentrationChanged()));
    connect(ui.conc_unit, SIGNAL(activated(int)), this, SLOT(concentrationChanged()));
    // Mode change
    connect(ui.mode, SIGNAL(activated(int)), this, SLOT(setMode(int)));

    connect(ui.reset, &QAbstractButton::clicked, this, &concCalculator::init);

    /**************************************************************************/
    //              concentration Calculator setup complete
    /**************************************************************************/
    if (Prefs::soluteMass()) {
        ui.amtSltType->setCurrentIndex(0);
    }
    if (Prefs::solventVolume()) {
        ui.amtSlvtType->setCurrentIndex(0);
    }
}

concCalculator::~concCalculator() = default;

// Initialises values and GUI.
void concCalculator::init()
{
    error(RESET_CONC_MESSAGE);

    ui.amtSltType->addItems({"Mass", "Volume", "Moles"});
    ui.amtSlvtType->addItems({"Volume", "Mass", "Moles"});
    ui.densSlt_unit->addItems({"grams per liter"});
    ui.densSlvt_unit->addItems({"grams per liter"});
    ui.conc_unit->addItems({"molar", "Normal", "% (mass)", "% (volume)", "% (moles)"});

    amtSoluteTypeChanged();
    amtSolventTypeChanged();

    ui.amtSolute->setValue(117.0);
    ui.molarMass->setValue(58.5);
    ui.eqtMass->setValue(58.5);
    ui.densitySolute->setValue(2.7);
    ui.amtSolvent->setValue(1.0);
    ui.molarMassSolvent->setValue(18.0);
    ui.densitySolvent->setValue(1000.0);
    ui.concentration->setValue(2.0);

    ui.amtSltType->setCurrentIndex(0);
    ui.amtSlt_unit->setCurrentIndex(0);
    ui.densSlt_unit->setCurrentIndex(0);
    ui.amtSlvtType->setCurrentIndex(0);
    ui.amtSlvt_unit->setCurrentIndex(0);
    ui.densSlvt_unit->setCurrentIndex(0);
    ui.conc_unit->setCurrentIndex(0);

    // Initialise values
    m_amtSolute = Value(117.0, KUnitConversion::Gram);
    m_amtSolvent = Value(1.0, KUnitConversion::Liter);
    m_molarMass = 58.5;
    m_eqtMass = 58.5;
    m_molesSolute = 2.0;
    m_molesSolvent = 55.5;
    m_molarMassSolvent = 18.0;
    m_densitySolute = Value(2.7, KUnitConversion::GramPerMilliliter);
    m_concentration = 2.0;
    m_densitySolvent = Value(1000.0, KUnitConversion::GramPerLiter);
    // Initialisation of values done

    setMode(5);
}

// Calculates the amount of solute
void concCalculator::calculateAmtSolute()
{
    int type1 = ui.conc_unit->currentIndex();
    int type2 = ui.amtSltType->currentIndex();

    double molesSolute, eqtsSolute, massSolute, volSolute; // variables
    int mode = 0;
    /*
     * mode = 1 (molesSolute) mode = 2 eqtsSolute, mode = 3 mass, 4 volume
     */
    // Calculate the number of moles of the solute
    switch (type1) {
        // calculate the number of moles of solute
    case 0: // molarity specified
        molesSolute = m_concentration * volumeSolvent();
        mode = 1;
        break;
        // Calculate the number of equivalents of solute
    case 1: // Normality specified
        eqtsSolute = m_concentration * volumeSolvent();
        mode = 2;
        break;
        // Calculate the number of moles of solute
    case 2: // molality specified
        molesSolute = m_concentration * massSolvent() / 1000.0;
        mode = 1;
        break;
        // Calculate the mass of solute
    case 3: // mass percentage specified
        if (m_concentration >= 100.0) {
            error(PERCENTAGE);
        }
        massSolute = m_concentration / (100.0 - m_concentration) * massSolvent();
        mode = 3;
        break;
        // Calculate the volume of solute
    case 4: // volume percentage specified
        if (m_concentration >= 100.0) {
            error(PERCENTAGE);
        }
        volSolute = m_concentration / (100.0 - m_concentration) * volumeSolvent();
        mode = 4;
        break;
        // Calculate the moles of solute
    case 5: // mole percentage specified
        if (m_concentration >= 100.0) {
            error(PERCENTAGE);
        }
        molesSolute = m_concentration / (100.0 - m_concentration) * molesSolvent();
        mode = 1;
        break;
    default:
        break;
    }

    // We have the amount of solvent in some form (moles, equivalents, mass, volume etc)
    // Now we have to present it in the UI
    switch (type2) {
    case 0: // amount should be specified in terms of mass
        switch (mode) {
        case 1: // we should get mass from moles
            massSolute = molesSolute * m_molarMass;
            break;
        case 2: // we should obtain mass from number of equivalents
            massSolute = eqtsSolute * m_eqtMass;
            break;
        case 3: // we already know the mass of the solute
            break;
        case 4: // we should get the mass from volume
            massSolute = volSolute * densitySolute();
            break;
        }
        // update mass of solute
        m_amtSolute = Value(massSolute, KUnitConversion::Gram);
        m_amtSolute = m_amtSolute.convertTo(ui.amtSlt_unit->currentText());
        ui.amtSolute->setValue(m_amtSolute.number());
        break;

    case 1: // amount should be specified in terms of volume
        // validate density
        if (densitySolute() == 0) {
            error(DENSITY_ZERO);
            return;
        }
        switch (mode) {
        case 1: // we should get the volume from moles
            volSolute = molesSolute * m_molarMass / densitySolute();
            break;
        case 2: // we should get the volume from equivalents
            volSolute = eqtsSolute * m_eqtMass / densitySolute();
            break;
        case 3: // we should get the volume from mass
            volSolute = massSolute / densitySolute();
            break;
        case 4: // we already know the volume
            break;
        }
        // update volume of solute
        m_amtSolute = Value(volSolute, KUnitConversion::Liter);
        m_amtSolute = m_amtSolute.convertTo(ui.amtSlt_unit->currentText());
        ui.amtSolute->setValue(m_amtSolute.number());
        break;

    case 2: // amount should be specified in terms of moles
        switch (mode) {
        case 1: // we already know the moles of solute
            break;
        case 2: // we should obtain moles from equivalents (not possible)
            molesSolute = 0.0;
            break;
        case 3: // we should obtain moles from mass
            molesSolute = massSolute / m_molarMass;
            break;
        case 4: // we should obtain moles from volume
            molesSolute = volSolute * densitySolute() / m_molarMass;
            break;
        }
        // Update the number of moles
        m_molesSolute = molesSolute;
        ui.amtSolute->setValue(molesSolute);
        break;
    }
    return;
}

// Calculates the molar mass
void concCalculator::calculateMolarMass()
{
    // molarity / molality / mole fraction required
    int type = ui.conc_unit->currentIndex();
    int type2 = ui.amtSlvtType->currentIndex();
    double numMoles;
    switch (type) {
    case 0: // molarity specified
        // number of moles = volume * concentration
        numMoles = volumeSolvent() * m_concentration;
        break;
    case 1: // cannot be calculated (insufficient data)
        error(INSUFFICIENT_DATA_MOLE);
        return;
    case 2: // molality specified
        numMoles = massSolvent() / 1000.0 * m_concentration;
        break;
    case 3: // cannot be calculated (insufficient data)
    case 4:
        error(INSUFFICIENT_DATA_MOLE);
        return;
    case 5: // mole fraction specified
        numMoles = m_concentration / (100.0 - m_concentration) * molesSolvent();
        break;
    }

    if (type2 == 2) { // amount of solute is specified in moles, cannot calculate
        error(INSUFFICIENT_DATA_MOLES);
        return;
    } else {
        if (numMoles == 0.0) {
            error(MOLES_ZERO);
            return;
        }
        m_molarMass = massSolute() / numMoles;
        ui.molarMass->setValue(m_molarMass);
    }
}

// Calculates the equivalent mass
void concCalculator::calculateEqtMass()
{
    // Normality required
    int type = ui.conc_unit->currentIndex();
    int type2 = ui.amtSltType->currentIndex();

    double numEqts;
    switch (type) {
        // Normality required
    case 0: // molarity not sufficient
        error(INSUFFICIENT_DATA_EQT);
        return;
    case 1: // normality specified
        numEqts = volumeSolvent() * m_concentration;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        error(INSUFFICIENT_DATA_EQT);
        return;
    }

    if (type2 == 2) { // Amount of solute specified in moles, cannot calculate
        error(INSUFFICIENT_DATA_MOLES);
    } else {
        if (numEqts == 0.0) {
            error(EQTS_ZERO);
            return;
        }
        m_eqtMass = massSolute() / numEqts;
        ui.eqtMass->setValue(m_eqtMass);
    }
    return;
}

// Calculates the calculate molar mass of the solvent
void concCalculator::calculateMolarMassSolvent()
{
    // molarity / molality / mole fraction required
    int type = ui.conc_unit->currentIndex();
    int type2 = ui.amtSlvtType->currentIndex();
    double numMoles;
    switch (type) {
    case 0: // molarity specified
    case 1: // normality specified
    case 2: // molality specified
    case 3: // mass fraction specified
    case 4: // volume fraction specified
        error(INSUFFICIENT_DATA_SOLVENT);
        return; // cannot be calculated (insufficient data)
    case 5: // mole fraction specified
        numMoles = (100.0 - m_concentration) / m_concentration * molesSolute();
        break;
    }

    if (type2 == 2) { // amount specified in moles
        error(INSUFFICIENT_DATA_MOLES);
    } else {
        m_molarMassSolvent = massSolvent() / numMoles;
        ui.molarMassSolvent->setValue(m_molarMassSolvent);
    }

    return;
}

// Calculates the amount of solvent
void concCalculator::calculateAmtSolvent()
{
    int type1 = ui.conc_unit->currentIndex();
    int type2 = ui.amtSlvtType->currentIndex();

    double moleSolvent, massSolvent, volSolvent;

    int mode = 0; // Indicates the mode in which we have calculated the amount of solvent
    /*
     * mode = 1 (molessolvent) mode = 2 eqtssolvent, mode = 3 mass, 4 volume
     */
    // Calculate the number of moles of the solvent
    if (m_concentration == 0.0) {
        error(CONC_ZERO);
        return;
    }

    switch (type1) {
        // calculate the number of moles of solvent
    case 0: // molarity specified
        volSolvent = molesSolute() / m_concentration;
        mode = 3;
        break;
        // Calculate the number of equivalents of solvent
    case 1: // Normality specified
        volSolvent = eqtsSolute() / m_concentration;
        mode = 3;
        break;
        // Calculate the number of moles of solvent
    case 2: // molality specified
        massSolvent = molesSolute() * 1000.0 / m_concentration;
        mode = 2;
        break;
        // Calculate the mass of solvent
    case 3: // mass percentage specified
        massSolvent = (100.0 - m_concentration) / m_concentration;
        massSolvent *= massSolute();
        mode = 2;
        break;
        // Calculate the volume of solvent
    case 4: // volume percentage specified
        volSolvent = (100.0 - m_concentration) / m_concentration;
        volSolvent *= volumeSolute();
        mode = 3;
        break;
        // Calculate the moles of solvent
    case 5: // mole percentage specified
        moleSolvent = (100.0 - m_concentration) / m_concentration;
        moleSolvent *= molesSolute();
        mode = 1;
        break;
    default:
        break;
    }

    // We have the amount of solvent in some form (moles, equivalents, mass, volume etc)
    // Now we have to present it in the UI
    if (densitySolvent() == 0.0) {
        error(DENSITY_ZERO);
        return;
    }
    if (m_molarMassSolvent == 0.0) {
        error(MOLAR_SOLVENT_ZERO);
        return;
    }
    switch (type2) {
    case 0: // amount should be specified interms of volume
        switch (mode) {
        case 1: // obtain volume from moles
            volSolvent = moleSolvent * m_molarMassSolvent / densitySolvent();
            break;
        case 2: // obtain volume from mass
            volSolvent = massSolvent / densitySolvent();
            break;
        case 3: // volume already known
            break;
        }
        m_amtSolvent = Value(volSolvent, KUnitConversion::Liter);
        m_amtSolvent = m_amtSolvent.convertTo(ui.amtSlvt_unit->currentText());
        ui.amtSolvent->setValue(m_amtSolvent.number());
        break;

    case 1: // amount should be specified in terms of mass
        switch (mode) {
        case 1: // obtain mass from moles
            massSolvent = moleSolvent / m_molarMassSolvent;
            break;
        case 2: // mass already known
            break;
        case 3: // obtain mass from volume
            massSolvent = volSolvent / densitySolvent();
            break;
        }
        m_amtSolvent = Value(massSolvent, KUnitConversion::Gram);
        m_amtSolvent = m_amtSolvent.convertTo(ui.amtSlvt_unit->currentText());
        ui.amtSolvent->setValue(m_amtSolvent.number());
        break;

    case 2: // amount should be specified in terms of moles
        switch (mode) {
        case 1: // moles already known
            break;
        case 2: // obtain moles from mass
            moleSolvent = massSolvent / m_molarMassSolvent;
            break;
        case 3: // obtain moles from volume
            moleSolvent = volSolvent * densitySolvent() / m_molarMassSolvent;
            break;
        }
        m_molesSolvent = moleSolvent;
        ui.amtSolvent->setValue(moleSolvent);
        break;
    }
    return;
}

// calculates the concentration
void concCalculator::calculateConcentration()
{
    int type = ui.conc_unit->currentIndex();

    if (volumeSolvent() == 0.0) {
        error(SOLVENT_VOLUME_ZERO);
        return;
    }
    if (massSolvent() == 0.0) {
        error(SOLVENT_MASS_ZERO);
        return;
    }
    if (molesSolvent() == 0.0) {
        error(SOLVENT_MOLES_ZERO);
        return;
    }
    switch (type) {
    case 0: // molarity
        m_concentration = molesSolute() / volumeSolvent();
        break;
    case 1: // normality
        m_concentration = eqtsSolute() / volumeSolvent();
        break;
    case 2: // molality
        m_concentration = molesSolute() * 1000.0 / massSolvent();
        break;
    case 3: // mass fraction
        m_concentration = massSolute() / (massSolute() + massSolvent()) * 100.0;
        break;
    case 4: // volume fraction
        m_concentration = volumeSolute() / (volumeSolute() + volumeSolvent()) * 100.0;
        break;
    case 5: // mole fraction
        m_concentration = molesSolute() / (molesSolute() + molesSolvent()) * 100.0;
        break;
    default:
        m_concentration = 0;
        break;
    }
    ui.concentration->setValue(m_concentration);
    return;
}

double concCalculator::volumeSolvent()
{
    int type = ui.amtSlvtType->currentIndex();
    switch (type) {
    case 0: // If volume is specified, return it in liters
        return m_amtSolvent.convertTo(KUnitConversion::Liter).number();
    case 1: // If mass is specified, calculate volume and return it.
        return massSolvent() / densitySolvent();
    case 2: // If moles are specified, calculated volume and return it.
        return massSolvent() / densitySolvent();
    default:
        return 0;
    }
    Q_UNREACHABLE();
}

double concCalculator::molesSolvent()
{
    int type = ui.amtSlvtType->currentIndex();

    double moles;
    switch (type) {
    case 0:
        moles = massSolvent() / m_molarMassSolvent;
        break;
    case 1:
        moles = massSolvent() / m_molarMassSolvent;
        break;
    case 2:
        moles = m_molesSolvent;
        break;
    default:
        moles = 0;
        break;
    }
    return moles;
}
double concCalculator::massSolvent()
{
    int type = ui.amtSlvtType->currentIndex();
    double mass;
    switch (type) {
    case 0:
        mass = volumeSolvent() * densitySolvent();
        break;
    case 1:
        mass = m_amtSolvent.convertTo(KUnitConversion::Gram).number();
        break;
    case 2:
        mass = m_molesSolvent * m_molarMassSolvent;
        break;
    default:
        mass = 0;
        break;
    }
    return mass;
}

double concCalculator::densitySolvent()
{
    return (m_densitySolvent.convertTo(KUnitConversion::GramPerLiter).number());
}

double concCalculator::volumeSolute()
{
    int type = ui.amtSltType->currentIndex();
    double volume;
    switch (type) {
    case 0:
        volume = massSolute() / densitySolute();
        break;
    case 1:
        volume = m_amtSolute.convertTo(KUnitConversion::Liter).number();
        break;
    case 2:
        volume = massSolute() / densitySolute();
        break;
    default:
        volume = 0;
        break;
    }
    return volume;
}

double concCalculator::molesSolute()
{
    int type = ui.amtSltType->currentIndex();

    double moles;
    if (m_molarMass == 0.0) {
        error(MOLAR_MASS_ZERO);
        return 1.0;
    }
    switch (type) {
    case 0:
        moles = massSolute() / m_molarMass;
        break;
    case 1:
        moles = massSolute() / m_molarMass;
        break;
    case 2:
        moles = m_molesSolute;
        break;
    default:
        moles = 0;
        break;
    }
    return moles;
}

double concCalculator::eqtsSolute()
{
    int type = ui.amtSltType->currentIndex();
    double eqts;
    if (m_eqtMass == 0.0) {
        error(EQT_MASS_ZERO);
        return 1.0;
    }
    switch (type) {
    case 0:
        eqts = massSolute() / m_eqtMass;
        break;
    case 1:
        eqts = massSolute() / m_eqtMass;
        break;
    case 2:
        // Cannot be calculated
        error(INSUFFICIENT_DATA_MOLES);
        eqts = 1.0;
        break;
    default:
        eqts = 0;
        break;
    }
    return eqts;
}

double concCalculator::massSolute()
{
    int type = ui.amtSltType->currentIndex();
    double mass;
    switch (type) {
    case 0:
        mass = m_amtSolute.convertTo(KUnitConversion::Gram).number();
        break;
    case 1:
        mass = volumeSolute() * densitySolute();
        break;
    case 2:
        mass = m_molesSolute * m_molarMass;
        break;
    default:
        mass = 0;
        break;
    }
    return mass;
}

double concCalculator::densitySolute()
{
    return (m_densitySolute.convertTo(KUnitConversion::GramPerLiter).number());
}

// occurs when the type in which amount of solute is specified is changed
void concCalculator::amtSoluteTypeChanged()
{
    int type = ui.amtSltType->currentIndex();
    if (type == 0) {
        // amount of solute specified in terms of mass
        massUnitCombobox(ui.amtSlt_unit);

        m_amtSolute = Value(ui.amtSolute->value(), ui.amtSlt_unit->currentText());
    } else if (type == 1) {
        // amount of solute is specified in terms of volume
        volumeUnitCombobox(ui.amtSlt_unit);

        m_amtSolute = Value(ui.amtSolute->value(), ui.amtSlt_unit->currentText());
    } else { // amount of solute is specified in terms of moles
        m_molesSolute = ui.amtSolute->value();
        ui.amtSlt_unit->hide();
    }
    calculate();
}

void concCalculator::massUnitCombobox(QComboBox *comboBox)
{
    comboBox->show();

    QList<int> units;
    units << Gram << Milligram << Kilogram << Ton << Carat << Pound << Ounce << TroyOunce;
    KalziumUtils::populateUnitCombobox(comboBox, units);

    comboBox->setCurrentIndex(0);
}

void concCalculator::volumeUnitCombobox(QComboBox *comboBox)
{
    comboBox->show();

    QList<int> units;
    units << Liter << Milliliter << CubicMeter << CubicFoot << CubicInch << CubicMile << FluidOunce << Cup << GallonUS << PintImperial;
    KalziumUtils::populateUnitCombobox(comboBox, units);

    comboBox->setCurrentIndex(0);
}

void concCalculator::amtSoluteChanged()
{
    int type = ui.amtSltType->currentIndex();
    switch (type) {
    case 0:
    case 1:
        m_amtSolute = Value(ui.amtSolute->value(), ui.amtSlt_unit->currentText());
        break;
    case 2:
        m_molesSolute = ui.amtSolute->value();
        break;
    }
    calculate();
}
// occurs when the type in which amount of solvent is specified is changed
void concCalculator::amtSolventTypeChanged()
{
    int type = ui.amtSlvtType->currentIndex();
    if (type == 0) {
        // amount of solvent specified in terms of volume
        volumeUnitCombobox(ui.amtSlvt_unit);

        m_amtSolvent = Value(ui.amtSolvent->value(), ui.amtSlvt_unit->currentText());
    } else if (type == 1) {
        // amount of solvent is specified in terms of mass
        massUnitCombobox(ui.amtSlvt_unit);

        m_amtSolvent = Value(ui.amtSolvent->value(), ui.amtSlvt_unit->currentText());
    } else {
        ui.amtSlvt_unit->hide();
        m_molesSolvent = ui.amtSolvent->value();
    }
    calculate();
}

// Occurs when the amount of solute is changed
void concCalculator::amtSolventChanged()
{
    int type = ui.amtSlvtType->currentIndex();
    switch (type) { // amount of solvent specified in terms of volume
    case 0:
    case 1:
        m_amtSolvent = Value(ui.amtSolvent->value(), ui.amtSlvt_unit->currentText());
        break;
    case 2:
        m_molesSolvent = ui.amtSolvent->value();
        break;
    }
    calculate();
}
// occurs when the molar mass of solute is changed
void concCalculator::molarMassChanged(double value)
{
    m_molarMass = value;
    calculate();
}

// occurs when the equivalent mass of solute is changed
void concCalculator::eqtMassChanged(double value)
{
    m_eqtMass = value;
    calculate();
}

// occurs when the molar mass of solvent is changed
void concCalculator::molarMassSolventChanged(double value)
{
    m_molarMassSolvent = value;
    calculate();
}

// occurs when the number of moles is changed
void concCalculator::densitySoluteChanged()
{
    m_densitySolute = Value(ui.densitySolute->value(), ui.densSlt_unit->currentText());
    calculate();
}

// occurs when the density of solvent is changed
void concCalculator::densitySolventChanged()
{
    m_densitySolvent = Value(ui.densitySolvent->value(), ui.densSlvt_unit->currentText());
    calculate();
}

// occurs when the concentration is changed
void concCalculator::concentrationChanged()
{
    m_concentration = ui.concentration->value();
    calculate();
}

// occurs when mode is changed. eg Concentration to molarMass
void concCalculator::setMode(int mode)
{
    // If there is no change, return.
    if (m_mode == mode) {
        return;
    }

    // set all to writable
    ui.amtSolute->setReadOnly(false);
    ui.molarMass->setReadOnly(false);
    ui.eqtMass->setReadOnly(false);
    ui.amtSolvent->setReadOnly(false);
    ui.molarMassSolvent->setReadOnly(false);
    ui.concentration->setReadOnly(false);

    // set the value that should be calculated to readOnly
    switch (mode) {
    case AMT_SOLUTE: // Calculate the amount of solute
        ui.amtSolute->setReadOnly(true);
        break;
    case MOLAR_MASS: // Calculate the molar mass of solute
        ui.molarMass->setReadOnly(true);
        break;
    case EQT_MASS: // Calculate the equivalent mass of solute
        ui.eqtMass->setReadOnly(true);
        break;
    case AMT_SOLVENT: // Calculate the amount of solvent
        ui.amtSolvent->setReadOnly(true);
        break;
    case MOLAR_MASS_SOLVENT: // Calculate the molar mass of solvent
        ui.molarMassSolvent->setReadOnly(true);
        break;
    case CONCENTRATION: // Calculate the concentration of the solution
        ui.concentration->setReadOnly(true);
        break;
    }

    m_mode = mode;
    calculate();
}
// occurs when any quantity is changed
void concCalculator::calculate()
{
    error(RESET_CONC_MESSAGE);
    // Calculate the amount of solute
    switch (m_mode) {
    case AMT_SOLUTE: // Calculate the amount of solute
        if (ui.conc_unit->currentIndex() > 2 && ui.concentration->value() > 100) {
            error(PERCENTAGE);
            return;
        }
        calculateAmtSolute();
        break;
    case MOLAR_MASS: // Calculate the molar mass of solute
        calculateMolarMass();
        break;
    case EQT_MASS: // Calculate the equivalent mass of solute
        calculateEqtMass();
        break;
    case AMT_SOLVENT: // Calculate the amount of solvent
        calculateAmtSolvent();
        break;
    case MOLAR_MASS_SOLVENT: // Calculate the molar mass of solvent
        calculateMolarMassSolvent();
        break;
    case CONCENTRATION: // Calculate the concentration of the solution
        calculateConcentration();
        break;
    }
    return;
}

// This function puts the error messages on the screen depending on the mode.
void concCalculator::error(int mode)
{
    switch (mode) { // Depending on the mode, set the error messages.
    case RESET_CONC_MESSAGE:
        ui.error->setText(QLatin1String(""));
        break;
    case PERCENTAGE:
        ui.error->setText(i18n("Percentage should be less than 100.0, please enter a valid value."));
        break;
    case DENSITY_ZERO:
        ui.error->setText(i18n("Density cannot be zero, please enter a valid value."));
        break;
    case MASS_ZERO:
        ui.error->setText(i18n("Mass cannot be zero, please enter a valid value."));
        break;
    case VOLUME_ZERO:
        ui.error->setText(i18n("Volume cannot be zero, please enter a valid value."));
        break;
    case MOLES_ZERO:
        ui.error->setText(i18n("Number of moles cannot be zero, please enter a valid value."));
        break;
    case MOLAR_SOLVENT_ZERO:
        ui.error->setText(i18n("Molar mass of solvent is zero, please enter a valid value."));
        break;
    case EQTS_ZERO:
        ui.error->setText(i18n("Number of equivalents is zero. Cannot calculate equivalent mass."));
        break;
    case CONC_ZERO:
        ui.error->setText(i18n("Concentration is zero, please enter a valid value."));
        break;
    case SOLVENT_VOLUME_ZERO:
        ui.error->setText(i18n("The volume of the solvent cannot be zero."));
        break;
    case SOLVENT_MOLES_ZERO:
        ui.error->setText(i18n("The number of moles of the solvent cannot be zero."));
        break;
    case SOLVENT_MASS_ZERO:
        ui.error->setText(i18n("The mass of the solvent cannot be zero."));
        break;
    case INSUFFICIENT_DATA_EQT:
        ui.error->setText(i18n("Insufficient data to calculate the required value, please specify normality."));
        break;
    case INSUFFICIENT_DATA_MOLE:
        ui.error->setText(i18n("Insufficient data, specify molarity / mole fraction / molality to calculate."));
        break;
    case INSUFFICIENT_DATA_MOLES:
        ui.error->setText(i18n("The amount is specified in moles, cannot calculate molar/equivalent masses. Please specify mass/volume."));
        break;
    case INSUFFICIENT_DATA_SOLVENT:
        ui.error->setText(i18n("You can calculate the molar mass of solvent only if the mole fraction is specified."));
        break;
    case MOLAR_MASS_ZERO:
        ui.error->setText(i18n("Molar mass cannot be zero, please enter a valid value."));
        break;
    case EQT_MASS_ZERO:
        ui.error->setText(i18n("Equivalent mass cannot be zero, please enter a valid value."));
        break;
    default:
        break;
    }
}

#include "moc_concCalculator.cpp"
