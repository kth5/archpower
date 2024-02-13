/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spectrum.h"

#include "element.h"

#include <KLocalizedString>
#include <KUnitConversion/Converter>

#include <cmath>

double Spectrum::minPeak()
{
    double value = m_peaklist.first()->wavelength;

    for (peak *p : std::as_const(m_peaklist)) {
        if (value > p->wavelength) {
            value = p->wavelength;
        }
    }
    return value;
}

double Spectrum::minPeak(const int unit)
{
    return KUnitConversion::Value(minPeak(), KUnitConversion::Angstrom).convertTo(KUnitConversion::UnitId(unit)).number();
}

double Spectrum::maxPeak()
{
    double value = m_peaklist.first()->wavelength;

    for (peak *p : std::as_const(m_peaklist)) {
        if (value < p->wavelength) {
            value = p->wavelength;
        }
    }

    return value;
}

double Spectrum::maxPeak(const int unit)
{
    return KUnitConversion::Value(maxPeak(), KUnitConversion::Angstrom).convertTo(KUnitConversion::UnitId(unit)).number();
}

Spectrum *Spectrum::adjustToWavelength(double min, double max)
{
    auto spec = new Spectrum();

    for (peak *p : std::as_const(m_peaklist)) {
        if (p->wavelength >= min || p->wavelength <= max) {
            spec->addPeak(p);
        }
    }

    return spec;
}

void Spectrum::adjustIntensities()
{
    int maxInt = 0;
    // find the highest intensity
    for (peak *p : std::as_const(m_peaklist)) {
        if (p->intensity > maxInt) {
            maxInt = p->intensity;
        }
    }

    // check if an adjustment is needed or not
    if (maxInt == 1000) {
        return;
    }

    // now adjust the intensities.
    for (peak *p : std::as_const(m_peaklist)) {
        double newInt = p->intensity * 1000 / maxInt;

        p->intensity = (int)qRound(newInt);
    }
}

QList<double> Spectrum::wavelengths(double min, double max)
{
    QList<double> list;

    for (peak *p : std::as_const(m_peaklist)) {
        if (p->wavelength >= min || p->wavelength <= max) {
            list.append(p->wavelength);
        }
    }

    return list;
}

int Spectrum::parentElementNumber() const
{
    return m_parentElementNumber;
}

Spectrum::~Spectrum()
{
    qDeleteAll(m_peaklist);
}

Spectrum::Spectrum()
{
    // FIXME this shouldn't be hardcoded
    m_parentElementNumber = 16;
}

double Spectrum::peak::wavelengthToUnit(const int unit)
{
    return KUnitConversion::Value(wavelength, KUnitConversion::Angstrom).convertTo(KUnitConversion::UnitId(unit)).number();
}
