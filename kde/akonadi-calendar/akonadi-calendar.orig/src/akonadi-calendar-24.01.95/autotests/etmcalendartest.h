/*
    SPDX-FileCopyrightText: 2011-2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <KCalendarCore/Calendar>
#include <QObject>

namespace Akonadi
{
class ETMCalendar;
}

class ETMCalendarTest : public QObject, KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCollectionChanged_data();
    void testCollectionChanged();
    void testIncidencesAdded();
    void testIncidencesModified();
    void testFilteredModel();
    void testUnfilteredModel();
    void testCheckableProxyModel();
    void testIncidencesDeleted();
    void testUnselectCollection();
    void testSelectCollection();
    void testSubTodos_data();
    void testSubTodos();
    void testNotifyObserverBug();
    void testUidChange();
    void testItem(); // tests item()
    void testShareETM();
    void testFilterInvitations();
    void testFilterInvitationsChanged();

public Q_SLOTS:
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override;
    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) override;
    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *cal) override;
    void handleCollectionsAdded(const Akonadi::Collection::List &collectionList);

private:
    // quiet --overloaded-virtual warning
    using KCalendarCore::Calendar::CalendarObserver::calendarIncidenceDeleted;

    void deleteIncidence(const QString &uid);
    void createIncidence(const QString &uid);
    void createTodo(const QString &uid, const QString &parentUid);
    void fetchCollection();
    void waitForIt();
    void checkExitLoop();

    Akonadi::ETMCalendar *mCalendar = nullptr;
    Akonadi::Collection mCollection;
    int mIncidencesToAdd;
    int mIncidencesToDelete;
    int mIncidencesToChange;
    QString mLastDeletedUid;
    QString mLastChangedUid;
};
