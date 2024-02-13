/*
    SPDX-FileCopyrightText: 2003, 2004, 2005 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ELEMENT_H
#define ELEMENT_H

#include "science_export.h"

#include <QList>
#include <QVariant>

#include "chemicaldataobject.h"

/**
 * In this class all information about an element are stored. This means that
 * both the chemical date and the data about the position are stored
 * in this class.
 * @short This class is the representation of a chemical element
 * @author Carsten Niehaus
 */
class SCIENCE_EXPORT Element
{
public:
    Element();

    virtual ~Element();

    /**
     * Add the ChemicalDataObject @p o to this Element
     * @param o the ChemicalDataObject to be added
     */
    void addData(const ChemicalDataObject &o);

    /**
     * Add a ChemicalDataObject with @p value of @p type to this
     * Element
     * @param value the QVariant to be added
     * @param type the BlueObelisk type to be added
     */
    void addData(const QVariant &value, ChemicalDataObject::BlueObelisk type);

    /**
     * @return the requested data of the type @p type as a QVariant
     */
    QVariant dataAsVariant(ChemicalDataObject::BlueObelisk type) const;

    /**
     * @return the requested data of the type @p type with the unit @p unit as a QVariant
     */
    QVariant dataAsVariant(ChemicalDataObject::BlueObelisk type, int unit) const;

    /**
     * @return the requested data of the type @p type as a QString
     */
    QString dataAsString(ChemicalDataObject::BlueObelisk type) const;
    /**
     * @return the requested data of the type @p type with the given unit @p unit as a QString
     */
    QString dataAsString(ChemicalDataObject::BlueObelisk type, int unit) const;

    /**
     * @return the requested data of the type @p type with the unit @p unit as a QString
     * The unit symbol is appended to the value. The value is round to show 4 significant decimals.
     */
    QString dataAsStringWithUnit(ChemicalDataObject::BlueObelisk type, int unit) const;

    /**
     * @return the data of the Element
     */
    QList<ChemicalDataObject> data() const
    {
        return dataList;
    }

private:
    /**
     * this QList stores all information about an element
     */
    QList<ChemicalDataObject> dataList;
};

#endif // ELEMENT_H
