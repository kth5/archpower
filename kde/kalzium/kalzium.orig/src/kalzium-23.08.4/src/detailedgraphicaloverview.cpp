/*
    SPDX-FileCopyrightText: 2004, 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detailedgraphicaloverview.h"

#include "kalziumelementproperty.h"
#include "kalziumdataobject.h"
#include "kalziumutils.h"

#include <KLocalizedString>

#include "kalzium_debug.h"
#include <QFile>
#include <QFontDatabase>
#include <QPainter>
#include <QRect>
#include <QStandardPaths>
#include <QSvgRenderer>

#include "prefs.h"
#include <element.h>

DetailedGraphicalOverview::DetailedGraphicalOverview(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    setMinimumSize(300, 200);

    // Set Hydrogen as initial element.
    setElement(1);
}

void DetailedGraphicalOverview::setElement(int el)
{
    m_element = KalziumDataObject::instance()->element(el);
    setBackgroundColor(KalziumElementProperty::instance()->getElementColor(el));
    update();
}

void DetailedGraphicalOverview::setBackgroundColor(QColor bgColor)
{
    if (bgColor == Qt::transparent) {
        bgColor = palette().window().color();
    }

    // add a gradient
    QLinearGradient grad(QPointF(0, 0), QPointF(0, height()));
    grad.setColorAt(0, bgColor);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qreal h, s, v, a;
#else
    float h, s, v, a;
#endif
    bgColor.getHsvF(&h, &s, &v, &a);
    bgColor.setHsvF(h, s, v * 0.6, a);
    grad.setColorAt(1, bgColor);

    m_backgroundBrush = QBrush(grad);
}

void DetailedGraphicalOverview::paintEvent(QPaintEvent *)
{
    qreal dpr = devicePixelRatioF();
    qreal dprWidth = dpr * width();
    qreal dprHeight = dpr * height();
    QRect rect(0, 0, dprWidth, dprHeight);

    QPixmap pm(dprWidth, dprHeight);
    pm.setDevicePixelRatio(dpr);

    QPainter p;
    p.begin(&pm);

    p.setBrush(Qt::SolidPattern);

    if (!m_element) {
        pm.fill(palette().window().color());
        p.drawText(0, 0, width(), height(), Qt::AlignCenter | Qt::TextWordWrap, i18n("No element selected"));
    } else if (Prefs::colorschemebox() == 2) { // The iconic view is the 3rd view (0,1,2,...)
        pm.fill(palette().window().color());

        QString pathname = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/iconsets/"), QStandardPaths::LocateDirectory);

        int enumii = m_element->dataAsVariant(ChemicalDataObject::atomicNumber).toInt();

        QString filename = pathname + "school" + '/' + QString::number(enumii) + ".svg";

        QSvgRenderer svgrenderer;
        if (QFile::exists(filename) && svgrenderer.load(filename)) {
            QSize size = svgrenderer.defaultSize();
            size.scale(width(), height(), Qt::KeepAspectRatio);

            QRect bounds(QPoint(0, 0), size);
            bounds.moveCenter(QPoint(width() / 2, height() / 2));
            svgrenderer.render(&p, bounds);
        } else {
            p.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("No graphic found"));
        }
    } else {
        const int h_t = 20; // height of the texts

        p.setBrush(m_backgroundBrush);
        p.drawRect(rect);
        p.setBrush(Qt::black);
        p.setBrush(Qt::NoBrush);

        QFont fA = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        QFont fB = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        QFont fC = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

        fA.setPointSize(fA.pointSize() + 20); // Huge font
        fA.setBold(true);
        fB.setPointSize(fB.pointSize() + 6); // Big font
        fC.setPointSize(fC.pointSize() + 4); // Big font
        fC.setBold(true);
        QFontMetrics fmA = QFontMetrics(fA);
        QFontMetrics fmB = QFontMetrics(fB);

        // coordinates for element symbol: near the center
        int xA = 4 * width() / 10;
        int yA = height() / 2;

        // coordinates for the atomic number: offset from element symbol to the upper left
        int xB = xA - fmB.boundingRect(m_element->dataAsString(ChemicalDataObject::atomicNumber)).width();
        int yB = yA + fmB.height() / 2;

        // Element Symbol
        p.setFont(fA);
        p.drawText(xA, yA, m_element->dataAsString(ChemicalDataObject::symbol));

        // Atomic number
        p.setFont(fB);
        p.drawText(xB, yB, m_element->dataAsString(ChemicalDataObject::atomicNumber));

        // Name and other data
        fC.setPointSize(h_t);
        p.setFont(fC);

        // Name
        p.drawText(1, 0, width(), height(), Qt::AlignLeft, m_element->dataAsString(ChemicalDataObject::name));

        // TODO Oxidationstates -> not there yet

        // Mass
        QString massString = i18nc("For example '1.0079u', the mass of an element in units", "%1 u", m_element->dataAsString(ChemicalDataObject::mass));
        int size3 = KalziumUtils::maxSize(massString, rect, fC, &p);
        fC.setPointSize(size3);
        p.setFont(fC);
        int offset = KalziumUtils::StringHeight(massString, fC, &p);
        p.drawText(0, height() - offset, width(), offset, Qt::AlignRight, massString);
    }

    p.end();

    p.begin(this);
    p.drawPixmap(0, 0, pm);
    p.end();
}

#include "moc_detailedgraphicaloverview.cpp"
