/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007-2008 Marcus D. Hanwell <marcus@cryos.org>
    SPDX-FileCopyrightText: 2016 Andreas Cord-Landwehr <cordlandwehr@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IOWRAPPER_H
#define IOWRAPPER_H

#include <avogadro/io/fileformat.h>
#include <avogadro/qtgui/molecule.h>

#include "compoundviewer_export.h"

#include <memory>

/**
 * @author Carsten Niehaus
 */
class COMPOUNDVIEWER_EXPORT IoWrapper
{
public:
    /**
     * This class reads the molecule in the file @p filename. It returns 0 if
     * the file couldn't be read.
     */
    static std::unique_ptr<Avogadro::Core::Molecule> readMolecule(const QString &filename);

    static bool writeMolecule(const QString &filename, Avogadro::Core::Molecule *);

    static QString getFormula(Avogadro::QtGui::Molecule *molecule);

    static QString getPrettyFormula(Avogadro::QtGui::Molecule *molecule);

private:
    /**
     * Get file format reader appropriate for the file format
     */
    static std::unique_ptr<Avogadro::Io::FileFormat> getFileReader(const QString &format);
};

#endif
