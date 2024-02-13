/*
    SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/fetchjobcalendar.h"
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <akonadi/qtest_akonadi.h>

using namespace Akonadi;
using namespace KCalendarCore;

class FetchJobCalendarTest : public QObject
{
    Q_OBJECT
    Collection mCollection;

    void createIncidence(const QString &uid)
    {
        Item item;
        item.setMimeType(Event::eventMimeType());
        Incidence::Ptr incidence(new Event());
        incidence->setUid(uid);
        incidence->setSummary(QStringLiteral("summary"));
        incidence->setDtStart(QDateTime::currentDateTimeUtc());
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
        auto job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
    }

    void fetchCollection()
    {
        auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
        // Get list of collections
        job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
        AKVERIFYEXEC(job);

        // Find our collection
        Collection::List collections = job->collections();
        QVERIFY(!collections.isEmpty());
        mCollection = collections.first();

        QVERIFY(mCollection.isValid());
    }

private Q_SLOTS:
    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();

        fetchCollection();
        qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    }

    void testFetching()
    {
        createIncidence(QStringLiteral("a"));
        createIncidence(QStringLiteral("b"));
        createIncidence(QStringLiteral("c"));
        createIncidence(QStringLiteral("d"));
        createIncidence(QStringLiteral("e"));
        createIncidence(QStringLiteral("f"));

        FetchJobCalendar calendar;
        QSignalSpy spy(&calendar, &FetchJobCalendar::loadFinished);
        QVERIFY(spy.wait(1000));
        QVERIFY2(spy.at(0).at(0).toBool(), qPrintable(spy.at(0).at(1).toString()));

        const Incidence::List incidences = calendar.incidences();
        QCOMPARE(incidences.count(), 6);
        QVERIFY(calendar.item(QStringLiteral("a")).isValid());
        QVERIFY(calendar.item(QStringLiteral("b")).isValid());
        QVERIFY(calendar.item(QStringLiteral("c")).isValid());
        QVERIFY(calendar.item(QStringLiteral("d")).isValid());
        QVERIFY(calendar.item(QStringLiteral("e")).isValid());
        QVERIFY(calendar.item(QStringLiteral("f")).isValid());
    }
};

QTEST_AKONADIMAIN(FetchJobCalendarTest)

#include "fetchjobcalendartest.moc"
