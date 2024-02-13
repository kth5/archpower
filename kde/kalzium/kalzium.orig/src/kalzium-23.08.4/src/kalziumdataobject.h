/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMDATAOBJECT_H
#define KALZIUMDATAOBJECT_H

#include <QHash>
#include <QPixmap>
#include <element.h>

class Search;
class Isotope;
class Spectrum;
/**
 * @brief This class contains all Element objects
 *
 * This singleton class collects all the information about the elements of the
 * Periodic Table as list of Element.
 *
 * Use:
 * @code
 * KalziumDataObject::instance()->ElementList;
 * @endcode
 * to get the whole list of Element, while a
 * @code
 * KalziumDataObject::instance()->element(num);
 * @endcode
 * will return you the pointer to the num'th element of the Periodic Table.
 *
 * @author Carsten Niehaus
 */
class KalziumDataObject
{
public:
    /**
     * @return the instance of this class
     */
    static KalziumDataObject *instance();

    /**
     * The list of elements
     */
    QList<Element *> ElementList;

    /**
     * Set the main Search to @p srch
     */
    void setSearch(Search *srch);

    /**
     * @return the main Search
     */
    Search *search() const;

    /**
     * @return the Element with the number @p number
     * @param number the number of the Element which will be returned
     */
    Element *element(int number);

    /**
     * retunrs the unit symbol from the given KUnitConversion UnitId.
     * @param unit KUnitConversion UnitId
     * @return unit symbol as string.
     */
    QString unitAsString(const int unit) const;

    /**
     * @return the isotopes of the Element with the number @p number
     */
    QList<Isotope *> isotopes(int number);

    /**
     * @return the isotopes of the Element @p Element
     */
    QList<Isotope *> isotopes(Element *element);

    /**
     * @return the Spectrum of the Element with the number @p number
     */
    Spectrum *spectrum(int number);

    QPixmap pixmap(int number);

    /**
     * Use this to get the number of elements we have. It is cached
     * so you are strongly suggested to use this instead of hardcode
     * the number of elements.
     * @return the number of elements we have
     */
    int numberOfElements() const
    {
        return m_numOfElements;
    }

private:
    KalziumDataObject();
    ~KalziumDataObject();

    static void cleanup();

    void loadIconSet();
    void cleanPixmaps();

    QList<QPixmap> PixmapList;

    QHash<int, QList<Isotope *>> m_isotopes;
    QList<Spectrum *> m_spectra;

    /**
     * Caching the number of elements
     */
    int m_numOfElements;

    Search *m_search = nullptr;

    friend struct StaticKalziumDataObject;
};

#endif // KALZIUMDATAOBJECT_H
