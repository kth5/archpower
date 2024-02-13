/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KCalendarCore/Calendar>

#include <QObject>
#include <QString>

namespace Akonadi
{
class IncidenceChanger;
}

class UnitTestBase : public QObject
{
    Q_OBJECT
public:
    UnitTestBase();
    void waitForIt(); // Waits 10 seconds for signals
    void stopWaiting();
    void createIncidence(const QString &uid);
    void createIncidence(const Akonadi::Item &item);

    void verifyExists(const QString &uid, bool exists);
    Akonadi::Item::List calendarItems();

public Q_SLOTS:
    void onLoadFinished(bool success, const QString &errorMessage);

protected:
    void compareCalendars(const KCalendarCore::Calendar::Ptr &expectedCalendar);
    static QByteArray readFile(const QString &filename);
    static Akonadi::Item generateIncidence(const QString &uid, const QString &organizer = QString());

    Akonadi::Collection mCollection;
    Akonadi::IncidenceChanger *mChanger = nullptr;
};
