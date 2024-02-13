/*
    SPDX-FileCopyrightText: 2003-2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalzium.h"

#include "detailedgraphicaloverview.h"
#include "detailinfodlg.h"
#include "exportdialog.h"
#include "gradientwidget_impl.h"
#include "kalziumdataobject.h"
#include "kalziumgradienttype.h"
#include "kalziumnumerationtype.h"
#include "kalziumschemetype.h"
#include "legendwidget.h"
#include "prefs.h"
#include "psetables.h"
#include "search.h"
#include "searchwidget.h"
#include "tableinfowidget.h"
#include <config-kalzium.h>
#include <element.h>
#include <kdeeduglossary.h>

#ifdef HAVE_FACILE
#include "eqchemview.h"
#endif

#ifdef HAVE_OPENBABEL
#if defined(HAVE_EIGEN) && defined(HAVE_AVOGADRO)
#include "tools/moleculeview.h"
#include <QGLFormat>
#endif
#include "tools/obconverter.h"
#endif

#include <QDockWidget>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolBox>
#include <QUrl>

#include <KActionCollection>
#include <KLocalizedString>
#include <KSelectAction>
#include <KStandardAction>

#define IDS_ELEMENTINFO 7

Kalzium::Kalzium()
    : KXmlGuiWindow(nullptr)
{
    setObjectName(QStringLiteral("KalziumMainWindow"));

    // Init pointers with null
    m_infoDialog = nullptr;
    m_isotopeDialog = nullptr;
    m_elementDataPlotter = nullptr;
    m_tablesDialog = nullptr;
    m_rsDialog = nullptr;
    m_calculator = nullptr;
    m_exportDialog = nullptr;
    m_glossarydlg = nullptr;
    m_elementInfo = nullptr;

    // reading the elements from file
    KalziumDataObject::instance();

    auto newsearch = new Search();
    KalziumDataObject::instance()->setSearch(newsearch);

    // Main pse-Table Tablewidget
    auto pseTempWidget = new QWidget(this);
    auto layout = new QVBoxLayout(pseTempWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    auto searchWidget = new SearchWidget(pseTempWidget);
    searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));

    // Creating the periodic table
    m_periodicTable = new PeriodicTableView(pseTempWidget);

    // Connecting the search to the periodic table
    connect(newsearch, &Search::searchChanged, KalziumElementProperty::instance(), &KalziumElementProperty::propertyChanged);
    connect(newsearch, &Search::searchReset, KalziumElementProperty::instance(), &KalziumElementProperty::propertyChanged);

    layout->addWidget(searchWidget);
    layout->addWidget(m_periodicTable);

    setCentralWidget(pseTempWidget);

    connect(m_periodicTable->pseScene(), &PeriodicTableScene::elementChanged, this, &Kalzium::openInformationDialog);
    connect(m_periodicTable->pseScene(), &PeriodicTableScene::elementHovered, this, &Kalzium::elementHover);
    connect(this, &Kalzium::numerationChanged, m_periodicTable, &PeriodicTableView::numerationChange);

    // layouting
    setupSidebars();
    setupActions();

    setupStatusBar();
}

void Kalzium::setupActions()
{
    export_action = actionCollection()->add<QAction>(QStringLiteral("file_exporter"));
    export_action->setText(i18n("&Export Data..."));
    connect(export_action, &QAction::triggered, this, &Kalzium::slotShowExportDialog);

    // the action for switching look: color schemes and gradients
    QStringList schemes = KalziumElementProperty::instance()->schemeList(); /*KalziumSchemeTypeFactory::instance()->schemes();*/
    QStringList gradients = KalziumElementProperty::instance()->gradientList();

    // the action for switching look: schemes
    look_action_schemes = actionCollection()->add<KSelectAction>(QStringLiteral("view_look_onlyschemes"));
    look_action_schemes->setText(i18n("&Scheme"));
    look_action_schemes->setItems(schemes);
    look_action_schemes->setToolBarMode(KSelectAction::MenuMode);
    look_action_schemes->setToolButtonPopupMode(QToolButton::InstantPopup);
    connect(look_action_schemes, &KSelectAction::indexTriggered, this, &Kalzium::slotSwitchtoLookScheme);

    // the action for switching look: gradients
    look_action_gradients = actionCollection()->add<KSelectAction>(QStringLiteral("view_look_onlygradients"));
    look_action_gradients->setText(i18n("&Gradients"));
    look_action_gradients->setItems(gradients);
    look_action_gradients->setToolBarMode(KSelectAction::MenuMode);
    look_action_gradients->setToolButtonPopupMode(QToolButton::InstantPopup);
    connect(look_action_gradients, &KSelectAction::indexTriggered, this, &Kalzium::slotSwitchtoLookGradient);

    // the action for switching tables
    QStringList table_schemes = pseTables::instance()->tables();
    table_action = actionCollection()->add<KSelectAction>(QStringLiteral("view_table"));
    table_action->setText(i18n("&Tables"));
    table_action->setItems(table_schemes);
    table_action->setCurrentItem(Prefs::table());
    connect(table_action, &KSelectAction::indexTriggered, this, &Kalzium::slotSwitchtoTable);

    // the actions for switching numeration
    numeration_action = actionCollection()->add<KSelectAction>(QStringLiteral("view_numerationtype"));
    numeration_action->setText(i18n("&Numeration"));
    numeration_action->setItems(KalziumNumerationTypeFactory::instance()->numerations());
    numeration_action->setCurrentItem(Prefs::numeration());
    connect(numeration_action, SIGNAL(indexTriggered(int)), this, SLOT(slotSwitchtoNumeration(int)));

    // tools actions
    m_pPlotAction = actionCollection()->addAction(QStringLiteral("tools_plotdata"));
    m_pPlotAction->setText(i18n("&Plot Data..."));
    m_pPlotAction->setIcon(QIcon::fromTheme(QStringLiteral("plot")));
    connect(m_pPlotAction, &QAction::triggered, this, &Kalzium::slotPlotData);

    // calculator actions
    m_pcalculator = actionCollection()->addAction(QStringLiteral("tools_calculate"));
    m_pcalculator->setText(i18n("Perform &Calculations..."));
    m_pcalculator->setIcon(QIcon::fromTheme(QStringLiteral("calculate")));
    m_pcalculator->setWhatsThis(i18nc("WhatsThis Help", "This is the calculator, it performs basic chemical calculations."));
    connect(m_pcalculator, &QAction::triggered, this, &Kalzium::showCalculator);

    m_pIsotopeTableAction = actionCollection()->addAction(QStringLiteral("tools_isotopetable"));
    m_pIsotopeTableAction->setText(i18n("&Isotope Table..."));
    m_pIsotopeTableAction->setIcon(QIcon::fromTheme(QStringLiteral("isotopemap")));
    m_pIsotopeTableAction->setWhatsThis(i18nc("WhatsThis Help", "This table shows all of the known isotopes of the chemical elements."));
    connect(m_pIsotopeTableAction, &QAction::triggered, this, &Kalzium::slotIsotopeTable);

    m_pGlossaryAction = actionCollection()->addAction(QStringLiteral("tools_glossary"));
    m_pGlossaryAction->setText(i18n("&Glossary..."));
    m_pGlossaryAction->setIcon(QIcon::fromTheme(QStringLiteral("glossary")));
    connect(m_pGlossaryAction, &QAction::triggered, this, &Kalzium::slotGlossary);

    m_pRSAction = actionCollection()->addAction(QStringLiteral("tools_rs"));
    m_pRSAction->setText(i18n("&R/S Phrases..."));
    m_pRSAction->setIcon(QIcon::fromTheme(QStringLiteral("kalzium_rs")));
    connect(m_pRSAction, &QAction::triggered, this, &Kalzium::slotRS);

    m_pOBConverterAction = actionCollection()->addAction(QStringLiteral("tools_obconverter"));
    m_pOBConverterAction->setText(i18n("Convert chemical files..."));
    m_pOBConverterAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    m_pOBConverterAction->setWhatsThis(i18nc("WhatsThis Help", "With this tool, you can convert files containing chemical data between various file formats."));
    connect(m_pOBConverterAction, &QAction::triggered, this, &Kalzium::slotOBConverter);
#ifndef HAVE_OPENBABEL
    m_pOBConverterAction->setEnabled(false);
#endif

    m_pMoleculesviewer = actionCollection()->addAction(QStringLiteral("tools_moleculeviewer"));
    m_pMoleculesviewer->setText(i18n("Molecular Editor..."));
    m_pMoleculesviewer->setIcon(QIcon::fromTheme(QStringLiteral("kalzium_molviewer")));
    m_pMoleculesviewer->setWhatsThis(i18nc("WhatsThis Help", "This tool allows you to view and edit 3D molecular structures."));
    connect(m_pMoleculesviewer, &QAction::triggered, this, &Kalzium::slotMoleculeviewer);
#if !defined(HAVE_OPENBABEL) || !defined(HAVE_EIGEN) || !defined(HAVE_AVOGADRO)
    m_pMoleculesviewer->setEnabled(false);
#endif

    m_pTables = actionCollection()->addAction(QStringLiteral("tools_tables"));
    m_pTables->setText(i18n("&Tables..."));
    m_pTables->setIcon(QIcon::fromTheme(QStringLiteral("kalzium_tables")));
    m_pTables->setWhatsThis(i18nc("WhatsThis Help", "This will open a dialog with listings of symbols and numbers related to chemistry."));

    connect(m_pTables, &QAction::triggered, this, &Kalzium::slotTables);

    // other period view options
    m_pLegendAction = m_legendDock->toggleViewAction();
    actionCollection()->addAction(QStringLiteral("view_legend"), m_pLegendAction);
    m_pLegendAction->setIcon(QIcon::fromTheme(QStringLiteral("legend")));
    m_pLegendAction->setWhatsThis(i18nc("WhatsThis Help", "This will show or hide the legend for the periodic table."));

    m_SidebarAction = m_dockWin->toggleViewAction();
    actionCollection()->addAction(QStringLiteral("view_sidebar"), m_SidebarAction);
    m_SidebarAction->setIcon(QIcon::fromTheme(QStringLiteral("sidebar")));
    m_SidebarAction->setWhatsThis(i18nc("WhatsThis Help", "This will show or hide a sidebar with additional information and a set of tools."));

    m_SidebarAction = m_tableDock->toggleViewAction();
    actionCollection()->addAction(QStringLiteral("view_tablebar"), m_SidebarAction);
    m_SidebarAction->setIcon(QIcon::fromTheme(QStringLiteral("kalzium_tables")));
    m_SidebarAction->setWhatsThis(i18nc("WhatsThis Help", "This will show or hide a sidebar with additional information about the table."));

    // the standard actions
    actionCollection()->addAction(QStringLiteral("saveAs"), KStandardAction::saveAs(this, SLOT(slotExportTable()), actionCollection()));

    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());

    actionCollection()->addAction(QStringLiteral("quit"), KStandardAction::quit(qApp, SLOT(quit()), actionCollection()));

    m_legendWidget->LockWidget();

    slotSwitchtoLookGradient(KalziumElementProperty::instance()->gradientId());
    slotSwitchtoLookScheme(KalziumElementProperty::instance()->schemeId());

    slotSwitchtoNumeration(Prefs::numeration());
    slotSwitchtoTable(Prefs::table());

    m_legendWidget->UnLockWidget();
    m_legendWidget->updateContent();

    // set the shell's ui resource file
    setXMLFile(QStringLiteral("kalziumui.rc"));
    setupGUI();
}

void Kalzium::setupSidebars()
{
    setDockNestingEnabled(true);

    m_legendWidget = new LegendWidget(this);
    m_legendDock = new QDockWidget(i18n("Legend"), this);
    m_legendDock->setObjectName(QStringLiteral("kalzium-legend"));
    m_legendDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    m_legendDock->setWidget(m_legendWidget);

    connect(m_legendDock, &QDockWidget::dockLocationChanged, m_legendWidget, &LegendWidget::setDockArea);
    connect(m_legendWidget, &LegendWidget::elementMatched, m_periodicTable, &PeriodicTableView::slotSelectAdditionalElement);
    connect(m_legendWidget, &LegendWidget::resetElementMatch, m_periodicTable, &PeriodicTableView::slotUnSelectElements);

    m_TableInfoWidget = new TableInfoWidget(this);
    m_tableDock = new QDockWidget(i18n("Table Information"), this);
    m_tableDock->setWidget(m_TableInfoWidget);
    m_tableDock->setObjectName(QStringLiteral("kalzium-tableinfo"));
    m_tableDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);

    m_dockWin = new QDockWidget(i18n("Information"), this);
    m_dockWin->setObjectName(QStringLiteral("kalzium-sidebar"));
    m_dockWin->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    m_dockWin->setMinimumWidth(250);

    m_toolbox = new QToolBox(m_dockWin);
    m_dockWin->setWidget(m_toolbox);

    m_detailWidget = new DetailedGraphicalOverview(m_toolbox);
    m_detailWidget->setObjectName(QStringLiteral("DetailedGraphicalOverview"));
    m_detailWidget->setMinimumSize(200, m_detailWidget->minimumSize().height());

    m_toolbox->addItem(m_detailWidget, QIcon::fromTheme(QStringLiteral("overview")), i18n("Overview"));

    m_gradientWidget = new GradientWidgetImpl(m_toolbox);
    m_gradientWidget->setObjectName(QStringLiteral("viewtWidget"));

    connect(m_gradientWidget, &GradientWidgetImpl::gradientValueChanged, KalziumElementProperty::instance(), &KalziumElementProperty::setSliderValue);
    connect(m_gradientWidget->scheme_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSwitchtoLookScheme(int)));
    connect(m_gradientWidget->gradient_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSwitchtoLookGradient(int)));

    m_toolbox->addItem(m_gradientWidget, QIcon::fromTheme(QStringLiteral("statematter")), i18n("View"));

    addDockWidget(Qt::LeftDockWidgetArea, m_dockWin);
    addDockWidget(Qt::BottomDockWidgetArea, m_tableDock, Qt::Horizontal);
    addDockWidget(Qt::BottomDockWidgetArea, m_legendDock, Qt::Horizontal);

    m_tableDock->setVisible(false);
}

void Kalzium::slotExportTable()
{
    const QString fileName = QFileDialog::getSaveFileName(this, i18n("Save Kalzium's Table In"), QString(), i18n("Image files (*.png *.xpm *.jpg *.svg)"));

    if (fileName.endsWith(QLatin1String(".svg"))) {
        m_periodicTable->generateSvg(fileName);
    } else {
        QPixmap pix = m_periodicTable->grab();
        pix.save(fileName);
    }
}

void Kalzium::slotGlossary()
{
    if (!m_glossarydlg) {
        // creating the glossary dialog and loading the glossaries we have
        m_glossarydlg = new GlossaryDialog(this);
        m_glossarydlg->setObjectName(QStringLiteral("glossary"));
        QString dir = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/"), QStandardPaths::LocateDirectory);
        dir = QFileInfo(dir).absolutePath();
        const QString picturepath = dir + QStringLiteral("/bg.jpg");
        QUrl u = QUrl::fromLocalFile(dir + "/knowledge.xml");
        auto g = new Glossary(u);
        g->setName(i18n("Knowledge"));
        g->setBackgroundPicture(picturepath);
        m_glossarydlg->addGlossary(g, true);
        u = QUrl::fromLocalFile(dir + "/tools.xml");
        g = new Glossary(u, dir + "/toolpics/");
        g->setName(i18n("Tools"));
        g->setBackgroundPicture(picturepath);
        m_glossarydlg->addGlossary(g, true);
    }

    m_glossarydlg->show();
}

void Kalzium::slotRS()
{
    if (!m_rsDialog) {
        m_rsDialog = new RSDialog(this);
    }
    m_rsDialog->show();
}

void Kalzium::slotOBConverter()
{
#ifdef HAVE_OPENBABEL
    KOpenBabel *d = new KOpenBabel(this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->show();
#endif
}

MoleculeDialog *Kalzium::slotMoleculeviewer()
{
#if defined(HAVE_OPENBABEL) && defined(HAVE_EIGEN) && defined(HAVE_AVOGADRO)

    if (!QGLFormat::hasOpenGL()) {
        QMessageBox::critical(Q_NULLPTR, i18n("Kalzium Error"), i18n("This system does not support OpenGL."));
        return nullptr;
    }

    MoleculeDialog *d = new MoleculeDialog(this);
    d->show();
    return d;

#if 0
    KPluginLoader loader("libkalziumglpart");
    KPluginFactory* factory = loader.factory();

    if (factory) {
        KParts::ReadOnlyPart *part = 0;
        part = static_cast<KParts::ReadOnlyPart*>(factory->create(this, "KalziumGLPart"));

        part->widget()->show();
    }
#endif
#endif
    return nullptr;
}

void Kalzium::slotTables()
{
    if (!m_tablesDialog) {
        m_tablesDialog = new TablesDialog(this);
    }
    m_tablesDialog->show();
}

void Kalzium::slotIsotopeTable()
{
    if (!m_isotopeDialog) {
        m_isotopeDialog = new IsotopeTableDialog(this);
        connect(Prefs::self(), &Prefs::isotopeTableModeChanged, m_isotopeDialog, &IsotopeTableDialog::updateMode);
    }
    m_isotopeDialog->show();
}

void Kalzium::slotPlotData()
{
    if (!m_elementDataPlotter) {
        m_elementDataPlotter = new ElementDataViewer(this);
    }
    m_elementDataPlotter->show();
}

void Kalzium::showCalculator()
{
    if (!m_calculator) {
        m_calculator = new calculator(this);
    }
    m_calculator->show();
}

void Kalzium::slotSwitchtoTable(int index)
{
    m_periodicTable->slotChangeTable(index);
    m_TableInfoWidget->setTableType(index);

    if (m_infoDialog) {
        m_infoDialog->setTableType(m_periodicTable->table());
    }
    Prefs::setTable(index);
    Prefs::self()->save();
}

void Kalzium::slotSwitchtoNumeration(int index)
{
    Q_EMIT numerationChanged(index);
    Prefs::setNumeration(index);
    Prefs::self()->save();
}

void Kalzium::slotSwitchtoLookGradient(int which)
{
    qCDebug(KALZIUM_LOG) << "slotSwitchtoLookGradient Kalzium";

    KalziumElementProperty::instance()->setGradient(which);

    look_action_gradients->blockSignals(true);
    m_gradientWidget->gradient_combo->blockSignals(true);

    look_action_gradients->setCurrentItem(which);
    m_gradientWidget->gradient_combo->setCurrentIndex(which);

    look_action_gradients->blockSignals(false);
    m_gradientWidget->gradient_combo->blockSignals(false);

    m_gradientWidget->slotGradientChanged();

    m_legendWidget->updateContent();
}

void Kalzium::slotSwitchtoLookScheme(int which)
{
    qCDebug(KALZIUM_LOG) << "slotSwitchtoLookScheme Kalzium";

    KalziumElementProperty::instance()->setScheme(which);

    m_gradientWidget->scheme_combo->blockSignals(true);
    look_action_schemes->blockSignals(true);

    look_action_schemes->setCurrentItem(which);
    m_gradientWidget->scheme_combo->setCurrentIndex(which);

    look_action_schemes->blockSignals(false);
    m_gradientWidget->scheme_combo->blockSignals(false);

    m_legendWidget->updateContent();
}

void Kalzium::showSettingsDialog()
{
    if (KalziumConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }

    // KalziumConfigDialog didn't find an instance of this dialog, so lets create it :
    m_configDialog = new KalziumConfigDialog(this, QStringLiteral("settings"), Prefs::self());

    connect(m_configDialog, &KalziumConfigDialog::settingsChanged, this, &Kalzium::slotUpdateSettings);
    connect(m_configDialog, &KalziumConfigDialog::settingsChanged, m_gradientWidget, &GradientWidgetImpl::slotGradientChanged);

    m_configDialog->show();
}

void Kalzium::slotUpdateSettings()
{
    /*This slot function calls change the color of pse elements immediately after prefs change*/
    slotSwitchtoLookGradient(Prefs::colorgradientbox());
    slotSwitchtoLookScheme(Prefs::colorschemebox());
}

void Kalzium::slotShowExportDialog()
{
    if (!m_exportDialog) {
        m_exportDialog = new ExportDialog(this);
    }
    m_exportDialog->show();
}

void Kalzium::setupStatusBar()
{
    auto statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    m_elementInfo = new QLabel(QLatin1String(""));
    m_elementInfo->setAlignment(Qt::AlignRight);
    statusBar->addWidget(m_elementInfo, 1);
    statusBar->show();
}

void Kalzium::elementHover(int num)
{
    //     extractIconicInformationAboutElement(num);
    Element *e = KalziumDataObject::instance()->element(num);
    m_elementInfo->setText(i18nc("For example: \"Carbon (6), Mass: 12.0107 u\"",
                                 "%1 (%2), Mass: %3 u",
                                 e->dataAsString(ChemicalDataObject::name),
                                 e->dataAsString(ChemicalDataObject::atomicNumber),
                                 e->dataAsString(ChemicalDataObject::mass)));
    qCDebug(KALZIUM_LOG) << "change item in status bar";

    m_detailWidget->setElement(num);
}

// FIXME What is that function for? Does not seem to do anything useful... yet?
void Kalzium::extractIconicInformationAboutElement(int elementNumber)
{
    const QString setname = QStringLiteral("school");
    QString pathname = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/iconsets/"), QStandardPaths::LocateDirectory);
    pathname = QFileInfo(pathname).absolutePath();
    const QString filename = pathname + setname + '/' + "iconinformation.txt";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QString infoline;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString tmp = in.readLine();
        if (tmp.startsWith(QString::number(elementNumber))) {
            infoline = tmp;
        }
    }

    QString realText = QStringLiteral("Moin dies ist ein test!");
    // X         QString realText = infoline.remove(QRegularExpression("\\d+ "));
}

void Kalzium::openInformationDialog(int number)
{
    if (m_infoDialog) {
        m_infoDialog->setElement(number);
    } else {
        m_infoDialog = new DetailedInfoDlg(number, this);

        // Remove the selection when this dialog finishes or hides.
        connect(m_infoDialog, &DetailedInfoDlg::elementChanged, m_periodicTable, &PeriodicTableView::slotSelectOneElement);
        connect(m_infoDialog, &DetailedInfoDlg::elementChanged, this, &Kalzium::elementHover);
    }

    m_infoDialog->setTableType(m_periodicTable->table());
    m_infoDialog->show();
}

Kalzium::~Kalzium()
{
    delete m_periodicTable;
    delete m_infoDialog;
    delete m_TableInfoWidget;
    delete m_legendWidget;
    delete m_gradientWidget;

    delete m_dockWin;
    delete m_legendDock;
    delete m_tableDock;
}

void Kalzium::loadMolecule(const QString &moleculeFile)
{
#if defined(HAVE_OPENBABEL) && defined(HAVE_EIGEN) && defined(HAVE_AVOGADRO)
    MoleculeDialog *d = slotMoleculeviewer();
    if (d) {
        d->loadMolecule(moleculeFile);
    }
#endif
}

QSize Kalzium::sizeHint() const
{
    return {700, 500};
}

#include "moc_kalzium.cpp"
