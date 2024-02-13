/*
    SPDX-FileCopyrightText: 2005-2006 Pino Toscano <toscano.pino@tiscali.it>
    SPDX-FileCopyrightText: 2003-2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2010-2011 Etienne Rebetez <etienne.rebetez@oberwallis.ch>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PERIODICTABLEVIEW_H
#define PERIODICTABLEVIEW_H

#include <QGraphicsView>

#include "elementitem.h"
#include "numerationitem.h"
#include "periodictablescene.h"
#include "periodictablestates.h"

/**
 * @class PeriodicTableView
 * In this class the periodic table of elements is created.
 * It provides slots to change the tables and accessing the element properties
 * @short Base class and creation for the pse System
 * @author Carsten Niehaus
 * @author Marcus D. Hanwell
 * @author Etienne Rebetez
 *
 */

class PeriodicTableView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PeriodicTableView(QWidget *parent = nullptr);
    ~PeriodicTableView() override;

    /**
     * Returns the qgraphicsscene
     */
    PeriodicTableScene *pseScene() const;

    /**
     * Returns the current id of the pse-table.
     * The id is the same as the menu and the pse list from the pseTables class.
     */
    int table() const;

    /**
     * Generates and saves the pse as svg in the given filename.
     * @param filename filename of the destination.
     */
    void generateSvg(const QString &filename);

Q_SIGNALS:
    /**
     * Is emitted when the pse table is changed.
     */
    void tableChanged(int tableTyp);
    /**
     * Is emitted when the numeration of the pse table is changed.
     */
    void numerationChange(int num);

public Q_SLOTS:
    /**
     * Change the pse table to the given id.
     * @param newtable id of the pse table.
     */
    void slotChangeTable(int newtable);
    /**
     * fits the pse in the qGraphicsView.
     */
    void fitPseInView();
    /**
     * One Element can be selected with this function.
     * The selection is only a graphical feedback for the user.
     * @param element number of the element.
     */
    void slotSelectOneElement(int element);
    /**
     * Selects an Element.
     * @param element number of the element.
     */
    void slotSelectAdditionalElement(int element);
    /**
     * Unselects all elements
     */
    void slotUnSelectElements();

private:
    QList<ElementItem *> createElementItems() const;
    QList<NumerationItem *> createNumerationItems() const;

    void setBiggerSceneRect();

    int m_currentTableInex;
    static int const RESIZE_SCENE_TIMEOUT = 2200;

    PeriodicTableScene *m_tableScene;

    PeriodicTableStates *m_tableStates;

protected:
    /**
     * Generic event handler, currently defaults to calling parent class
     * (included for future compatibility)
     */
    bool event(QEvent *e) override;

    /**
     * is called every time the view is resized.
     */
    void resizeEvent(QResizeEvent *event) override;
};

#endif // PERIODICTABLEVIEW_H
