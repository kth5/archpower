/*
    SPDX-FileCopyrightText: 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spectrumviewimpl.h"

#include "kalzium_debug.h"
#include <QTableWidget>
#include <QTreeWidget>

#include <KLocalizedString>
#include <KUnitConversion/Converter>

#include "kalziumdataobject.h"
#include "prefs.h"

SpectrumViewImpl::SpectrumViewImpl(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QStringList headers = QStringList() << i18n("Wavelength") << i18n("Intensity");
    peakListTable->setHeaderLabels(headers);
    peakListTable->setRootIsDecorated(false);

    QList<int> length;
    length << KUnitConversion::Nanometer << KUnitConversion::Angstrom;
    m_lengthUnit->setUnitList(length);

    m_lengthUnit->setIndexWithUnitId(Prefs::spectrumWavelengthUnit());

    m_spectrumType->setCurrentIndex(Prefs::spectrumType());

    connect(minimumValue, SIGNAL(valueChanged(int)), this, SLOT(updateMin(int)));
    connect(maximumValue, SIGNAL(valueChanged(int)), this, SLOT(updateMax(int)));
    connect(m_spectrumWidget, &SpectrumWidget::bordersChanged, this, &SpectrumViewImpl::updateUI);
    connect(m_spectrumWidget, &SpectrumWidget::peakSelected, this, &SpectrumViewImpl::updatePeakInformation);

    connect(m_spectrumType, SIGNAL(currentIndexChanged(int)), m_spectrumWidget, SLOT(slotActivateSpectrum(int)));

    connect(btn_resetZoom, &QAbstractButton::pressed, m_spectrumWidget, &SpectrumWidget::resetSpectrum);
    connect(this, &SpectrumViewImpl::settingsChanged, m_spectrumWidget, &SpectrumWidget::resetSpectrum);

    connect(m_lengthUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnit()));

    resize(minimumSizeHint());
}

void SpectrumViewImpl::fillPeakList()
{
    peakListTable->clear();

    QList<QTreeWidgetItem *> items;

    for (int i = 0; i < m_spectrumWidget->spectrum()->peaklist().count(); ++i) {
        Spectrum::peak *peak = m_spectrumWidget->spectrum()->peaklist().at(i);

        double peakWavelength = peak->wavelengthToUnit(Prefs::spectrumWavelengthUnit());

        const QStringList row = QStringList() << QString::number(peakWavelength) << QString::number(peak->intensity);

        items.append(new QTreeWidgetItem((QTreeWidget *)nullptr, row));
    }
    peakListTable->insertTopLevelItems(0, items);
}

void SpectrumViewImpl::updateUI(int l, int r)
{
    minimumValue->setValue(l);
    maximumValue->setValue(r);
    minimumValue->setSuffix(KalziumDataObject::instance()->unitAsString(Prefs::spectrumWavelengthUnit()));
    maximumValue->setSuffix(KalziumDataObject::instance()->unitAsString(Prefs::spectrumWavelengthUnit()));
}

void SpectrumViewImpl::updatePeakInformation(Spectrum::peak *peak)
{
    double peakWavelength = peak->wavelengthToUnit(Prefs::spectrumWavelengthUnit());

    const QList<QTreeWidgetItem *> foundItems = peakListTable->findItems(QString::number(peakWavelength), Qt::MatchExactly);

    if (foundItems.isEmpty()) {
        return;
    }

    foreach (QTreeWidgetItem *item, peakListTable->selectedItems())
        item->setSelected(false);

    foundItems.first()->setSelected(true);
    peakListTable->scrollToItem(foundItems.first());
}

void SpectrumViewImpl::setUnit()
{
    Prefs::setSpectrumWavelengthUnit(m_lengthUnit->getCurrentUnitId());
    Prefs::self()->save();
    qCDebug(KALZIUM_LOG) << "Unit changed: " << m_lengthUnit->getCurrentUnitId();

    Q_EMIT settingsChanged();
    fillPeakList();
}

void SpectrumViewImpl::updateMin(int left)
{
    m_spectrumWidget->setRightBorder(maximumValue->value());
    m_spectrumWidget->setLeftBorder(left);
}

void SpectrumViewImpl::updateMax(int right)
{
    m_spectrumWidget->setLeftBorder(minimumValue->value());
    m_spectrumWidget->setRightBorder(right);
}

#include "moc_spectrumviewimpl.cpp"
