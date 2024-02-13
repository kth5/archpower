/*
    SPDX-FileCopyrightText: 2010-2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarbasetest.h"

#include "../src/calendarbase.h"
#include "../src/incidencechanger.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>
#include <akonadi/qtest_akonadi.h>

using namespace Akonadi;
using namespace KCalendarCore;

QTEST_AKONADIMAIN(CalendarBaseTest)

static bool compareUids(const QStringList &_uids, const Incidence::List &incidences)
{
    QStringList uids = _uids;

    for (const KCalendarCore::Incidence::Ptr &incidence : incidences) {
        if (uids.contains(incidence->uid())) {
            uids.removeAll(incidence->uid());
        }
    }

    if (uids.isEmpty() && _uids.count() == incidences.count()) {
        return true;
    } else {
        qDebug() << uids.count() << incidences.count();
        return false;
    }
}

void CalendarBaseTest::fetchCollection()
{
    auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
    // Get list of collections
    job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
    AKVERIFYEXEC(job);

    // Find our collection
    const Collection::List collections = job->collections();
    QVERIFY(!collections.isEmpty());
    mCollection = collections.first();

    QVERIFY(mCollection.isValid());
}

void CalendarBaseTest::createInitialIncidences()
{
    mExpectedSlotResult = true;

    for (int i = 0; i < 5; ++i) {
        Event::Ptr event = Event::Ptr(new Event());
        event->setUid(QStringLiteral("event") + QString::number(i));
        event->setSummary(QStringLiteral("summary") + QString::number(i));
        event->setDtStart(QDateTime::currentDateTimeUtc());
        mUids.append(event->uid());
        QVERIFY(mCalendar->addEvent(event));
        QTestEventLoop::instance().enterLoop(5);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }
    mOneEventUid = mUids.last();

    for (int i = 0; i < 5; ++i) {
        Todo::Ptr todo = Todo::Ptr(new Todo());
        todo->setUid(QStringLiteral("todo") + QString::number(i));
        todo->setDtStart(QDateTime::currentDateTimeUtc());
        todo->setSummary(QStringLiteral("summary") + QString::number(i));
        mUids.append(todo->uid());
        QVERIFY(mCalendar->addTodo(todo));
        QTestEventLoop::instance().enterLoop(5);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }
    mOneTodoUid = mUids.last();

    for (int i = 0; i < 5; ++i) {
        Journal::Ptr journal = Journal::Ptr(new Journal());
        journal->setUid(QStringLiteral("journal") + QString::number(i));
        journal->setSummary(QStringLiteral("summary") + QString::number(i));
        journal->setDtStart(QDateTime::currentDateTimeUtc());
        mUids.append(journal->uid());
        QVERIFY(mCalendar->addJournal(journal));
        QTestEventLoop::instance().enterLoop(5);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }
    mOneJournalUid = mUids.last();

    for (int i = 0; i < 5; ++i) {
        Incidence::Ptr incidence = Incidence::Ptr(new Event());
        incidence->setUid(QStringLiteral("incidence") + QString::number(i));
        incidence->setSummary(QStringLiteral("summary") + QString::number(i));
        incidence->setDtStart(QDateTime::currentDateTimeUtc());
        mUids.append(incidence->uid());
        QVERIFY(mCalendar->addIncidence(incidence));
        QTestEventLoop::instance().enterLoop(5);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }
    mOneIncidenceUid = mUids.last();
}

void CalendarBaseTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();

    fetchCollection();
    qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    mCalendar = new CalendarBase();
    mCalendar->incidenceChanger()->setDestinationPolicy(IncidenceChanger::DestinationPolicyDefault);
    mCalendar->incidenceChanger()->setDefaultCollection(mCollection);
    connect(mCalendar, &CalendarBase::createFinished, this, &CalendarBaseTest::handleCreateFinished);

    connect(mCalendar, &CalendarBase::deleteFinished, this, &CalendarBaseTest::handleDeleteFinished);
    createInitialIncidences();
}

void CalendarBaseTest::cleanupTestCase()
{
    delete mCalendar;
}

void CalendarBaseTest::testItem()
{
    for (const QString &uid : std::as_const(mUids)) {
        const Item item1 = mCalendar->item(uid);
        const Item item2 = mCalendar->item(item1.id());
        QVERIFY(item1.isValid());
        QVERIFY(item2.isValid());
        QCOMPARE(item1.id(), item2.id());
        QCOMPARE(item1.payload<KCalendarCore::Incidence::Ptr>()->uid(), uid);
        QCOMPARE(item2.payload<KCalendarCore::Incidence::Ptr>()->uid(), uid);
    }
}

void CalendarBaseTest::testChildIncidences_data()
{
    QTest::addColumn<QString>("parentUid");
    QTest::addColumn<Akonadi::Item::Id>("parentId");
    QTest::addColumn<QStringList>("childrenUids");

    QTest::newRow("Invalid parent") << "doesn't exist" << Item::Id(404) << QStringList();
    Item::Id id = createTodo(tr("parent1"));
    QVERIFY(id > -1);
    QVERIFY(createTodo(tr("child1"), tr("parent1")) > -1);
    QVERIFY(createTodo(tr("child2"), tr("parent1")) > -1);
    QTest::newRow("2 children") << "parent1" << id << (QStringList() << tr("child1") << tr("child2"));
}

void CalendarBaseTest::testChildIncidences()
{
    QFETCH(QString, parentUid);
    QFETCH(Akonadi::Item::Id, parentId);
    QFETCH(QStringList, childrenUids);
    KCalendarCore::Incidence::List children = mCalendar->childIncidences(parentId);
    QVERIFY(compareUids(childrenUids, children));
    children = mCalendar->childIncidences(parentUid);
    QVERIFY(compareUids(childrenUids, children));
}

void CalendarBaseTest::testDelete()
{ // No need for _data()
    const Item event = mCalendar->item(mOneEventUid);
    QVERIFY(event.isValid());
    const Item todo = mCalendar->item(mOneTodoUid);
    QVERIFY(todo.isValid());
    const Item journal = mCalendar->item(mOneJournalUid);
    QVERIFY(journal.isValid());
    const Item incidence = mCalendar->item(mOneIncidenceUid);
    QVERIFY(incidence.isValid());

    mExpectedSlotResult = true;
    QVERIFY(mCalendar->deleteEvent(event.payload<KCalendarCore::Event::Ptr>()));
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QVERIFY(mCalendar->deleteTodo(todo.payload<KCalendarCore::Todo::Ptr>()));
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QVERIFY(mCalendar->deleteJournal(journal.payload<KCalendarCore::Journal::Ptr>()));
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QVERIFY(mCalendar->deleteIncidence(incidence.payload<KCalendarCore::Incidence::Ptr>()));
    QTestEventLoop::instance().enterLoop(5);
    QVERIFY(!QTestEventLoop::instance().timeout());

    auto job1 = new ItemFetchJob(event, this);
    auto job2 = new ItemFetchJob(todo, this);
    auto job3 = new ItemFetchJob(journal, this);
    auto job4 = new ItemFetchJob(incidence, this);
    QVERIFY(!job1->exec());
    QVERIFY(!job2->exec());
    QVERIFY(!job3->exec());
    QVERIFY(!job4->exec());
    QVERIFY(mCalendar->item(event.id()) == Item());
    QVERIFY(mCalendar->item(todo.id()) == Item());
    QVERIFY(mCalendar->item(journal.id()) == Item());
    QVERIFY(mCalendar->item(incidence.id()) == Item());
}

/*
void CalendarBaseTest::testDeleteAll()
{
    mCalendar->deleteAllEvents();
    QTestEventLoop::instance().enterLoop( 5 );
    QVERIFY( !QTestEventLoop::instance().timeout() );
    QVERIFY( mCalendar->events().isEmpty() );
    QVERIFY( !mCalendar->journals().isEmpty() );
    QVERIFY( !mCalendar->todos().isEmpty() );

    mCalendar->deleteAllTodos();
    QTestEventLoop::instance().enterLoop( 5 );
    QVERIFY( !QTestEventLoop::instance().timeout() );
    QVERIFY( mCalendar->events().isEmpty() );
    QVERIFY( !mCalendar->journals().isEmpty() );
    QVERIFY( mCalendar->todos().isEmpty() );

    mCalendar->deleteAllJournals();
    QTestEventLoop::instance().enterLoop( 5 );
    QVERIFY( !QTestEventLoop::instance().timeout() );
    QVERIFY( mCalendar->events().isEmpty() );
    QVERIFY( mCalendar->journals().isEmpty() );
    QVERIFY( mCalendar->todos().isEmpty() );

    QVERIFY( mCalendar->incidences().isEmpty() );

    foreach( const QString &uid, mUids ) {
        QCOMPARE( mCalendar->item( uid ), Item() );
    }
}
*/

void CalendarBaseTest::handleCreateFinished(bool success, const QString &errorString)
{
    if (!success) {
        qDebug() << "handleCreateFinished(): " << errorString;
    }
    QCOMPARE(success, mExpectedSlotResult);
    QTestEventLoop::instance().exitLoop();
}

void CalendarBaseTest::handleDeleteFinished(bool success, const QString &errorString)
{
    if (!success) {
        qDebug() << "handleDeleteFinished(): " << errorString;
    }
    QCOMPARE(success, mExpectedSlotResult);
    QTestEventLoop::instance().exitLoop();
}

Item::Id CalendarBaseTest::createTodo(const QString &uid, const QString &parentUid)
{
    Todo::Ptr todo = Todo::Ptr(new Todo());
    todo->setUid(uid);
    todo->setSummary(QStringLiteral("summary"));
    if (!parentUid.isEmpty()) {
        todo->setRelatedTo(parentUid);
    }
    mCalendar->addTodo(todo);
    QTestEventLoop::instance().enterLoop(5);
    // QVERIFY( !QTestEventLoop::instance().timeout() );

    return mCalendar->item(uid).id();
}

#include "moc_calendarbasetest.cpp"
