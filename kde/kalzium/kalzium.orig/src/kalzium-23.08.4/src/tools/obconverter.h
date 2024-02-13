/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2006 Jerome Pansanel <j.pansanel@pansanel.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OBCONVERTER_H
#define OBCONVERTER_H

#include "ui_obconverterwidget.h"

#include <QDialog>

// OpenBabel includes
#include <openbabel/obconversion.h>

/**
 * @author Carsten Niehaus
 * @author Jerome Pansanel
 */
class KOpenBabel : public QDialog
{
    Q_OBJECT

public:
    /**
     * public constructor
     *
     * @param parent the parent widget
     */
    explicit KOpenBabel(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~KOpenBabel();

    /**
     * Add file to the list
     */
    void addFile(const QString &filename);

private:
    Ui::OBConverterWidget ui;

    OpenBabel::OBConversion *OBConvObject;

    QString File;

    /**
     * Setup the interface for the window
     */
    void setupWindow();

private slots:
    /**
     * Add file to the list
     */
    void slotAddFile();

    /**
     * Select every file in the list
     */
    void slotSelectAll();

    /**
     * Delete file from the list
     */
    void slotDeleteFile();

    /**
     * Try to guess the input file type from the selection
     */
    void slotGuessInput();

    /**
     * Convert the file in the selected type
     */
    void slotConvert();

    /**
     * Open help page
     */
    void slotHelpRequested();
};

#endif // OBCONVERTER_H
