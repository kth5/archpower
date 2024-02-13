/*
    SPDX-FileCopyrightText: 2003, 2004, 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "element.h"


#include <KUnitConversion/Converter>

Element::Element() = default;

QVariant Element::dataAsVariant(ChemicalDataObject::BlueObelisk type) const
{
    for (const ChemicalDataObject &o : std::as_const(dataList)) {
        if (o.type() == type) {
            return o.value();
        }
    }
    return {};
}

QVariant Element::dataAsVariant(ChemicalDataObject::BlueObelisk type, int unit) const
{
    for (const ChemicalDataObject &o : dataList) {
        if (o.type() == type) {
            if (unit == KUnitConversion::NoUnit) {
                return o.value();
            }
            KUnitConversion::Value data(o.value().toDouble(), KUnitConversion::UnitId(o.unit()));
            return {data.convertTo(KUnitConversion::UnitId(unit)).number()};
        }
    }
    return {};
}

QString Element::dataAsString(ChemicalDataObject::BlueObelisk type) const
{
    return dataAsVariant(type).toString();
}

QString Element::dataAsString(ChemicalDataObject::BlueObelisk type, int unit) const
{
    return dataAsVariant(type, unit).toString();
}

QString Element::dataAsStringWithUnit(ChemicalDataObject::BlueObelisk type, int unit) const
{
    QString valueAndUnit(QString::number(dataAsVariant(type, unit).toDouble(), 'g', 4));

    if (valueAndUnit.isEmpty()) {
        return {};
    }

    valueAndUnit.append(" ");
    valueAndUnit.append(KUnitConversion::Converter().unit(KUnitConversion::UnitId(unit)).symbol());
    return valueAndUnit;
}

Element::~Element() = default;

void Element::addData(const ChemicalDataObject &o)
{
    dataList.append(o);
}

void Element::addData(const QVariant &value, ChemicalDataObject::BlueObelisk type)
{
    ChemicalDataObject tmp(value, type);
    dataList.append(tmp);
}
