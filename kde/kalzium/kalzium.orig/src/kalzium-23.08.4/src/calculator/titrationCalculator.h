/*
    SPDX-FileCopyrightText: 2010 Luca Tringali <TRINGALINVENT@libero.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TITRATIONCALCULATOR_H
#define TITRATIONCALCULATOR_H

#include "kalzium_debug.h"
#include <QString>
#include <QStringList>

#include <KPlotObject>
#include <KPlotPoint>
#include <KPlotWidget>

#include <element.h>
#include <isotope.h>
#include <prefs.h>

#include "ui_titrationCalculator.h"

/*
 * This class implements the titration calculator: it can solve a system of chemical equilibria equations
 * and find out the equivalence point of an experimental titration.
 *
 * @author Luca Tringali
 */
class titrationCalculator : public QWidget
{
    Q_OBJECT

public:
    explicit titrationCalculator(QWidget *parent = nullptr);
    ~titrationCalculator() override;

public Q_SLOTS:
    void on_pushButton_clicked();
    void on_xmin_valueChanged(double val);
    void on_xmax_valueChanged(double val);
    void on_ymin_valueChanged(double val);
    void on_ymax_valueChanged(double val);

    void on_actionSave_image_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionNew_triggered();
    void on_actionRapid_Help_triggered();

private:
    void resize();
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int width;
    int end;
    int lettere;
    int temponu;
    double a;
    void plot();
    QString solve(char *yvalue);
    QString solvex(char *yvalue, const QString &dnum);
    QImage tempi;
    double setttanh(double x);
    QString redplot;
    QString greenplot;
    QString blueplot;

    Ui::titrationCalculator uid; // The user interface
};

#endif // TITRATIONCALCULATOR_H
