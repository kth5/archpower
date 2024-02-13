/*
    SPDX-FileCopyrightText: 2003, 2004, 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "orbitswidget.h"

#include "kalziumdataobject.h"
#include "kalziumutils.h"

// QT-Includes
#include <QPainter>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <cmath>

static QStringList hulllist;

/**
 * @return the delta of the x-coordinate
 * @param r is the radius of the circle
 * @param angle is the n'st circle out of num
 * @param num is the number of circles
 */
inline static double translateToDX(double r, double angle, int num)
{
    const double t = 2 * M_PI * angle / num;
    return r * sin(t);
}

/**
 * @return the delta of the y-coordinate
 * @param r is the radius of the circle
 * @param angle is the n'st circle out of num
 * @param num is the number of circles
 */
inline static double translateToDY(double r, double angle, int num)
{
    const double t = 2 * M_PI * angle / num;
    return r * cos(t);
}

OrbitsWidget::OrbitsWidget(QWidget *parent)
    : QWidget(parent)
    , m_electronConf(new QLabel(this))
{
    m_electronConf->setIndent(20);
    auto layout = new QVBoxLayout(m_electronConf);
    setLayout(layout);

    if (hulllist.count() == 0) {
        hulllist.append(QStringLiteral("1"));
        hulllist.append(QStringLiteral("2")); // Helium
        hulllist.append(QStringLiteral("2 1"));
        hulllist.append(QStringLiteral("2 2"));
        hulllist.append(QStringLiteral("2 3"));
        hulllist.append(QStringLiteral("2 4"));
        hulllist.append(QStringLiteral("2 5"));
        hulllist.append(QStringLiteral("2 6"));
        hulllist.append(QStringLiteral("2 7"));
        hulllist.append(QStringLiteral("2 8")); // Neon
        hulllist.append(QStringLiteral("2 8 1"));
        hulllist.append(QStringLiteral("2 8 2"));
        hulllist.append(QStringLiteral("2 8 3"));
        hulllist.append(QStringLiteral("2 8 4"));
        hulllist.append(QStringLiteral("2 8 5"));
        hulllist.append(QStringLiteral("2 8 6"));
        hulllist.append(QStringLiteral("2 8 7"));
        hulllist.append(QStringLiteral("2 8 8")); // Argon
        hulllist.append(QStringLiteral("2 8 8 1"));
        hulllist.append(QStringLiteral("2 8 8 2")); // Calcium
        hulllist.append(QStringLiteral("2 8 9 2"));
        hulllist.append(QStringLiteral("2 8 10 2"));
        hulllist.append(QStringLiteral("2 8 11 2"));
        hulllist.append(QStringLiteral("2 8 13 1"));
        hulllist.append(QStringLiteral("2 8 13 2")); // Manganese
        hulllist.append(QStringLiteral("2 8 14 2"));
        hulllist.append(QStringLiteral("2 8 15 2"));
        hulllist.append(QStringLiteral("2 8 16 2"));
        hulllist.append(QStringLiteral("2 8 18 1")); // Copper
        hulllist.append(QStringLiteral("2 8 18 2"));
        hulllist.append(QStringLiteral("2 8 18 3"));
        hulllist.append(QStringLiteral("2 8 18 4"));
        hulllist.append(QStringLiteral("2 8 18 5"));
        hulllist.append(QStringLiteral("2 8 18 6"));
        hulllist.append(QStringLiteral("2 8 18 7"));
        hulllist.append(QStringLiteral("2 8 18 8")); // Krypton
        hulllist.append(QStringLiteral("2 8 18 8 1"));
        hulllist.append(QStringLiteral("2 8 18 8 2")); // Rubidium
        hulllist.append(QStringLiteral("2 8 18 9 2"));
        hulllist.append(QStringLiteral("2 8 18 10 2")); // Zirconium
        hulllist.append(QStringLiteral("2 8 18 12 1"));
        hulllist.append(QStringLiteral("2 8 18 13 1"));
        hulllist.append(QStringLiteral("2 8 18 14 1")); // Techneticum
        hulllist.append(QStringLiteral("2 8 18 15 1"));
        hulllist.append(QStringLiteral("2 8 18 16 1"));
        hulllist.append(QStringLiteral("2 8 18 18")); // Palladium
        hulllist.append(QStringLiteral("2 8 18 18 1"));
        hulllist.append(QStringLiteral("2 8 18 18 2"));
        hulllist.append(QStringLiteral("2 8 18 18 3")); // Indium
        hulllist.append(QStringLiteral("2 8 18 18 4"));
        hulllist.append(QStringLiteral("2 8 18 18 5"));
        hulllist.append(QStringLiteral("2 8 18 18 6"));
        hulllist.append(QStringLiteral("2 8 18 18 7"));
        hulllist.append(QStringLiteral("2 8 18 18 8")); // Xenon
        hulllist.append(QStringLiteral("2 8 18 18 8 1")); // Caesium
        hulllist.append(QStringLiteral("2 8 18 18 8 2")); // Barium
        hulllist.append(QStringLiteral("2 8 18 18 9 2"));
        hulllist.append(QStringLiteral("2 8 18 20 8 2")); // Cerium
        hulllist.append(QStringLiteral("2 8 18 21 8 2"));
        hulllist.append(QStringLiteral("2 8 18 22 8 2"));
        hulllist.append(QStringLiteral("2 8 18 23 8 2"));
        hulllist.append(QStringLiteral("2 8 18 24 8 2"));
        hulllist.append(QStringLiteral("2 8 18 25 8 2"));
        hulllist.append(QStringLiteral("2 8 18 25 9 2")); // Gadolinium
        hulllist.append(QStringLiteral("2 8 18 27 8 2")); // Terbium
        hulllist.append(QStringLiteral("2 8 18 28 8 2"));
        hulllist.append(QStringLiteral("2 8 18 29 8 2"));
        hulllist.append(QStringLiteral("2 8 18 30 8 2"));
        hulllist.append(QStringLiteral("2 8 18 31 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 8 2")); // Ytterbium
        hulllist.append(QStringLiteral("2 8 18 32 9 2")); // Lutetium
        hulllist.append(QStringLiteral("2 8 18 32 10 2")); // Hafnium
        hulllist.append(QStringLiteral("2 8 18 32 11 2"));
        hulllist.append(QStringLiteral("2 8 18 32 12 2"));
        hulllist.append(QStringLiteral("2 8 18 32 13 2"));
        hulllist.append(QStringLiteral("2 8 18 32 14 2"));
        hulllist.append(QStringLiteral("2 8 18 32 15 2")); // Irdium
        hulllist.append(QStringLiteral("2 8 18 32 17 1"));
        hulllist.append(QStringLiteral("2 8 18 32 18 1"));
        hulllist.append(QStringLiteral("2 8 18 32 18 2")); // Mercury
        hulllist.append(QStringLiteral("2 8 18 32 18 3"));
        hulllist.append(QStringLiteral("2 8 18 32 18 4"));
        hulllist.append(QStringLiteral("2 8 18 32 18 5"));
        hulllist.append(QStringLiteral("2 8 18 32 18 6"));
        hulllist.append(QStringLiteral("2 8 18 32 18 7"));
        hulllist.append(QStringLiteral("2 8 18 32 18 8")); // Radon
        hulllist.append(QStringLiteral("2 8 18 32 18 8 1")); // Francium
        hulllist.append(QStringLiteral("2 8 18 32 18 8 2")); // Radium
        hulllist.append(QStringLiteral("2 8 18 32 18 9 2")); // Actinium
        hulllist.append(QStringLiteral("2 8 18 32 20 8 2")); // Thorium
        hulllist.append(QStringLiteral("2 8 18 32 21 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 22 8 2")); // Uran
        hulllist.append(QStringLiteral("2 8 18 32 23 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 24 8 2")); // Plutonium
        hulllist.append(QStringLiteral("2 8 18 32 25 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 26 8 2")); // Cm
        hulllist.append(QStringLiteral("2 8 18 32 27 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 28 8 2")); // Cf
        hulllist.append(QStringLiteral("2 8 18 32 29 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 30 8 2")); // Fm
        hulllist.append(QStringLiteral("2 8 18 32 31 8 2"));
        hulllist.append(QStringLiteral("2 8 18 32 32 8 2")); // Nobelium
        hulllist.append(QStringLiteral("2 8 18 32 32 9 2")); // Lawrencium
        hulllist.append(QStringLiteral("2 8 18 32 32 10 2"));
        hulllist.append(QStringLiteral("2 8 18 32 32 11 2")); // Dubnium (105)
        hulllist.append(QStringLiteral("2 8 18 32 32 12 2"));
        hulllist.append(QStringLiteral("2 8 18 32 32 13 2")); // Bohrium
        hulllist.append(QStringLiteral("2 8 18 32 32 14 2"));
        hulllist.append(QStringLiteral("2 8 18 32 32 15 2")); // Mt
        hulllist.append(QStringLiteral("2 8 18 32 32 16 2")); // Darmstadtium
        hulllist.append(QStringLiteral("2 8 18 32 32 17 2")); // Roentgenium
    }
}

void OrbitsWidget::setElementNumber(int num)
{
    Elemno = num;

    numOfElectrons.clear();
    QString o;
    if ((Elemno > 0) && (Elemno <= hulllist.count())) {
        o = hulllist[Elemno - 1];
    }

    foreach (const QString &str, o.split(' ', Qt::SkipEmptyParts))
        numOfElectrons.append(str.toInt());

    m_electronConf->setMinimumWidth(width());
    // setting the electronic configuration in the label.
    m_electronConf->setText(KalziumUtils::prettyUnit(KalziumDataObject::instance()->element(Elemno), ChemicalDataObject::electronicConfiguration));
    update();
}

void OrbitsWidget::paintEvent(QPaintEvent *)
{
    QPainter DC;
    DC.begin(this);
    DC.setRenderHint(QPainter::Antialiasing, true);

    int min_size = qMin(width(), height());
    int min_delta = min_size / 10;

    const int num = numOfElectrons.count();
    if (num == 0) {
        DC.drawText(QPoint(width() / 3, height() / 3), i18n("Unknown Electron Distribution"));
        return; // no orbits, do nothing
    }

    // make sure the biggest orbit fits in the widget
    // diameter
    int d = min_size - 2 * min_delta;

    // the radius of the current orbit
    int r = d / 2;

    // the radius of an 'electron'
    int r_electron = r / 20;

    // difference to the previous circle
    int ddx = r / num;

    QList<int>::Iterator it = numOfElectrons.end();
    --it;

    for (int i = 0; i < num; ++i) {
        int mx = min_delta + ddx * i; // the x-coordinate for the current circle
        int my = min_delta + ddx * i; // the y-coordinate for the current circle

        DC.setBrush(Qt::NoBrush);
        DC.setPen(Qt::black);

        // draw the big ellipses in concentric circles
        DC.drawEllipse(mx, my, d, d);

        DC.setPen(Qt::NoPen);

        for (int e = 0; e < *it; ++e) {
            int x = (int)translateToDX(d / 2.0, (double)e, *it);
            int y = (int)translateToDY(d / 2.0, (double)e, *it);

            // Creating a gradient for the current electron.
            QRadialGradient grad(QPointF(x + mx + d / 2 - r_electron / 2, y + my + d / 2 - r_electron / 2), r_electron);
            grad.setColorAt(0, Qt::white);
            grad.setColorAt(0.2, Qt::yellow);
            grad.setColorAt(1, QColor(Qt::yellow).darker());
            DC.setBrush(QBrush(grad));

            DC.drawEllipse(x + mx + d / 2 - r_electron, y + my + d / 2 - r_electron, 2 * r_electron, 2 * r_electron);
        }
        --it;
        d -= 2 * ddx;
    }
}

#include "moc_orbitswidget.cpp"
