/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/CalendarBase>

/** Calendar representing a single Akonadi::Collection. */
class SingleCollectionCalendar : public Akonadi::CalendarBase
{
public:
    explicit SingleCollectionCalendar(const Akonadi::Collection &col, QObject *parent = nullptr);
    ~SingleCollectionCalendar() override;

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &c);

    bool addEvent(const KCalendarCore::Event::Ptr &event) override;
    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;
    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;

private:
    Akonadi::Collection m_collection;
};
