/*
    SPDX-FileCopyrightText: 2007 Johannes Simon <johannes.simon@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "kalziumdataobject.h"
#include "ui_exportdialog.h"
#include <element.h>

class ElementListEntry : public QListWidgetItem
{
public:
    explicit ElementListEntry(Element *element);
    ~ElementListEntry() override;

    int m_atomicNum;
    QString m_name;
    Element *m_element = nullptr;
};

class PropertyListEntry : public QListWidgetItem
{
public:
    PropertyListEntry(const QString &name, ChemicalDataObject::BlueObelisk type);
    ~PropertyListEntry() override;

    ChemicalDataObject::BlueObelisk m_type;
};

/**
 * @author: Johannes Simon
 */
class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent);
    ~ExportDialog() override;

    void populateElementList();
    void exportToHtml();
    void exportToXml();
    void exportToCsv();

private:
    Ui::exportDialogForm ui;
    QTextStream *m_outputStream = nullptr;

public Q_SLOTS:
    void slotOkClicked();
    /**
     * Open help page
     */
    void slotHelpRequested();
};

#endif // EXPORTDIALOG_H
