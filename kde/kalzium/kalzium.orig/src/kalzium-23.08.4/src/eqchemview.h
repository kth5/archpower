/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EQCHEMVIEW_H
#define EQCHEMVIEW_H

#include <QWidget>

#include "ui_equationview.h"

/**
 * @author Carsten Niehaus
 */
class EQChemDialog : public QWidget
{
    Q_OBJECT

public:
    /**
     * public constructor
     *
     * @param parent the parent widget
     */
    explicit EQChemDialog(QWidget *parent);

private:
    const QString getHelpText();

    Ui::EquationView ui;

public slots:
    /**
      start the computation (balancing) of the equation
      */
    void compute();

    void copyAnswer();

protected:
    QSize sizeHint() const override;
};

#endif // EQCHEMVIEW_H
