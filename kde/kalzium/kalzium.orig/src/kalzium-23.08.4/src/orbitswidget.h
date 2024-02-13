/*
    SPDX-FileCopyrightText: 2003, 2004, 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORBITSWIDGET_H
#define ORBITSWIDGET_H

#include <QLabel>
#include <QWidget>

/**
 * @brief the widget which displays the Bohr-orbit of the element
 * @author Carsten Niehaus
 */
class OrbitsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrbitsWidget(QWidget *parent = nullptr);

    void setElementNumber(int num);

private:
    /**
     * the elementnumber we are looking at
     */
    int Elemno;

    /// Label that shows the electronic configuration
    QLabel *const m_electronConf;

    QList<int> numOfElectrons;

protected Q_SLOTS:
    virtual void paintEvent(QPaintEvent *) override;
};

#endif // ORBITSWIDGET_H
