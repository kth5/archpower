/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TABLEINFOWIDGET_H
#define TABLEINFOWIDGET_H

#include <QLabel>

#include "kalziumschemetype.h"

/**
 * @author Carsten Niehaus
 */
class TableInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableInfoWidget(QWidget *parent);

    ~TableInfoWidget() override = default;

private:
    QLabel *const m_tableType;

public Q_SLOTS:
    void setTableType(int type);
};

#endif // TABLEINFOWIDGET_H
