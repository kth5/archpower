/*
    SPDX-FileCopyrightText: 2011 Rebetez Etienne <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumunitcombobox.h"

#include "kalziumutils.h"

KalziumUnitCombobox::KalziumUnitCombobox(QWidget *parent)
    : QComboBox(parent)
{
}

KalziumUnitCombobox::KalziumUnitCombobox(const QList<int> &unitList, QWidget *parent)
    : QComboBox(parent)
{
    setUnitList(unitList);
}

void KalziumUnitCombobox::setUnitList(const QList<int> &unitList)
{
    KalziumUtils::populateUnitCombobox(this, unitList);
}

int KalziumUnitCombobox::getCurrentUnitId() const
{
    return itemData(currentIndex()).toInt();
}

void KalziumUnitCombobox::setIndexWithUnitId(int unit)
{
    setCurrentIndex(findData(unit));
}
