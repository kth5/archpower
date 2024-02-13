/*
    NumerationItem - Numeration Item, part of the Periodic Table Graphics View
    for Kalzium

    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileContributor: 2010 Konstantin Tokarev
    SPDX-FileCopyrightText: 2010 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    This file is part of the Avogadro molecular editor project.
    For more information, see <https://avogadro.cc/>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "numerationitem.h"

#include <prefs.h>

#include <QFont>
#include <QPainter>
#include <QStyleOption>

#include <KLocalizedString>

#include "kalziumnumerationtype.h"

NumerationItem::NumerationItem(int xPosition)
    : m_width(40)
    , m_height(20)
    , m_xPosition(xPosition)
{
    setNumerationType(Prefs::numeration());
}

NumerationItem::~NumerationItem() = default;

QRectF NumerationItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

QPainterPath NumerationItem::shape() const
{
    QPainterPath path;
    path.addRect(0, 0, m_width, m_height);
    return path;
}

void NumerationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    QLinearGradient grad(QPointF(0, 0), QPointF(0, m_height));
    grad.setColorAt(0, m_color);
    grad.setColorAt(1, m_color.darker());
    painter->setBrush(grad);
    pen.setColor(m_color.darker(1000));
    painter->setPen(pen);

    QRectF rect(0, 0, m_width, m_height);
    painter->drawRoundedRect(rect, m_width / 10, m_width / 10);
    painter->drawText(rect, Qt::AlignCenter, m_numeration);
}

void NumerationItem::setNumerationType(int type)
{
    type == 0 ? m_color = QColor(Qt::transparent) : m_color = QColor(Qt::white);

    m_numeration = KalziumNumerationTypeFactory::instance()->build(type)->item(m_xPosition);

    update();
}

#include "moc_numerationitem.cpp"
