/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "singlecollectioncalendar.h"

#include "../src/calendarbase_p.h"

#include <Akonadi/CalendarUtils>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>

SingleCollectionCalendar::SingleCollectionCalendar(const Akonadi::Collection &col, QObject *parent)
    : Akonadi::CalendarBase(parent)
{
    setCollection(col);

    incidenceChanger()->setDefaultCollection(col);
    incidenceChanger()->setGroupwareCommunication(false);
    incidenceChanger()->setDestinationPolicy(Akonadi::IncidenceChanger::DestinationPolicyNeverAsk);
    setIsLoading(true);

    // here and below for monitoring: ensure items have a parent collection
    // before calling internalInsert to prevent an extra collection fetch job
    auto job = new Akonadi::ItemFetchJob(col, this);
    job->fetchScope().fetchFullPayload();
    connect(job, &Akonadi::ItemFetchJob::finished, this, [this, job]() {
        Q_D(Akonadi::CalendarBase);
        const auto items = job->items();
        for (auto item : items) {
            item.setParentCollection(m_collection);
            d->internalInsert(item);
        }
        setIsLoading(false);
    });

    auto monitor = new Akonadi::Monitor(this);
    monitor->setCollectionMonitored(m_collection, true);
    monitor->setItemFetchScope(job->fetchScope());
    connect(monitor, &Akonadi::Monitor::itemAdded, this, [this](Akonadi::Item item) {
        Q_D(Akonadi::CalendarBase);
        item.setParentCollection(m_collection);
        d->internalInsert(item);
    });
    connect(monitor, &Akonadi::Monitor::itemChanged, this, [this](Akonadi::Item item) {
        Q_D(Akonadi::CalendarBase);
        item.setParentCollection(m_collection);
        d->internalInsert(item);
    });
    connect(monitor, &Akonadi::Monitor::itemRemoved, this, [this](const Akonadi::Item &item) {
        Q_D(Akonadi::CalendarBase);
        d->internalRemove(item);
    });
}

SingleCollectionCalendar::~SingleCollectionCalendar() = default;

Akonadi::Collection SingleCollectionCalendar::collection() const
{
    return m_collection;
}

void SingleCollectionCalendar::setCollection(const Akonadi::Collection &c)
{
    Q_ASSERT(c.id() == m_collection.id() || !m_collection.isValid());
    m_collection = c;

    setName(Akonadi::CalendarUtils::displayName(m_collection));
    setAccessMode((m_collection.rights() & (Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanChangeItem)) ? KCalendarCore::ReadWrite
                                                                                                                      : KCalendarCore::ReadOnly);
}

bool SingleCollectionCalendar::addEvent(const KCalendarCore::Event::Ptr &event)
{
    if (m_collection.contentMimeTypes().contains(event->mimeType()) || m_collection.contentMimeTypes().contains(QLatin1String("text/calendar"))) {
        return CalendarBase::addEvent(event);
    }
    return false;
}

bool SingleCollectionCalendar::addTodo(const KCalendarCore::Todo::Ptr &todo)
{
    if (m_collection.contentMimeTypes().contains(todo->mimeType()) || m_collection.contentMimeTypes().contains(QLatin1String("text/calendar"))) {
        return CalendarBase::addTodo(todo);
    }
    return false;
}

bool SingleCollectionCalendar::addJournal(const KCalendarCore::Journal::Ptr &journal)
{
    if (m_collection.contentMimeTypes().contains(journal->mimeType()) || m_collection.contentMimeTypes().contains(QLatin1String("text/calendar"))) {
        return CalendarBase::addJournal(journal);
    }
    return false;
}
