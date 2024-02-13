/*
    SPDX-FileCopyrightText: 2011 Rebetez Etienne <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUMUNITCOMBOBOX_H
#define KALZIUMUNITCOMBOBOX_H

#include <QComboBox>

class KalziumUnitCombobox : public QComboBox
{
public:
    explicit KalziumUnitCombobox(QWidget *parent = nullptr);
    explicit KalziumUnitCombobox(const QList<int> &unitList, QWidget *parent = nullptr);

    void setUnitList(const QList<int> &unitList);
    int getCurrentUnitId() const;
    void setIndexWithUnitId(int unit);
};

#endif // KALZIUMUNITCOMBOBOX_H
