/*
    SPDX-FileCopyrightText: 2004, 2005, 2006 Thomas Nagy <tnagy2^8@yahoo.fr>
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "eqchemview.h"

#include <QClipboard>
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <LineEditUrlDropEventFilter>
#else
#include <KLineEditUrlDropEventFilter>
#endif


#include <stdlib.h>

#include <config-kalzium.h>

#ifdef HAVE_FACILE
extern "C" {
char *solve_equation(const char *);
}
#else
char *solve_equation(const char *)
{
    return NULL;
}
#endif

void EQChemDialog::compute()
{
    QString equation(ui.lineEdit->text());
    equation.replace(QLatin1String("->"), QLatin1String(" -> "));
    equation.append(' ');
    equation.prepend(' ');

    char *result = solve_equation(equation.toLatin1().constData());

    QString answer = QString(result);

    qDebug() << "Answer: " << answer;

    ui.answer_label->setText(answer);

    // mem leak ?
    free(result);
}

EQChemDialog::EQChemDialog(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    LineEditUrlDropEventFilter *dropUrlEventFilter = new LineEditUrlDropEventFilter(parent);
    dropUrlEventFilter->installEventFilter(ui.lineEdit);
#else
    KLineEditUrlDropEventFilter *dropUrlEventFilter = new KLineEditUrlDropEventFilter(parent);
    dropUrlEventFilter->installEventFilter(ui.lineEdit);
#endif
    ui.lblHelp->setText(getHelpText());

    connect(ui.calculateButton, &QAbstractButton::clicked, this, &EQChemDialog::compute);
    connect(ui.btnCopy, &QAbstractButton::clicked, this, &EQChemDialog::copyAnswer);
}

void EQChemDialog::copyAnswer()
{
    qDebug() << "EQChemDialog::copyAnswer()";
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui.answer_label->text(), QClipboard::Clipboard);
}

const QString EQChemDialog::getHelpText()
{
    return i18nc("Help text for the chemical equation solver",
                 "The equation solver allows you to balance a chemical equation.<br> "
                 "<br>"
                 "<b>Using Variables</b><br>"
                 "To express variable quantities of an element, put a single character in front "
                 "of the element's symbol, as shown in this example:<br>"
                 "<i>aH + bO -> 5H2O</i> (Result: <b>10</b> H + <b>5</b> O -&gt; <b>5</b> H<sub>2</sub>O)<br>"
                 "Solving this expression will give you the needed amount of Hydrogen and Oxygen.<br>"
                 "<br>"
                 "<b>Defining electric charges</b><br>"
                 "Use box brackets to specify the electric charge of an element, as shown in this example:<br>"
                 "<i>4H[+] + 2O -> cH2O[2+]</i> (Result: <b>4</b> H<b><sup>+</sup></b> + <b>2</b> O -&gt; <b>2</b> H<b><sub>2</sub></b>O<b><sup>2+</sub></b>)");
}

QSize EQChemDialog::sizeHint() const
{
    QSize size;
    size.setWidth(200);
    return size;
}

#include "moc_eqchemview.cpp"
