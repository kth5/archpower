/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "nuclearCalculator.h"

#include <cmath>

#include <KLocalizedString>

#include "kalziumutils.h"
#include "prefs.h"

using namespace KUnitConversion;

nuclearCalculator::nuclearCalculator(QWidget *parent)
    : QFrame(parent)
{
    ui.setupUi(this);

    /**************************************************************************/
    //                       Nuclear Calculator set up                        //
    /**************************************************************************/
    KalziumDataObject *kdo = KalziumDataObject::instance();

    // add all element names to the comboBox in the user interface
    foreach (Element *e, kdo->ElementList) {
        ui.element->addItem(e->dataAsString(ChemicalDataObject::name));
    }
    /// FIXME
    /* The last three elemenents will be removed because information is not available
       and causes the program to crash when selected. */
    int count = ui.element->count();
    ui.element->removeItem(count - 1);
    ui.element->removeItem(count - 2);
    ui.element->removeItem(count - 3);

    // initialise data
    init();
    // Connect signals with slots
    connect(ui.element, SIGNAL(activated(int)), this, SLOT(elementChanged(int)));
    connect(ui.isotope, SIGNAL(activated(int)), this, SLOT(isotopeChanged(int)));
    connect(ui.halfLife, SIGNAL(valueChanged(double)), this, SLOT(halfLifeChanged()));
    connect(ui.halfLife_unit, SIGNAL(activated(int)), this, SLOT(halfLifeChanged()));
    connect(ui.initAmt, SIGNAL(valueChanged(double)), this, SLOT(initAmtChanged()));
    connect(ui.initAmt_unit, SIGNAL(activated(int)), this, SLOT(initAmtChanged()));
    connect(ui.initAmtType, SIGNAL(activated(int)), this, SLOT(initAmtChanged()));
    connect(ui.finalAmt, SIGNAL(valueChanged(double)), this, SLOT(finalAmtChanged()));
    connect(ui.finalAmt_unit, SIGNAL(activated(int)), this, SLOT(finalAmtChanged()));
    connect(ui.finalAmtType, SIGNAL(activated(int)), this, SLOT(finalAmtChanged()));
    connect(ui.time, SIGNAL(valueChanged(double)), this, SLOT(timeChanged()));
    connect(ui.time_unit, SIGNAL(activated(int)), this, SLOT(timeChanged()));
    connect(ui.slider, &QAbstractSlider::valueChanged, this, &nuclearCalculator::sliderMoved);
    connect(ui.mode, SIGNAL(activated(int)), this, SLOT(setMode(int)));
    connect(ui.reset, &QAbstractButton::clicked, this, &nuclearCalculator::init);

    /**************************************************************************/
    // Nuclear Calculator setup complete
    /**************************************************************************/

    if (Prefs::mass()) {
        ui.initAmtType->hide();
        ui.finalAmtType->hide();
    }
}

nuclearCalculator::~nuclearCalculator() = default;

// The function that initialises data
void nuclearCalculator::init()
{
    const int ISOTOPE_NUM = 22;
    // Add all isotope names of Uranium (by default)to the isotope comboBox
    const QList<Isotope *> list = KalziumDataObject::instance()->isotopes(92);
    QString isotope;

    ui.isotope->clear();
    for (Isotope *i : list) {
        isotope.setNum(i->mass());
        ui.isotope->addItem(isotope);
    }

    // initialise the data, initially selected values (Uranium, 92, 238)
    ui.element->setCurrentIndex(91);
    ui.isotope->setCurrentIndex(ISOTOPE_NUM);
    ui.halfLife->setValue(list.at(ISOTOPE_NUM)->halflife());
    ui.initAmt->setValue(6.0);
    ui.finalAmt->setValue(3.0);
    ui.time->setValue(list.at(ISOTOPE_NUM)->halflife());

    timeUnitCombobox(ui.halfLife_unit);

    ui.initAmtType->setCurrentIndex(0);

    ui.finalAmtType->setCurrentIndex(0);

    massUnitCombobox(ui.initAmt_unit);

    massUnitCombobox(ui.finalAmt_unit);

    timeUnitCombobox(ui.time_unit);

    QString tempStr;
    tempStr.setNum(list.at(ISOTOPE_NUM)->mass());
    ui.mass->setText(tempStr);

    // Setup of the UI done
    // Initialise values
    m_initAmount = Value(6.0, KUnitConversion::Gram);
    m_finalAmount = Value(3.0, KUnitConversion::Gram);
    m_mass = list.at(ISOTOPE_NUM)->mass();
    m_time = Value(list.at(ISOTOPE_NUM)->halflife(), KUnitConversion::Year);
    m_halfLife = Value(list.at(ISOTOPE_NUM)->halflife(), KUnitConversion::Year);

    m_element = *KalziumDataObject::instance()->element(92);
    m_isotope = *list.at(ISOTOPE_NUM);

    setMode(2);
}

void nuclearCalculator::massUnitCombobox(QComboBox *comboBox)
{
    QList<int> units;
    units << Gram << Milligram << Kilogram << Ton << Carat << Pound << Ounce << TroyOunce;
    KalziumUtils::populateUnitCombobox(comboBox, units);

    comboBox->setCurrentIndex(0);
}

void nuclearCalculator::timeUnitCombobox(QComboBox *comboBox)
{
    QList<int> units;
    units << KUnitConversion::Year << KUnitConversion::Week << KUnitConversion::Day << KUnitConversion::Hour << KUnitConversion::Minute
          << KUnitConversion::Second;

    KalziumUtils::populateUnitCombobox(comboBox, units);

    comboBox->setCurrentIndex(0);
}

// This function is executed when the element is changed
void nuclearCalculator::elementChanged(int index)
{
    // set the newly chosen element
    m_element = *KalziumDataObject::instance()->element(index + 1);

    // Add all isotope names of Uranium (by default) to the isotope comboBox
    const QList<Isotope *> list = KalziumDataObject::instance()->isotopes(index + 1);
    QString isotope; // A temporary string
    ui.isotope->clear(); // Clear the contents of the combo box

    // update the combobox with isotopes of the new element
    for (Isotope *i : list) {
        isotope.setNum(i->mass());
        ui.isotope->addItem(isotope);
    }

    // Set the halfLife to that of the first isotope of the element.
    ui.halfLife->setValue(list.at(0)->halflife());
    // Recalculate and update
    calculate();
}

// This function is executed when the isotope is changed
void nuclearCalculator::isotopeChanged(int index)
{
    // update the nuclear Calculator
    int elementNumber = ui.element->currentIndex() + 1;
    QList<Isotope *> list = KalziumDataObject::instance()->isotopes(elementNumber);
    m_isotope = *list.at(index);

    // get the halfLife of the new isotope
    double halfLife = list.at(index)->halflife();
    m_mass = list.at(index)->mass();

    // A string in isotope for searching the right unit
    int halfLifeUnit = (list.at(index)->halflifeUnit().operator==("y")) ? KUnitConversion::Year : KUnitConversion::Second;

    QString tempStr;
    tempStr.setNum(m_mass);
    ui.mass->setText(tempStr);
    // Update the UI with the halfLife value
    ui.halfLife->setValue(halfLife);
    int x = ui.halfLife_unit->findData(halfLifeUnit);
    if (x >= 0) {
        ui.halfLife_unit->setCurrentIndex(x);
    }
    m_halfLife = Value(halfLife, KUnitConversion::UnitId(halfLifeUnit));
    // Recalculate and update
    calculate();
}

// This function is executed when the halfLife is changed
void nuclearCalculator::halfLifeChanged()
{
    // update the halfLife value
    m_halfLife = Value(ui.halfLife->value(), getUnitIdFromCombobox(ui.halfLife_unit));
    // recalculate the required
    calculate();
}

KUnitConversion::UnitId nuclearCalculator::getUnitIdFromCombobox(QComboBox *comboBox)
{
    return KUnitConversion::UnitId(comboBox->itemData(comboBox->currentIndex()).toInt());
}

void nuclearCalculator::initAmtChanged()
{
    // If quantity is specified in terms of mass, quantity <- (mass, unit)
    if (ui.initAmtType->currentIndex() == 0) {
        ui.initAmt_unit->show();
        m_initAmount = Value(ui.initAmt->value(), getUnitIdFromCombobox(ui.initAmt_unit));
    } else { // If quantity is specified in terms of moles quantity <- (moles * atomicMass, unit)
        ui.initAmt_unit->hide();
        m_initAmount = Value(((ui.initAmt->value()) * m_mass), getUnitIdFromCombobox(ui.initAmt_unit));
    }

    calculate();
}

void nuclearCalculator::finalAmtChanged()
{
    // If quantity is specified in terms of mass, quantity <- (mass, unit)
    if (ui.finalAmtType->currentIndex() == 0) {
        ui.finalAmt_unit->show();
        m_finalAmount = Value(ui.finalAmt->value(), getUnitIdFromCombobox(ui.finalAmt_unit));
    } else { // If quantity is specified in terms of moles quantity <- (moles * atomicMass, unit)
        ui.finalAmt_unit->hide();
        m_finalAmount = Value(((ui.finalAmt->value()) * m_mass), getUnitIdFromCombobox(ui.finalAmt_unit));
    }

    calculate();
}

void nuclearCalculator::sliderMoved(int numHlives)
{
    double num = numHlives / 10.0;
    m_time = Value(num * m_halfLife.number(), m_halfLife.unit());

    ui.time->setValue(m_time.number());
    ui.time_unit->setCurrentIndex(ui.halfLife_unit->currentIndex());
    ui.numHalfLives->setText(m_time.toString());
}

void nuclearCalculator::timeChanged()
{
    m_time = Value(ui.time->value(), getUnitIdFromCombobox(ui.time_unit));

    calculate();
}

void nuclearCalculator::setMode(int mode)
{
    m_mode = mode;

    ui.initAmt->setReadOnly(false);
    ui.finalAmt->setReadOnly(false);
    ui.time->setReadOnly(false);

    // set the quantity that should be calculated to readOnly
    switch (mode) {
    case INIT_AMT:
        ui.initAmt->setReadOnly(true);
        ui.time_in_halfLives->show();
        break;
    case FINAL_AMT:
        ui.finalAmt->setReadOnly(true);
        ui.time_in_halfLives->show();
        break;
    case TIME:
        ui.time->setReadOnly(true);
        ui.time_in_halfLives->hide();
        break;
    }

    calculate();
}
void nuclearCalculator::calculate()
{
    error(RESET_NUKE_MESSAGE);
    // Validate the values involved in calculation
    if (m_halfLife.number() == 0.0) {
        error(HALFLIFE_ZERO);
        return;
    }

    switch (m_mode) {
    case 0: // calculate initial amount before given time
        if (ui.finalAmt->value() == 0.0) {
            error(FINAL_AMT_ZERO);
            return;
        }
        calculateInitAmount();
        break;
    case 1: // calculate final amount after given time
        if (ui.initAmt->value() == 0.0) {
            error(INIT_AMT_ZERO);
            return;
        }
        calculateFinalAmount();
        break;
    case 2: // final amount greater than initial
        if (m_finalAmount.number() > m_initAmount.convertTo(m_finalAmount.unit()).number()) {
            error(FINAL_AMT_GREATER);
            return;
        }
        // one of the amounts is 0.0
        if (ui.finalAmt->value() == 0.0) {
            error(FINAL_AMT_ZERO);
            return;
        } else if (ui.initAmt->value() == 0.0) {
            error(INIT_AMT_ZERO);
            return;
        }
        calculateTime();
        break;
    }
}

void nuclearCalculator::calculateInitAmount()
{
    error(RESET_NUKE_MESSAGE);

    // If no time has elapsed, initial and final amounts are the same
    if (m_time.number() == 0.0) {
        m_initAmount = m_finalAmount.convertTo(m_initAmount.unit());
        ui.initAmt->setValue(m_initAmount.number());
        return;
    }
    // Calculate the number of halfLives that have elapsed
    double ratio = m_time.convertTo(m_halfLife.unit()).number() / m_halfLife.number();
    // find out the initial amount
    m_initAmount = Value(m_initAmount.number() * pow(2.0, ratio), m_initAmount.unit());
    // Convert into the required units
    m_initAmount = m_initAmount.convertTo(getUnitIdFromCombobox(ui.initAmt_unit));
    ui.initAmt->setValue(m_initAmount.number());
}

void nuclearCalculator::calculateFinalAmount()
{
    // If no time has elapsed, initial and final amounts are the same
    if (m_time.number() == 0.0) {
        m_finalAmount = m_initAmount.convertTo(m_finalAmount.unit());
        ui.finalAmt->setValue(m_finalAmount.number());
        return;
    }
    // Calculate the number of halfLives that have elapsed
    double ratio = m_time.convertTo(m_halfLife.unit()).number() / m_halfLife.number();
    // Calculate the final amount
    m_finalAmount = Value(m_finalAmount.number() / pow(2.0, ratio), m_initAmount.unit());
    // Convert into the required units
    m_finalAmount = m_finalAmount.convertTo(getUnitIdFromCombobox(ui.finalAmt_unit));
    ui.finalAmt->setValue(m_finalAmount.number());
}

void nuclearCalculator::calculateTime()
{
    // If initial and final masses are the same (both units and value)
    // the time is also 0
    if (m_initAmount.number() == m_finalAmount.number() && m_initAmount.unit() == m_finalAmount.unit()) {
        m_time = Value(0.0, m_time.unit());
        ui.time->setValue(0.0);
        return;
    }

    // calculate the ratio of final to initial masses
    double ratio = m_initAmount.convertTo(m_finalAmount.unit()).number() / m_finalAmount.number();

    // The number of halfLives (log 2 (x) = log x / log 2)
    double numHalfLives = log(ratio) / log(2.0);
    double time_value = numHalfLives * m_halfLife.number();
    // Calculate the total time taken
    Value time = Value(time_value, m_halfLife.unit());
    m_time = time.convertTo(getUnitIdFromCombobox(ui.time_unit));
    ui.time->setValue(m_time.number());

    return;
}

void nuclearCalculator::error(int mode)
{
    switch (mode) { // Depending on the mode, set the error messages.
    case RESET_NUKE_MESSAGE:
        ui.error->setText(QLatin1String(""));
        break;
    case INIT_AMT_ZERO:
        ui.error->setText(i18n("Initial amount cannot be zero."));
        break;
    case FINAL_AMT_ZERO:
        ui.error->setText(i18n("Final amount cannot be zero."));
        break;
    case HALFLIFE_ZERO:
        ui.error->setText(i18n("Time is zero, please enter a valid value."));
        break;
    case FINAL_AMT_GREATER:
        ui.error->setText(i18n("The final amount is greater than the initial amount."));
        break;
    }
}

#include "moc_nuclearCalculator.cpp"
