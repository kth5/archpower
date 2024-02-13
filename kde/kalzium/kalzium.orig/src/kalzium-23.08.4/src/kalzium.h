/*
    SPDX-FileCopyrightText: 2003, 2004, 2005, 2006, 2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KALZIUM_H
#define KALZIUM_H

#include "calculator/calculator.h"
#include "elementdataviewer.h"
#include "isotopetabledialog.h"
#include "periodictableview.h"
#include "rsdialog.h"
#include "settings/kalziumconfigdialog.h"
#include "tablesdialog.h"
#include <kxmlguiwindow.h>

class KAction;
class KSelectAction;
class QToolBox;
class QDockWidget;
class DetailedInfoDlg;
class PeriodicTableView;
class DetailedGraphicalOverview;
class GradientWidgetImpl;
class GlossaryDialog;
class IsotopeTable;
class LegendWidget;
class TableInfoWidget;
class ExportDialog;
class MoleculeDialog;

/**
 * @brief Application Main Window
 * @author Carsten Niehaus <cniehaus@kde.org>
 * @version 1.2
 *
 * This class connects the parts of kalzium. All actions will
 * be set, the menu generated and so on.
 */
class Kalzium : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    Kalzium();

    /**
     * Default Destructor
     */
    ~Kalzium() override;

    void loadMolecule(const QString &moleculeFile);

private:
    DetailedInfoDlg *m_infoDialog;

    LegendWidget *m_legendWidget;

    TableInfoWidget *m_TableInfoWidget;

    PeriodicTableView *m_periodicTable;

    IsotopeTableDialog *m_isotopeDialog;

    ElementDataViewer *m_elementDataPlotter;

    TablesDialog *m_tablesDialog;

    RSDialog *m_rsDialog;

    calculator *m_calculator;

    /**
     *initialize actions
     */
    void setupActions();

    /**
     *initialize the sidebars
     */
    void setupSidebars();

    void extractIconicInformationAboutElement(int elementNumber);

    /**
     * all KActions Kalzium uses
     */
    KSelectAction *look_action_schemes;
    KSelectAction *look_action_gradients;

    KSelectAction *table_action;
    KSelectAction *table_action_schemes;
    KSelectAction *numeration_action;
    // settings
    QAction *m_pLegendAction;
    QAction *m_SidebarAction;
    // tools
    QAction *m_pPlotAction;
    QAction *m_pcalculator;
    QAction *m_pIsotopeTableAction;
    QAction *m_pGlossaryAction;
    QAction *m_pRSAction;
    QAction *m_pTables;
    QAction *m_pMoleculesviewer;
    QAction *m_pOBConverterAction;
    QAction *export_action;

    QAction *m_legendAction;

    GradientWidgetImpl *m_gradientWidget;

    DetailedGraphicalOverview *m_detailWidget;

    /**
     * the dock of the central pse Widget
     */
    QDockWidget *m_dockWin;
    QDockWidget *m_legendDock;
    QDockWidget *m_tableDock;

    QToolBox *m_toolbox;

    GlossaryDialog *m_glossarydlg;

    ExportDialog *m_exportDialog;

    KalziumConfigDialog *m_configDialog;

    QLabel *m_elementInfo;

Q_SIGNALS:
    void numerationChanged(int);

private Q_SLOTS:
    /**
     * opens the information dialog for the element @p number
     */
    void openInformationDialog(int number);

    void elementHover(int num);
    void setupStatusBar();

    /**
     * switched to the table with the index @p index
     */
    void slotSwitchtoTable(int index);

    /**
     * start the glossary
     */
    void slotGlossary();

    /**
     * show the R/S-phrases
     */
    void slotRS();

    MoleculeDialog *slotMoleculeviewer();

    void slotOBConverter();

    void slotTables();

    void slotSwitchtoLookGradient(int);
    void slotSwitchtoLookScheme(int);
    void slotSwitchtoNumeration(int);

    /**
     *start the isotopetable
     */
    void slotIsotopeTable();

    void slotPlotData();
    /**
     * Start the calculator
     */
    void showCalculator();

    void slotExportTable();

    void slotShowExportDialog();

    /**
     * These slots are for the standard actions
     */
    void showSettingsDialog();

    void slotUpdateSettings();

protected:
    QSize sizeHint() const override;
};

#endif // KALZIUM_H
