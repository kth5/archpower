/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "moleculeview.h"

#include <avogadro/qtgui/elementtranslator.h>
#include <avogadro/qtgui/periodictableview.h>
#include <avogadro/qtgui/scenepluginmodel.h>
#include <avogadro/qtgui/toolplugin.h>

#include <QDebug>
#include <QFileInfo>
#include <QGLFormat>
#include <QUrl>

#include <KIO/Job>
#include <KJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNS3/QtQuickDialogWrapper>

#include "iowrapper.h"

#include <openbabel/mol.h>
#include <openbabel/obiter.h>
// This is needed to ensure that the forcefields are set up right with GCC vis
#ifdef __KDE_HAVE_GCC_VISIBILITY
#define HAVE_GCC_VISIBILITY
#endif
#include <KGuiItem>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <openbabel/forcefield.h>

using namespace OpenBabel;
using namespace Avogadro::QtGui;

MoleculeDialog::MoleculeDialog(QWidget *parent)
    : QDialog(parent)
    , m_path(QString())
    , m_periodicTable(nullptr)
{
    // use multi-sample (anti-aliased) OpenGL if available
    QGLFormat defFormat = QGLFormat::defaultFormat();
    defFormat.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(defFormat);

    setWindowTitle(i18nc("@title:window", "Molecular Editor"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    QPushButton *user2Button = new QPushButton;
    buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
    QPushButton *user3Button = new QPushButton;
    buttonBox->addButton(user3Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MoleculeDialog::reject);

    user1Button->setDefault(true);

    KGuiItem::assign(user1Button, KGuiItem(i18n("Load Molecule")));

    KGuiItem::assign(user2Button, KGuiItem(i18n("Download New Molecules")));

    KGuiItem::assign(user3Button, KGuiItem(i18n("Save Molecule")));

    ui.setupUi(mainWidget);

    // Attempt to set up the UFF forcefield
    //     m_forceField = OBForceField::FindForceField("UFF");
    //     if (!m_forceField) {
    //         ui.optimizeButton->setEnabled(false);
    //     }

    ui.styleCombo->addItems({"Ball and Stick", "Licorice", "Van der Waals", "Van der Waals (AO)", "Wireframe"});
    connect(ui.styleCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MoleculeDialog::slotUpdateScenePlugin);
    slotUpdateScenePlugin();

    connect(ui.tabWidget, &QTabWidget::currentChanged, this, &MoleculeDialog::setViewEdit);

    // Editing parameters
    // commented out until we find new API for pumbling to OpenBabel
    //     connect(ui.optimizeButton, &QPushButton::clicked,
    //             this, &MoleculeDialog::slotGeometryOptimize);
    connect(ui.clearDrawingButton, &QPushButton::clicked, this, &MoleculeDialog::clearAllElementsInEditor);

    connect(ui.glWidget->molecule(), &Avogadro::QtGui::Molecule::changed, this, &MoleculeDialog::slotUpdateStatistics);

    connect(user1Button, &QPushButton::clicked, this, &MoleculeDialog::slotLoadMolecule);
    connect(user2Button, &QPushButton::clicked, this, &MoleculeDialog::slotDownloadNewStuff);
    connect(user3Button, &QPushButton::clicked, this, &MoleculeDialog::slotSaveMolecule);

    mainLayout->addWidget(buttonBox);

    // Check that we have managed to load up some tools and engines
    int nTools = ui.glWidget->tools().size();
    if (!nTools) {
        QString error = i18n("No tools loaded - it is likely that the Avogadro plugins could not be located.");
        // use parent as parent for the messagebix, as this dialog is not shown yet
        // and thus not known to the window system to position to dialog relative to it
        KMessageBox::error(parent, error, i18n("Kalzium"));
    }

    // objectName is also used in Avogadro2 for identifying tools
    foreach (auto *tool, ui.glWidget->tools()) {
        if (tool->objectName() == QLatin1String("Editor")) {
            ui.editTabLayout->insertWidget(0, tool->toolWidget());
            break;
        }
    }
}

void MoleculeDialog::slotLoadMolecule()
{
    // Check that we have managed to load up some tools and engines
    int nTools = ui.glWidget->tools().size();

    if (!nTools) {
        QString error = i18n(
            "No tools loaded - it is likely that the Avogadro plugins could not be located. "
            "No molecules can be viewed until this issue is resolved.");
        KMessageBox::information(this, error);
    }

    m_path = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("data/molecules/"), QStandardPaths::LocateDirectory);

    QString commonMoleculeFormats = i18n("Common molecule formats");
    QString allFiles = i18n("All files");

    QString filename = QFileDialog::getOpenFileName(
                       this,
                       i18n("Choose a file to open"),
                       m_path,
                       commonMoleculeFormats + "(*.cml *.xyz *.ent *.pdb *.alc *.chm *.cdx *.cdxml *.c3d1 *.c3d2"
                       " *.gpr *.mdl *.mol *.sdf *.sd *.crk3d *.cht *.dmol *.bgf"
                       " *.gam *.inp *.gamin *.gamout *.tmol *.fract"
                       " *.mpd *.mol2);;" + allFiles + "(*)");

    loadMolecule(filename);
}

void MoleculeDialog::slotUpdateScenePlugin()
{
    const QString text = ui.styleCombo->currentText();
    for (int i = 0; i < ui.glWidget->sceneModel().rowCount(QModelIndex()); ++i) {
        QModelIndex index = ui.glWidget->sceneModel().index(i, 0);
        if (text == ui.glWidget->sceneModel().data(index, Qt::DisplayRole)) {
            ui.glWidget->sceneModel().setData(index, Qt::Checked, Qt::CheckStateRole);
        } else {
            ui.glWidget->sceneModel().setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void MoleculeDialog::loadMolecule(const QString &filename)
{
    if (filename.isEmpty()) {
        return;
    }

    // 1. workaround for missing copy-constructor: fixed in Avogadro2 > 0.9
    // 2. another workaround for broken copy-constructor that does not
    //    initialize the m_undoMolecule private member variable;
    //    this molecule should be created on the heap instead of the stack
    auto molecule_ptr = IoWrapper::readMolecule(filename);
    if (!molecule_ptr) {
        KMessageBox::error(this, i18n("Could not load molecule"), i18n("Loading the molecule failed."));
        return;
    }

    m_molecule = *molecule_ptr;

    if (m_molecule.atomCount() != 0) {
        disconnect(ui.glWidget->molecule(), nullptr, this, nullptr);
        ui.glWidget->setMolecule(&m_molecule);
        ui.glWidget->update();
        slotUpdateStatistics();
        connect(&m_molecule, &Avogadro::QtGui::Molecule::changed, this, &MoleculeDialog::slotUpdateStatistics);
    }
    ui.glWidget->resetCamera();
    ui.glWidget->updateScene();
}

void MoleculeDialog::clearAllElementsInEditor()
{
    ui.glWidget->molecule()->clearBonds();
    ui.glWidget->molecule()->clearAtoms();
    ui.glWidget->updateScene();
}

void MoleculeDialog::slotSaveMolecule()
{
    QString commonMoleculeFormats = i18n("Common molecule formats");
    QString allFiles = i18n("All files");
    QString filename = QFileDialog::getSaveFileName(this,
                                                    i18n("Choose a file to save to"),
                                                    QString(),
                                                    commonMoleculeFormats
                                                        + QStringLiteral(" (*.cml *.xyz *.ent *.pdb *.alc *.chm *.cdx *.cdxml *.c3d1 *.c3d2"
                                                                         " *.gpr *.mdl *.mol *.sdf *.sd *.crk3d *.cht *.dmol *.bgf"
                                                                         " *.gam *.inp *.gamin *.gamout *.tmol *.fract"
                                                                         " *.mpd *.mol2);;")
                                                        + allFiles + QStringLiteral(" (*)"));

    if (!filename.contains(QLatin1String("."))) {
        filename.append(QLatin1String(".cml"));
    }

    IoWrapper io;
    io.writeMolecule(filename, ui.glWidget->molecule());
}

void MoleculeDialog::setViewEdit(int mode)
{
    if (mode == 0) {
        ui.glWidget->setActiveTool(QStringLiteral("Navigator"));
    } else if (mode == 1) {
        ui.glWidget->setActiveTool(QStringLiteral("Editor"));
    } else if (mode == 2) {
        ui.glWidget->setActiveTool(QStringLiteral("MeasureTool"));
    }
}

MoleculeDialog::~MoleculeDialog()
{
}

void MoleculeDialog::slotUpdateStatistics()
{
    Molecule *mol = ui.glWidget->molecule();
    if (!mol) {
        return;
    }
    const std::string name = mol->data(QStringLiteral("name").toStdString()).toString();
    ui.nameLabel->setText(QString::fromStdString(name));
    ui.weightLabel->setText(
        i18nc("This 'u' stands for the chemical unit (u for 'units'). Most likely this does not need to be translated at all!", "%1 u", mol->mass()));
    ui.formulaLabel->setText(IoWrapper::getPrettyFormula(mol));
    ui.glWidget->update();
}

void MoleculeDialog::slotDownloadNewStuff()
{
    qDebug() << "Kalzium new stuff";

    KNS3::QtQuickDialogWrapper *dialog = new KNS3::QtQuickDialogWrapper(QStringLiteral("kalzium.knsrc"), this);
    dialog->open();
    connect(dialog, &KNS3::QtQuickDialogWrapper::closed, this, [this, dialog] {
        // list of changed entries
        QString destinationDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir dir(destinationDir);
        if (!dir.exists()) {
            destinationDir = QDir::homePath();
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QList<KNSCore::EntryInternal> entries = dialog->changedEntries();
#else
        const QList<KNSCore::Entry> entries = dialog->changedEntries();
#endif
        bool anyError = false;
        bool anySuccess = false;
        bool moreOneInstalledFile = false;
        QString exactlyOneFile;
        foreach (const auto &entry, entries) {
            // care only about installed ones
            if (entry.status() == KNS3::Entry::Installed) {
                qDebug() << "Changed Entry: " << entry.installedFiles();
                foreach (const QString &origFile, entry.installedFiles()) {
                    const QString destFile = destinationDir + '/' + QFileInfo(origFile).fileName();
                    KJob *job = KIO::file_move(QUrl::fromLocalFile(origFile), QUrl::fromLocalFile(destFile));
                    const bool success = job->exec();
                    if (success) {
                        if (exactlyOneFile.isEmpty()) {
                            exactlyOneFile = destFile;
                        } else {
                            moreOneInstalledFile = true;
                        }
                        anySuccess = true;
                    } else {
                        KMessageBox::error(this, i18n("Failed to download molecule %1 to %2.", entry.name(), destFile));
                        anyError = true;
                    }
                }
            }
        }
        if (anySuccess) {
            if (anyError) {
                KMessageBox::information(this, i18n("The molecules that could be downloaded have been saved to %1.", destinationDir));
            } else {
                KMessageBox::information(this, i18n("The molecules have been saved to %1.", destinationDir));
            }
            if (!moreOneInstalledFile) {
                loadMolecule(exactlyOneFile);
            }
        }
    });
}

// TODO there is currently no API to perform the necessary OpenBabel-Avogadro
//      conversions, after the migration to Avogadro2; at least with v0.9
void MoleculeDialog::slotGeometryOptimize()
{
    //     // Perform a geometry optimization
    //     if (!m_forceField) {
    //         return;
    //     }
    //
    //     Molecule* molecule = ui.glWidget->molecule();
    //     OpenBabel::OBMol obmol;//(molecule->OBMol());
    //
    //     // Warn the user if the force field cannot be set up for the molecule
    //     if (!m_forceField->Setup(obmol)) {
    //         KMessageBox::error(this,
    //                         i18n("Could not set up force field for this molecule"),
    //                         i18n("Kalzium"));
    //         return;
    //     }
    //
    //     // Reasonable default values for most users
    //     m_forceField->SteepestDescentInitialize(500, 1.0e-5);
    //     // Provide some feedback as the optimization runs
    //     while (m_forceField->SteepestDescentTakeNSteps(5)) {
    //         m_forceField->UpdateCoordinates(obmol);
    //         molecule->setOBMol(&obmol);
    //         molecule->update();
    //     }
}

#include "moc_moleculeview.cpp"
