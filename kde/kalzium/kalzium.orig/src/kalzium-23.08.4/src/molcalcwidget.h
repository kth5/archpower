/*
    SPDX-FileCopyrightText: 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MOLCALCWIDGET_H
#define MOLCALCWIDGET_H

#include "ui_molcalcwidgetbase.h"
#include <QWidget>

#include "moleculeparser.h"

class QTimer;
class QKeyEvent;

/**
 * This widget calculates mass for molecules.
 *
 * @author Carsten Niehaus
 * @author Pino Toscano
 * @author Inge Wallin
 * @author Kashyap R Puranik
 */
class MolcalcWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param parent parent widget
     */
    explicit MolcalcWidget(QWidget *parent = nullptr);
    void hideExtra();
    ~MolcalcWidget() override;

protected Q_SLOTS:
    void slotCalculate();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private Q_SLOTS:
    void clear();
    void addAlias();

private:
    /**
     * @return the HTML code of an element symbol and its
     * subscripted amount. Eg: Mg<sub>2</sub>
     */
    QString compositionString(ElementCountMap &_map);

    /**
     * This methods gathers all the data and updates the
     * contents of the widget.
     */
    void updateUI();

    Ui_MolcalcWidgetBase ui;

    QTimer *m_timer = nullptr;

private:
    MoleculeParser *m_parser = nullptr;
    QSet<QString> m_aliasList;
    double m_mass;
    bool m_validInput;
    ElementCountMap m_elementMap;
};

#endif // MOLCALCWIDGET_H
