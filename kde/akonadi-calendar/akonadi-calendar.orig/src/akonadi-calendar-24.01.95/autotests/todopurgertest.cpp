/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "todopurgertest.h"

#include "../etmcalendar.h"
#include "../todopurger.h"
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <KCheckableProxyModel>
#include <akonadi/qtest_akonadi.h>

#include <QTestEventLoop>

using namespace Akonadi;
using namespace KCalendarCore;

Q_DECLARE_METATYPE(QSet<QByteArray>)

void TodoPurgerTest::createTodo(const QString &uid, const QString &parentUid, bool completed, bool recurring)
{
    Item item;
    item.setMimeType(Todo::todoMimeType());
    Todo::Ptr todo = Todo::Ptr(new Todo());
    todo->setUid(uid);

    const KDateTime today = KDateTime::currentDateTime(KDateTime::UTC);
    const KDateTime yesterday = today.addDays(-1);

    todo->setDtStart(yesterday);
    todo->setRelatedTo(parentUid);

    if (recurring) {
        todo->recurrence()->setDaily(1);
    }

    if (recurring && completed) {
        todo->setCompleted(today);
    } else {
        todo->setCompleted(completed);
    }

    todo->setSummary(QStringLiteral("summary"));

    item.setPayload<KCalendarCore::Incidence::Ptr>(todo);
    ItemCreateJob *job = new ItemCreateJob(item, m_collection, this);
    m_pendingCreations++;
    AKVERIFYEXEC(job);
}

void TodoPurgerTest::fetchCollection()
{
    CollectionFetchJob *job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
    // Get list of collections
    job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.todo"));
    AKVERIFYEXEC(job);

    // Find our collection
    Collection::List collections = job->collections();
    QVERIFY(!collections.isEmpty());
    m_collection = collections.first();

    QVERIFY(m_collection.isValid());
}

void TodoPurgerTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();

    qRegisterMetaType<QSet<QByteArray>>("QSet<QByteArray>");
    fetchCollection();

    m_pendingCreations = 0;
    m_pendingDeletions = 0;
    m_calendar = new ETMCalendar();
    m_calendar->registerObserver(this);
    m_todoPurger = new TodoPurger(this);

    connect(m_todoPurger, &Akonadi::TodoPurger::todosPurged, this, &TodoPurgerTest::onTodosPurged);

    connect(m_calendar, SIGNAL(collectionsAdded(Akonadi::Collection::List)), &QTestEventLoop::instance(), SLOT(exitLoop()));

    // Wait for the collection
    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(!QTestEventLoop::instance().timeout());

    KCheckableProxyModel *checkable = m_calendar->checkableProxyModel();
    const QModelIndex firstIndex = checkable->index(0, 0);
    QVERIFY(firstIndex.isValid());
    checkable->setData(firstIndex, Qt::Checked, Qt::CheckStateRole);
}

void TodoPurgerTest::cleanupTestCase()
{
    delete m_calendar;
}

void TodoPurgerTest::testPurge()
{
    createTree();

    m_pendingDeletions = 8;
    m_pendingPurgeSignal = true;

    m_todoPurger->purgeCompletedTodos();

    // Wait for deletions and purged signal
    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(m_numDeleted, 10);
    QCOMPARE(m_numIgnored, 2);
}

void TodoPurgerTest::calendarIncidenceAdded(const Incidence::Ptr &incidence)
{
    --m_pendingCreations;
    if (m_pendingCreations == 0) {
        QTestEventLoop::instance().exitLoop();
    }
}

void TodoPurgerTest::calendarIncidenceDeleted(const Incidence::Ptr &incidence, const KCalendarCore::Calendar *calendar)
{
    --m_pendingDeletions;
    if (m_pendingDeletions == 0 && !m_pendingPurgeSignal) {
        QTestEventLoop::instance().exitLoop();
    }
}

void TodoPurgerTest::onTodosPurged(bool success, int numDeleted, int numIgnored)
{
    QVERIFY(success);
    m_pendingPurgeSignal = false;
    m_numDeleted = numDeleted;
    m_numIgnored = numIgnored;

    if (m_pendingDeletions == 0) {
        QTestEventLoop::instance().enterLoop(10);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }
}

void TodoPurgerTest::createTree()
{
    createTodo(tr("a"), QString(), true); // Will be deleted
    createTodo(tr("b"), QString(), false); // Won't be deleted

    // Completed tree
    createTodo(tr("c"), QString(), true); // Will be deleted
    createTodo(tr("c1"), tr("c"), true); // Will be deleted
    createTodo(tr("c1.1"), tr("c1"), true); // Will be deleted
    createTodo(tr("c1.2"), tr("c1"), true); // Will be deleted

    // Root completed but children not completed
    createTodo(tr("d"), QString(), true); // Will be ignored (incomplete children)
    createTodo(tr("d1"), tr("d"), false); // Won't be deleted
    createTodo(tr("d1.1"), tr("d1"), false); // Won't be deleted
    createTodo(tr("d1.2"), tr("d1"), false); // Won't be deleted

    // Root incomplete with children complete
    createTodo(tr("e"), QString(), false); // Won't be deleted
    createTodo(tr("e1"), tr("e"), true); // Will be deleted
    createTodo(tr("e1.1"), tr("e1"), true); // Will be deleted
    createTodo(tr("e1.2"), tr("e1"), true); // Will be deleted

    // Recurring incomplete
    createTodo(tr("f"), QString(), false, true); // Won't be deleted

    // Recurring complete, this one is not deleted because recurrence didn't end
    createTodo(tr("g"), QString(), true, true); // Won't be deleted

    createTodo(tr("h"), QString(), true); // Will be ignored (incomplete children)
    createTodo(tr("h1"), tr("h"), false); // Won't be deleted
    createTodo(tr("h1.1"), tr("h1"), true); // Will be deleted
    createTodo(tr("h1.2"), tr("h1"), true); // Will be deleted

    // Now wait for incidences do be created
    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

QTEST_AKONADIMAIN(TodoPurgerTest, GUI)

#include "moc_todopurgertest.cpp"
