/*
    SPDX-FileCopyrightText: 2010 Luca Tringali <TRINGALINVENT@libero.it>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "titrationCalculator.h"

#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include <QFileDialog>
#include <QMessageBox>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QScriptClass>
#include <QScriptEngine>
#include <QScriptValue>
#endif
#include <QVarLengthArray>

#include <KLocalizedString>

#include "prefs.h"

using namespace std;

titrationCalculator::titrationCalculator(QWidget *parent)
    : QWidget(parent)
{
    xmin = 0;
    xmax = 50;
    ymin = 0;
    ymax = 50;
    width = int(xmax - xmin);

    uid.setupUi(this);
    uid.tabWidget->setTabText(0, i18n("Experimental values"));
    uid.tabWidget->setTabText(1, i18n("Theoretical equations"));
    uid.tab->setFocus();
    plot();

    connect(uid.pushButton, &QAbstractButton::clicked, this, &titrationCalculator::on_pushButton_clicked);
    connect(uid.xmin, SIGNAL(valueChanged(double)), this, SLOT(on_xmin_valueChanged(double)));
    connect(uid.xmax, SIGNAL(valueChanged(double)), this, SLOT(on_xmax_valueChanged(double)));
    connect(uid.ymin, SIGNAL(valueChanged(double)), this, SLOT(on_ymin_valueChanged(double)));
    connect(uid.ymax, SIGNAL(valueChanged(double)), this, SLOT(on_ymax_valueChanged(double)));

    connect(uid.saveimage, &QAbstractButton::clicked, this, &titrationCalculator::on_actionSave_image_triggered);
    connect(uid.open, &QAbstractButton::clicked, this, &titrationCalculator::on_actionOpen_triggered);
    connect(uid.save, &QAbstractButton::clicked, this, &titrationCalculator::on_actionSave_triggered);
    connect(uid.newfile, &QAbstractButton::clicked, this, &titrationCalculator::on_actionNew_triggered);
    connect(uid.rapidhelp, &QAbstractButton::clicked, this, &titrationCalculator::on_actionRapid_Help_triggered);
}

titrationCalculator::~titrationCalculator() = default;

void titrationCalculator::plot()
{
    width = int(xmax - xmin);
    // now I'm preparing the kplot widget
    uid.kplotwidget->removeAllPlotObjects();
    uid.kplotwidget->setLimits(xmin, xmax, ymin, ymax); // now I need to set the limits of the plot

    auto kpor = new KPlotObject(Qt::red, KPlotObject::Lines);
    auto kpog = new KPlotObject(Qt::green, KPlotObject::Lines);
    auto kpob = new KPlotObject(Qt::blue, KPlotObject::Lines);
    redplot = QStringLiteral("<polyline points=\"");
    greenplot = QStringLiteral("<polyline points=\"");
    blueplot = QStringLiteral("<polyline points=\"");

    if (!uid.tableWidget->item(0, 0) || uid.tableWidget->item(0, 0)->text().isEmpty()) {
        // go on
    } else {
        char yvalue[80];
        int tmpy = 0;
        for (int i = 0; i < uid.tableWidget->rowCount(); ++i) {
            if (!uid.tableWidget->item(i, 0) || uid.tableWidget->item(i, 0)->text().isEmpty()) {
                break;
            } else {
                if (uid.tableWidget->item(i, 0)->data(Qt::DisplayRole).toString() == uid.yaxis->text()) {
                    QString yvalueq = uid.tableWidget->item(i, 1)->data(Qt::DisplayRole).toString();
                    QByteArray ba = yvalueq.toLatin1();
                    char *yvaluen = ba.data();
                    strcpy(yvalue, yvaluen);
                    tmpy = 1;
                }
            }
        }
        QString mreporto;
        int iter = 0;
        if (uid.xaxis->text().isEmpty() || uid.xaxis->text() == QLatin1String(" ")) {
            uid.xaxis->setText(i18n("nothing"));
        }
        if (tmpy == 0) {
            QMessageBox::critical(this, i18n("Error"), i18n("Unable to find an equation for Y-axis variable."));
        } else {
            // now we have to solve the system of equations NOTE:yvalue contains the equation of Y-axis variable
            // we iterates the process until you have an equation in one only unknown variable or a numeric expression
            mreporto = solve(yvalue);
            while (end == 0 || lettere == 1) {
                QByteArray ba = mreporto.toLatin1();
                char *tmreport = ba.data();
                ++iter;
                if (end == 1 || lettere == 0) {
                    break;
                }
                if (iter > 100) {
                    break; // preventing from an endless iteration
                }
                mreporto = solve(tmreport);
            }
        }
        // if (mreporto!="") uid.note->setText("Theoretical Curve: "+mreporto);
        if (!mreporto.isEmpty()) {
            uid.note->setText(i18n("Theoretical curve") + ": " + mreporto);
            for (int i = int(xmin); i < int(xmax); ++i) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                double id = i;
                QScriptEngine myEngine;
                QByteArray ban = mreporto.toLatin1();
                char *tmreporto = ban.data();

                QString istr;
                istr.append(QStringLiteral("%1").arg((id)));
                // now i'm using QScript language to solve the expression
                // in a future we can consider to change it supporting some backends, but it's really complex
                QString myscript = solvex(tmreporto, istr);
                QScriptValue three = myEngine.evaluate(myscript);

                double tvalue = three.toNumber();
                kpor->addPoint(id, tvalue);
                redplot = redplot + ' ' + QString::number((id * 10) + 5).replace(QChar(','), QChar('.')) + ','
                    + QString::number((ymax - tvalue) * 10).replace(QChar(','), QChar('.'));
#else
#pragma "NEED TO PORT QTSCRIPT";
#endif
            }
        }
        temponu = 0;
    } // here ends the equations mode

    // uid.tableWidget_2->sortItems(1, Qt::AscendingOrder); //seems that the sorting doesn't work correctly
    if (!uid.tableWidget_2->item(0, 0) || uid.tableWidget_2->item(0, 0)->text().isEmpty()) {
        // go on
    } else {
        // now we can plot the values
        double a, b, c, d, xval;
        QVarLengthArray<double, 64> px(uid.tableWidget_2->rowCount());
        QVarLengthArray<double, 64> py(uid.tableWidget_2->rowCount());
        int totaldata = 0;
        for (int i = 0; i < uid.tableWidget_2->rowCount(); ++i) {
            if (!uid.tableWidget_2->item(i, 0) || uid.tableWidget_2->item(i, 0)->text().isEmpty()) {
                break;
            } else {
                ++totaldata;
                kpob->addPoint(uid.tableWidget_2->item(i, 1)->data(Qt::DisplayRole).toDouble(),
                               uid.tableWidget_2->item(i, 0)->data(Qt::DisplayRole).toDouble());
                py[i] = uid.tableWidget_2->item(i, 0)->data(Qt::DisplayRole).toDouble();
                px[i] = uid.tableWidget_2->item(i, 1)->data(Qt::DisplayRole).toDouble();
                blueplot = blueplot + ' '
                    + QString::number((uid.tableWidget_2->item(i, 1)->data(Qt::DisplayRole).toDouble() * 10) + 5).replace(QChar(','), QChar('.')) + ','
                    + QString::number((ymax - uid.tableWidget_2->item(i, 0)->data(Qt::DisplayRole).toDouble()) * 10).replace(QChar(','), QChar('.'));
            }
        }
        a = py[totaldata - 1] - py[0];
        b = 4 / (px[totaldata - 1] - px[0]);
        d = 0;
        if (a > 0) {
            d = py[0] + (a / 2);
        } else if (a < 0) {
            d = py[totaldata - 1] - (a / 2);
        }
        double cn = 0.0;
        int th = 0;
        for (int i = 1; i < (totaldata - 1); ++i) {
            // now i'm using the value of the points to fit the curve
            double ci = ((setttanh((py[i] - d) / a)) / b) - px[i];
            if ((ci * 0) == 0) {
                cn = cn + ci;
                ++th;
            }
        }
        // c = cn/(th); it doesn't wok, but i found out this little hack. The strange thing is that in the standalone application it works fine.
        c = cn / (th * 2);
        // THIS IS THE PLOT OF APPROXIMATED CURVE
        for (int i = int(xmin); i < (int(xmax)); ++i) {
            double id = i;
            xval = (a * tanh(b * (id + c))) + d;
            kpog->addPoint(id, xval);
            greenplot = greenplot + ' ' + QString::number((id * 10) + 5).replace(QChar(','), QChar('.')) + ','
                + QString::number((ymax - xval) * 10).replace(QChar(','), QChar('.'));
        }
        // THIS IS THE EQUIVALENCE POINT (THE INFLECTION OF THE CURVE)
        QString es = QString::number(-c);
        QString as = QString::number(a);
        QString bs = QString::number(b);
        QString cs = QString::number(c);
        QString ds = QString::number(d);
        QString tempon = uid.note->toPlainText() + QChar('\n');
        if (temponu != 0) {
            tempon = QLatin1String("");
        }
        uid.note->setText(tempon + '\n' + i18n("Approximated curve") + ": " + as + "*tanh(" + bs + "*(x+" + cs + "))+" + ds + '\n' + i18n("Equivalence point")
                          + ": " + es);
    } // here ends the experimental values mode

    uid.kplotwidget->addPlotObject(kpor);
    uid.kplotwidget->addPlotObject(kpog);
    uid.kplotwidget->addPlotObject(kpob);

    redplot = redplot + R"(" style="stroke:red;fill:none"/> )";
    blueplot = blueplot + R"(" style="stroke:blue;fill:none"/> )";
    greenplot = greenplot + R"(" style="stroke:green;fill:none"/> )";
}

double titrationCalculator::setttanh(double x)
{
    double temp;
    temp = log((1 + x) / (1 - x)) / 2;
    return temp;
}

QString titrationCalculator::solve(char *yvalue)
{
    QString mreport;
    lettere = 0;
    // now we have to solve the system of equations
    // yvalue contains the equation of Y-axis variable
    QString tempy = QLatin1String("");
    end = 1;
    mreport = QLatin1String("");
    QString tempyval;
    QString ptem;
    for (int i = 0; strlen(yvalue) + 1; ++i) {
        if (!(yvalue[i] == 'q' || yvalue[i] == 'w' || yvalue[i] == 'e' || yvalue[i] == 'r' || yvalue[i] == 't' || yvalue[i] == 'y' || yvalue[i] == 'u'
              || yvalue[i] == 'i' || yvalue[i] == 'o' || yvalue[i] == 'p' || yvalue[i] == 'a' || yvalue[i] == 's' || yvalue[i] == 'd' || yvalue[i] == 'f'
              || yvalue[i] == 'g' || yvalue[i] == 'h' || yvalue[i] == 'j' || yvalue[i] == 'k' || yvalue[i] == 'l' || yvalue[i] == 'z' || yvalue[i] == 'x'
              || yvalue[i] == 'c' || yvalue[i] == 'v' || yvalue[i] == 'b' || yvalue[i] == 'n' || yvalue[i] == 'm' || yvalue[i] == '+' || yvalue[i] == '-'
              || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '(' || yvalue[i] == ')' || yvalue[i] == 'Q' || yvalue[i] == 'W'
              || yvalue[i] == 'E' || yvalue[i] == 'R' || yvalue[i] == 'T' || yvalue[i] == 'Y' || yvalue[i] == 'U' || yvalue[i] == 'I' || yvalue[i] == 'O'
              || yvalue[i] == 'P' || yvalue[i] == 'A' || yvalue[i] == 'S' || yvalue[i] == 'D' || yvalue[i] == 'F' || yvalue[i] == 'G' || yvalue[i] == 'H'
              || yvalue[i] == 'J' || yvalue[i] == 'K' || yvalue[i] == 'L' || yvalue[i] == 'Z' || yvalue[i] == 'X' || yvalue[i] == 'C' || yvalue[i] == 'V'
              || yvalue[i] == 'B' || yvalue[i] == 'N' || yvalue[i] == 'M' || yvalue[i] == '1' || yvalue[i] == '2' || yvalue[i] == '3' || yvalue[i] == '4'
              || yvalue[i] == '5' || yvalue[i] == '6' || yvalue[i] == '7' || yvalue[i] == '8' || yvalue[i] == '9' || yvalue[i] == '0' || yvalue[i] == '.'
              || yvalue[i] == ',')) {
            break; // if current value is not a permitted value, this means that something is wrong
        }
        if (yvalue[i] == 'q' || yvalue[i] == 'w' || yvalue[i] == 'e' || yvalue[i] == 'r' || yvalue[i] == 't' || yvalue[i] == 'y' || yvalue[i] == 'u'
            || yvalue[i] == 'i' || yvalue[i] == 'o' || yvalue[i] == 'p' || yvalue[i] == 'a' || yvalue[i] == 's' || yvalue[i] == 'd' || yvalue[i] == 'f'
            || yvalue[i] == 'g' || yvalue[i] == 'h' || yvalue[i] == 'j' || yvalue[i] == 'k' || yvalue[i] == 'l' || yvalue[i] == 'z' || yvalue[i] == 'x'
            || yvalue[i] == 'c' || yvalue[i] == 'v' || yvalue[i] == 'b' || yvalue[i] == 'n' || yvalue[i] == 'm' || yvalue[i] == 'Q' || yvalue[i] == 'W'
            || yvalue[i] == 'E' || yvalue[i] == 'R' || yvalue[i] == 'T' || yvalue[i] == 'Y' || yvalue[i] == 'U' || yvalue[i] == 'I' || yvalue[i] == 'O'
            || yvalue[i] == 'P' || yvalue[i] == 'A' || yvalue[i] == 'S' || yvalue[i] == 'D' || yvalue[i] == 'F' || yvalue[i] == 'G' || yvalue[i] == 'H'
            || yvalue[i] == 'J' || yvalue[i] == 'K' || yvalue[i] == 'L' || yvalue[i] == 'Z' || yvalue[i] == 'X' || yvalue[i] == 'C' || yvalue[i] == 'V'
            || yvalue[i] == 'B' || yvalue[i] == 'N' || yvalue[i] == 'M' || yvalue[i] == '.' || yvalue[i] == ',') {
            lettere = 1; // if lettere == 0 then the equation contains only mnumbers
        }
        if (yvalue[i] == '+' || yvalue[i] == '-' || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '(' || yvalue[i] == ')'
            || yvalue[i] == '1' || yvalue[i] == '2' || yvalue[i] == '3' || yvalue[i] == '4' || yvalue[i] == '5' || yvalue[i] == '6' || yvalue[i] == '7'
            || yvalue[i] == '8' || yvalue[i] == '9' || yvalue[i] == '0' || yvalue[i] == '.' || yvalue[i] == ',') {
            tempyval = tempyval + QString(yvalue[i]);
        } else {
            tempy = tempy + QString(yvalue[i]);
            for (int i = 0; i < uid.tableWidget->rowCount(); ++i) {
                QTableWidgetItem *titem = uid.tableWidget->item(i, 0);
                QTableWidgetItem *titemo = uid.tableWidget->item(i, 1);
                if (!titem || titem->text().isEmpty()) {
                    break;
                } else {
                    if (tempy == uid.xaxis->text()) {
                        tempyval = uid.xaxis->text();
                        tempy = QLatin1String("");
                    }
                    if (titem->data(Qt::DisplayRole).toString() == tempy) {
                        QString yvaluerq = titemo->data(Qt::DisplayRole).toString();
                        QByteArray ba = yvaluerq.toLatin1();
                        char *yvalure = ba.data();
                        tempyval = QChar('(') + QString(yvalure) + QChar(')');
                        tempy = QLatin1String("");
                        end = 1;
                    }
                    if (tempy != uid.xaxis->text()) {
                        if (yvalue[i] == '+' || yvalue[i] == '-' || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '('
                            || yvalue[i] == ')' || yvalue[i] == '1' || yvalue[i] == '2' || yvalue[i] == '3' || yvalue[i] == '4' || yvalue[i] == '5'
                            || yvalue[i] == '6' || yvalue[i] == '7' || yvalue[i] == '8' || yvalue[i] == '9' || yvalue[i] == '0' || yvalue[i] == '.'
                            || yvalue[i] == ',') {
                            // actually nothing
                        } else {
                            end = 0;
                        }
                    }
                }
            }
        } // simbol end
        if (!tempyval.isEmpty()) {
            mreport = mreport + tempyval;
        }
        tempyval = QLatin1String("");
    }
    return mreport;
}

QString titrationCalculator::solvex(char *yvalue, const QString &dnum)
{
    QString mreport = QLatin1String("");
    lettere = 0;
    // now we have to solve the system of equations
    // yvalue contains the equation of Y-axis variable
    // Remember that the function to elevate to power is Math.pow(b,e)
    QString tempy;
    QString tempyold;
    QString tempyolda = QLatin1String("");
    int olda = 0;
    end = 1;
    QString tempyval;
    tempy = QLatin1String("");
    for (int i = 0; strlen(yvalue) + 1; ++i) {
        if (!(yvalue[i] == 'q' || yvalue[i] == 'w' || yvalue[i] == 'e' || yvalue[i] == 'r' || yvalue[i] == 't' || yvalue[i] == 'y' || yvalue[i] == 'u'
              || yvalue[i] == 'i' || yvalue[i] == 'o' || yvalue[i] == 'p' || yvalue[i] == 'a' || yvalue[i] == 's' || yvalue[i] == 'd' || yvalue[i] == 'f'
              || yvalue[i] == 'g' || yvalue[i] == 'h' || yvalue[i] == 'j' || yvalue[i] == 'k' || yvalue[i] == 'l' || yvalue[i] == 'z' || yvalue[i] == 'x'
              || yvalue[i] == 'c' || yvalue[i] == 'v' || yvalue[i] == 'b' || yvalue[i] == 'n' || yvalue[i] == 'm' || yvalue[i] == '+' || yvalue[i] == '-'
              || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '(' || yvalue[i] == ')' || yvalue[i] == 'Q' || yvalue[i] == 'W'
              || yvalue[i] == 'E' || yvalue[i] == 'R' || yvalue[i] == 'T' || yvalue[i] == 'Y' || yvalue[i] == 'U' || yvalue[i] == 'I' || yvalue[i] == 'O'
              || yvalue[i] == 'P' || yvalue[i] == 'A' || yvalue[i] == 'S' || yvalue[i] == 'D' || yvalue[i] == 'F' || yvalue[i] == 'G' || yvalue[i] == 'H'
              || yvalue[i] == 'J' || yvalue[i] == 'K' || yvalue[i] == 'L' || yvalue[i] == 'Z' || yvalue[i] == 'X' || yvalue[i] == 'C' || yvalue[i] == 'V'
              || yvalue[i] == 'B' || yvalue[i] == 'N' || yvalue[i] == 'M' || yvalue[i] == '1' || yvalue[i] == '2' || yvalue[i] == '3' || yvalue[i] == '4'
              || yvalue[i] == '5' || yvalue[i] == '6' || yvalue[i] == '7' || yvalue[i] == '8' || yvalue[i] == '9' || yvalue[i] == '0' || yvalue[i] == '.'
              || yvalue[i] == ',')) {
            break; // if current value is not a permitted value, this means that something is wrong
        }
        if (yvalue[i] == 'q' || yvalue[i] == 'w' || yvalue[i] == 'e' || yvalue[i] == 'r' || yvalue[i] == 't' || yvalue[i] == 'y' || yvalue[i] == 'u'
            || yvalue[i] == 'i' || yvalue[i] == 'o' || yvalue[i] == 'p' || yvalue[i] == 'a' || yvalue[i] == 's' || yvalue[i] == 'd' || yvalue[i] == 'f'
            || yvalue[i] == 'g' || yvalue[i] == 'h' || yvalue[i] == 'j' || yvalue[i] == 'k' || yvalue[i] == 'l' || yvalue[i] == 'z' || yvalue[i] == 'x'
            || yvalue[i] == 'c' || yvalue[i] == 'v' || yvalue[i] == 'b' || yvalue[i] == 'n' || yvalue[i] == 'm' || yvalue[i] == 'Q' || yvalue[i] == 'W'
            || yvalue[i] == 'E' || yvalue[i] == 'R' || yvalue[i] == 'T' || yvalue[i] == 'Y' || yvalue[i] == 'U' || yvalue[i] == 'I' || yvalue[i] == 'O'
            || yvalue[i] == 'P' || yvalue[i] == 'A' || yvalue[i] == 'S' || yvalue[i] == 'D' || yvalue[i] == 'F' || yvalue[i] == 'G' || yvalue[i] == 'H'
            || yvalue[i] == 'J' || yvalue[i] == 'K' || yvalue[i] == 'L' || yvalue[i] == 'Z' || yvalue[i] == 'X' || yvalue[i] == 'C' || yvalue[i] == 'V'
            || yvalue[i] == 'B' || yvalue[i] == 'N' || yvalue[i] == 'M' || yvalue[i] == '.' || yvalue[i] == ',') {
            tempy = tempy + yvalue[i]; // if lettere == 0 then the equation contains only mnumbers
        }
        if (yvalue[i] == '+' || yvalue[i] == '-' || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '(' || yvalue[i] == ')'
            || yvalue[i] == '1' || yvalue[i] == '2' || yvalue[i] == '3' || yvalue[i] == '4' || yvalue[i] == '5' || yvalue[i] == '6' || yvalue[i] == '7'
            || yvalue[i] == '8' || yvalue[i] == '9' || yvalue[i] == '0' || yvalue[i] == '.' || yvalue[i] == ',') {
            if (!tempyolda.isEmpty()) {
                tempy = tempy + yvalue[i];
                if (tempyolda == uid.xaxis->text()) {
                    tempyolda = dnum;
                }
                tempyval = tempyval + QStringLiteral("Math.pow(") + tempyolda + QChar(',') + tempy + QChar(')');
                tempyolda = QLatin1String("");
                tempyold = QLatin1String("");
                olda = 1;
            }
            if (yvalue[i] == '^') {
                tempyolda = tempyold;
            } else {
                tempyold = QLatin1String("");
                if (((olda != 1) && (yvalue[i + 1] != '^'))
                    || (yvalue[i] == '+' || yvalue[i] == '-' || yvalue[i] == '^' || yvalue[i] == '*' || yvalue[i] == '/' || yvalue[i] == '('
                        || yvalue[i] == ')')) {
                    tempyval = tempyval + QString(yvalue[i]);
                }
            }

        } else {
            if (!tempyolda.isEmpty()) {
                tempyval = tempyval + QStringLiteral("Math.pow(") + tempyolda + QChar(',') + tempy + QChar(')');
                tempyolda = QLatin1String("");
                tempyold = QLatin1String("");
                olda = 1;
            }
            if ((tempy == uid.xaxis->text()) && (!tempyolda.isEmpty())) {
                if (yvalue[i + 1] != '^') {
                    tempyval = tempyval + dnum;
                }
                tempyold = tempy;
                tempy = QLatin1String("");
            }
        } // simbol end
        if (!tempyval.isEmpty()) {
            mreport = mreport + tempyval;
        }
        tempyval = QLatin1String("");
    }
    // QMessageBox::information(this, "report", mreport);
    return mreport;
}

void titrationCalculator::on_xmin_valueChanged(double val)
{
    xmin = val;
    on_pushButton_clicked(); // please take note that calling directly the plot() function will give a wrong value for equivalence point
}

void titrationCalculator::on_xmax_valueChanged(double val)
{
    xmax = val;
    on_pushButton_clicked(); // please take note that calling directly the plot() function will give a wrong value for equivalence point
}

void titrationCalculator::on_ymin_valueChanged(double val)
{
    ymin = val;
    on_pushButton_clicked(); // please take note that calling directly the plot() function will give a wrong value for equivalence point
}

void titrationCalculator::on_ymax_valueChanged(double val)
{
    ymax = val;
    on_pushButton_clicked(); // please take note that calling directly the plot() function will give a wrong value for equivalence point
}

void titrationCalculator::on_pushButton_clicked()
{
    plot();
}

void titrationCalculator::on_actionRapid_Help_triggered()
{
    on_actionNew_triggered();
    // now I'm going to fill the tables with the example values

    // table1
    QTableWidgetItem *titemo = uid.tableWidget->item(0, 0);
    titemo->setText(QStringLiteral("A"));
    titemo = uid.tableWidget->item(0, 1);
    titemo->setText(QStringLiteral("(C*D)/(B*K)"));
    titemo = uid.tableWidget->item(1, 0);
    titemo->setText(QStringLiteral("K"));
    titemo = uid.tableWidget->item(1, 1);
    titemo->setText(QStringLiteral("10^-3"));
    titemo = uid.tableWidget->item(2, 0);
    titemo->setText(QStringLiteral("C"));
    titemo = uid.tableWidget->item(2, 1);
    titemo->setText(QStringLiteral("OH"));
    titemo = uid.tableWidget->item(3, 0);
    titemo->setText(QStringLiteral("OH"));
    titemo = uid.tableWidget->item(3, 1);
    titemo->setText(QStringLiteral("(10^-14)/H"));
    titemo = uid.tableWidget->item(4, 0);
    titemo->setText(QStringLiteral("H"));
    titemo = uid.tableWidget->item(4, 1);
    titemo->setText(QStringLiteral("10^-4"));
    titemo = uid.tableWidget->item(5, 0);
    titemo->setText(QStringLiteral("B"));
    titemo = uid.tableWidget->item(5, 1);
    titemo->setText(QStringLiteral("6*(10^-2)"));
    // xaxis
    uid.xaxis->setText(QStringLiteral("D"));
    // yaxis
    uid.yaxis->setText(QStringLiteral("A"));
    // table2
    titemo = uid.tableWidget_2->item(0, 0);
    titemo->setText(QStringLiteral("7,19"));
    titemo = uid.tableWidget_2->item(0, 1);
    titemo->setText(QStringLiteral("30"));
    titemo = uid.tableWidget_2->item(1, 0);
    titemo->setText(QStringLiteral("7,64"));
    titemo = uid.tableWidget_2->item(1, 1);
    titemo->setText(QStringLiteral("30,5"));
    titemo = uid.tableWidget_2->item(2, 0);
    titemo->setText(QStringLiteral("10,02"));
    titemo = uid.tableWidget_2->item(2, 1);
    titemo->setText(QStringLiteral("31"));
    titemo = uid.tableWidget_2->item(3, 0);
    titemo->setText(QStringLiteral("10,45"));
    titemo = uid.tableWidget_2->item(3, 1);
    titemo->setText(QStringLiteral("31,5"));

    // I think it's better if I don't give so much information here.
    //  This information could be included into kalzium help, but I don't know how to do
    // QMessageBox::information(this, "IceeQt Rapid Help", "There are two ways to use IceeQt:\n\nTheoretical Equations\n Here you can fill the table with the
    // equations you have previously obtained for the chemical equilibria. FOR EXAMPLE if you have this reaction A + B -> C + D then you will have the equation
    // K=(C*D)/(A*B) so you must write 'K' in the Parameter column and '(C*D)/(A*B)' in the Value column. If you want to assign a known value to a parameter you
    // can simply write the numeric value in the Value field. FOR EXAMPLE you can use the system \nA=(C*D)/(B*K) \nK=10^-3 \nC=OH \nOH=(10^-14)/H \nH=10^-4
    // \nB=6*(10^-2) \nThen you have to write D as X axis and A as Y axis: so you will find out how the concentration of A change in function of D
    // concentration.\nPlease don't use parenthesis for exponents: 10^-3 is correct, while 10^(-3) is wrong. \n\nExperimental Values\n You can use this program
    // to draw the plot of your experimental data obtained during a titration and find out the volume of equivalence. It's strongly recommended to insert a even
    // number of points, because of the best fit algorithm, sorted by volume (the X axis value).\n\nPlot\n The plot shows in red the curve that comes from
    // theoretical equations, in blue the experimental points, and in green the approximated curve for experimental points.");
}

/*
void titrationCalculator::on_actionAbout_triggered()
{
    QMessageBox::information(this, "IceeQt About", "I\nCompute\nEquilibria\nExactly\n\nIceeQt is a program for computing chemical equilibria in a easy way. The
first version of Icee was written by Gabriele Balducci(University of Trieste, Italy) using matheval, gnuplot, and tk. This version, called IceeQt, was written
by Luca Tringali using Qtopia. \n IceeQt is installable on every system supported by Qt: Windows, MacOS, GNU/Linux, FreeBSD, Solaris, Symbian, etc..\n This
program is released under GPL3 licence.\n\nThe website is http://web.archive.org/web/20041207065103/http://www.dsch.units.it/~balducci/lca1/");
}
*/
void titrationCalculator::on_actionNew_triggered()
{
    // set all the table cells as empty ("")
    for (int i = 0; i < uid.tableWidget->rowCount(); ++i) {
        auto titem = new QTableWidgetItem;
        titem->setText(QLatin1String(""));
        uid.tableWidget->setItem(i, 0, titem);
        auto titemo = new QTableWidgetItem;
        titemo->setText(QLatin1String(""));
        uid.tableWidget->setItem(i, 1, titemo);
    }
    uid.xaxis->setText(QLatin1String(""));
    uid.yaxis->setText(QLatin1String(""));
    for (int i = 0; i < uid.tableWidget_2->rowCount(); ++i) {
        auto titem = new QTableWidgetItem;
        titem->setText(QLatin1String(""));
        uid.tableWidget_2->setItem(i, 0, titem);
        auto titemo = new QTableWidgetItem;
        titemo->setText(QLatin1String(""));
        uid.tableWidget_2->setItem(i, 1, titemo);
    }
    uid.note->setText(QLatin1String(""));
}

void titrationCalculator::on_actionSave_triggered()
{
    // save all the cells values
    //   if we have for example:
    //   table1:
    //   |a|f|
    //   |d|h|
    //   table2:
    //   |w|q|
    //   |h|l|
    //   then the file would be:
    //   table1|
    //   a|
    //   f|
    //   d|
    //   h|
    //   table2|
    //   w|
    //   q|
    //   h|
    //   l|
    //   note|
    //    ewewewww|
    //    as you can see we don't save also the empty cells, this is obvious.

    QString tempyval;
    tempyval = QStringLiteral("table1|");
    for (int i = 0; i < uid.tableWidget->rowCount(); ++i) {
        QTableWidgetItem *titem = uid.tableWidget->item(i, 0);
        QTableWidgetItem *titemo = uid.tableWidget->item(i, 1);
        if (!titem || titem->text().isEmpty()) {
            break;
        } else {
            QString yvaluerq = titemo->data(Qt::DisplayRole).toString();
            QString valuerq = titem->data(Qt::DisplayRole).toString();
            tempyval = tempyval + QChar('\n') + valuerq + QStringLiteral("|\n") + yvaluerq + QChar('|');
        }
    }
    tempyval = tempyval + QStringLiteral("\nxaxis|");
    tempyval = tempyval + QStringLiteral("\n") + uid.xaxis->text() + QChar('|');
    tempyval = tempyval + QStringLiteral("\nyaxis|");
    tempyval = tempyval + QStringLiteral("\n") + uid.yaxis->text() + QChar('|');
    tempyval = tempyval + QStringLiteral("\ntable2|");
    for (int i = 0; i < uid.tableWidget_2->rowCount(); ++i) {
        QTableWidgetItem *titem = uid.tableWidget_2->item(i, 0);
        QTableWidgetItem *titemo = uid.tableWidget_2->item(i, 1);
        if (!titem || titem->text().isEmpty()) {
            break;
        } else {
            QString yvaluerq = titemo->data(Qt::DisplayRole).toString();
            QString valuerq = titem->data(Qt::DisplayRole).toString();
            tempyval = tempyval + QChar('\n') + valuerq + QStringLiteral("|\n") + yvaluerq + QChar('|');
        }
    }
    tempyval = tempyval + QStringLiteral("\nnote|\n") + uid.note->toPlainText() + QChar('|');

    QString file = QFileDialog::getSaveFileName(this, i18n("Save work"), QLatin1String(""), i18n("Icee File (*.icee)"));
    if (!file.isEmpty()) {
        QByteArray ba = tempyval.toLatin1();
        char *strsave = ba.data();
        QByteArray bac = file.toLatin1();
        char *filec = bac.data();

        ofstream out(filec);
        cout << "|";
        cout << filec;
        cout << "|";
        if (!out) {
            QMessageBox::critical(this, i18n("Error"), i18n("Unable to create %1", file));
        }
        out << strsave;
        out.close();
        // if (out) QMessageBox::information(this, "Information", "File " + file + " successfully saved.");
    }
}

void titrationCalculator::on_actionOpen_triggered()
{
    // loads all the cells text from a file previously saved
    QString file = QFileDialog::getOpenFileName(this, i18n("Open work"), QLatin1String(""), i18n("Icee File (*.icee)"));
    if (!file.isEmpty()) {
        QByteArray bac = file.toLatin1();
        char *filec = bac.data();
        ifstream texto(filec);
        if (!texto) {
            QMessageBox::critical(this, i18n("Error"), i18n("Unable to open %1", file));
        }
        if (texto) {
            on_actionNew_triggered();
            QString tempyval;
            char tmpchr;
            int i = 0;
            int tablea = 0;
            int tableb = 0;
            int xax = 0;
            int yax = 0;
            int notea = 0;
            do {
                texto >> tmpchr;
                if (tmpchr != '|') {
                    tempyval = tempyval + tmpchr;
                } else {
                    if ((tablea == 1) && (tempyval != QStringLiteral("table1")) && (tempyval != QStringLiteral("table2"))
                        && (tempyval != QStringLiteral("xaxis")) && (tempyval != QStringLiteral("yaxis")) && (tempyval != QStringLiteral("note"))) {
                        if ((i % 2) != 0) {
                            QTableWidgetItem *titemo = uid.tableWidget->item((i - 1) / 2, 1);
                            if (titemo) {
                                titemo->setText(tempyval);
                            }
                        } else {
                            QTableWidgetItem *titem = uid.tableWidget->item((i / 2), 0);
                            if (titem) {
                                titem->setText(tempyval);
                            }
                        }
                        ++i;
                    }

                    if ((tableb == 1) && (tempyval != QStringLiteral("table1")) && (tempyval != QStringLiteral("table2"))
                        && (tempyval != QStringLiteral("xaxis")) && (tempyval != QStringLiteral("yaxis")) && (tempyval != QStringLiteral("note"))) {
                        if ((i % 2) != 0) {
                            QTableWidgetItem *titemo = uid.tableWidget_2->item((i - 1) / 2, 1);
                            if (titemo) {
                                titemo->setText(tempyval);
                            }
                        } else {
                            QTableWidgetItem *titem = uid.tableWidget_2->item((i / 2), 0);
                            if (titem) {
                                titem->setText(tempyval);
                            }
                            // cout << i;
                        }
                        ++i;
                    }
                    if ((xax == 1) && (tempyval != QStringLiteral("table1")) && (tempyval != QStringLiteral("table2")) && (tempyval != QStringLiteral("xaxis"))
                        && (tempyval != QStringLiteral("yaxis")) && (tempyval != QStringLiteral("note"))) {
                        uid.xaxis->setText(tempyval);
                    }
                    if ((yax == 1) && (tempyval != QStringLiteral("table1")) && (tempyval != QStringLiteral("table2")) && (tempyval != QStringLiteral("xaxis"))
                        && (tempyval != QStringLiteral("yaxis")) && (tempyval != QStringLiteral("note"))) {
                        uid.yaxis->setText(tempyval);
                    }
                    if ((notea == 1) && (tempyval != QStringLiteral("table1")) && (tempyval != QStringLiteral("table2"))
                        && (tempyval != QStringLiteral("xaxis")) && (tempyval != QStringLiteral("yaxis")) && (tempyval != QStringLiteral("note"))) {
                        uid.note->setText(tempyval);
                    }

                    if (tempyval == QStringLiteral("table1")) {
                        i = 0;
                        tablea = 1;
                        tableb = 0;
                        xax = 0;
                        yax = 0;
                        notea = 0;
                    }
                    if (tempyval == QStringLiteral("table2")) {
                        i = 0;
                        tablea = 0;
                        tableb = 1;
                        xax = 0;
                        yax = 0;
                        notea = 0;
                    }
                    if (tempyval == QStringLiteral("xaxis")) {
                        tablea = 0;
                        tableb = 0;
                        xax = 1;
                        yax = 0;
                        notea = 0;
                    }
                    if (tempyval == QStringLiteral("yaxis")) {
                        tablea = 0;
                        tableb = 0;
                        xax = 0;
                        yax = 1;
                        notea = 0;
                    }
                    if (tempyval == QStringLiteral("note")) {
                        tablea = 0;
                        tableb = 0;
                        xax = 0;
                        yax = 0;
                        notea = 1;
                    }
                    tempyval = QLatin1String("");
                }
            } while (!texto.eof());
            texto.close();
        }
    }
}

void titrationCalculator::on_actionSave_image_triggered()
{
    // This function saves the plot into a SVG file
    QString svgheader =
        R"(<?xml version="1.0" encoding="iso-8859-1" standalone="no"?> <!DOCTYPE svg PUBLIC "-//W3C//Dtd SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/Dtd/svg11.dtd"> <svg width=")"
        + QString::number((xmax * 10) + 5) + "\" height=\"" + QString::number((ymax * 10) + 5)
        + R"("  version="1.1" xmlns="http://www.w3.org/2000/svg"><polyline points="5,)" + QString::number(ymax * 10) + " " + QString::number((xmax * 10) - 5)
        + "," + QString::number(ymax * 10) + " " + QString::number((xmax * 10) - 5) + "," + QString::number((ymax * 10) - 5) + " " + QString::number(xmax * 10)
        + "," + QString::number(ymax * 10) + " " + QString::number((xmax * 10) - 5) + "," + QString::number((ymax * 10) + 5) + " "
        + QString::number((xmax * 10) - 5) + "," + QString::number(ymax * 10) + R"(" style="stroke:black;fill:none"/> <polyline points="5,)"
        + QString::number(ymax * 10) + R"( 5,5 10,5 5,0 0,5 5,5" style="stroke:black;fill:none"/> )";
    QString svgcomplete = svgheader + redplot + greenplot + blueplot + "</svg> ";

    QString file = QFileDialog::getSaveFileName(this, i18n("Save plot"), QLatin1String(""), i18n("Svg image (*.svg)"));
    if (!file.isEmpty()) {
        QByteArray svgt = svgcomplete.toLatin1();
        char *strsave = svgt.data();
        QByteArray ban = file.toLatin1();
        char *filec = ban.data();

        ofstream out(filec);
        cout << "|";
        cout << filec;
        cout << "|";
        if (!out) {
            QMessageBox::critical(this, i18n("Error"), i18n("Unable to create %1", file));
        }
        out << strsave;
        out.close();
    }
}

#include "moc_titrationCalculator.cpp"
