/*
    ElementItem - Element Item, part of the Periodic Table Graphics View for
    Kalzium

    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    This file is part of the Avogadro molecular editor project.
    For more information, see <https://avogadro.cc/>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef ELEMENTITEM_H
#define ELEMENTITEM_H

#include <QGraphicsItem>

#include "kalziumdataobject.h"
#include "kalziumelementproperty.h"
#include <chemicaldataobject.h>

/**
 * @class ElementItem
 * @author Marcus D. Hanwell, Etienne Rebetez
 * @brief An element item, intended to display a single element.
 *
 * This class implements a QGraphicsItem for displaying single elements in a
 * perdiodic table. It currently allows the setting of the proton number.
 * All other information come frome the kalziumElementProperty class.
 */
class ElementItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * Constructor. Should be called with the element number for this item. The
     * constructor uses setData to set the element number using the key 0. This
     * is then used by PeriodicTable to figure out which element was clicked on.
     */
    explicit ElementItem(KalziumElementProperty *property, int elementNumber = 0);

    /**
     * Destructor.
     */
    ~ElementItem() override;

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
    void redraw();

private:
    QString getCurrentElementValue();

    /**
     * Width of the elements.
     */
    int m_width;

    /**
     * Height of the elements.
     */
    int m_height;

    /**
     * The proton number of the item - all other attributes are derived from this.
     */
    int m_element;

    /**
     * The element numbers symbol.
     */
    QString m_symbol;

    /**
     * The color of the element which will also be used as the background color
     * for the item box.
     */
    QBrush m_brush;

    QColor m_textColor;

    QColor m_borderColor;

    QString m_textValue;

    KalziumElementProperty *m_property;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif // ELEMENTITEM_H
