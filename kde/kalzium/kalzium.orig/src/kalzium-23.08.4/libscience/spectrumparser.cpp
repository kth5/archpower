/*
    SPDX-FileCopyrightText: 2005, 2008 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spectrumparser.h"


#include "kalzium_libscience_debug.h"
#include <QList>

class SpectrumParser::Private
{
public:
    Private()
    {
    }

    ~Private()
    {
        delete currentSpectrum;
        delete currentPeak;
    }

    Spectrum *currentSpectrum = nullptr;
    Spectrum::peak *currentPeak = nullptr;

    bool inMetadata_ = false;
    bool inSpectrum_ = false;
    bool inSpectrumList_ = false;
    bool inPeakList_ = false;
    bool inPeak_ = false;

    double wavelength;
    int intensity;

    QList<Spectrum *> spectra;
};

SpectrumParser::SpectrumParser()
    : QXmlDefaultHandler()
    , d(new Private)
{
}

SpectrumParser::~SpectrumParser()
{
    delete d;
}

bool SpectrumParser::startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &attrs)
{
    if (localName == QLatin1String("spectrum")) {
        d->currentSpectrum = new Spectrum();
        d->inSpectrum_ = true;

        // now save the element of the current spectrum
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("id")) {
                currentElementID = attrs.value(i);
            }
        }

    } else if (d->inSpectrum_ && localName == QLatin1String("peakList")) {
        d->inPeakList_ = true;
    } else if (d->inSpectrum_ && d->inPeakList_ && localName == QLatin1String("peak")) {
        d->inPeak_ = true;
        for (int i = 0; i < attrs.length(); ++i) {
            if (attrs.localName(i) == QLatin1String("xValue")) {
                d->intensity = attrs.value(i).toInt();
            } else if (attrs.localName(i) == QLatin1String("yValue")) {
                d->wavelength = attrs.value(i).toDouble();
            }
        }
        d->currentPeak = new Spectrum::peak(d->wavelength, d->intensity);
    }
    return true;
}

bool SpectrumParser::endElement(const QString &, const QString &localName, const QString &)
{
    if (localName == QLatin1String("spectrum")) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        int num = currentElementID.midRef(1).toInt();
#else
        int num = QStringView(currentElementID).mid(1).toInt();
#endif
        d->currentSpectrum->setParentElementNumber(num);

        d->spectra.append(d->currentSpectrum);

        d->currentSpectrum = nullptr;
        d->inSpectrum_ = false;
    } else if (localName == QLatin1String("peakList")) {
        d->inSpectrumList_ = false;
    } else if (localName == QLatin1String("peak")) {
        // X             qCDebug(KALZIUM_LIBSCIENCE_LOG) << "in 'peak'" << " with this data: " << d->currentPeak->intensity << " (intensity)" ;
        d->currentSpectrum->addPeak(d->currentPeak);
        d->currentPeak = nullptr;
        d->inPeak_ = false;
    }
    return true;
}

bool SpectrumParser::characters(const QString &ch)
{
    Q_UNUSED(ch)
    return true;
}

QList<Spectrum *> SpectrumParser::getSpectrums() const
{
    return d->spectra;
}
