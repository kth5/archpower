/*
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectioncalendartest.h"
#include "collectioncalendar.h"

#include <QSignalSpy>
#include <QTest>
#include <QUuid>

#include <akonadi/qtest_akonadi.h>

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <akonadi/private/dbus_p.h>

#include <KCalendarCore/Event>

namespace
{

class EphemeralItem
{
public:
    EphemeralItem(const Akonadi::Item &item)
        : mItem(item)
    {
    }
    EphemeralItem &operator=(const Akonadi::Item &item)
    {
        deleteItem();
        mItem = item;
        return *this;
    }

    EphemeralItem(const EphemeralItem &) = delete;
    EphemeralItem(EphemeralItem &&) = delete;
    EphemeralItem &operator=(const EphemeralItem &) = delete;
    EphemeralItem &operator=(EphemeralItem &&) = delete;

    ~EphemeralItem()
    {
        deleteItem();
    }

    const Akonadi::Item *operator->() const
    {
        return &mItem;
    }

    const Akonadi::Item &operator*() const
    {
        return mItem;
    }

    Akonadi::Item &operator*()
    {
        return mItem;
    }

private:
    void deleteItem()
    {
        if (mItem.isValid()) {
            auto *job = new Akonadi::ItemDeleteJob(mItem);
            job->exec();
            mItem = {};
        }
    }
    Akonadi::Item mItem;
};

class Observer : public QObject, public KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT
public:
    Observer(Akonadi::CollectionCalendar &calendar)
        : mCalendar(calendar)
    {
        mCalendar.registerObserver(this);
    }

    ~Observer()
    {
        mCalendar.unregisterObserver(this);
    }

    QSignalSpy incidenceAddedSpy{this, &Observer::incidenceAdded};
    QSignalSpy incidenceChangedSpy{this, &Observer::incidenceChanged};
    QSignalSpy incidenceRemovedSpy{this, &Observer::incidenceRemoved};

Q_SIGNALS:
    void incidenceAdded(const Akonadi::Item &item);
    void incidenceChanged(const Akonadi::Item &item);
    void incidenceRemoved(const Akonadi::Item &item);

private:
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceAdded(mCalendar.item(incidence));
    }

    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceChanged(mCalendar.item(incidence));
    }

    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *) override
    {
        Akonadi::Item item(incidence->customProperty("VOLATILE", "AKONADI-ID").toLongLong());
        item.setParentCollection(Akonadi::Collection(incidence->customProperty("VOLATILE", "COLLECTION-ID").toLongLong()));

        Q_EMIT incidenceRemoved(item);
    }

    Akonadi::CollectionCalendar &mCalendar;
};

Akonadi::Collection findResourceCollection(const QString &resource)
{
    auto *fetch = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
    fetch->fetchScope().setResource(resource);
    if (!fetch->exec()) {
        qWarning() << "Failed to fetch collection tree";
        return {};
    }
    for (const auto &col : fetch->collections()) {
        if (col.resource() == resource) {
            return col;
        }
    }

    qWarning() << "Failed to find a suitable parent collection for our test";
    return {};
}

Akonadi::Item createIncidence(const Akonadi::Collection &parent, const QString &summary)
{
    auto event = KCalendarCore::Event::Ptr::create();
    event->setSummary(summary);
    event->setDtStart(QDateTime(QDate::currentDate(), QTime(10, 0, 0), QTimeZone::utc()));
    event->setDtEnd(QDateTime(QDate::currentDate(), QTime(11, 0, 0), QTimeZone::utc()));
    event->setUid(QUuid::createUuid().toString());

    Akonadi::Item item;
    item.setMimeType(event->mimeType());
    item.setParentCollection(parent);
    item.setPayload(event);
    item.setGid(event->uid());

    auto *job = new Akonadi::ItemCreateJob(item, parent);
    if (!job->exec()) {
        qWarning() << "Failed to create test incidence" << summary << ":" << job->errorString();
        return {};
    }
    return job->item();
}

std::unique_ptr<Akonadi::Monitor> createMonitor()
{
    auto monitor = std::make_unique<Akonadi::Monitor>();
    monitor->setAllMonitored(true);
    monitor->itemFetchScope().fetchFullPayload(true);
    monitor->itemFetchScope().setCacheOnly(true);

    return monitor;
}

bool waitForEtmToPopulate(Akonadi::EntityTreeModel *etm)
{
    return QTest::qWaitFor([etm]() {
        return etm->isFullyPopulated();
    });
}

bool populateCollection(const Akonadi::Collection &collection, int count)
{
    for (int i = 0; i < count; ++i) {
        const auto item = createIncidence(collection, QStringLiteral("Test Incidence %1").arg(i));
        if (!item.isValid()) {
            return false;
        }
    }

    return true;
}

void configureResource(const Akonadi::AgentInstance &instance, const QString &file)
{
    QDBusInterface iface(Akonadi::DBus::agentServiceName(instance.identifier(), Akonadi::DBus::Resource),
                         QStringLiteral("/Settings"),
                         QStringLiteral("org.kde.Akonadi.ICal.Settings"));
    iface.call(QStringLiteral("setPath"), file);
    iface.call(QStringLiteral("save"));
    instance.reconfigure();
}
}

CollectionCalendarTest::CollectionCalendarTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Akonadi::Collection::Id>();
}

void CollectionCalendarTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();

    otherResourceConfig.open();

    // We need two agents to test that the calendar properly ignores events in other collections
    // However we can't just add it to unittestenv, as having multiple resources breaks other
    // more complex tests that expect exactly one calendar agent.
    auto *agentJob = new Akonadi::AgentInstanceCreateJob(QStringLiteral("akonadi_ical_resource"));
    QVERIFY(agentJob->exec());
    configureResource(agentJob->instance(), otherResourceConfig.fileName());

    // Populate the ETM
    testCollection = findResourceCollection(QStringLiteral("akonadi_ical_resource_0"));
    QVERIFY(testCollection.isValid());
    QVERIFY(populateCollection(testCollection, 10));

    otherCollection = findResourceCollection(agentJob->instance().identifier());
    QVERIFY(otherCollection.isValid());
    QVERIFY(populateCollection(otherCollection, 5));
}

void CollectionCalendarTest::testPopulateFromReadyETM()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);

    // Should populate right away
    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testPopulateWhenETMLoads()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    QVERIFY(!etm.isFullyPopulated());
    QVERIFY(!etm.isCollectionPopulated(testCollection.id()));
    QVERIFY(calendar.incidences().empty());

    QVERIFY(waitForEtmToPopulate(&etm));

    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testItemAdded()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    EphemeralItem item = createIncidence(testCollection, QStringLiteral("New Test Item"));
    QVERIFY(observer.incidenceAddedSpy.wait());

    const auto newItem = std::as_const(observer.incidenceAddedSpy)[0][0].value<Akonadi::Item>();
    QCOMPARE(newItem.id(), item->id());
    QVERIFY(newItem.hasPayload<KCalendarCore::Event::Ptr>());
    QCOMPARE(*newItem.payload<KCalendarCore::Event::Ptr>(), *item->payload<KCalendarCore::Event::Ptr>());

    // Also make sure that the incidence is actually obtainable from the calendar
    auto newEvent = calendar.event(item->gid());
    QVERIFY(newEvent);
    QCOMPARE(*newItem.payload<KCalendarCore::Event::Ptr>(), *newEvent);

    QCOMPARE(calendar.incidences().size(), 11);
}

void CollectionCalendarTest::testItemRemoved()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    Akonadi::Item newItem;
    {
        EphemeralItem item = createIncidence(testCollection, QStringLiteral("Will be deleted"));
        newItem = *item;
        QVERIFY(observer.incidenceAddedSpy.wait());
        QCOMPARE(calendar.incidences().size(), 11);
        // EphemeralItem will delete the Item here
    }

    QVERIFY(observer.incidenceRemovedSpy.wait());
    const auto removedItem = std::as_const(observer.incidenceRemovedSpy)[0][0].value<Akonadi::Item>();
    QCOMPARE(removedItem.id(), newItem.id());

    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testItemChanged()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    {
        EphemeralItem item = createIncidence(testCollection, QStringLiteral("Will be changed"));
        QVERIFY(observer.incidenceAddedSpy.wait());
        QCOMPARE(calendar.incidences().size(), 11);

        KCalendarCore::Event::Ptr copy(item->payload<KCalendarCore::Event::Ptr>()->clone());
        copy->setSummary(QStringLiteral("Changed"));
        Akonadi::Item newItem = *item;
        newItem.setPayload(copy);

        auto *modifyJob = new Akonadi::ItemModifyJob(newItem);
        QVERIFY(modifyJob->exec());

        QVERIFY(observer.incidenceChangedSpy.wait());
        const auto changedItem = std::as_const(observer.incidenceChangedSpy)[0][0].value<Akonadi::Item>();
        QCOMPARE(changedItem.id(), newItem.id());
        QVERIFY(changedItem.hasPayload<KCalendarCore::Event::Ptr>());
        QCOMPARE(*changedItem.payload<KCalendarCore::Event::Ptr>(), *copy);
    }
}

void CollectionCalendarTest::testUnrelatedItemIsNotSeen()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));
    QSignalSpy etmAddedSpy(&etm, &Akonadi::EntityTreeModel::rowsInserted);
    QSignalSpy etmChangedSpy(&etm, &Akonadi::EntityTreeModel::dataChanged);
    QSignalSpy etmRemovedSpy(&etm, &Akonadi::EntityTreeModel::rowsRemoved);

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    {
        // Create new incidence in the other collection
        EphemeralItem item = createIncidence(otherCollection, QStringLiteral("Invisible"));
        // Wait for it to appear in the ETM
        QVERIFY(etmAddedSpy.wait());

        // Our calendar should remain unaffected
        QVERIFY(observer.incidenceAddedSpy.empty());
        QVERIFY(observer.incidenceChangedSpy.empty());
        QVERIFY(observer.incidenceRemovedSpy.empty());
        QCOMPARE(calendar.incidences().size(), 10);

        // Now we modify the incidence in the other collection
        item->payload<KCalendarCore::Event::Ptr>()->setSummary(QStringLiteral("Invisible and Changed"));
        auto *modifyJob = new Akonadi::ItemModifyJob(*item);
        QVERIFY(modifyJob->exec());
        // Wait for the change to appear in the ETM
        QVERIFY(etmChangedSpy.wait());

        // Our calendar should still remain unaffected
        QVERIFY(observer.incidenceAddedSpy.empty());
        QVERIFY(observer.incidenceChangedSpy.empty());
        QVERIFY(observer.incidenceRemovedSpy.empty());
        QCOMPARE(calendar.incidences().size(), 10);

        // The item will be deleted when EphemeralItem leaves this scope
    }

    // Wait for ETM to notice the Item has been removed
    QVERIFY(etmRemovedSpy.wait());

    // Our calendar still remains unaffected
    QVERIFY(observer.incidenceAddedSpy.empty());
    QVERIFY(observer.incidenceChangedSpy.empty());
    QVERIFY(observer.incidenceRemovedSpy.empty());
    QCOMPARE(calendar.incidences().size(), 10);
}

QTEST_AKONADIMAIN(CollectionCalendarTest)

#include "collectioncalendartest.moc"
#include "moc_collectioncalendartest.cpp"
