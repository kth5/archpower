/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
    SPDX-FileContributor: Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kcolumnfilterproxymodel_p.h"

#include <QList>

using namespace Akonadi;

KColumnFilterProxyModel::KColumnFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

KColumnFilterProxyModel::~KColumnFilterProxyModel() = default;

QList<int> KColumnFilterProxyModel::visbileColumns() const
{
    return m_visibleColumns;
}

void KColumnFilterProxyModel::setVisibleColumn(int column)
{
    setVisibleColumns(QList<int>() << column);
}

void KColumnFilterProxyModel::setVisibleColumns(const QList<int> &visibleColumns)
{
    m_visibleColumns = visibleColumns;
    invalidateFilter();
}

bool KColumnFilterProxyModel::filterAcceptsColumn(int column, const QModelIndex &parent) const
{
    if (!m_visibleColumns.isEmpty() && !m_visibleColumns.contains(column)) {
        // We only filter columns out when m_visibleColumns actually contains values.
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsColumn(column, parent);
}

#include "moc_kcolumnfilterproxymodel_p.cpp"
