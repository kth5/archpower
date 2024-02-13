/*
    SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-FileCopyrightText: 2007 Ian Monroe <ian@monroe.nu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TABLESDIALOG_H
#define TABLESDIALOG_H

#include <QTableWidget>
#include <QTableWidgetItem>

#include <KPageDialog>

/**
 * @author Carsten Niehaus
 */
class TablesDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit TablesDialog(QWidget *parent = nullptr);
    ~TablesDialog() override;

    void createNumbersTable();
    void createGreekSymbolTable();
};

/**
 * Disallows the table widget item from being edited.
 * @author Ian Monroe
 */
class MyWidgetItem : public QTableWidgetItem
{
public:
    explicit MyWidgetItem(const QString &s)
        : QTableWidgetItem(s)
    {
        setFlags(Qt::ItemIsEnabled);
    }
};

/**
 * Adds a context menu which copies to the clipboard the current cell.
 * @author Ian Monroe
 */
class MyTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit MyTableWidget(QWidget *parent);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private Q_SLOTS:
    void copyToClipboard();
};

#endif // TABLESDIALOG_H
