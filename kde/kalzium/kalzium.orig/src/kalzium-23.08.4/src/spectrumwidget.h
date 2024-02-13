/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QWidget>

#include "prefs.h"
#include "spectrum.h"

/**
 * @author Carsten Niehaus
 */
class SpectrumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumWidget(QWidget *parent);

    ~SpectrumWidget() override = default;

    void setSpectrum(Spectrum *spec);

    Spectrum *spectrum() const
    {
        return m_spectrum;
    }

    /**
     * This limits the width of the spectrum in terms of
     * wavelength. For example you can set it to only
     * show the area between 500 and 550 nm
     *
     * @param left the left border
     * @param right the right border
     */
    void setBorders(double left, double right);

    /**
     * there are several possible types.
     */
    enum SpectrumType { EmissionSpectrum = 0, AbsorptionSpectrum };

    /**
     * sets the type of the spectrum to @p t
     * @param t the type of the spectrum
     */
    void setType(int t)
    {
        m_type = t;
    }

    /**
     * @return the currently active type
     * of the spectrum
     */
    int spectrumType() const
    {
        return m_type;
    }

    /**
     * @return the adjusted value of the @p color. The
     * correction depends on @p factor which has been
     * figured out empirically
     */
    int Adjust(double color, double factor);

    /**
     * @return the position in the widget of a band
     * with the wavelength @p wavelength
     *
     * @param wavelength the wavelength for which the position is needed
     */
    inline int xPos(double wavelength)
    {
        return (int)((wavelength - m_startValue) * width() / (m_endValue - m_startValue));
    }

    /**
     * based on the current position of the mouse-cursor the nearest
     * peak is searched. If found, it will be emitted.
     *
     * @see peakSelected
     */
    void findPeakFromMouseposition(double wavelength);

    /**
     * @param xpos The ratio of the position relative to the width
     * of the widget.
     * @return the wavelength on position @p xpos
     */
    inline double Wavelength(double xpos)
    {
        return m_startValue + ((m_endValue - m_startValue) * xpos);
    }

    /**
     * This method changes the three values @p r, @p g and @p b to the
     * correct values
     * @param wavelength the wavelength for which the color is searched
     * @return the wavelenth color
     */
    QColor wavelengthToRGB(double wavelength);

    /**
     * set the maximum value to @p value
     */
    void setRightBorder(int value)
    {
        if (value != m_endValue) {
            m_endValue = value;
            if (m_endValue < m_startValue) {
                m_startValue = m_endValue - 1;
            }
            update();
        }
    }

    /**
     * set the minimum value to @p value
     */
    void setLeftBorder(int value)
    {
        if (value != m_startValue) {
            m_startValue = value;
            if (m_startValue > m_endValue) {
                m_endValue = m_startValue + 1;
            }
            update();
        }
    }

private:
    QList<double> m_spectra;

    int m_type;

    Spectrum *m_spectrum;

    QPixmap m_pixmap;

    void paintBands(QPainter *p);
    void drawZoomLine(QPainter *p);

    /**
     * Draw the scale
     */
    void drawTickmarks(QPainter *p);

    double m_startValue;
    double m_endValue;

    double m_gamma;
    int m_intensityMax;

    int m_realHeight;

    /**
     * this QPoint stores the information where
     * the left mouse button has been pressed. This
     * is used for the mouse zooming
     */
    QPoint m_LMBPointPress;

    QPoint m_LMBPointCurrent;

public Q_SLOTS:
    ///(re)create startconditions
    void resetSpectrum();

    /**
     * activates the spectrum of the type @p spectrumtype
     */
    void slotActivateSpectrum(int spectrumtype)
    {
        m_type = spectrumtype;
        Prefs::setSpectrumType(spectrumtype);
        Prefs::self()->save();
        update();
    }

Q_SIGNALS:
    /**
     * the minimum and maximum displayed wavelength have
     * changed so emit the new minimum and maximum
     */
    void bordersChanged(int, int);

    /**
     * the user selected a peak
     */
    void peakSelected(Spectrum::peak *peak);

private Q_SLOTS:
    void slotZoomIn();
    void slotZoomOut();

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
};

#endif // SPECTRUMWIDGET_H
