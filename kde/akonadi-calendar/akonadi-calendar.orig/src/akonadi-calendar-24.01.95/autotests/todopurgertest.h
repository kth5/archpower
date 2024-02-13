/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <KCalendarCore/Calendar>
#include <QObject>
#include <QString>

namespace Akonadi
{
class ETMCalendar;
class TodoPurger;
}

class TodoPurgerTest : public QObject, KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testPurge();

public:
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override;
    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *calendar) override;

public Q_SLOTS:
    void onTodosPurged(bool success, int numDeleted, int numIgnored);

private:
    void createTree();
    void createTodo(const QString &uid, const QString &parentUid, bool completed, bool recurring = false);
    void fetchCollection();

    Akonadi::ETMCalendar *m_calendar = nullptr;
    Akonadi::Collection m_collection;
    int m_pendingCreations;
    int m_pendingDeletions;
    bool m_pendingPurgeSignal = false;

    int m_numDeleted;
    int m_numIgnored;

    Akonadi::TodoPurger *m_todoPurger = nullptr;
};
