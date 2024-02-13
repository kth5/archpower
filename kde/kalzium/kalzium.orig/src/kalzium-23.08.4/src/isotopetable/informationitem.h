/*
    SPDX-FileCopyrightText: 2007, 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFORMATIONITEM_H
#define INFORMATIONITEM_H

#include <QGraphicsRectItem>

class IsotopeItem;

class QGraphicsTextItem;

class InformationItem : public QGraphicsRectItem
{
public:
    enum { Type = UserType + 2 };

    /**
     * @param isotope The Isotope represented
     */
    InformationItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    /**
     * @return the Type of the item
     */
    int type() const override
    {
        return Type;
    }

    void setIsotope(IsotopeItem *item);

private:
    QGraphicsTextItem *m_textitem = nullptr;
};

#endif // INFORMATIONITEM_H
