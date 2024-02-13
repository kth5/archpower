/*
    SPDX-FileCopyrightText: 2006-2008 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RSDIALOG_H
#define RSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QWidget>

#include "ui_rswidget.h"

/**
 * This class is the main class for R- and S-Phrases dialog.
 *
 * @author Carsten Niehaus
 */
class RSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RSDialog(QWidget *parent);

    Ui::RSWidget ui;

    /**
     * Filter the R- and S-Phrases.
     */
    void filterRS(const QList<int> &r, const QList<int> &s);

    QString rphrase(int number);

    QString sphrase(int number);

public Q_SLOTS:
    void filter();

private Q_SLOTS:
    void slotHelp();

private:
    QMap<int, QString> rphrases_map;
    QMap<int, QString> sphrases_map;

    void createSPhrases();
    void createRPhrases();

    void invalidPhaseString();
};

#endif // RSDIALOG_H
