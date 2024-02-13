/*
    SPDX-FileCopyrightText: 2011 Rebetez Etienne <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PERIODICTABLESTATES_H
#define PERIODICTABLESTATES_H

#include <QParallelAnimationGroup>
#include <QStateMachine>


#include "elementitem.h"
#include "numerationitem.h"
#include "statemachine.h"

/**
 * @class PeriodicTableStates
 * In this class all states of element items and numeration items are defined.
 * The position and animations of the items are set.
 *
 * @short Setting up the pse states
 * @author Etienne Rebetez
 *
 */

class PeriodicTableStates
{
public:
    /**
     * Constructor need list of Items.
     * @param elementItemList List of the Element items in the table.
     * @param numerationItemList List of the Numeration items in the table.
     */
    PeriodicTableStates(const QList<ElementItem *> &elementItemList, const QList<NumerationItem *> &numerationItemList);

    virtual ~PeriodicTableStates();

    /**
     * Get the rectangle of the given periodic table.
     * @param tableIndex Index of the table @see pseTables.
     * @return Rectangle in floating point precision.
     */
    QRectF pseRect(int tableIndex) const;

    /**
     * Set the table index
     * @param tableIndex Index of the table @see pseTables..
     */
    void setTableState(int tableIndex);

private:
    void setNumerationItemPositions(int tableIndex);
    void hideAllNumerationItems(int tableIndex);
    int maxNumerationItemXCoordinate(int tableIndex);

    void addElementAnimation(QGraphicsObject *object, int factor = 1);

    void setElementItemPositions(int tableIndex);

    QPoint hiddenPoint() const;

    QStateMachine m_states;
    QParallelAnimationGroup *m_group;
    StateSwitcher *m_stateSwitcher;

    QList<ElementItem *> m_elementItemList;
    QList<NumerationItem *> m_numerationItemList;

    int m_width;
    int m_height;

    QList<QState *> m_tableStatesList;
};

#endif // PERIODICTABLESTATES_H
