/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kalziumglwidget.h"
#include "iowrapper.h"

#include <avogadro/qtgui/molecule.h>
#include <avogadro/qtgui/sceneplugin.h>
#include <avogadro/qtgui/toolplugin.h>
#include <avogadro/qtplugins/pluginmanager.h>
#include <avogadro/rendering/primitive.h>

#include <QDebug>
#include <QWidget>

#include <config-kalzium.h>

KalziumGLWidget::KalziumGLWidget(QWidget *parent)
    : Avogadro::QtOpenGL::GLWidget(parent)
{
    // work around a bug in OpenBabel: the chemical data files parsing
    // is dependent on the LC_NUMERIC locale.
    m_lc_numeric = QByteArray(setlocale(LC_NUMERIC, nullptr));
    setlocale(LC_NUMERIC, "C");

    // Prevent What's this from intercepting right mouse clicks
    setContextMenuPolicy(Qt::PreventContextMenu);
    // Load the tools and set navigate as the default
    // first set the Avogadro plugin directory,
    // avoiding overwriting an already set envvar
    static bool s_pluginDirSet = false;
    if (!s_pluginDirSet) {
        if (qEnvironmentVariableIsEmpty("AVOGADRO_PLUGINS")) {
            qputenv("AVOGADRO_PLUGINS", AVOGADRO_PLUGIN_DIR);
        }
        s_pluginDirSet = true;
    }
    Avogadro::QtPlugins::PluginManager *manager = Avogadro::QtPlugins::PluginManager::instance();
    manager->load();

    // load render engines
    QList<Avogadro::QtGui::ScenePluginFactory *> scenePluginFactories = manager->pluginFactories<Avogadro::QtGui::ScenePluginFactory>();
    foreach (auto *factory, scenePluginFactories) {
        auto *scenePlugin = factory->createInstance();
        // enable Ball-and-Sticks
        if (scenePlugin->objectName() == QLatin1String("BallStick")) {
            scenePlugin->setEnabled(true);
        }
        sceneModel().addItem(scenePlugin);
    }

    // load tools
    if (!tools().isEmpty()) {
        qCritical() << "Updating non-empty toolset, erasing first.";
        qDeleteAll(tools());
    }
    auto toolPluginFactories = manager->pluginFactories<Avogadro::QtGui::ToolPluginFactory>();
    foreach (auto *factory, toolPluginFactories) {
        auto *tool = factory->createInstance();
        if (tool) {
            addTool(tool);
            if (factory->identifier() == QStringLiteral("Navigator")) {
                setDefaultTool(tool);
                setActiveTool(tool);
            }
        }
    }

    setMolecule(new Avogadro::QtGui::Molecule(this));
    update();
}

KalziumGLWidget::~KalziumGLWidget()
{
    // restore the LC_NUMERIC locale.
    setlocale(LC_NUMERIC, m_lc_numeric.constData());
}

bool KalziumGLWidget::openFile(const QString &file)
{
    // workaround for missing copy-constructor: fixed in Avogadra2 > 0.9
    Avogadro::QtGui::Molecule temp;
    temp = *IoWrapper::readMolecule(file);
    auto mol = new Avogadro::QtGui::Molecule(temp);

    if (!mol) {
        return false;
    }
    Avogadro::QtGui::Molecule *oldmol = molecule();
    if (oldmol) {
        oldmol->deleteLater();
    }
    setMolecule(mol);
    update();
    return true;
}

#include "moc_kalziumglwidget.cpp"
