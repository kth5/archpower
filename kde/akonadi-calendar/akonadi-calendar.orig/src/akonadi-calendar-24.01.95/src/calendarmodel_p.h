/*
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/EntityTreeModel>
#include <Akonadi/Monitor>
#include <QSharedPointer>
#include <QWeakPointer>

#include <memory>

namespace Akonadi
{
class CalendarModelPrivate;

class CalendarModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CalendarModel>;
    enum ItemColumn { Summary = 0, Type, DateTimeStart, DateTimeEnd, DateTimeDue, Priority, PercentComplete, ItemColumnCount };

    enum CollectionColumn { CollectionTitle = 0, CollectionColumnCount };

    enum Role { SortRole = Akonadi::EntityTreeModel::UserRole, RecursRole };

    static Akonadi::CalendarModel::Ptr create(Akonadi::Monitor *monitor);
    ~CalendarModel() override;

    [[nodiscard]] QWeakPointer<CalendarModel> weakPointer() const;
    void setWeakPointer(const QWeakPointer<CalendarModel> &weakPointer);

    [[nodiscard]] QVariant entityData(const Akonadi::Item &item, int column, int role = Qt::DisplayRole) const override;

    [[nodiscard]] QVariant entityData(const Akonadi::Collection &collection, int column, int role = Qt::DisplayRole) const override;

    [[nodiscard]] int entityColumnCount(EntityTreeModel::HeaderGroup headerSet) const override;

    [[nodiscard]] QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet) const override;

private:
    explicit CalendarModel(Akonadi::Monitor *monitor);

    std::unique_ptr<CalendarModelPrivate> const d;
};
}
