/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECTRUMVIEWIMPL_H
#define SPECTRUMVIEWIMPL_H

#include "spectrum.h"

#include "ui_spectrumview.h"

/**
 * @author Carsten Niehaus
 */
class SpectrumViewImpl : public QWidget, Ui_SpectrumView
{
    Q_OBJECT

public:
    /**
     * @param parent the parent widget
     */
    explicit SpectrumViewImpl(QWidget *parent);

    /**
     * sets the spectrum to @p spec
     * @param spec the spectrum to display
     */
    void setSpectrum(Spectrum *spec)
    {
        m_spectrumWidget->setSpectrum(spec);

        fillPeakList();

        m_spectrumWidget->update();
    }

Q_SIGNALS:
    void settingsChanged();

private Q_SLOTS:
    /**
     * set the correct ranges and min/max values of the
     * GUI elements
     * @param left The left border of the spectrum
     * @param right The right border of the spectrum
     */
    void updateUI(int left, int right);

    void updatePeakInformation(Spectrum::peak *peak);

    void setUnit();

    void updateMin(int left);
    void updateMax(int right);

private:
    /**
     * filling the list of peaks
     */
    void fillPeakList();
};

#endif // SPECTRUMVIEWIMPL_H
