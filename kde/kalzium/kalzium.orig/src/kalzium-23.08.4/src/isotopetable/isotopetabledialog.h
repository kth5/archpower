/*
    SPDX-FileCopyrightText: 2007, 2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ISOTOPETABLEDIALOG_H
#define ISOTOPETABLEDIALOG_H

#include "ui_isotopedialog.h"

#include <QDialog>

class IsotopeItem;

/**
 * This class is the drawing widget for the whole table
 *
 * @author Pino Toscano
 * @author Carsten Niehaus
 */
class IsotopeTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IsotopeTableDialog(QWidget *parent = nullptr);

private:
    Ui::isotopeWidget ui;

public Q_SLOTS:
    void setMode(int mode);
    void updateMode();

private Q_SLOTS:
    void updateDockWidget(IsotopeItem *);
    void zoom(int);
    void slotZoomLevelChanged(double);
};

#endif // ISOTOPETABLEDIALOG_H
