/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net
    SPDX-FileContributor: Bertjan Broeksema <broeksema@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QSortFilterProxyModel>
template<typename T>
class QList;

namespace Akonadi
{
class KColumnFilterProxyModelPrivate;

/**
  Filter model to make only certain columns of a model visible. By default all
  columns are visible.
 */
class KColumnFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit KColumnFilterProxyModel(QObject *parent = nullptr);
    ~KColumnFilterProxyModel() override;

    /**
      Returns a vector containing the visible columns. If the vector is empty, all
      columns are visible.
    */
    [[nodiscard]] QList<int> visbileColumns() const;

    /**
      Convenience function. Has the same effect as:
      @code
      setVisibleColumns( QList<int>() << column );
      @endcode
      @param column the column to set as visible
      @see setVisbileColumns
     */
    void setVisibleColumn(int column);

    /**
      Change the visible columns. Pass an empty vector to make all columns visible.
      @param visibleColumns the vector changing visible columns
     */
    void setVisibleColumns(const QList<int> &visibleColumns);

protected:
    bool filterAcceptsColumn(int column, const QModelIndex &parent) const override;

private:
    QList<int> m_visibleColumns;
};
}
