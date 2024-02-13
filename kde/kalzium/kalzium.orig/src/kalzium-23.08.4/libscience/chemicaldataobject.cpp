/*
    SPDX-FileCopyrightText: 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "chemicaldataobject.h"

#include "kalzium_libscience_debug.h"

#include <KUnitConversion/Converter>

class ChemicalDataObjectPrivate : public QSharedData
{
public:
    ChemicalDataObjectPrivate();
    ~ChemicalDataObjectPrivate();

    QVariant m_value;
    QVariant m_errorValue;
    ChemicalDataObject::BlueObelisk m_type;
    int m_unit;
};

//########################
ChemicalDataObjectPrivate::ChemicalDataObjectPrivate()
    : QSharedData()
{
}

ChemicalDataObjectPrivate::~ChemicalDataObjectPrivate() = default;
//##############

ChemicalDataObject::ChemicalDataObject(const QVariant &v, BlueObelisk type, const QVariant &errorValue)
    : d(new ChemicalDataObjectPrivate)
{
    d->m_value = v;
    d->m_errorValue = errorValue;
    d->m_type = type;
    d->m_unit = KUnitConversion::NoUnit;
}

ChemicalDataObject::ChemicalDataObject()
    : d(new ChemicalDataObjectPrivate)
{
    d->m_errorValue = QVariant();
    d->m_unit = KUnitConversion::NoUnit;
}

ChemicalDataObject::ChemicalDataObject(const ChemicalDataObject &other) = default;

ChemicalDataObject::~ChemicalDataObject() = default;

ChemicalDataObject &ChemicalDataObject::operator=(const ChemicalDataObject &other) = default;

bool ChemicalDataObject::operator==(const int v) const
{
    if (d->m_value.type() != QVariant::Int) {
        return false;
    }

    return d->m_value.toInt() == v;
}

bool ChemicalDataObject::operator==(const bool v) const
{
    if (d->m_value.type() != QVariant::Bool) {
        return false;
    }

    return d->m_value.toBool() == v;
}

bool ChemicalDataObject::operator==(const double v) const
{
    if (d->m_value.type() != QVariant::Double) {
        return false;
    }

    return d->m_value.toDouble() == v;
}

bool ChemicalDataObject::operator==(const QString &v) const
{
    if (d->m_value.type() != QVariant::String) {
        return false;
    }

    return d->m_value.toString() == v;
}

bool ChemicalDataObject::operator==(const ChemicalDataObject &other) const
{
    return d == other.d;
}

bool ChemicalDataObject::operator!=(const ChemicalDataObject &other) const
{
    return d != other.d;
}

QString ChemicalDataObject::valueAsString() const
{
    return d->m_value.toString();
}

ChemicalDataObject::BlueObelisk ChemicalDataObject::type() const
{
    return d->m_type;
}

QVariant ChemicalDataObject::value() const
{
    return d->m_value;
}

QVariant ChemicalDataObject::errorValue() const
{
    return d->m_errorValue;
}

void ChemicalDataObject::setUnit(int unit)
{
    d->m_unit = unit;
}

int ChemicalDataObject::unit() const
{
    return d->m_unit;
}

void ChemicalDataObject::setData(const QVariant &v)
{
    d->m_value = v;
}

void ChemicalDataObject::setErrorValue(const QVariant &v)
{
    d->m_errorValue = v;
}

void ChemicalDataObject::setType(BlueObelisk type)
{
    d->m_type = type;
}

void ChemicalDataObject::setType(int type)
{
    d->m_type = (ChemicalDataObject::BlueObelisk)type;
}

QString ChemicalDataObject::unitAsString() const
{
    return KUnitConversion::Converter().unit(KUnitConversion::UnitId(d->m_unit)).symbol();
}
