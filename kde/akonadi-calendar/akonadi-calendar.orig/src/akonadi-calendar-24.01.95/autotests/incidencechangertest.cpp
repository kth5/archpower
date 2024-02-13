/*
    SPDX-FileCopyrightText: 2010-2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/incidencechanger.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <akonadi/qtest_akonadi.h>

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KCalendarCore/Event>

#include <QBitArray>

using namespace Akonadi;
using namespace KCalendarCore;

Q_DECLARE_METATYPE(QList<Akonadi::IncidenceChanger::ChangeType>)
Q_DECLARE_METATYPE(QList<bool>)
Q_DECLARE_METATYPE(QList<Akonadi::Collection::Right>)
Q_DECLARE_METATYPE(QList<Akonadi::Collection::Rights>)
Q_DECLARE_METATYPE(QList<Akonadi::IncidenceChanger::ResultCode>)
Q_DECLARE_METATYPE(KCalendarCore::RecurrenceRule::PeriodType)
QString s_ourEmail = QStringLiteral("unittests@dev.nul"); // change also in kdepimlibs/akonadi/calendar/tests/unittestenv/kdehome/share/config
QString s_outEmail2 = QStringLiteral("identity2@kde.org");

static Akonadi::Item item()
{
    Item item;
    Incidence::Ptr incidence = Incidence::Ptr(new Event());
    incidence->setSummary(QStringLiteral("random summary"));
    item.setMimeType(incidence->mimeType());
    item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
    return item;
}

static Akonadi::Item createItem(const Akonadi::Collection &collection)
{
    Item i = item();
    auto createJob = new ItemCreateJob(i, collection);

    createJob->exec();
    return createJob->item();
}

class IncidenceChangerTest : public QObject
{
    Q_OBJECT
    Collection mCollection;

    QHash<int, IncidenceChanger::ResultCode> mExpectedResultByChangeId;
    IncidenceChanger *mChanger = nullptr;

    int mIncidencesToDelete;
    int mIncidencesToAdd;
    int mIncidencesToModify;

    QHash<int, Akonadi::Item::Id> mItemIdByChangeId;
    QHash<QString, Akonadi::Item::Id> mItemIdByUid;
    int mChangeToWaitFor;
    bool mPermissionsOrRollback;
    bool mDiscardedEqualsSuccess;
    Akonadi::Item mLastItemCreated;

private Q_SLOTS:
    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();

        mIncidencesToDelete = 0;
        mIncidencesToAdd = 0;
        mIncidencesToModify = 0;
        mPermissionsOrRollback = false;
        mDiscardedEqualsSuccess = false;

        mChangeToWaitFor = -1;
        // Control::start(); //TODO: uncomment when using testrunner
        qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
        qRegisterMetaType<QList<Akonadi::IncidenceChanger::ChangeType>>("QList<Akonadi::IncidenceChanger::ChangeType>");
        auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
        // Get list of collections
        job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
        AKVERIFYEXEC(job);

        // Find our collection
        Collection::List collections = job->collections();
        QVERIFY(!collections.isEmpty());
        mCollection = collections.first();
        mCollection.setRights(Collection::Rights(Collection::AllRights));

        QVERIFY(mCollection.isValid());
        QVERIFY((mCollection.rights() & Akonadi::Collection::CanCreateItem));

        mChanger = new IncidenceChanger(this);
        mChanger->setShowDialogsOnError(false);

        connect(mChanger, &IncidenceChanger::createFinished, this, &IncidenceChangerTest::createFinished);

        connect(mChanger, &IncidenceChanger::deleteFinished, this, &IncidenceChangerTest::deleteFinished);

        connect(mChanger, &IncidenceChanger::modifyFinished, this, &IncidenceChangerTest::modifyFinished);
    }

    void testCreating_data()
    {
        QTest::addColumn<bool>("sendInvalidIncidence");
        QTest::addColumn<QString>("uid");
        QTest::addColumn<QString>("summary");
        QTest::addColumn<Akonadi::Collection>("destinationCollection");
        QTest::addColumn<Akonadi::Collection>("defaultCollection");
        QTest::addColumn<bool>("respectsCollectionRights");
        QTest::addColumn<Akonadi::IncidenceChanger::DestinationPolicy>("destinationPolicy");
        QTest::addColumn<bool>("failureExpected");
        QTest::addColumn<Akonadi::IncidenceChanger::ResultCode>("expectedResultCode");

        QTest::newRow("Simple Creation1") << false << "SomeUid1"
                                          << "Summary1" << mCollection << Collection() << true << IncidenceChanger::DestinationPolicyNeverAsk << false
                                          << IncidenceChanger::ResultCodeSuccess;

        QTest::newRow("Simple Creation2") << false << "SomeUid2"
                                          << "Summary2" << mCollection << Collection() << true << IncidenceChanger::DestinationPolicyNeverAsk << false
                                          << IncidenceChanger::ResultCodeSuccess;

        QTest::newRow("Invalid incidence") << true << "SomeUid3"
                                           << "Summary3" << mCollection << Collection() << true << IncidenceChanger::DestinationPolicyNeverAsk << true;

        QTest::newRow("Invalid collection") << false << "SomeUid4"
                                            << "Summary4" << Collection() << Collection() << true << IncidenceChanger::DestinationPolicyNeverAsk << false
                                            << IncidenceChanger::ResultCodeInvalidDefaultCollection;

        QTest::newRow("Default collection") << false << "SomeUid5"
                                            << "Summary5" << Collection() << mCollection << true << IncidenceChanger::DestinationPolicyDefault << false
                                            << IncidenceChanger::ResultCodeSuccess;

        // In this case, the collection dialog shouldn't be shown, as we only have 1 collection
        QTest::newRow("Only one collection") << false << "SomeUid6"
                                             << "Summary6" << Collection() << Collection() << true << IncidenceChanger::DestinationPolicyAsk << false
                                             << IncidenceChanger::ResultCodeSuccess;

        Collection collectionWithoutRights = Collection(mCollection.id());
        collectionWithoutRights.setRights(Collection::Rights());
        Q_ASSERT((mCollection.rights() & Akonadi::Collection::CanCreateItem));

        QTest::newRow("No rights") << false << "SomeUid6"
                                   << "Summary6" << Collection() << collectionWithoutRights << true << IncidenceChanger::DestinationPolicyNeverAsk << false
                                   << IncidenceChanger::ResultCodePermissions;

        QTest::newRow("No rights but its ok") << false << "SomeUid7"
                                              << "Summary7" << Collection() << collectionWithoutRights << false << IncidenceChanger::DestinationPolicyNeverAsk
                                              << false << IncidenceChanger::ResultCodeSuccess;
    }

    void testCreating()
    {
        QFETCH(bool, sendInvalidIncidence);
        QFETCH(QString, uid);
        QFETCH(QString, summary);
        QFETCH(Akonadi::Collection, destinationCollection);
        QFETCH(Akonadi::Collection, defaultCollection);
        QFETCH(bool, respectsCollectionRights);
        QFETCH(Akonadi::IncidenceChanger::DestinationPolicy, destinationPolicy);
        QFETCH(bool, failureExpected);

        Incidence::Ptr incidence;

        if (!sendInvalidIncidence) {
            incidence = Incidence::Ptr(new Event());
            incidence->setUid(uid);
            incidence->setSummary(summary);
        }
        mChanger->setRespectsCollectionRights(respectsCollectionRights);
        mChanger->setDestinationPolicy(destinationPolicy);
        mChanger->setDefaultCollection(defaultCollection);
        const int changeId = mChanger->createIncidence(incidence, destinationCollection);

        QVERIFY(failureExpected ^ (changeId != -1));

        if (!failureExpected) {
            QFETCH(Akonadi::IncidenceChanger::ResultCode, expectedResultCode);
            mIncidencesToAdd = 1;

            mExpectedResultByChangeId.insert(changeId, expectedResultCode);
            waitForSignals();

            if (expectedResultCode == IncidenceChanger::ResultCodeSuccess && !failureExpected) {
                Item item;
                QVERIFY(mItemIdByChangeId.contains(changeId));
                item.setId(mItemIdByChangeId.value(changeId));
                auto fetchJob = new ItemFetchJob(item, this);
                fetchJob->fetchScope().fetchFullPayload();
                AKVERIFYEXEC(fetchJob);
                QVERIFY(!fetchJob->items().isEmpty());
                Item retrievedItem = fetchJob->items().first();
                QVERIFY(retrievedItem.isValid());
                QVERIFY(retrievedItem.hasPayload());
                QVERIFY(retrievedItem.hasPayload<KCalendarCore::Event::Ptr>());
                QVERIFY(retrievedItem.hasPayload<KCalendarCore::Incidence::Ptr>());
                auto incidence = retrievedItem.payload<KCalendarCore::Incidence::Ptr>();
                QCOMPARE(incidence->summary(), summary);
                QCOMPARE(incidence->uid(), uid);
            }
        }
        mItemIdByChangeId.clear();
    }

    void testDeleting_data()
    {
        QTest::addColumn<Akonadi::Item::List>("items");
        QTest::addColumn<bool>("respectsCollectionRights");
        QTest::addColumn<bool>("failureExpected");
        QTest::addColumn<Akonadi::IncidenceChanger::ResultCode>("expectedResultCode");

        QTest::newRow("Delete empty list") << Item::List() << true << true;
        QTest::newRow("Delete invalid item") << (Item::List() << Item()) << true << true;

        auto fetchJob = new ItemFetchJob(mCollection);
        fetchJob->fetchScope().fetchFullPayload();
        AKVERIFYEXEC(fetchJob);
        Item::List items = fetchJob->items();

        // 5 Incidences were created in testCreating(). Keep this in sync.
        QCOMPARE(items.count(), 5);
        QTest::newRow("Simple delete") << (Item::List() << items.at(0)) << true << false << IncidenceChanger::ResultCodeSuccess;

        QTest::newRow("Delete already deleted") << (Item::List() << items.at(0)) << true << false << IncidenceChanger::ResultCodeAlreadyDeleted;

        QTest::newRow("Delete all others") << (Item::List() << items.at(1) << items.at(2)) << true << false << IncidenceChanger::ResultCodeSuccess;

        Collection collectionWithoutRights = Collection(mCollection.id());
        collectionWithoutRights.setRights(Collection::Rights());
        Item item = items.at(3);
        item.setParentCollection(collectionWithoutRights);
        QTest::newRow("Delete can't delete") << (Item::List() << item) << true << false << IncidenceChanger::ResultCodePermissions;
    }

    void testDeleting()
    {
        QFETCH(Akonadi::Item::List, items);
        QFETCH(bool, respectsCollectionRights);
        QFETCH(bool, failureExpected);
        mChanger->setRespectsCollectionRights(respectsCollectionRights);
        const int changeId = mChanger->deleteIncidences(items);

        QVERIFY(failureExpected ^ (changeId != -1));

        if (!failureExpected) {
            QFETCH(Akonadi::IncidenceChanger::ResultCode, expectedResultCode);
            mIncidencesToDelete = 1;
            mExpectedResultByChangeId.insert(changeId, expectedResultCode);
            waitForSignals();

            if (expectedResultCode == IncidenceChanger::ResultCodeSuccess) {
                // Check that the incidence was really deleted
                for (const Akonadi::Item &item : std::as_const(items)) {
                    auto fetchJob = new ItemFetchJob(item, this);
                    fetchJob->fetchScope().fetchFullPayload();
                    QVERIFY(!fetchJob->exec());
                    QVERIFY(fetchJob->items().isEmpty());
                    delete fetchJob;
                }
            }
        }
    }

    void testModifying_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<QString>("newSummary");
        QTest::addColumn<bool>("respectsCollectionRights");
        QTest::addColumn<int>("expectedRevision");
        QTest::addColumn<bool>("failureExpected");
        QTest::addColumn<Akonadi::IncidenceChanger::ResultCode>("expectedResultCode");

        QTest::newRow("Invalid item") << Item() << QString() << true << 0 << true;
        QTest::newRow("Valid item, invalid payload") << Item(1) << QString() << true << 0 << true;

        Item item;
        item.setMimeType(Event::eventMimeType());
        Incidence::Ptr incidence = Incidence::Ptr(new Event());
        incidence->setUid(QStringLiteral("test123uid"));
        incidence->setSummary(QStringLiteral("summary"));
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
        auto job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
        item = job->item();
        incidence->setSummary(QStringLiteral("New Summary"));
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

        QTest::newRow("Change summary") << item << "New Summary" << true << 1 << false << IncidenceChanger::ResultCodeSuccess;

        Collection collectionWithoutRights = Collection(mCollection.id());
        collectionWithoutRights.setRights(Collection::Rights());
        item.setParentCollection(collectionWithoutRights);

        QTest::newRow("Can't change") << item << "New Summary" << true << 1 << false << IncidenceChanger::ResultCodePermissions;
    }

    void testModifying()
    {
        QFETCH(Akonadi::Item, item);
        QFETCH(QString, newSummary);
        QFETCH(bool, respectsCollectionRights);
        QFETCH(int, expectedRevision);
        QFETCH(bool, failureExpected);

        mChanger->setRespectsCollectionRights(respectsCollectionRights);
        const int changeId = mChanger->modifyIncidence(item);
        QVERIFY(failureExpected ^ (changeId != -1));

        if (!failureExpected) {
            QFETCH(Akonadi::IncidenceChanger::ResultCode, expectedResultCode);

            mIncidencesToModify = 1;
            mExpectedResultByChangeId.insert(changeId, expectedResultCode);
            waitForSignals();
            auto fetchJob = new ItemFetchJob(item, this);
            fetchJob->fetchScope().fetchFullPayload();
            AKVERIFYEXEC(fetchJob);
            QVERIFY(fetchJob->items().count() == 1);
            Item fetchedItem = fetchJob->items().constFirst();
            QVERIFY(fetchedItem.isValid());
            QVERIFY(fetchedItem.hasPayload<KCalendarCore::Incidence::Ptr>());
            auto incidence = fetchedItem.payload<KCalendarCore::Incidence::Ptr>();
            QCOMPARE(incidence->summary(), newSummary);
            QCOMPARE(incidence->revision(), expectedRevision);
            delete fetchJob;
        }
    }

    void testModifyingAlarmSettings()
    {
        // A user should be able to change alarm settings independently
        // do not trigger a revision increment
        // kolab #1386
        Item item;
        item.setMimeType(Event::eventMimeType());
        Incidence::Ptr incidence = Incidence::Ptr(new Event());
        incidence->setUid(QStringLiteral("test123uid"));
        incidence->setSummary(QStringLiteral("summary"));
        incidence->setOrganizer(Person(QStringLiteral("orga"), QStringLiteral("orga@dev.nul")));
        incidence->setDirtyFields(QSet<IncidenceBase::Field>());
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
        auto job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
        item = job->item();
        Alarm::Ptr alarm = Alarm::Ptr(new Alarm(incidence.data()));
        alarm->setStartOffset(Duration(-15));
        alarm->setType(Alarm::Display);
        incidence->addAlarm(alarm);
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

        mChanger->setRespectsCollectionRights(true);
        const int changeId = mChanger->modifyIncidence(item);
        QVERIFY(changeId != -1);

        mIncidencesToModify = 1;
        mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
        waitForSignals();
        auto fetchJob = new ItemFetchJob(item, this);
        fetchJob->fetchScope().fetchFullPayload();
        AKVERIFYEXEC(fetchJob);
        QVERIFY(fetchJob->items().count() == 1);
        Item fetchedItem = fetchJob->items().constFirst();
        QVERIFY(fetchedItem.isValid());
        QVERIFY(fetchedItem.hasPayload<KCalendarCore::Incidence::Ptr>());
        auto incidence2 = fetchedItem.payload<KCalendarCore::Incidence::Ptr>();
        QCOMPARE(incidence2->alarms().count(), 1);
        QCOMPARE(incidence2->revision(), 0);
        delete fetchJob;
    }

    void testModifyRescedule_data()
    {
        // When a event is resceduled than all attendees part status should set to NEEDS-ACTION
        // kolab #4533
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<Event::Ptr>("event");
        QTest::addColumn<bool>("expectReset");

        Attendee us(QString(), s_ourEmail);
        us.setStatus(Attendee::Accepted);
        Attendee mia(QStringLiteral("Mia Wallace"), QStringLiteral("mia@dev.nul"));
        mia.setStatus(Attendee::Declined);
        mia.setRSVP(false);
        Attendee vincent(QStringLiteral("Vincent"), QStringLiteral("vincent@dev.nul"));
        vincent.setStatus(Attendee::Delegated);
        Attendee jules(QStringLiteral("Jules"), QStringLiteral("jules@dev.nul"));
        jules.setStatus(Attendee::Accepted);
        jules.setRole(Attendee::NonParticipant);

        // we as organizator
        Item item;
        item.setMimeType(Event::eventMimeType());
        Event::Ptr incidence = Event::Ptr(new Event());
        incidence->setUid(QStringLiteral("test123uid"));
        incidence->setDtStart(QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), QTimeZone::utc()));
        incidence->setDtEnd(QDateTime(QDate(2006, 1, 8), QTime(14, 0, 0), QTimeZone::utc()));
        incidence->setAllDay(false);
        incidence->setLocation(QStringLiteral("location"));
        incidence->setOrganizer(Person(QString(), s_ourEmail));
        incidence->addAttendee(us);
        incidence->addAttendee(mia);
        incidence->addAttendee(vincent);
        incidence->addAttendee(jules);
        incidence->setDirtyFields(QSet<IncidenceBase::Field>());
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence));
            event->setDtStart(QDateTime(QDate(2006, 1, 8), QTime(13, 0, 0), QTimeZone::utc()));
            QCOMPARE(event->dirtyFields().count(), 1);
            QTest::newRow("organizator:start Date") << item << event << true;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence));
            event->setDtEnd(QDateTime(QDate(2006, 1, 8), QTime(13, 0, 0), QTimeZone::utc()));
            QCOMPARE(event->dirtyFields().count(), 1);
            QTest::newRow("organizator:end Date") << item << event << true;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence));
            event->setAllDay(true);
            QCOMPARE(event->dirtyFields().count(), 2);
            QTest::newRow("organizator:allDay") << item << event << true;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence));
            event->setLocation(QStringLiteral("location2"));
            QCOMPARE(event->dirtyFields().count(), 1);
            QTest::newRow("organizator:location") << item << event << true;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence));
            event->setSummary(QStringLiteral("summary"));
            QCOMPARE(event->dirtyFields().count(), 1);
            QTest::newRow("organizator:summary") << item << event << false;
        }

        // we are normal attendee
        Item item2;
        item2.setMimeType(Event::eventMimeType());
        Event::Ptr incidence2 = Event::Ptr(new Event());
        incidence2->setUid(QStringLiteral("test123uid"));
        incidence2->setDtStart(QDateTime(QDate(2006, 1, 8), QTime(12, 0, 0), QTimeZone::utc()));
        incidence2->setDtEnd(QDateTime(QDate(2006, 1, 8), QTime(14, 0, 0), QTimeZone::utc()));
        incidence2->setAllDay(false);
        incidence2->setLocation(QStringLiteral("location"));
        incidence2->setOrganizer(Person(QStringLiteral("External organizator"), QStringLiteral("exorga@dev.nul")));
        incidence2->addAttendee(us);
        incidence2->addAttendee(mia);
        incidence2->addAttendee(vincent);
        incidence2->addAttendee(jules);
        incidence2->setDirtyFields(QSet<IncidenceBase::Field>());
        item2.setPayload<KCalendarCore::Incidence::Ptr>(incidence2);

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence2));
            event->setDtStart(QDateTime(QDate(2006, 1, 8), QTime(13, 0, 0), QTimeZone::utc()));
            QTest::newRow("attendee:start Date") << item2 << event << false;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence2));
            event->setDtEnd(QDateTime(QDate(2006, 1, 8), QTime(13, 0, 0), QTimeZone::utc()));
            QTest::newRow("attendee:end Date") << item2 << event << false;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence2));
            event->setAllDay(false);
            QTest::newRow("attendee:allDay") << item2 << event << false;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence2));
            event->setLocation(QStringLiteral("location2"));
            QTest::newRow("attendee:location") << item2 << event << false;
        }

        {
            Event::Ptr event = Event::Ptr(new Event(*incidence2));
            event->setSummary(QStringLiteral("summary"));
            QTest::newRow("attendee:summary") << item2 << event << false;
        }
    }

    void testModifyRescedule()
    {
        QFETCH(Akonadi::Item, item);
        QFETCH(Event::Ptr, event);
        QFETCH(bool, expectReset);

        item.setId(-1);
        auto job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
        item = job->item();
        item.setPayload<KCalendarCore::Incidence::Ptr>(event);

        int revision = event->revision();

        mChanger->setRespectsCollectionRights(true);
        const int changeId = mChanger->modifyIncidence(item);
        QVERIFY(changeId != -1);

        mIncidencesToModify = 1;
        mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
        waitForSignals();
        auto fetchJob = new ItemFetchJob(item, this);
        fetchJob->fetchScope().fetchFullPayload();
        AKVERIFYEXEC(fetchJob);
        QVERIFY(fetchJob->items().count() == 1);
        Item fetchedItem = fetchJob->items().constFirst();

        QVERIFY(fetchedItem.isValid());
        QVERIFY(fetchedItem.hasPayload<KCalendarCore::Event::Ptr>());
        auto incidence = fetchedItem.payload<KCalendarCore::Event::Ptr>();

        QCOMPARE(incidence->revision(), revision + 1);

        if (expectReset) {
            if (incidence->organizer().email() == s_ourEmail) {
                QCOMPARE(incidence->attendeeByMail(s_ourEmail).status(), Attendee::Accepted);
            } else {
                QCOMPARE(incidence->attendeeByMail(s_ourEmail).status(), Attendee::NeedsAction);
            }
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("mia@dev.nul")).status(), Attendee::NeedsAction);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("mia@dev.nul")).RSVP(), true);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("vincent@dev.nul")).status(), Attendee::Delegated);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("vincent@dev.nul")).RSVP(), false);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("jules@dev.nul")).status(), Attendee::Accepted);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("jules@dev.nul")).RSVP(), false);
        } else {
            QCOMPARE(incidence->attendeeByMail(s_ourEmail).status(), Attendee::Accepted);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("mia@dev.nul")).status(), Attendee::Declined);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("vincent@dev.nul")).status(), Attendee::Delegated);
            QCOMPARE(incidence->attendeeByMail(QStringLiteral("jules@dev.nul")).status(), Attendee::Accepted);
        }
        delete fetchJob;
    }

    void testMassModifyForConflicts_data()
    {
        QTest::addColumn<Akonadi::Item>("item");
        QTest::addColumn<bool>("waitForPreviousJob");
        QTest::addColumn<int>("numberOfModifications");

        // Create an incidence
        Item item;
        item.setMimeType(Event::eventMimeType());
        Incidence::Ptr incidence = Incidence::Ptr(new Event());
        incidence->setUid(QStringLiteral("test123uid"));
        incidence->setSummary(QStringLiteral("summary"));
        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
        auto job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
        item = job->item();

        QTest::newRow("15 modifications in sequence") << item << true << 15;
        QTest::newRow("15 modifications in parallel") << item << false << 15;
    }

    void testMassModifyForConflicts()
    {
        QFETCH(Akonadi::Item, item);
        QFETCH(bool, waitForPreviousJob);
        QFETCH(int, numberOfModifications);
        mDiscardedEqualsSuccess = true;

        Q_ASSERT(numberOfModifications > 0);

        if (!waitForPreviousJob) {
            mIncidencesToModify = numberOfModifications;
        }

        int changeId = -1;
        for (int i = 0; i < numberOfModifications; ++i) {
            auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
            Q_ASSERT(incidence);
            incidence->setSummary(QString::number(i));
            changeId = mChanger->modifyIncidence(item);
            QVERIFY(changeId != -1);

            if (waitForPreviousJob) {
                mIncidencesToModify = 1;
                mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
                waitForSignals();
                auto fetchJob = new ItemFetchJob(item, this);
                fetchJob->fetchScope().fetchFullPayload();
                AKVERIFYEXEC(fetchJob);
                QVERIFY(fetchJob->items().count() == 1);
                QCOMPARE(fetchJob->items().first().payload<KCalendarCore::Incidence::Ptr>()->summary(), QString::number(i));
            }
        }

        if (!waitForPreviousJob) {
            mIncidencesToModify = numberOfModifications;
            // Wait for the last one
            mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
            waitForChange(changeId);
            auto fetchJob = new ItemFetchJob(item, this);
            fetchJob->fetchScope().fetchFullPayload();
            AKVERIFYEXEC(fetchJob);
            QVERIFY(fetchJob->items().count() == 1);
            QCOMPARE(fetchJob->items().first().payload<KCalendarCore::Incidence::Ptr>()->summary(), QString::number(numberOfModifications - 1));
            if (mIncidencesToModify > 0) {
                waitForSignals();
            }
        }

        mDiscardedEqualsSuccess = false;
    }

    void testAtomicOperations_data()
    {
        QTest::addColumn<Akonadi::Item::List>("items");
        QTest::addColumn<QList<Akonadi::IncidenceChanger::ChangeType>>("changeTypes");
        QTest::addColumn<QList<bool>>("failureExpectedList");
        QTest::addColumn<QList<Akonadi::IncidenceChanger::ResultCode>>("expectedResults");
        QTest::addColumn<QList<Akonadi::Collection::Rights>>("rights");
        QTest::addColumn<bool>("permissionsOrRollback");

        Akonadi::Item::List items;
        QList<Akonadi::IncidenceChanger::ChangeType> changeTypes;
        QList<bool> failureExpectedList;
        QList<Akonadi::IncidenceChanger::ResultCode> expectedResults;
        QList<Akonadi::Collection::Rights> rights;
        const Collection::Rights allRights = Collection::AllRights;
        const Collection::Rights noRights = Collection::Rights();
        //------------------------------------------------------------------------------------------
        // Create two incidences, should succeed
        items << item() << item();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate;
        failureExpectedList << false << false;
        expectedResults << IncidenceChanger::ResultCodeSuccess << IncidenceChanger::ResultCodeSuccess;
        rights << allRights << allRights;

        QTest::newRow("create two - success ") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeModify << IncidenceChanger::ChangeTypeModify;
        items.clear();
        items << createItem(mCollection) << createItem(mCollection);

        QTest::newRow("modify two - success ") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeDelete << IncidenceChanger::ChangeTypeDelete;
        QTest::newRow("delete two - success ") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // Creation succeeds but deletion doesn't ( invalid item case )
        items.clear();
        items << item() << Item(); // Invalid item on purpose
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeDelete;
        failureExpectedList.clear();
        failureExpectedList << false << true;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("create,try delete") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // deletion doesn't succeed, but creation does ( invalid item case )
        items.clear();
        items << Item() << item(); // Invalid item on purpose
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeDelete << IncidenceChanger::ChangeTypeCreate;
        failureExpectedList.clear();
        failureExpectedList << true << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("try delete,create") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // Creation succeeds but deletion doesn't ( valid, inexistent item case )
        items.clear();
        items << item() << Item(10101010);
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeDelete;
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeJobError;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("create,try delete v2") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // deletion doesn't succeed, but creation does ( valid, inexistent item case )
        items.clear();
        items << Item(10101010) << item();
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeDelete << IncidenceChanger::ChangeTypeCreate;
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeJobError << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("try delete,create v2") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // deletion doesn't succeed, but creation does ( NO ACL case )
        items.clear();
        items << createItem(mCollection) << item();
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeDelete << IncidenceChanger::ChangeTypeCreate;
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << noRights << allRights;

        QTest::newRow("try delete(ACL),create") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // Creation succeeds but deletion doesn't ( NO ACL case )
        items.clear();
        items << item() << createItem(mCollection);
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeDelete;
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodePermissions;
        rights.clear();
        rights << allRights << noRights;

        QTest::newRow("create,try delete(ACL)") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 1 successful modification, 1 failed creation
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeModify << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << createItem(mCollection) << Item();
        failureExpectedList.clear();
        failureExpectedList << false << true;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("modify,try create") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 1 successful modification, 1 failed creation
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeModify << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << createItem(mCollection) << item();
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodePermissions;
        rights.clear();
        rights << allRights << noRights;

        QTest::newRow("modify,try create v2") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 1 failed creation, 1 successful modification
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeModify;
        items.clear();
        items << Item() << createItem(mCollection);
        failureExpectedList.clear();
        failureExpectedList << true << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << allRights;

        QTest::newRow("try create,modify") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 1 failed creation, 1 successful modification
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeModify;
        items.clear();
        items << item() << createItem(mCollection);
        failureExpectedList.clear();
        failureExpectedList << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << noRights << allRights;

        QTest::newRow("try create,modify v2") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 4 creations, last one fails
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate
                    << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << item() << item() << item() << item();
        failureExpectedList.clear();
        failureExpectedList << false << false << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback
                        << IncidenceChanger::ResultCodePermissions;
        rights.clear();
        rights << allRights << allRights << allRights << noRights;

        QTest::newRow("create 4, last fails") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 4 creations, first one fails
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate
                    << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << item() << item() << item() << item();
        failureExpectedList.clear();
        failureExpectedList << false << false << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodeRolledback
                        << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << noRights << allRights << allRights << allRights;

        QTest::newRow("create 4, first fails") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 4 creations, second one fails
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate
                    << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << item() << item() << item() << item();
        failureExpectedList.clear();
        failureExpectedList << false << false << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodeRolledback << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodeRolledback
                        << IncidenceChanger::ResultCodeRolledback;
        rights.clear();
        rights << allRights << noRights << allRights << allRights;

        QTest::newRow("create 4, second fails") << items << changeTypes << failureExpectedList << expectedResults << rights << false;
        //------------------------------------------------------------------------------------------
        // 4 fails
        changeTypes.clear();
        changeTypes << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate << IncidenceChanger::ChangeTypeCreate
                    << IncidenceChanger::ChangeTypeCreate;
        items.clear();
        items << item() << item() << item() << item();
        failureExpectedList.clear();
        failureExpectedList << false << false << false << false;
        expectedResults.clear();
        expectedResults << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodePermissions << IncidenceChanger::ResultCodePermissions
                        << IncidenceChanger::ResultCodePermissions;
        rights.clear();
        rights << noRights << noRights << noRights << noRights;

        QTest::newRow("create 4, all fail") << items << changeTypes << failureExpectedList << expectedResults << rights << true;
        //------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------
        //------------------------------------------------------------------------------------------
    }

    void testAtomicOperations()
    {
        QFETCH(Akonadi::Item::List, items);
        QFETCH(QList<Akonadi::IncidenceChanger::ChangeType>, changeTypes);
        QFETCH(QList<bool>, failureExpectedList);
        QFETCH(QList<Akonadi::IncidenceChanger::ResultCode>, expectedResults);
        QFETCH(QList<Akonadi::Collection::Rights>, rights);
        QFETCH(bool, permissionsOrRollback);

        QCOMPARE(items.count(), changeTypes.count());
        QCOMPARE(items.count(), failureExpectedList.count());
        QCOMPARE(items.count(), expectedResults.count());
        QCOMPARE(items.count(), rights.count());

        mPermissionsOrRollback = permissionsOrRollback;
        mChanger->setDefaultCollection(mCollection);
        mChanger->setRespectsCollectionRights(true);
        mChanger->setDestinationPolicy(IncidenceChanger::DestinationPolicyNeverAsk);
        mChanger->startAtomicOperation();
        mIncidencesToAdd = 0;
        mIncidencesToModify = 0;
        mIncidencesToDelete = 0;
        for (int i = 0; i < items.count(); ++i) {
            mCollection.setRights(rights[i]);
            mChanger->setDefaultCollection(mCollection);
            const Akonadi::Item item = items[i];
            int changeId = -1;
            switch (changeTypes[i]) {
            case IncidenceChanger::ChangeTypeCreate:
                changeId = mChanger->createIncidence(item.hasPayload() ? item.payload<KCalendarCore::Incidence::Ptr>() : Incidence::Ptr());
                if (changeId != -1) {
                    ++mIncidencesToAdd;
                }
                break;
            case IncidenceChanger::ChangeTypeDelete:
                changeId = mChanger->deleteIncidence(item);
                if (changeId != -1) {
                    ++mIncidencesToDelete;
                }
                break;
            case IncidenceChanger::ChangeTypeModify:
                QVERIFY(item.isValid());
                QVERIFY(item.hasPayload<KCalendarCore::Incidence::Ptr>());
                item.payload<KCalendarCore::Incidence::Ptr>()->setSummary(QStringLiteral("Changed"));
                changeId = mChanger->modifyIncidence(item);
                if (changeId != -1) {
                    ++mIncidencesToModify;
                }
                break;
            default:
                QVERIFY(false);
            }
            QVERIFY(!((changeId == -1) ^ failureExpectedList[i]));
            if (changeId != -1) {
                mExpectedResultByChangeId.insert(changeId, expectedResults[i]);
            }
        }
        mChanger->endAtomicOperation();

        waitForSignals();

        // Validate:
        for (int i = 0; i < items.count(); ++i) {
            const bool expectedSuccess = (expectedResults[i] == IncidenceChanger::ResultCodeSuccess);
            mCollection.setRights(rights[i]);

            Akonadi::Item item = items[i];

            switch (changeTypes[i]) {
            case IncidenceChanger::ChangeTypeCreate:
                if (expectedSuccess) {
                    QVERIFY(item.hasPayload<KCalendarCore::Incidence::Ptr>());
                    auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
                    QVERIFY(incidence);
                    QVERIFY(!incidence->uid().isEmpty());
                    QVERIFY(mItemIdByUid.contains(incidence->uid()));
                    auto fJob = new ItemFetchJob(Item(mItemIdByUid.value(incidence->uid())));
                    fJob->fetchScope().fetchFullPayload();
                    AKVERIFYEXEC(fJob);
                    QCOMPARE(fJob->items().count(), 1);
                    QVERIFY(fJob->items().constFirst().isValid());
                    QVERIFY(fJob->items().constFirst().hasPayload());
                    QVERIFY(fJob->items().constFirst().hasPayload<KCalendarCore::Incidence::Ptr>());
                    QCOMPARE(item.payload<KCalendarCore::Incidence::Ptr>()->uid(), fJob->items().constFirst().payload<KCalendarCore::Incidence::Ptr>()->uid());
                }
                break;
            case IncidenceChanger::ChangeTypeDelete:
                if (expectedSuccess) {
                    auto fJob = new ItemFetchJob(Item(item.id()));
                    QVERIFY(!fJob->exec());
                }
                break;
            case IncidenceChanger::ChangeTypeModify:
                if (expectedSuccess) {
                    auto fJob = new ItemFetchJob(Item(item.id()));
                    fJob->fetchScope().fetchFullPayload();
                    AKVERIFYEXEC(fJob);
                    QCOMPARE(item.payload<KCalendarCore::Incidence::Ptr>()->summary(),
                             fJob->items().first().payload<KCalendarCore::Incidence::Ptr>()->summary());
                }
                break;
            default:
                QVERIFY(false);
            }
        }
        mPermissionsOrRollback = false;
    }

    void testAdjustRecurrence_data()
    {
        QTest::addColumn<bool>("allDay");
        QTest::addColumn<QDateTime>("dtStart");
        QTest::addColumn<QDateTime>("dtEnd");
        QTest::addColumn<int>("offsetToMove");
        QTest::addColumn<int>("frequency");
        QTest::addColumn<KCalendarCore::RecurrenceRule::PeriodType>("recurrenceType");

        // For weekly recurrences
        QTest::addColumn<QBitArray>("weekDays");
        QTest::addColumn<QBitArray>("expectedWeekDays");

        // Recurrence end
        QTest::addColumn<QDate>("recurrenceEnd");
        QTest::addColumn<QDate>("expectedRecurrenceEnd");

        const QDateTime dtStart = QDateTime(QDate::currentDate(), QTime(8, 0));
        const QDateTime dtEnd = dtStart.addSecs(3600);
        const int one_day = 3600 * 24;
        const int one_hour = 3600;
        QBitArray days(7);
        QBitArray expectedDays(7);

        //-------------------------------------------------------------------------
        days.setBit(dtStart.date().dayOfWeek() - 1);
        expectedDays.setBit(dtStart.addSecs(one_day).date().dayOfWeek() - 1);

        QTest::newRow("weekly") << false << dtStart << dtEnd << one_day << 1 << KCalendarCore::RecurrenceRule::rWeekly << days << expectedDays << QDate()
                                << QDate();
        //-------------------------------------------------------------------------
        days.fill(false);
        days.setBit(dtStart.date().dayOfWeek() - 1);
        expectedDays.setBit(dtStart.addSecs(one_day).date().dayOfWeek() - 1);
        QTest::newRow("weekly allday") << true << QDateTime(dtStart.date().startOfDay()) << QDateTime(dtEnd.date().startOfDay()) << one_day << 1
                                       << KCalendarCore::RecurrenceRule::rWeekly << days << expectedDays << QDate() << QDate();
        //-------------------------------------------------------------------------
        // Here nothing should change
        days.fill(false);
        days.setBit(dtStart.date().dayOfWeek() - 1);

        QTest::newRow("weekly nop") << false << dtStart << dtEnd << one_hour << 1 << KCalendarCore::RecurrenceRule::rWeekly << days << days << QDate()
                                    << QDate();
        //-------------------------------------------------------------------------
        // Test with multiple week days. Only the weekday from the old DTSTART should be unset.
        days.fill(true);
        expectedDays = days;
        expectedDays.clearBit(dtStart.date().dayOfWeek() - 1);
        QTest::newRow("weekly multiple") << false << dtStart << dtEnd << one_day << 1 << KCalendarCore::RecurrenceRule::rWeekly << days << expectedDays
                                         << QDate() << QDate();
        //-------------------------------------------------------------------------
        // Testing moving an event such that DTSTART > recurrence end, which would
        // result in the event disappearing from all views.
        QTest::newRow("recur end") << false << dtStart << dtEnd << one_day * 7 << 1 << KCalendarCore::RecurrenceRule::rDaily << QBitArray() << QBitArray()
                                   << dtStart.date().addDays(3) << QDate();
        //-------------------------------------------------------------------------
        mCollection.setRights(Collection::Rights(Collection::AllRights));
    }

    void testAdjustRecurrence()
    {
        QFETCH(bool, allDay);
        QFETCH(QDateTime, dtStart);
        QFETCH(QDateTime, dtEnd);
        QFETCH(int, offsetToMove);
        QFETCH(int, frequency);
        QFETCH(KCalendarCore::RecurrenceRule::PeriodType, recurrenceType);
        QFETCH(QBitArray, weekDays);
        QFETCH(QBitArray, expectedWeekDays);
        QFETCH(QDate, recurrenceEnd);
        QFETCH(QDate, expectedRecurrenceEnd);

        Event::Ptr incidence = Event::Ptr(new Event());
        incidence->setSummary(QStringLiteral("random summary"));
        incidence->setDtStart(dtStart);
        incidence->setDtEnd(dtEnd);
        incidence->setAllDay(allDay);

        Recurrence *recurrence = incidence->recurrence();

        switch (recurrenceType) {
        case KCalendarCore::RecurrenceRule::rDaily:
            recurrence->setDaily(frequency);
            break;
        case KCalendarCore::RecurrenceRule::rWeekly:
            recurrence->setWeekly(frequency, weekDays);
            break;
        default:
            // Not tested yet
            Q_ASSERT(false);
            QVERIFY(false);
        }

        if (recurrenceEnd.isValid()) {
            recurrence->setEndDate(recurrenceEnd);
        }

        // Create the recurring incidence
        int changeId = mChanger->createIncidence(incidence, mCollection);
        QVERIFY(changeId != -1);
        mIncidencesToAdd = 1;
        mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
        waitForSignals();

        // Now lets move it
        Incidence::Ptr originalIncidence = Incidence::Ptr(incidence->clone());
        incidence->setDtStart(dtStart.addSecs(offsetToMove));

        incidence->setDtEnd(dtEnd.addSecs(offsetToMove));
        mLastItemCreated.setPayload(incidence);
        changeId = mChanger->modifyIncidence(mLastItemCreated, originalIncidence);
        QVERIFY(changeId != -1);
        mIncidencesToModify = 1;
        mExpectedResultByChangeId.insert(changeId, IncidenceChanger::ResultCodeSuccess);
        waitForSignals();

        // Now check the results
        switch (recurrenceType) {
        case KCalendarCore::RecurrenceRule::rDaily:
            break;
        case KCalendarCore::RecurrenceRule::rWeekly:
            QCOMPARE(incidence->recurrence()->days(), expectedWeekDays);
            if (weekDays != expectedWeekDays) {
                QVERIFY(incidence->dirtyFields().contains(IncidenceBase::FieldRecurrence));
            }
            break;
        default:
            // Not tested yet
            Q_ASSERT(false);
            QVERIFY(false);
        }

        if (recurrenceEnd.isValid() && !expectedRecurrenceEnd.isValid()) {
            QVERIFY(incidence->recurrence()->endDate() >= incidence->dtStart().date());
            QVERIFY(incidence->dirtyFields().contains(IncidenceBase::FieldRecurrence));
        }
    }

public Q_SLOTS:

    void waitForSignals()
    {
        QTestEventLoop::instance().enterLoop(10);

        if (QTestEventLoop::instance().timeout()) {
            qDebug() << "Remaining: " << mIncidencesToAdd << mIncidencesToDelete << mIncidencesToModify;
        }
        QVERIFY(!QTestEventLoop::instance().timeout());
    }

    // Waits for a specific change
    void waitForChange(int changeId)
    {
        mChangeToWaitFor = changeId;

        int i = 0;
        while (mChangeToWaitFor != -1 && i++ < 10) { // wait 10 seconds max.
            QTest::qWait(100);
        }

        QVERIFY(mChangeToWaitFor == -1);
    }

    void deleteFinished(int changeId, const QList<Akonadi::Item::Id> &deletedIds, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
    {
        QVERIFY(changeId != -1);
        mChangeToWaitFor = -1;
        --mIncidencesToDelete;

        if (resultCode != IncidenceChanger::ResultCodeSuccess) {
            qDebug() << "Error string is " << errorMessage;
        } else {
            QVERIFY(!deletedIds.isEmpty());
            for (Akonadi::Item::Id id : std::as_const(deletedIds)) {
                QVERIFY(id != -1);
            }
        }

        compareExpectedResult(resultCode, mExpectedResultByChangeId[changeId], QStringLiteral("createFinished"));

        maybeQuitEventLoop();
    }

    void createFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
    {
        QVERIFY(changeId != -1);
        mChangeToWaitFor = -1;
        --mIncidencesToAdd;

        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            QVERIFY(item.isValid());
            QVERIFY(item.parentCollection().isValid());
            mItemIdByChangeId.insert(changeId, item.id());
            QVERIFY(item.hasPayload());
            auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
            mItemIdByUid.insert(incidence->uid(), item.id());
            mLastItemCreated = item;
        } else {
            qDebug() << "Error string is " << errorString;
        }

        compareExpectedResult(resultCode, mExpectedResultByChangeId[changeId], QStringLiteral("createFinished"));

        qDebug() << "Createfinished " << mIncidencesToAdd;
        maybeQuitEventLoop();
    }

    void modifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
    {
        mChangeToWaitFor = -1;
        --mIncidencesToModify;
        QVERIFY(changeId != -1);

        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            QVERIFY(item.isValid());
        } else {
            qDebug() << "Error string is " << errorString;
        }

        compareExpectedResult(resultCode, mExpectedResultByChangeId[changeId], QStringLiteral("modifyFinished"));

        maybeQuitEventLoop();
    }

    void maybeQuitEventLoop()
    {
        if (mIncidencesToDelete == 0 && mIncidencesToAdd == 0 && mIncidencesToModify == 0) {
            QTestEventLoop::instance().exitLoop();
        }
    }

    void testDefaultCollection()
    {
        const Collection newCollection(42);
        IncidenceChanger changer;
        QCOMPARE(changer.defaultCollection(), Collection());
        changer.setDefaultCollection(newCollection);
        QCOMPARE(changer.defaultCollection(), newCollection);
    }

    void testDestinationPolicy()
    {
        IncidenceChanger changer;
        QCOMPARE(changer.destinationPolicy(), IncidenceChanger::DestinationPolicyDefault);
        changer.setDestinationPolicy(IncidenceChanger::DestinationPolicyNeverAsk);
        QCOMPARE(changer.destinationPolicy(), IncidenceChanger::DestinationPolicyNeverAsk);
    }

    void testDialogsOnError()
    {
        IncidenceChanger changer;
        QCOMPARE(changer.showDialogsOnError(), true);
        changer.setShowDialogsOnError(false);
        QCOMPARE(changer.showDialogsOnError(), false);
    }

    void testRespectsCollectionRights()
    {
        IncidenceChanger changer;
        QCOMPARE(changer.respectsCollectionRights(), true);
        changer.setRespectsCollectionRights(false);
        QCOMPARE(changer.respectsCollectionRights(), false);
    }

    void compareExpectedResult(IncidenceChanger::ResultCode result, IncidenceChanger::ResultCode expected, const QString &str)
    {
        if (mPermissionsOrRollback) {
            if (expected == IncidenceChanger::ResultCodePermissions) {
                expected = IncidenceChanger::ResultCodeRolledback;
            }

            if (result == IncidenceChanger::ResultCodePermissions) {
                result = IncidenceChanger::ResultCodeRolledback;
            }
        }

        if (mDiscardedEqualsSuccess) {
            if (expected == IncidenceChanger::ResultCodeModificationDiscarded) {
                expected = IncidenceChanger::ResultCodeSuccess;
            }

            if (result == IncidenceChanger::ResultCodeModificationDiscarded) {
                result = IncidenceChanger::ResultCodeSuccess;
            }
        }

        if (result != expected) {
            qDebug() << str << "Expected " << expected << " got " << result;
        }

        QCOMPARE(result, expected);
    }
};

QTEST_AKONADIMAIN(IncidenceChangerTest)

#include "incidencechangertest.moc"
