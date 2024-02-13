/*
    ElementItem - Element Item, part of the Periodic Table Graphics View for
    Avogadro

    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileContributor: 2010 Konstantin Tokarev
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    This file is part of the Avogadro molecular editor project.
    For more information, see <https://avogadro.cc/>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "elementitem.h"

#include <prefs.h>

#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include <KLocalizedString>

ElementItem::ElementItem(KalziumElementProperty *elProperty, int elementNumber)
    : m_width(40)
    , m_height(40)
    , m_element(elementNumber)
    , m_property(elProperty)
{
    // Want these items to be selectable
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);

    m_symbol = KalziumDataObject::instance()->element(m_element)->dataAsString(ChemicalDataObject::symbol);

    // Set some custom data to make it easy to figure out which element we are
    setData(0, m_element);
}

ElementItem::~ElementItem()
{
}

QRectF ElementItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath ElementItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, m_width, m_height);
    return path;
}

void ElementItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    pen.setColor(m_borderColor);
    pen.setWidth(1);
    painter->setPen(pen);

    painter->setBrush(m_brush);

    painter->drawRoundedRect(boundingRect(), m_width / 10, m_width / 10);

    if (isSelected()) {
        QColor selectedBackgroundColor = m_borderColor;
        selectedBackgroundColor.setAlpha(160);
        painter->setBrush(QBrush(QColor(selectedBackgroundColor)));
        painter->drawRoundedRect(boundingRect(), m_width / 10, m_width / 10);
    }

    pen.setColor(m_textColor);
    painter->setPen(pen);

    QFont symbolFont;

    switch (m_property->getMode()) {
    case KalziumElementProperty::NORMAL:
        symbolFont.setPointSize(12);
        symbolFont.setBold(true);
        painter->setFont(symbolFont);
        painter->drawText(boundingRect(), Qt::AlignCenter, m_symbol);
        symbolFont.setPointSize(7);
        symbolFont.setBold(false);
        painter->setFont(symbolFont);
        painter->drawText(QRectF(m_width / 14, m_height / 20, m_width, m_height / 2), Qt::AlignLeft, QString::number(m_element));
        break;

    case KalziumElementProperty::GRADIENTVALUE:
        painter->drawText(QRectF(0, m_height / 20, m_width, m_height / 2), Qt::AlignCenter, m_symbol);

        symbolFont.setPointSize(7);
        painter->setFont(symbolFont);

        painter->drawText(QRectF(0, m_height / 2 - m_height / 20, m_width, m_height / 2), Qt::AlignCenter, m_textValue);
        break;
    }
}

void ElementItem::redraw()
{
    m_brush = m_property->getElementBrush(m_element);
    m_textColor = m_property->getTextColor(m_element);
    m_borderColor = m_property->getBorderColor(m_element);
    m_textValue = getCurrentElementValue();
    update();
}

QString ElementItem::getCurrentElementValue()
{
    double value = m_property->getValue(m_element);

    if (value == -1) {
        return i18n("n/a");
    }

    return QString::number(value);
}

void ElementItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setZValue(200);
    moveBy(-m_width / 4, -m_height / 4);
    setScale(1.5);
    QGraphicsItem::hoverEnterEvent(event);
}

void ElementItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    resetTransform();
    moveBy(m_width / 4, m_height / 4);
    setZValue(100);
    setScale(1);
    QGraphicsItem::hoverLeaveEvent(event);
}

#include "moc_elementitem.cpp"
