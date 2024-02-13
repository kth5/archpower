/*
    SPDX-FileCopyrightText: 2003-2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2005 Inge Wallin <inge@lysator.liu.se>
    SPDX-FileCopyrightText: 2009 Kashyap. R. Puranik <kashthealien@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "molcalcwidget.h"

// libscience
#include <element.h>

#include "kalziumdataobject.h"
#include "kalziumutils.h"
#include "prefs.h"
#include "search.h"

#include "kalzium_debug.h"
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>

#include <KLocalizedString>

MolcalcWidget::MolcalcWidget(QWidget *parent)
    : QWidget(parent)
{
    m_parser = new MoleculeParser(KalziumDataObject::instance()->ElementList);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

    ui.setupUi(this);

    connect(ui.calcButton, &QAbstractButton::clicked, this, &MolcalcWidget::slotCalculate);
    connect(ui.formulaEdit, &QLineEdit::returnPressed, this, &MolcalcWidget::slotCalculate);
    connect(m_timer, &QTimer::timeout, this, &MolcalcWidget::slotCalculate);

    ui.formulaEdit->setClearButtonEnabled(true);

    clear();
    if (!Prefs::addAlias()) {
        hideExtra();
        ui.details->show();
    }
    if (!Prefs::alias()) {
        ui.details->hide();
    }

    if (Prefs::addAlias()) {
        connect(ui.alias, &QAbstractButton::clicked, this, &MolcalcWidget::addAlias);
        QString shortForm, fullForm; // short form (symbol) and full form (expansion)
        QList<QString> shortList, fullList; // Used to store the short and full forms
        int i = 0; // loop counter

        // Search in User defined aliases.
        QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/symbols2.csv"));
        QFile file(fileName);

        // Check file validity
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCDebug(KALZIUM_LOG) << fileName << " opened";
            QTextStream in(&file);
            // Get all shortForms and fullForms in the file.
            // eg the short form and full form extracted from ("Me","CH3")
            // are (Me) and (CH3) respectively
            while (!in.atEnd()) {
                QString line = in.readLine();
                shortForm = line.section(',', 0, 0);
                shortForm.remove(QChar('\"'));
                fullForm = line.section(',', 1, 1);
                fullForm.remove(QChar('\"'));
                shortList << shortForm;
                fullList << fullForm;
            }

            int length = shortList.length();
            ui.user_defined->setRowCount(length);
            // Put all the aliases on to the table in the user interface
            for (i = 0; i < length; ++i) {
                shortForm = shortList.takeFirst();
                fullForm = fullList.takeFirst();
                ui.user_defined->setItem((int)i, 0, new QTableWidgetItem(i18n("%1", shortForm + " : " + fullForm)));
            }
        } else {
            qCDebug(KALZIUM_LOG) << fileName << " could not be opened!";
        }

        // Find the system defined aliases
        // Open the file
        fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/symbols.csv"));
        QFile file2(fileName);
        shortList.clear();
        fullList.clear();

        // Check file validity
        if (!(!file2.open(QIODevice::ReadOnly | QIODevice::Text))) {
            qCDebug(KALZIUM_LOG) << fileName << " opened";
            QTextStream in(&file2);

            // Get all shortForms and fullForms in the file.
            while (!in.atEnd()) {
                QString line = in.readLine();
                shortForm = line.section(',', 0, 0);
                shortForm.remove(QChar('\"'));
                fullForm = line.section(',', 1, 1);
                fullForm.remove(QChar('\"'));
                shortList << shortForm;
                fullList << fullForm;
            }
            int length = shortList.length();
            ui.pre_defined->setRowCount(length);

            // Put all the aliases on to the table in the user interface
            for (i = 0; i < length; ++i) {
                shortForm = shortList.takeFirst();
                fullForm = fullList.takeFirst();
                ui.pre_defined->setItem((int)i, 0, new QTableWidgetItem(i18n("%1", shortForm + " : " + fullForm)));
            }
        } else {
            qCDebug(KALZIUM_LOG) << fileName << " could not be opened!";
        }
    }
}

MolcalcWidget::~MolcalcWidget()
{
    delete m_parser;
}

void MolcalcWidget::clear()
{
    // Clear the data.
    m_mass = 0;
    m_elementMap.clear();

    // stop the selection in the periodic table
    KalziumDataObject::instance()->search()->resetSearch();

    // Clear the widgets.
    ui.resultLabel->clear();
    ui.resultMass->clear();
    ui.resultValue->hide();

    ui.resultComposition->setText(i18n("Enter a formula in the\nwidget above and\nclick on 'Calc'.\nE.g. #Et#OH"));

    ui.resultMass->setToolTip(QString());
    ui.resultComposition->setToolTip(QString());
    ui.resultLabel->setToolTip(QString());
}

void MolcalcWidget::updateUI()
{
    qCDebug(KALZIUM_LOG) << "MolcalcWidget::updateUI()";

    if (m_validInput) {
        qCDebug(KALZIUM_LOG) << "m_validInput == true";

        // The complexString stores the whole molecule like this:
        // 1 Seaborgium. Cumulative Mass: 263.119 u (39.2564 %)
        QString complexString;
        if (Prefs::alias()) {
            double mass;
            QString str;
            int i = 0; // counter
            int rows = m_elementMap.elements().count(); // number of columns
            ui.table->setRowCount(rows);

            foreach (ElementCount *count, m_elementMap.map()) {
                // Update the resultLabel
                mass = count->element()->dataAsVariant(ChemicalDataObject::mass).toDouble();

                ui.table->setItem(i, 0, new QTableWidgetItem(i18n("%1", count->element()->dataAsString(ChemicalDataObject::name))));
                ui.table->setItem(i, 1, new QTableWidgetItem(i18n("%1", count->count())));
                ui.table->setItem(i, 2, new QTableWidgetItem(i18n("%1", count->element()->dataAsString(ChemicalDataObject::mass))));
                ui.table->setItem(i, 3, new QTableWidgetItem(i18n("%1", mass * count->count())));
                ui.table->setItem(i, 4, new QTableWidgetItem(i18n("%1", mass * count->count() / m_mass * 100)));

                ++i;
            }
            // The alias list
            i = 0;
            rows = m_aliasList.count();
            ui.alias_list->setRowCount(rows);
            foreach (const QString &alias, m_aliasList) {
                ui.alias_list->setItem(i++, 0, new QTableWidgetItem(alias));
            }
        }

        // The composition
        ui.resultComposition->setText(compositionString(m_elementMap));

        // The mass
        ui.resultMass->setText(i18n("Molecular mass: "));

        ui.resultValue->setText(QString::number(m_mass) + " u");
        ui.resultValue->show();
        ui.resultMass->setToolTip(complexString);
        ui.resultComposition->setToolTip(complexString);

#if 0
        // FIXME
        //select the elements in the table
        QList<Element*> list = m_elementMap.elements();
        KalziumDataObject::instance()->findElements(list);
#endif
    } else { // the input was invalid, so tell this the user
        qCDebug(KALZIUM_LOG) << "m_validInput == false";
        ui.resultComposition->setText(i18n("Invalid input"));
        ui.resultLabel->setText(QString());
        ui.resultMass->setText(QString());

        ui.resultMass->setToolTip(i18n("Invalid input"));
        ui.resultComposition->setToolTip(i18n("Invalid input"));
        ui.resultLabel->setToolTip(i18n("Invalid input"));
    }
}

QString MolcalcWidget::compositionString(ElementCountMap &_map)
{
    QString str;

    foreach (ElementCount *count, _map.map()) {
        str += i18n("%1<sub>%2</sub> ", count->element()->dataAsString(ChemicalDataObject::symbol), count->count());
    }

    return str;
}

// ----------------------------------------------------------------
//                            slots

void MolcalcWidget::slotCalculate()
{
    qCDebug(KALZIUM_LOG) << "MolcalcWidget::slotCalcButtonClicked()";
    QString molecule = ui.formulaEdit->text();

    // Parse the molecule, and at the same time calculate the total
    // mass, and the composition of it.
    if (!molecule.isEmpty()) {
        m_validInput = m_parser->weight(molecule, &m_mass, &m_elementMap);
        m_aliasList = m_parser->aliasList();
    }
    qCDebug(KALZIUM_LOG) << "done calculating.";

    updateUI();
}

void MolcalcWidget::keyPressEvent(QKeyEvent * /* e */)
{
    m_timer->start(500);
}

void MolcalcWidget::addAlias()
{
    const QString shortForm = ui.shortForm->text();
    const QString fullForm = ui.fullForm->text();

    // Validate the alias
    double x;
    ElementCountMap y;

    ui.aliasMessage->setText(QLatin1String(""));
    if (shortForm.length() < 2) {
        ui.aliasMessage->setText(i18n("Symbol should consist of two or more letters."));
        return;
    }

    if (m_parser->weight(shortForm, &x, &y)) {
        ui.aliasMessage->setText(i18n("Symbol already being used"));
        return;
    }

    if (fullForm.isEmpty() || !m_parser->weight(fullForm, &x, &y)) {
        ui.aliasMessage->setText(i18n("Expansion is invalid, please specify a valid expansion"));
        return;
    }

    // Open the file to write
    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("libkdeedu/data/symbols2.csv"));
    QFile file(fileName);

    if (!(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))) {
        QTextStream out(&file);
        out << "\"" + shortForm + "\",\"" + fullForm + "\"\n";
        qCDebug(KALZIUM_LOG) << fileName << "is the file.";
        qCDebug(KALZIUM_LOG) << "\"" + shortForm + "\",\"" + fullForm + "\"\n";
        ui.aliasMessage->setText(i18n("done!"));
        return;
    } else {
        ui.aliasMessage->setText((i18n("Unable to find the user defined alias file.")) + fileName);
        return;
    }
}

void MolcalcWidget::hideExtra()
{
    ui.details->hide();
    ui.tabWidget->removeTab(1);
}

#include "moc_molcalcwidget.cpp"
