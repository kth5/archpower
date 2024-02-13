/*
    NumerationItem - Numeration Item, part of the Periodic Table Graphics View
    for Kalzium

    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    This file is part of the Avogadro molecular editor project.
    For more information, see <https://avogadro.cc/>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef NUMERATIONITEM_H
#define NUMERATIONITEM_H

#include <QGraphicsItem>

#include <chemicaldataobject.h>

/**
 * @class NumerationItem
 * @author Marcus D. Hanwell
 * @author Etienne Rebetez
 * @brief An Numeration item, intended to display a id of the numeration row.
 *
 */
class NumerationItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * Constructor. Should be called with the element number for this item. The
     * constructor uses setData to set the element number using the key 0. This
     * is then used by PeriodicTable to figure out which element was clicked on.
     */
    explicit NumerationItem(int xPosition = 0);

    /**
     * Destructor.
     */
    ~NumerationItem() override;

    /**
     * @return the bounding rectangle of the element item.
     */
    QRectF boundingRect() const override;

    /**
     * @return the painter path which is also a rectangle in this case.
     */
    QPainterPath shape() const override;

    /**
     * This is where most of the action takes place. The element box is drawn
     * along with its symbol.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

public Q_SLOTS:
    void setNumerationType(int type);

private:
    /**
     * Width of the elements.
     */
    int m_width;

    /**
     * Height of the elements.
     */
    int m_height;

    /**
     * The row Position of the Numeration item
     */
    int m_xPosition;

    /**
     * The numeration symbol.
     */
    QString m_numeration;

    /**
     * The color of the element.
     */
    QColor m_color;
};

#endif // NUMERATIONITEM_H
