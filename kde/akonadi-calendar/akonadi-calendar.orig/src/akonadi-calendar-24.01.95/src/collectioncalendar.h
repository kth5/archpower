/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

namespace Akonadi
{
class Collection;
class EntityTreeModel;

class CollectionCalendarPrivate;

/** Calendar representing a single Akonadi::Collection. */
class AKONADI_CALENDAR_EXPORT CollectionCalendar : public Akonadi::CalendarBase
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CollectionCalendar>;

    explicit CollectionCalendar(const Akonadi::Collection &col, QObject *parent = nullptr);
    CollectionCalendar(Akonadi::EntityTreeModel *model, const Akonadi::Collection &col, QObject *parent = nullptr);
    ~CollectionCalendar() override;

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &c);

    Akonadi::EntityTreeModel *model() const;

    bool addEvent(const KCalendarCore::Event::Ptr &event) override;
    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;
    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;

    bool hasRight(Akonadi::Collection::Right right) const;

Q_SIGNALS:
    /**
     * @brief Emitted whenever an incidence is added, removed or changed
     */
    void calendarChanged();

private:
    Q_DECLARE_PRIVATE(CollectionCalendar)
};

} // namespace Akonadi