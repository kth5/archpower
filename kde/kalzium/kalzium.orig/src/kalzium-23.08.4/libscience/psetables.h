/*
    SPDX-FileCopyrightText: 2005, 2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2007 Carste Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PSETABLES_H
#define PSETABLES_H

#include "science_export.h"

#include <QList>
#include <QPoint>
#include <QStringList>

/**
 * @class pseTables
 * Provides functions to easily create pse tables with qGridLayouts or qGraphicsView.
 *
 * creating a table for the gridlayout
@code
  foreach (int elementIndex, pseTables::instance()->getTabletype(m_psTableType)->elements()) {
       int group = pseTables::instance()->getTabletype(m_psTableType)->elementCoords(elementIndex).x();
       int period = pseTables::instance()->getTabletype(m_psTableType)->elementCoords(elementIndex).y();

       ElementLabel *element = new ElementLabel(elementIndex);

       gridLayoutOfPeriodSystem->addItem(element, period, group);
  }

@endcode

 * position elements in a qGraphicsScene
@code


@endcode

 * getting the position of the Numerations for the periodic system of elements(j)
 @code
    for (int i = 0; i < pseTables::instance()->getTabletype(j)->tableSize().x() || i < numerationItems.count(); ++i) {
        int itemAtPos = pseTables::instance()->getTabletype(j)->numerationAtPos(i);
    }
 @endcode
 * @short Provides shape and elements of different periodic tables of elements
 * @author Etienne Rebetez
 */
class pseTable;

/**
 * Holds all periodic system tables and make them accessible.
 */
class SCIENCE_EXPORT pseTables
{
public:
    static pseTables *instance();

    ~pseTables();

    /**
     * Returns the KalziumTableType with the @p id specified.
     * It will gives 0 if none found.
     */
    pseTable *getTabletype(const int tableType);

    /**
     * Returns the KalziumTableType whose name is the @p id
     * specified.
     * It will gives 0 if none found.
     */
    pseTable *getTabletype(const QString &tableType);

    /**
     * Returns a list with the names of the table types we support.
     */
    QStringList tables() const;

private:
    pseTables();
    QList<pseTable *> m_tables;
    // int m_currentTable;
};

/**
 * defines a Periodic Table.
 * Holds the position (x,y) and all the displayed elements
 */
class pseTable

{
public:
    static pseTable *init();

    virtual ~pseTable();

    /**
     * Returns the ID of this table type.
     */
    virtual QString name() const;

    /**
     * Returns a short description of the periodic table in use
     */
    virtual QString description() const;

    /**
     * Returns the coordinates of an element  @p element in
     * the periodic system.
     * If the element is not in the periodic system QPoint(-1,-1) is returned.
     */
    virtual QPoint elementCoords(int element) const;

    /**
     * Returns a list with all elements in the actual periodic table
     */
    virtual QList<int> elements() const;

    /**

     * Returns the element that comes right before the specified @p element.
     * -1 means that @p element is the first in this table type.
     *
     * The default implementation returns <tt>element - 1</tt> if @p element
     * is not 1, else -1.
     */
    virtual int previousOf(int element) const;

    /**
     * Returns the element that comes right after the specified @p element.
     * -1 means that @p element is the last in this table type.
     *
     * The default implementation returns <tt>element + 1</tt> if @p element
     * is not the latest element, else -1.
     */
    virtual int nextOf(int element) const;

    /**
     * Returns the first element of the table.
     */
    virtual int firstElement() const;

    /**
     * Returns the last element of the table.
     */
    virtual int lastElement() const;

    /**
     * Returns the maximal size of the periodic table.
     */
    virtual QPoint tableSize() const;

    /**
     * Returns the Numeration for the current Table according to the position in the Table.
     * Coordinates beginning with 0. Returns -1 if none is found.
     */
    virtual int numerationAtPos(int xPos) const;

protected:
    pseTable();

    QString m_name;
    QString m_description;

    QList<int> m_posX;
    QList<int> m_posY;
    QList<int> m_xCoordsNumeration;
    QList<int> m_elementList;
};

class pseRegularTable : public pseTable
{
public:
    static pseRegularTable *init();

private:
    pseRegularTable();
};

class pseLongTable : public pseTable
{
public:
    static pseLongTable *init();

private:
    pseLongTable();
};

class pseShortTable : public pseTable
{
public:
    static pseShortTable *init();

private:
    pseShortTable();
};

class pseDTable : public pseTable
{
public:
    static pseDTable *init();

private:
    pseDTable();
};

class pseDZTable : public pseTable
{
public:
    static pseDZTable *init();

private:
    pseDZTable();
};

#endif // PSTABLES_H
