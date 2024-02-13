/*
    SPDX-FileCopyrightText: 2011 Rebetez Etienne <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "periodictablestates.h"

#include "psetables.h"

#include <prefs.h>

#include <QEasingCurve>
#include <QPropertyAnimation>

PeriodicTableStates::PeriodicTableStates(const QList<ElementItem *> &elementItemList, const QList<NumerationItem *> &numerationItemList)
    : m_elementItemList(elementItemList)
    , m_numerationItemList(numerationItemList)
    , m_width(42)
    , m_height(42) // Some space between the elements (px40) looks nice.
{
    m_stateSwitcher = new StateSwitcher(&m_states);
    m_group = new QParallelAnimationGroup;

    // For every Tabletyp the Position of the Elements are set up.
    for (int tableIndex = 0; tableIndex < pseTables::instance()->tables().count(); ++tableIndex) {
        m_tableStatesList << new QState(m_stateSwitcher);

        setNumerationItemPositions(tableIndex);

        setElementItemPositions(tableIndex);

        m_stateSwitcher->addState(m_tableStatesList.at(tableIndex), m_group, tableIndex);
    }

    m_stateSwitcher->setInitialState(m_tableStatesList.at(Prefs::table()));
    m_states.setInitialState(m_stateSwitcher);
    m_states.start();
}

void PeriodicTableStates::setNumerationItemPositions(int tableIndex)
{
    hideAllNumerationItems(tableIndex);

    for (int x = 0; x < maxNumerationItemXCoordinate(tableIndex); ++x) {
        int numerationId = pseTables::instance()->getTabletype(tableIndex)->numerationAtPos(x);

        if (numerationId >= 0) {
            m_tableStatesList.at(tableIndex)->assignProperty(m_numerationItemList.at(numerationId), "pos", QPointF(x * m_width, -m_height * 3 / 4));
            addElementAnimation(m_numerationItemList.at(numerationId), x);
        }
    }
}

void PeriodicTableStates::hideAllNumerationItems(int tableIndex)
{
    for (NumerationItem *item : std::as_const(m_numerationItemList)) {
        m_tableStatesList.at(tableIndex)->assignProperty(item, "pos", QPointF(hiddenPoint()));
    }
}

QPoint PeriodicTableStates::hiddenPoint() const
{
    return QPoint(-40, -400);
}

int PeriodicTableStates::maxNumerationItemXCoordinate(int tableIndex)
{
    const int maxTableLenght = pseTables::instance()->getTabletype(tableIndex)->tableSize().x();

    return maxTableLenght > m_numerationItemList.count() ? maxTableLenght : m_numerationItemList.count();
}

void PeriodicTableStates::addElementAnimation(QGraphicsObject *object, int duration)
{
    QPropertyAnimation *anim = new QPropertyAnimation(object, "pos");
    anim->setDuration(1600 + duration * 2);
    anim->setEasingCurve(QEasingCurve::InOutExpo);
    m_group->addAnimation(anim);
}

void PeriodicTableStates::setElementItemPositions(int tableIndex)
{
    for (int i = 0; i < m_elementItemList.size(); ++i) {
        const int elementNumber = m_elementItemList.at(i)->data(0).toInt();
        QPoint itemPosition = pseTables::instance()->getTabletype(tableIndex)->elementCoords(elementNumber);

        // put the not needed elements a bit away
        if (itemPosition.x() < 0) {
            itemPosition = hiddenPoint();
        }

        m_tableStatesList.at(tableIndex)->assignProperty(m_elementItemList.at(i), "pos", QPointF((itemPosition.x()) * m_width, (itemPosition.y()) * m_height));

        addElementAnimation(m_elementItemList.at(i), i);
    }
}

QRectF PeriodicTableStates::pseRect(int tableIndex) const
{
    const QPoint maxTableCoords = pseTables::instance()->getTabletype(tableIndex)->tableSize();

    const int x = maxTableCoords.x();

    // adding one for the numeration row.
    const int y = maxTableCoords.y() + 1;

    return QRectF(0, -m_height, x * m_width, y * m_height);
}

void PeriodicTableStates::setTableState(int tableIndex)
{
    m_stateSwitcher->switchToState(tableIndex);
}

PeriodicTableStates::~PeriodicTableStates()
{
    delete m_group;
    qDeleteAll(m_tableStatesList);
    delete m_stateSwitcher;
}
