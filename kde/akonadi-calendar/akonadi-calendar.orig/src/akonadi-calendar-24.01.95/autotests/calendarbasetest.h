/*
    SPDX-FileCopyrightText: 2010-2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QObject>
#include <QStringList>

namespace Akonadi
{
class CalendarBase;
}

class CalendarBaseTest : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void handleCreateFinished(bool success, const QString &errorString);
    void handleDeleteFinished(bool success, const QString &errorString);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testItem();
    void testChildIncidences_data();
    void testChildIncidences();
    void testDelete();
    // void testDeleteAll(); This has been disabled in KCalendarCore::Calendar::deleteAll*() so no need to test

private:
    void fetchCollection();
    void createInitialIncidences();
    Akonadi::Item::Id createTodo(const QString &uid, const QString &parentUid = QString());

    Akonadi::Collection mCollection;
    Akonadi::CalendarBase *mCalendar = nullptr;
    bool mExpectedSlotResult = false;
    QStringList mUids;
    QString mOneEventUid;
    QString mOneTodoUid;
    QString mOneJournalUid;
    QString mOneIncidenceUid;
};
