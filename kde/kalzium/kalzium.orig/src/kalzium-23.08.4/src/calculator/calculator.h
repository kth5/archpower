/*
    SPDX-FileCopyrightText: 2009 Kashyap R Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDialog>

#include <KActionCollection>

#include "concCalculator.h"
#include "gasCalculator.h"
#include "molcalcwidget.h"
#include "nuclearCalculator.h"
#include "titrationCalculator.h"
#include "ui_calculator.h"

#include <config-kalzium.h>
#ifdef HAVE_FACILE
#include <eqchemview.h>
#endif

/**
 * This widget implements the body of the calculator widget,
 * various calculators like the nuclear Calculator will be added to this.
 *
 * @author Kashyap R Puranik
 */
class calculator : public QDialog
{
    Q_OBJECT

public:
    /*
     * The class constructor and destructor, takes in a Widget as parent
     */
    explicit calculator(QWidget *parent = nullptr); // constructor
    ~calculator() override; // destructor

private:
    Ui::calculator ui; // The user interface
    KActionCollection *m_actionCollection;

    // These are the various calculator widgets that will be added to this calculator

    nuclearCalculator *m_nuclearCalculator; // The nuclear calculator
    gasCalculator *m_gasCalculator; // The gas calculator
    concCalculator *m_concCalculator; // The concentration calculator
    titrationCalculator *m_titraCalculator; // The concentration calculator
    MolcalcWidget *m_moleCalculator; // The molecular mass calculator
#ifdef HAVE_FACILE
    EQChemDialog *m_equationBalancer; // The equation balancer
#endif
protected Q_SLOTS:
    /**
     * invoke the help of the correct chapter
     */
    virtual void slotHelp();
private Q_SLOTS:

    // occurs when an tree item is selected, opens the corresponding calculator
    void slotItemSelection(QTreeWidgetItem *item);
};

#endif // CALCULATOR_H
