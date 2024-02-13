/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spectrumwidget.h"

#include "kalziumutils.h"

#include <config-kalzium.h>

#include <cmath>
#include <element.h>

#include "kalzium_debug.h"
#include <QGlobalStatic>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QSizePolicy>

#include <KUnitConversion/Converter>

#if defined(HAVE_IEEEFP_H)
#include <ieeefp.h>
#endif

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent)
{
    m_startValue = 0;
    m_endValue = 0;

    m_LMBPointCurrent.setX(-1);
    m_LMBPointPress.setX(-1);

    m_realHeight = 200;

    m_gamma = 0.8;
    m_intensityMax = 255;

    setType(Prefs::spectrumType());

    setMinimumSize(400, 230);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void SpectrumWidget::paintEvent(QPaintEvent * /*e*/)
{
    if (!m_spectrum) {
        return;
    }

    m_pixmap = QPixmap(width(), height());
    m_pixmap.fill();

    QPainter p;
    p.begin(&m_pixmap);
    p.fillRect(0, 0, width(), m_realHeight, Qt::black);

    paintBands(&p);

    drawTickmarks(&p);

    if (m_LMBPointPress.x() != -1 && m_LMBPointCurrent.x() != -1) {
        drawZoomLine(&p);
    }

    p.end();

    QPainter p2(this);
    p2.drawPixmap(0, 0, m_pixmap);
}

void SpectrumWidget::drawZoomLine(QPainter *p)
{
    p->setPen(Qt::white);
    p->drawLine(m_LMBPointPress.x(), m_LMBPointPress.y(), m_LMBPointCurrent.x(), m_LMBPointPress.y());
    p->drawLine(m_LMBPointCurrent.x(), m_LMBPointPress.y() + 10, m_LMBPointCurrent.x(), m_LMBPointPress.y() - 10);
    p->drawLine(m_LMBPointPress.x(), m_LMBPointPress.y() + 10, m_LMBPointPress.x(), m_LMBPointPress.y() - 10);
}

void SpectrumWidget::paintBands(QPainter *p)
{
    if (m_type == AbsorptionSpectrum) {
        for (double va = m_startValue; va <= m_endValue; va += 0.1) {
            int x = xPos(va);
            p->setPen(wavelengthToRGB(va));
            p->drawLine(x, 0, x, m_realHeight);
        }

        p->setPen(Qt::black);
    }

    int i = 0;
    int x = 0;
    double currentWavelength;

    foreach (Spectrum::peak *peak, m_spectrum->peaklist()) {
        currentWavelength = peak->wavelengthToUnit(Prefs::spectrumWavelengthUnit());

        if (currentWavelength < m_startValue || currentWavelength > m_endValue) {
            continue;
        }

        x = xPos(currentWavelength);

        switch (m_type) {
        case EmissionSpectrum:
            p->setPen(wavelengthToRGB(currentWavelength));
            p->drawLine(x, 0, x, m_realHeight - 1);

            p->setPen(Qt::black);
            p->drawLine(x, m_realHeight, x, m_realHeight);
            break;

        case AbsorptionSpectrum:
            p->setPen(Qt::black);
            p->drawLine(x, 0, x, m_realHeight - 1);
            break;
        }

        ++i;
    }
}

QColor SpectrumWidget::wavelengthToRGB(double wavelength)
{
    double blue = 0.0, green = 0.0, red = 0.0, factor = 0.0;

    // wavelengthTo RGB function works with nanometers.
    wavelength = KUnitConversion::Value(wavelength, KUnitConversion::UnitId(Prefs::spectrumWavelengthUnit())).convertTo(KUnitConversion::Nanometer).number();

    int wavelength_ = (int)floor(wavelength);

    if (wavelength_ < 380 || wavelength_ > 780) {
        return {Qt::white};
    } else if (wavelength_ >= 380 && wavelength_ < 440) {
        red = -(wavelength - 440) / (440 - 380);
        green = 0.0;
        blue = 1.0;
    } else if (wavelength_ >= 440 && wavelength_ < 490) {
        red = 0.0;
        green = (wavelength - 440) / (490 - 440);
        blue = 1.0;
    } else if (wavelength_ >= 490 && wavelength_ < 510) {
        red = 0.0;
        green = 1.0;
        blue = -(wavelength - 510) / (510 - 490);
    } else if (wavelength_ >= 510 && wavelength_ < 580) {
        red = (wavelength - 510) / (580 - 510);
        green = 1.0;
        blue = 0.0;
    } else if (wavelength_ >= 580 && wavelength_ < 645) {
        red = 1.0;
        green = -(wavelength - 645) / (645 - 580);
        blue = 0.0;
    } else if (wavelength_ >= 645 && wavelength_ < 780) {
        red = 1.0;
        green = 0.0;
        blue = 0.0;
    }

    if (wavelength_ > 380 && wavelength_ <= 420) {
        factor = 0.3 + 0.7 * (wavelength - 380) / (420 - 380);
    } else if (wavelength_ > 420 && wavelength_ <= 701) {
        factor = 1.0;
    } else if (wavelength_ > 701 && wavelength_ <= 780) {
        factor = 0.3 + 0.7 * (780 - wavelength) / (780 - 700);
    } else {
        factor = 0.0;
    }

    return {Adjust(red, factor), Adjust(green, factor), Adjust(blue, factor)};
}

int SpectrumWidget::Adjust(double color, double /*factor*/)
{
    if (color == 0.0) {
        return 0;
    } else {
        //         return qRound(m_intensityMax * pow(color*factor, m_gamma)); FIXME
        return m_intensityMax * color;
    }
}

void SpectrumWidget::drawTickmarks(QPainter *p)
{
    // the size of the text on the tickmarks
    const int space = 20;

    // the distance between the tickmarks in pixel
    const int d = 10;

    // the total number of tickmarks to draw (small and long)
    const int numberOfTickmarks = (int)floor((double)(width() / d));

    double pos = 0.0;

    for (int i = 0; i < numberOfTickmarks; ++i) {
        if (i % 5 == 0) {
            // long tickmarks plus text
            p->drawLine(i * d, m_realHeight, i * d, m_realHeight + 10);
            if (i % 10 == 0 && i * d > space && i * d < width() - space) {
                pos = (double)(i * d) / width();
                p->fillRect(i * d - space, m_realHeight + 12, 2 * space, 15, Qt::white);
                p->drawText(i * d - space, m_realHeight + 12, 2 * space, 15, Qt::AlignCenter, QString::number(KalziumUtils::strippedValue(Wavelength(pos))));
            }
        } else { // small tickmarks
            p->drawLine(i * d, m_realHeight, i * d, m_realHeight + 5);
        }
    }
}

void SpectrumWidget::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Plus:
        slotZoomIn();
        break;
    case Qt::Key_Minus:
        slotZoomOut();
        break;
    }
}

void SpectrumWidget::slotZoomOut()
{
    qCDebug(KALZIUM_LOG) << "SpectrumWidget::slotZoomOut() " << m_startValue << ":: " << m_endValue;

    double diff = m_endValue - m_startValue;

    double offset = diff * 0.05;

    m_endValue = m_endValue + offset;
    m_startValue = m_startValue - offset;

    // check for invalid values
    if (m_startValue < 0.0) {
        m_startValue = 0.0;
    }

    if (m_endValue > 10000.0) { // FIXME: Magic numbers...
        m_endValue = 40000.0;
    }

    setBorders(m_startValue, m_endValue);
}

void SpectrumWidget::setBorders(double left, double right)
{
    qCDebug(KALZIUM_LOG) << "setBorders    " << left << ".." << right;

    m_startValue = left;
    m_endValue = right;

    // round the startValue down and the endValue up
    Q_EMIT bordersChanged(int(m_startValue + 0.5), int(m_endValue + 0.5));

    update();
}

void SpectrumWidget::slotZoomIn()
{
    qCDebug(KALZIUM_LOG) << "SpectrumWidget::slotZoomIn() " << m_startValue << ":: " << m_endValue;

    double diff = m_endValue - m_startValue;

    double offset = diff * 0.05;

    m_endValue = m_endValue - offset;
    m_startValue = m_startValue + offset;

    setBorders(m_startValue, m_endValue);
}

void SpectrumWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_LMBPointCurrent = e->pos();
    update();
}

void SpectrumWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_LMBPointPress = e->pos();
    }
    if (e->button() == Qt::RightButton) {
        resetSpectrum();
    }

    findPeakFromMouseposition(Wavelength((double)e->pos().x() / width()));
}

void SpectrumWidget::findPeakFromMouseposition(double wavelength)
{
    qCDebug(KALZIUM_LOG) << "SpectrumWidget::findPeakFromMouseposition()";
    Spectrum::peak *peak = nullptr;

    // find the difference in percent (1.0 is 100%, 0.1 is 10%)
    double dif = 0.0;

    bool foundWavelength = false;

    // find the highest intensity
    foreach (Spectrum::peak *currentPeak, m_spectrum->peaklist()) {
        double currentWavelength = currentPeak->wavelengthToUnit(Prefs::spectrumWavelengthUnit());

        double thisdif = currentWavelength / wavelength;

        if (thisdif < 0.9 || thisdif > 1.1) {
            continue;
        }

        if (thisdif > 1.0) { // convert for example 1.3 to 0.7
            thisdif = thisdif - 1;
            thisdif = 1 - thisdif;
        }

        if (thisdif > dif) {
            dif = thisdif;
            peak = currentPeak;
            foundWavelength = true;
        }
    }

    if (foundWavelength) {
        Q_EMIT peakSelected(peak);
    }
}

void SpectrumWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        int left = (int)Wavelength((double)m_LMBPointPress.x() / width());
        int right = (int)Wavelength((double)e->pos().x() / width());

        if (left == right) {
            return;
        }

        if (left > right) {
            setBorders(right, left);
        } else {
            setBorders(left, right);
        }
    }

    m_LMBPointPress.setX(-1);
    m_LMBPointCurrent.setX(-1);
}

void SpectrumWidget::setSpectrum(Spectrum *spec)
{
    m_spectrum = spec;
    resetSpectrum();
}

void SpectrumWidget::resetSpectrum()
{
    // set the minimum and maximum peak to the min/max wavelength
    // plus/minus ten. This makes then always visible
    double minimumPeak = m_spectrum->minPeak(Prefs::spectrumWavelengthUnit()) - 20.0;
    double maximumPeak = m_spectrum->maxPeak(Prefs::spectrumWavelengthUnit()) + 20.0;

    setBorders(minimumPeak, maximumPeak);
}

#include "moc_spectrumwidget.cpp"
