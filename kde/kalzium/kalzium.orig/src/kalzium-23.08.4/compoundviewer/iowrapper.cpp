/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iowrapper.h"

#include <fstream>
#include <iostream>

#include <avogadro/core/molecule.h>
#include <avogadro/io/cmlformat.h>
#include <avogadro/io/pdbformat.h>
#include <avogadro/io/xyzformat.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>

#include <KLocalizedString>

std::unique_ptr<Avogadro::Core::Molecule> IoWrapper::readMolecule(const QString &filename)
{
    std::ifstream inFileStream(QFile::encodeName(filename).constData());
    if (!inFileStream) {
        QMessageBox::warning(nullptr, i18n("Problem while opening the file"), i18n("Cannot open the specified file."));
        return nullptr;
    }

    auto mol = std::make_unique<Avogadro::Core::Molecule>();
    auto format = getFileReader(QFileInfo(filename).suffix());

    if (!format) {
        qCritical() << "Could not initialize file reader for file " << filename;
        return nullptr;
    }

    if (!format->read(inFileStream, *mol)) {
        qCritical() << "Could not read file " << filename << "; Error message: " << QString().fromStdString(format->error());
        return nullptr;
    }

    return mol;
}

bool IoWrapper::writeMolecule(const QString &filename, Avogadro::Core::Molecule *mol)
{
    std::ofstream outFileStream(QFile::encodeName(filename).constData());
    if (!outFileStream) {
        QMessageBox::warning(nullptr, i18n("Sorry"), i18n("Cannot save to the specified file."));
        return false;
    }
    Avogadro::Io::CmlFormat cmlFormat;
    if (!cmlFormat.write(outFileStream, *mol)) {
        qCritical() << "Could not read file:" << filename;
        return false;
    }
    return true;
}

QString IoWrapper::getFormula(Avogadro::QtGui::Molecule *molecule)
{
    return QString::fromStdString(molecule->formula());
}

QString IoWrapper::getPrettyFormula(Avogadro::QtGui::Molecule *molecule)
{
    QString formula = QString::fromStdString(molecule->formula());
    formula.replace(QRegularExpression("(\\d+)"), "<sub>\\1</sub>");
    return formula;
}

std::unique_ptr<Avogadro::Io::FileFormat> IoWrapper::getFileReader(const QString &format)
{
    if (format == QStringLiteral("cml")) {
        return std::make_unique<Avogadro::Io::CmlFormat>();
    } else if (format == QStringLiteral("pdb")) {
        return std::make_unique<Avogadro::Io::PdbFormat>();
    } else if (format == QStringLiteral("xyz")) {
        return std::make_unique<Avogadro::Io::XyzFormat>();
    } else {
        return nullptr;
    }
}
