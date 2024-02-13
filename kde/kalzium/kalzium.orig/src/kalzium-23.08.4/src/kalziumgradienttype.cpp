/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumgradienttype.h"

#include "element.h"
#include "kalziumdataobject.h"
#include "prefs.h"

#include "kalzium_debug.h"
#include <QVariant>

#include <KLocalizedString>
#include <KUnitConversion/Converter>

#include <cmath>

KalziumGradientTypeFactory::KalziumGradientTypeFactory()
{
    m_gradients << KalziumSOMGradientType::instance();
    m_gradients << KalziumCovalentRadiusGradientType::instance();
    m_gradients << KalziumVanDerWaalsRadiusGradientType::instance();
    m_gradients << KalziumMassGradientType::instance();
    m_gradients << KalziumBoilingPointGradientType::instance();
    m_gradients << KalziumMeltingPointGradientType::instance();
    m_gradients << KalziumElectronegativityGradientType::instance();
    m_gradients << KalziumElectronaffinityGradientType::instance();
    m_gradients << KalziumDiscoverydateGradientType::instance();
    m_gradients << KalziumIonizationGradientType::instance();
}

KalziumGradientTypeFactory *KalziumGradientTypeFactory::instance()
{
    static KalziumGradientTypeFactory kttf;
    return &kttf;
}

KalziumGradientType *KalziumGradientTypeFactory::build(int id) const
{
    if ((id < 0) || (id >= m_gradients.count()))
        return nullptr;

    return m_gradients.at(id);
}

KalziumGradientType *KalziumGradientTypeFactory::build(const QByteArray &id) const
{
    for (int i = 0; i < m_gradients.count(); ++i) {
        if (m_gradients.at(i)->name() == id) {
            return m_gradients.at(i);
        }
    }

    return nullptr;
}

QStringList KalziumGradientTypeFactory::gradients() const
{
    QStringList l;
    for (int i = 0; i < m_gradients.count(); ++i) {
        l << m_gradients.at(i)->description();
    }
    return l;
}

KalziumGradientType::KalziumGradientType() = default;

KalziumGradientType::~KalziumGradientType() = default;

KalziumGradientType *KalziumGradientType::instance()
{
    return nullptr;
}

double KalziumGradientType::elementCoeff(int el) const
{
    double val = value(el);
    if (val == -1) {
        return val;
    }

    if (logarithmicGradient()) {
        double minVal = minValue();
        double maxVal = maxValue();

        // Fixing negative values for log calculation (no negative values allowed -> NaN)
        if (minVal < 0) {
            minVal = abs(minVal) + 1; // make sure it's bigger than 0
            maxVal += minVal;
            val += minVal;
            minVal = 0.01;
        }

        double result = (log(val) - log(minVal)) / (log(maxVal) - log(minVal));

        // now we perform a "gamma-correction" on the result. Indeed, logarithmic gradients
        // often have the problem that all high values have roughly the same color. Note that
        // as firstColor() is not necessarily black and secondColor() is not necessarily white,
        // this is not exactly a "gamma-correction" in the usual sense.
        const double gamma = 1.4;
        result = exp(gamma * log(result));
        return result;
    } else {
        return (val - minValue()) / (maxValue() - minValue());
    }
}

QColor KalziumGradientType::firstColor() const
{
    return Prefs::minColor();
}

QColor KalziumGradientType::secondColor() const
{
    return Prefs::maxColor();
}

QColor KalziumGradientType::notAvailableColor() const
{
    return Qt::lightGray;
}

QColor KalziumGradientType::calculateColor(const double coeff) const
{
    if ((coeff < 0.0) || (coeff > 1.0))
        return notAvailableColor();

    QColor color2 = secondColor();
    QColor color1 = firstColor();

    int red = static_cast<int>((color2.red() - color1.red()) * coeff + color1.red());
    int green = static_cast<int>((color2.green() - color1.green()) * coeff + color1.green());
    int blue = static_cast<int>((color2.blue() - color1.blue()) * coeff + color1.blue());

    return {red, green, blue};
}

KalziumCovalentRadiusGradientType *KalziumCovalentRadiusGradientType::instance()
{
    static KalziumCovalentRadiusGradientType kcrgt;
    return &kcrgt;
}

KalziumCovalentRadiusGradientType::KalziumCovalentRadiusGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumCovalentRadiusGradientType::name() const
{
    return "CovalentRadius";
}

QString KalziumCovalentRadiusGradientType::description() const
{
    return i18n("Covalent Radius");
}

double KalziumCovalentRadiusGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::radiusCovalent, Prefs::lengthUnit());
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumCovalentRadiusGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::lengthUnit());
}

int KalziumCovalentRadiusGradientType::decimals() const
{
    return 2;
}

double KalziumCovalentRadiusGradientType::minValue() const
{
    KUnitConversion::Value minValue(0.32, KUnitConversion::Angstrom);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::lengthUnit())).number();
}

double KalziumCovalentRadiusGradientType::maxValue() const
{
    KUnitConversion::Value minValue(2.25, KUnitConversion::Angstrom);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::lengthUnit())).number();
}

bool KalziumCovalentRadiusGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicCovalentRadiusGradient();
}

KalziumVanDerWaalsRadiusGradientType *KalziumVanDerWaalsRadiusGradientType::instance()
{
    static KalziumVanDerWaalsRadiusGradientType kvdwrgt;
    return &kvdwrgt;
}

KalziumVanDerWaalsRadiusGradientType::KalziumVanDerWaalsRadiusGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumVanDerWaalsRadiusGradientType::name() const
{
    return "KalziumVanDerWaalsRadiusGradientType";
}

QString KalziumVanDerWaalsRadiusGradientType::description() const
{
    return i18n("Van Der Waals");
}

double KalziumVanDerWaalsRadiusGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::radiusVDW, Prefs::lengthUnit());
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumVanDerWaalsRadiusGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::lengthUnit());
}

int KalziumVanDerWaalsRadiusGradientType::decimals() const
{
    return 1;
}

double KalziumVanDerWaalsRadiusGradientType::minValue() const
{
    KUnitConversion::Value minValue(1.2, KUnitConversion::Angstrom);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::lengthUnit())).number();
}

double KalziumVanDerWaalsRadiusGradientType::maxValue() const
{
    KUnitConversion::Value minValue(3.0, KUnitConversion::Angstrom);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::lengthUnit())).number();
}

bool KalziumVanDerWaalsRadiusGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicVanDerWaalsRadiusGradient();
}

KalziumMassGradientType *KalziumMassGradientType::instance()
{
    static KalziumMassGradientType kargt;
    return &kargt;
}

KalziumMassGradientType::KalziumMassGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumMassGradientType::name() const
{
    return "AtomicMass";
}

QString KalziumMassGradientType::description() const
{
    return i18n("Atomic Mass");
}

double KalziumMassGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::mass);
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumMassGradientType::unit() const
{
    return i18n("u");
}

int KalziumMassGradientType::decimals() const
{
    return 5;
}

double KalziumMassGradientType::minValue() const
{
    return 1.00794;
}

double KalziumMassGradientType::maxValue() const
{
    return 294.0;
}

bool KalziumMassGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicMassGradient();
}

KalziumBoilingPointGradientType *KalziumBoilingPointGradientType::instance()
{
    static KalziumBoilingPointGradientType kbpgt;
    return &kbpgt;
}

KalziumBoilingPointGradientType::KalziumBoilingPointGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumBoilingPointGradientType::name() const
{
    return "BoilingPoint";
}

QString KalziumBoilingPointGradientType::description() const
{
    return i18n("Boiling Point");
}

double KalziumBoilingPointGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::boilingpoint, Prefs::temperatureUnit());
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumBoilingPointGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::temperatureUnit());
}

int KalziumBoilingPointGradientType::decimals() const
{
    return 3;
}

double KalziumBoilingPointGradientType::minValue() const
{
    KUnitConversion::Value minValue(4.216, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

double KalziumBoilingPointGradientType::maxValue() const
{
    KUnitConversion::Value minValue(5870.0, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

bool KalziumBoilingPointGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicBoilingPointGradient();
}

KalziumMeltingPointGradientType *KalziumMeltingPointGradientType::instance()
{
    static KalziumMeltingPointGradientType kmpgt;
    return &kmpgt;
}

KalziumMeltingPointGradientType::KalziumMeltingPointGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumMeltingPointGradientType::name() const
{
    return "MeltingPoint";
}

QString KalziumMeltingPointGradientType::description() const
{
    return i18n("Melting Point");
}

double KalziumMeltingPointGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::meltingpoint, Prefs::temperatureUnit());
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumMeltingPointGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::temperatureUnit());
}

int KalziumMeltingPointGradientType::decimals() const
{
    return 2;
}

double KalziumMeltingPointGradientType::minValue() const
{
    KUnitConversion::Value minValue(0.94, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

double KalziumMeltingPointGradientType::maxValue() const
{
    KUnitConversion::Value minValue(3825.0, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

bool KalziumMeltingPointGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicMeltingPointGradient();
}

KalziumSOMGradientType *KalziumSOMGradientType::instance()
{
    static KalziumSOMGradientType kargt;
    return &kargt;
}

KalziumSOMGradientType::KalziumSOMGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumSOMGradientType::name() const
{
    return "SOM";
}

QString KalziumSOMGradientType::description() const
{
    return i18n("State of matter");
}

double KalziumSOMGradientType::value(int el) const
{
    Q_UNUSED(el);
    return -1000;
}

QString KalziumSOMGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::temperatureUnit());
    ;
}

int KalziumSOMGradientType::decimals() const
{
    return 2;
}

double KalziumSOMGradientType::minValue() const
{
    KUnitConversion::Value minValue(0.94, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

double KalziumSOMGradientType::maxValue() const
{
    KUnitConversion::Value minValue(5870.0, KUnitConversion::Kelvin);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::temperatureUnit())).number();
}

bool KalziumSOMGradientType::logarithmicGradient() const
{
    return true;
    //     return Prefs::logarithmicSOMGradient();
}

KalziumElectronegativityGradientType *KalziumElectronegativityGradientType::instance()
{
    static KalziumElectronegativityGradientType kegt;
    return &kegt;
}

KalziumElectronegativityGradientType::KalziumElectronegativityGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumElectronegativityGradientType::name() const
{
    return "Electronegativity";
}

QString KalziumElectronegativityGradientType::description() const
{
    return i18n("Electronegativity (Pauling)");
}

double KalziumElectronegativityGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::electronegativityPauling);
    if (v.type() != QVariant::Double)
        return -1;
    return v.toDouble();
}

QString KalziumElectronegativityGradientType::unit() const
{
    return {};
}

int KalziumElectronaffinityGradientType::decimals() const
{
    return 2;
}

double KalziumElectronegativityGradientType::minValue() const
{
    return 0.7;
}

double KalziumElectronegativityGradientType::maxValue() const
{
    return 3.98;
}

bool KalziumElectronegativityGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicElectronegativityGradient();
}

/// DISCOVERYDATE///

KalziumDiscoverydateGradientType *KalziumDiscoverydateGradientType::instance()
{
    static KalziumDiscoverydateGradientType kegt;
    return &kegt;
}

KalziumDiscoverydateGradientType::KalziumDiscoverydateGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumDiscoverydateGradientType::name() const
{
    return "Discoverydate";
}

QString KalziumDiscoverydateGradientType::description() const
{
    return i18n("Discovery date");
}

double KalziumDiscoverydateGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::date);
    if (v.value<int>() == 0)
        return -1;
    return v.toDouble();
}

QString KalziumDiscoverydateGradientType::unit() const
{
    return {};
}

int KalziumDiscoverydateGradientType::decimals() const
{
    return 0;
}

double KalziumDiscoverydateGradientType::minValue() const
{
    return 1669.0;
}

double KalziumDiscoverydateGradientType::maxValue() const
{
    return QDate::currentDate().year();
}

bool KalziumDiscoverydateGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicDiscoverydateGradient();
}

/// ELECTRONAFFINITY///

KalziumElectronaffinityGradientType *KalziumElectronaffinityGradientType::instance()
{
    static KalziumElectronaffinityGradientType kegt;
    return &kegt;
}

KalziumElectronaffinityGradientType::KalziumElectronaffinityGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumElectronaffinityGradientType::name() const
{
    return "Electronaffinity";
}

QString KalziumElectronaffinityGradientType::description() const
{
    return i18n("Electronaffinity");
}

double KalziumElectronaffinityGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::electronAffinity, Prefs::energiesUnit());
    return v.toDouble();
}

QString KalziumElectronaffinityGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::energiesUnit());
}

int KalziumElectronegativityGradientType::decimals() const
{
    return 2;
}

double KalziumElectronaffinityGradientType::minValue() const
{
    KUnitConversion::Value minValue(-0.07, KUnitConversion::Electronvolt);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::energiesUnit())).number();
}

double KalziumElectronaffinityGradientType::maxValue() const
{
    KUnitConversion::Value minValue(3.7, KUnitConversion::Electronvolt);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::energiesUnit())).number();
}

bool KalziumElectronaffinityGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicElectronaffinityGradient();
}

/// FIRST IONIZATINO///

KalziumIonizationGradientType *KalziumIonizationGradientType::instance()
{
    static KalziumIonizationGradientType kegt;
    return &kegt;
}

KalziumIonizationGradientType::KalziumIonizationGradientType()
    : KalziumGradientType()
{
}

QByteArray KalziumIonizationGradientType::name() const
{
    return "Ionization";
}

QString KalziumIonizationGradientType::description() const
{
    return i18n("First Ionization");
}

double KalziumIonizationGradientType::value(int el) const
{
    QVariant v = KalziumDataObject::instance()->element(el)->dataAsVariant(ChemicalDataObject::ionization, Prefs::energiesUnit());
    return v.toDouble();
}

QString KalziumIonizationGradientType::unit() const
{
    return KalziumDataObject::instance()->unitAsString(Prefs::energiesUnit());
}

int KalziumIonizationGradientType::decimals() const
{
    return 2;
}

double KalziumIonizationGradientType::minValue() const
{
    KUnitConversion::Value minValue(3.89, KUnitConversion::Electronvolt);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::energiesUnit())).number();
}

double KalziumIonizationGradientType::maxValue() const
{
    KUnitConversion::Value minValue(25.0, KUnitConversion::Electronvolt);
    return minValue.convertTo(KUnitConversion::UnitId(Prefs::energiesUnit())).number();
}

bool KalziumIonizationGradientType::logarithmicGradient() const
{
    return Prefs::logarithmicIonizationGradient();
}
