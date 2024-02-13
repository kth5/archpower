/*
    SPDX-FileCopyrightText: 2007 Carsten Niehaus <cniehaus@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tableinfowidget.h"

#include "kalzium_debug.h"
#include <QHBoxLayout>

#include <KLocalizedString>

#include "prefs.h"
#include "psetables.h"

TableInfoWidget::TableInfoWidget(QWidget *parent)
    : QWidget(parent)
    , m_tableType(new QLabel(QStringLiteral("test"), this))
{

    auto la = new QHBoxLayout(this);
    la->addWidget(m_tableType);
}

void TableInfoWidget::setTableType(int type)
{
    m_tableType->setText(pseTables::instance()->getTabletype(type)->description());
}

#include "moc_tableinfowidget.cpp"
