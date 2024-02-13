/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unittestbase.h"
#include "../src/fetchjobcalendar.h"
#include "helper.h"
#include "mailclient_p.h"

#include <Akonadi/ItemCreateJob>
#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <incidencechanger.h>
#include <itiphandler.h>

#include <QByteArray>
#include <QFile>
#include <QTest>
#include <QTestEventLoop>

using namespace Akonadi;
using namespace KCalendarCore;

UnitTestBase::UnitTestBase()
{
    qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    qRegisterMetaType<QList<Akonadi::IncidenceChanger::ChangeType>>("QList<Akonadi::IncidenceChanger::ChangeType>");
    qRegisterMetaType<QList<Akonadi::Item::Id>>("QList<Akonadi::Item::Id>");
    qRegisterMetaType<Akonadi::MailClient::Result>("Akonadi::MailClient::Result");

    mChanger = new IncidenceChanger(this);
    mChanger->setShowDialogsOnError(false);
    mChanger->setHistoryEnabled(true);

    mCollection = Helper::fetchCollection();
    Q_ASSERT(mCollection.isValid());
    mChanger->setDefaultCollection(mCollection);
}

void UnitTestBase::waitForIt()
{
    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

void UnitTestBase::stopWaiting()
{
    QTestEventLoop::instance().exitLoop();
}

void UnitTestBase::createIncidence(const QString &uid)
{
    Item item = generateIncidence(uid);
    createIncidence(item);
}

void UnitTestBase::createIncidence(const Item &item)
{
    QVERIFY(mCollection.isValid());
    auto job = new ItemCreateJob(item, mCollection, this);
    QVERIFY(job->exec());
}

void UnitTestBase::verifyExists(const QString &uid, bool exists)
{
    auto calendar = new FetchJobCalendar();
    connect(calendar, &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();
    calendar->deleteLater();

    QCOMPARE(calendar->incidence(uid) != nullptr, exists);
}

Akonadi::Item::List UnitTestBase::calendarItems()
{
    FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr(new FetchJobCalendar());
    connect(calendar.data(), &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();
    KCalendarCore::ICalFormat format;
    QString dump = format.toString(calendar.staticCast<KCalendarCore::Calendar>());
    qDebug() << dump;
    calendar->deleteLater();
    return calendar->items();
}

void UnitTestBase::onLoadFinished(bool success, const QString &)
{
    QVERIFY(success);
    stopWaiting();
}

void UnitTestBase::compareCalendars(const KCalendarCore::Calendar::Ptr &expectedCalendar)
{
    FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr(new FetchJobCalendar());
    connect(calendar.data(), &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();

    // Now compare the expected calendar to the calendar we got.
    Incidence::List incidences = calendar->incidences();
    const Incidence::List expectedIncidences = expectedCalendar->incidences();

    // First, replace the randomly generated UIDs with the UID that came in the invitation e-mail...
    for (const KCalendarCore::Incidence::Ptr &incidence : std::as_const(incidences)) {
        incidence->setUid(incidence->schedulingID());
        qDebug() << "We have incidece with uid=" << incidence->uid() << "; instanceidentifier=" << incidence->instanceIdentifier();
        auto attendees = incidence->attendees();
        for (auto &attendee : attendees) {
            attendee.setUid(attendee.email());
        }
        incidence->setAttendees(attendees);
    }

    // ... so we can compare them
    for (const KCalendarCore::Incidence::Ptr &incidence : expectedIncidences) {
        incidence->setUid(incidence->schedulingID());
        qDebug() << "We expect incidece with uid=" << incidence->uid() << "; instanceidentifier=" << incidence->instanceIdentifier();
        auto attendees = incidence->attendees();
        for (auto &attendee : attendees) {
            attendee.setUid(attendee.email());
        }
        incidence->setAttendees(attendees);
    }

    QCOMPARE(incidences.count(), expectedIncidences.count());

    for (const KCalendarCore::Incidence::Ptr &expectedIncidence : expectedIncidences) {
        KCalendarCore::Incidence::Ptr incidence;
        for (int i = 0; i < incidences.count(); i++) {
            if (incidences.at(i)->instanceIdentifier() == expectedIncidence->instanceIdentifier()) {
                incidence = incidences.at(i);
                incidences.remove(i);
                break;
            }
        }
        QVERIFY(incidence);
        // Don't fail on creation times, which are obviously different
        expectedIncidence->setCreated(incidence->created());
        incidence->removeCustomProperty(QByteArray("LIBKCAL"), QByteArray("ID"));

        if (*expectedIncidence != *incidence) {
            ICalFormat format;
            QString expectedData = format.toString(expectedIncidence);
            QString gotData = format.toString(incidence);
            qDebug() << "Test failed, expected:\n" << expectedData << "\nbut got " << gotData;
            QVERIFY(false);
        }
    }
}

/** static */
QByteArray UnitTestBase::readFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "File could not be opened for reading:" << filename;
        return {};
    }

    return file.readAll();
}

Item UnitTestBase::generateIncidence(const QString &uid, const QString &organizer)
{
    Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    KCalendarCore::Incidence::Ptr incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());

    if (!uid.isEmpty()) {
        incidence->setUid(uid);
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    incidence->setDtStart(now);
    incidence->setDateTime(now.addSecs(3600), Incidence::RoleEnd);
    incidence->setSummary(QStringLiteral("summary"));
    item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

    if (!organizer.isEmpty()) {
        incidence->setOrganizer(organizer);
    }

    return item;
}

#include "moc_unittestbase.cpp"
