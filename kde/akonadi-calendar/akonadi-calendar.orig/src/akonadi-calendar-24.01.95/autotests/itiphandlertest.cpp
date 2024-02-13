/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itiphandlertest.h"
#include "fetchjobcalendar.h"
#include "helper.h"
#include "mailclient_p.h"
#include "utils_p.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/MessageQueueJob>
#include <KCalendarCore/Attendee>
#include <KCalendarCore/ICalFormat>
#include <akonadi/qtest_akonadi.h>

#include <QString>
#include <QTimeZone>

using namespace Akonadi;
using namespace KCalendarCore;

Q_DECLARE_METATYPE(Akonadi::IncidenceChanger::InvitationPolicy)
Q_DECLARE_METATYPE(QList<Akonadi::IncidenceChanger::ChangeType>)
Q_DECLARE_METATYPE(Akonadi::ITIPHandler::Result)
Q_DECLARE_METATYPE(KCalendarCore::Attendee::PartStat)
Q_DECLARE_METATYPE(QList<int>)

static const char *s_ourEmail = "unittests@dev.nul"; // change also in kdepimlibs/akonadi/calendar/tests/unittestenv/kdehome/share/config
static const char *s_outEmail2 = "identity2@kde.org";

class FakeMessageQueueJob : public Akonadi::MessageQueueJob
{
public:
    explicit FakeMessageQueueJob(QObject *parent = nullptr)
        : Akonadi::MessageQueueJob(parent)
    {
    }

    void start() override
    {
        UnitTestResult unitTestResult;
        unitTestResult.message = message();
        unitTestResult.from = addressAttribute().from();
        unitTestResult.to = addressAttribute().to();
        unitTestResult.cc = addressAttribute().cc();
        unitTestResult.bcc = addressAttribute().bcc();
        unitTestResult.transportId = transportAttribute().transportId();
        FakeMessageQueueJob::sUnitTestResults << unitTestResult;

        setError(Akonadi::MailClient::ResultSuccess);
        setErrorText(QString());

        emitResult();
    }

    static UnitTestResult::List sUnitTestResults;
};

UnitTestResult::List FakeMessageQueueJob::sUnitTestResults;

class FakeITIPHandlerComponentFactory : public ITIPHandlerComponentFactory
{
public:
    FakeITIPHandlerComponentFactory(QObject *parent = nullptr)
        : ITIPHandlerComponentFactory(parent)
    {
    }

    Akonadi::MessageQueueJob *createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence,
                                                    const KIdentityManagementCore::Identity &identity,
                                                    QObject *parent = nullptr) override
    {
        Q_UNUSED(incidence)
        Q_UNUSED(identity)
        return new FakeMessageQueueJob(parent);
    }
};

void ITIPHandlerTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();
    extern AKONADI_CALENDAR_TESTS_EXPORT bool akonadi_calendar_running_unittests;
    akonadi_calendar_running_unittests = true;
    m_pendingItipMessageSignal = 0;
    m_pendingIncidenceChangerSignal = 0;
    m_itipHandler = nullptr;
    m_cancelExpected = false;
    m_changer = new IncidenceChanger(new FakeITIPHandlerComponentFactory(this), this);
    m_changer->setHistoryEnabled(false);
    m_changer->setGroupwareCommunication(true);
    m_changer->setInvitationPolicy(IncidenceChanger::InvitationPolicySend); // don't show dialogs

    connect(m_changer, &IncidenceChanger::createFinished, this, &ITIPHandlerTest::onCreateFinished);

    connect(m_changer, &IncidenceChanger::deleteFinished, this, &ITIPHandlerTest::onDeleteFinished);

    connect(m_changer, &IncidenceChanger::modifyFinished, this, &ITIPHandlerTest::onModifyFinished);
}

void ITIPHandlerTest::testProcessITIPMessage_data()
{
    QTest::addColumn<QString>("data_filename");
    QTest::addColumn<QString>("action");
    QTest::addColumn<QString>("receiver");
    QTest::addColumn<QString>("incidenceUid"); // uid of incidence in invitation
    QTest::addColumn<Akonadi::ITIPHandler::Result>("expectedResult");
    QTest::addColumn<int>("expectedNumIncidences");
    QTest::addColumn<KCalendarCore::Attendee::PartStat>("expectedPartStat");

    QString data_filename;
    QString action = QStringLiteral("accepted");
    QString incidenceUid = QStringLiteral("uosj936i6arrtl9c2i5r2mfuvg");
    QString receiver = QLatin1String(s_ourEmail);
    Akonadi::ITIPHandler::Result expectedResult;
    int expectedNumIncidences = 0;
    KCalendarCore::Attendee::PartStat expectedPartStat;

    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, and we accept
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("invited_us");
    expectedNumIncidences = 1;
    expectedPartStat = KCalendarCore::Attendee::Accepted;
    action = QStringLiteral("accepted");
    QTest::newRow("invited us1") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, and we accept conditionally
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("invited_us");
    expectedNumIncidences = 1;
    expectedPartStat = KCalendarCore::Attendee::Tentative;
    action = QStringLiteral("tentative");
    QTest::newRow("invited us2") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, we delegate it
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("invited_us");

    // The e-mail to the delegate is sent by kmail's text_calendar.cpp
    expectedNumIncidences = 1;
    expectedPartStat = KCalendarCore::Attendee::Delegated;
    action = QStringLiteral("delegated");
    QTest::newRow("invited us3") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Process a CANCEL without having the incidence in our calendar.
    // itiphandler should return success and not error
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("invited_us");
    expectedNumIncidences = 0;
    action = QStringLiteral("cancel");
    QTest::newRow("invited us4") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Process a REQUEST without having the incidence in our calendar.
    // itiphandler should return success and add the request to a calendar
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("invited_us");
    expectedNumIncidences = 1;
    expectedPartStat = KCalendarCore::Attendee::NeedsAction;
    action = QStringLiteral("request");
    QTest::newRow("invited us5") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Here we're testing an error case, where data is null.
    expectedResult = ITIPHandler::ResultError;
    expectedNumIncidences = 0;
    action = QStringLiteral("accepted");
    QTest::newRow("invalid data") << QString() << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Testing invalid action
    expectedResult = ITIPHandler::ResultError;
    data_filename = QStringLiteral("invitation_us");
    expectedNumIncidences = 0;
    action = QStringLiteral("accepted");
    QTest::newRow("invalid action") << data_filename << QString() << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Test bug 235749
    expectedResult = ITIPHandler::ResultSuccess;
    data_filename = QStringLiteral("bug235749");
    expectedNumIncidences = 1;
    expectedPartStat = KCalendarCore::Attendee::Accepted;
    action = QStringLiteral("accepted");
    incidenceUid = QStringLiteral("b6f0466a-8877-49d0-a4fc-8ee18ffd8e07"); // Don't change, hardcoded in data file
    QTest::newRow("bug 235749") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
    // Test counterproposal without a UI delegat set
    expectedResult = ITIPHandler::ResultError;
    data_filename = QStringLiteral("invited_us");
    expectedNumIncidences = 0;
    expectedPartStat = KCalendarCore::Attendee::Accepted;
    action = QStringLiteral("counter");
    incidenceUid = QStringLiteral("b6f0466a-8877-49d0-a4fc-8ee18ffd8e07");
    QTest::newRow("counter error") << data_filename << action << receiver << incidenceUid << expectedResult << expectedNumIncidences << expectedPartStat;
    //----------------------------------------------------------------------------------------------
}

void ITIPHandlerTest::testProcessITIPMessage()
{
    QFETCH(QString, data_filename);
    QFETCH(QString, action);
    QFETCH(QString, receiver);
    QFETCH(QString, incidenceUid);
    QFETCH(Akonadi::ITIPHandler::Result, expectedResult);
    QFETCH(int, expectedNumIncidences);
    QFETCH(KCalendarCore::Attendee::PartStat, expectedPartStat);

    FakeMessageQueueJob::sUnitTestResults.clear();
    createITIPHandler();

    m_expectedResult = expectedResult;

    QString iCalData = icalData(data_filename);
    Akonadi::Item::List items;
    processItip(iCalData, receiver, action, expectedNumIncidences, items);

    if (expectedNumIncidences == 1) {
        auto incidence = items.first().payload<KCalendarCore::Incidence::Ptr>();
        QVERIFY(incidence);
        QCOMPARE(incidence->schedulingID(), incidenceUid);
        QVERIFY(incidence->schedulingID() != incidence->uid());

        KCalendarCore::Attendee me = ourAttendee(incidence);
        QVERIFY(!me.isNull());
        QCOMPARE(me.status(), expectedPartStat);
    }

    cleanup();
}

void ITIPHandlerTest::testProcessITIPMessages_data()
{
    QTest::addColumn<QStringList>("invitation_filenames"); // filename to create incidence (inputs)
    QTest::addColumn<QString>("expected_filename"); // filename with expected data   (reference)
    QTest::addColumn<QStringList>("actions"); // we must specify the METHOD. This is an ITipHandler API workaround, not sure why we must pass it as argument
                                              // since it's already inside the icaldata.
    QStringList invitation_filenames;
    QString expected_filename;
    QStringList actions;
    actions << QStringLiteral("accepted") << QStringLiteral("accepted");

    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, we accept, then organizer changes event, and we record update:
    invitation_filenames.clear();
    invitation_filenames << QStringLiteral("invited_us") << QStringLiteral("invited_us_update01");
    expected_filename = QStringLiteral("expected_data/update1");
    QTest::newRow("accept update") << invitation_filenames << expected_filename << actions;
    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, we accept, then organizer changes event, and we record update:
    invitation_filenames.clear();
    invitation_filenames << QStringLiteral("invited_us") << QStringLiteral("invited_us_daily_update01");
    expected_filename = QStringLiteral("expected_data/update2");
    QTest::newRow("accept recurringupdate") << invitation_filenames << expected_filename << actions;
    //----------------------------------------------------------------------------------------------
    // We accept a recurring event, then the organizer changes the summary to the second instance (RECID)
    expected_filename = QStringLiteral("expected_data/update3");
    invitation_filenames.clear();
    invitation_filenames << QStringLiteral("invited_us_daily") << QStringLiteral("invited_us_daily_update_recid01");
    QTest::newRow("accept recid update") << invitation_filenames << expected_filename << actions;
    //----------------------------------------------------------------------------------------------
    // We accept a recurring event, then we accept a CANCEL with recuring-id.
    // The result is that a exception with status CANCELLED should be created, and our main incidence
    // should not be touched
    invitation_filenames.clear();
    invitation_filenames << QStringLiteral("invited_us_daily") << QStringLiteral("invited_us_daily_cancel_recid01");
    expected_filename = QStringLiteral("expected_data/cancel1");
    actions << QStringLiteral("accepted") << QStringLiteral("cancel");
    QTest::newRow("accept recid cancel") << invitation_filenames << expected_filename << actions;

    //----------------------------------------------------------------------------------------------
}

void ITIPHandlerTest::testProcessITIPMessages()
{
    QFETCH(QStringList, invitation_filenames);
    QFETCH(QString, expected_filename);
    QFETCH(QStringList, actions);

    const QString receiver = QLatin1String(s_ourEmail);

    FakeMessageQueueJob::sUnitTestResults.clear();
    createITIPHandler();

    m_expectedResult = Akonadi::ITIPHandler::ResultSuccess;

    for (int i = 0; i < invitation_filenames.count(); i++) {
        // First accept the invitation that creates the incidence:
        QString iCalData = icalData(invitation_filenames.at(i));
        Item::List items;
        qDebug() << "Processing " << invitation_filenames.at(i);
        processItip(iCalData, receiver, actions.at(i), -1, items);
    }

    QString expectedICalData = icalData(expected_filename);
    KCalendarCore::MemoryCalendar::Ptr expectedCalendar = KCalendarCore::MemoryCalendar::Ptr(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
    KCalendarCore::ICalFormat format;
    format.fromString(expectedCalendar, expectedICalData);
    compareCalendars(expectedCalendar); // Here's where the cool and complex comparisons are done

    cleanup();
}

void ITIPHandlerTest::testProcessITIPMessageCancel_data()
{
    QTest::addColumn<QString>("creation_data_filename"); // filename to create incidence
    QTest::addColumn<QString>("cancel_data_filename"); // filename with incidence cancellation
    QTest::addColumn<QString>("incidenceUid"); // uid of incidence in invitation

    QString creation_data_filename;
    QString cancel_data_filename;
    QString incidenceUid = QStringLiteral("uosj936i6arrtl9c2i5r2mfuvg");
    //----------------------------------------------------------------------------------------------
    // Someone invited us to an event, we accept, then organizer cancels event
    creation_data_filename = QStringLiteral("invited_us");
    cancel_data_filename = QStringLiteral("invited_us_cancel01");

    QTest::newRow("cancel1") << creation_data_filename << cancel_data_filename << incidenceUid;
    //----------------------------------------------------------------------------------------------
    // Someone invited us to daily event, we accept, then organizer cancels the whole recurrence series
    creation_data_filename = QStringLiteral("invited_us_daily");
    cancel_data_filename = QStringLiteral("invited_us_daily_cancel01");

    QTest::newRow("cancel_daily") << creation_data_filename << cancel_data_filename << incidenceUid;
    //----------------------------------------------------------------------------------------------
}

void ITIPHandlerTest::testProcessITIPMessageCancel()
{
    QFETCH(QString, creation_data_filename);
    QFETCH(QString, cancel_data_filename);
    QFETCH(QString, incidenceUid);

    const QString receiver = QLatin1String(s_ourEmail);
    FakeMessageQueueJob::sUnitTestResults.clear();
    createITIPHandler();

    m_expectedResult = Akonadi::ITIPHandler::ResultSuccess;

    // First accept the invitation that creates the incidence:
    QString iCalData = icalData(creation_data_filename);
    Item::List items;
    processItip(iCalData, receiver, QStringLiteral("accepted"), 1, items);

    auto incidence = items.first().payload<KCalendarCore::Incidence::Ptr>();
    QVERIFY(incidence);

    // good, now accept the invitation that has the CANCEL
    iCalData = icalData(cancel_data_filename);
    processItip(iCalData, receiver, QStringLiteral("accepted"), 0, items);
}

void ITIPHandlerTest::testOutgoingInvitations_data()
{
    QTest::addColumn<Akonadi::Item>("item"); // existing incidence that will be target of creation, deletion or modification
    QTest::addColumn<Akonadi::IncidenceChanger::ChangeType>("changeType"); // creation, deletion, modification
    QTest::addColumn<int>("expectedEmailCount");
    QTest::addColumn<IncidenceChanger::InvitationPolicy>("invitationPolicy");
    QTest::addColumn<bool>("userCancels");

    const bool userDoesntCancel = false;
    const bool userCancels = true;

    Akonadi::Item item;
    KCalendarCore::Incidence::Ptr incidence;
    IncidenceChanger::ChangeType changeType;
    const IncidenceChanger::InvitationPolicy invitationPolicyAsk = IncidenceChanger::InvitationPolicyAsk;
    const IncidenceChanger::InvitationPolicy invitationPolicySend = IncidenceChanger::InvitationPolicySend;
    const IncidenceChanger::InvitationPolicy invitationPolicyDontSend = IncidenceChanger::InvitationPolicyDontSend;
    int expectedEmailCount = 0;
    Q_UNUSED(invitationPolicyAsk)

    const QString ourEmail = QLatin1String(s_ourEmail);
    Attendee us(QString(), ourEmail);
    const Attendee mia(QStringLiteral("Mia Wallace"), QStringLiteral("mia@dev.nul"));
    const Attendee vincent(QStringLiteral("Vincent"), QStringLiteral("vincent@dev.nul"));
    const Attendee jules(QStringLiteral("Jules"), QStringLiteral("jules@dev.nul"));
    const QString uid = QStringLiteral("random-uid-123");

    //----------------------------------------------------------------------------------------------
    // Creation. We are organizer. We invite another person.
    changeType = IncidenceChanger::ChangeTypeCreate;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    expectedEmailCount = 1;
    QTest::newRow("Creation. We organize.") << item << changeType << expectedEmailCount << invitationPolicySend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // Creation. We are organizer. We invite another person. But we choose not to send invitation e-mail.
    changeType = IncidenceChanger::ChangeTypeCreate;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    expectedEmailCount = 0;
    QTest::newRow("Creation. We organize.2") << item << changeType << expectedEmailCount << invitationPolicyDontSend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event that we organized, and has attendees, that will be notified.
    changeType = IncidenceChanger::ChangeTypeDelete;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    expectedEmailCount = 1;
    QTest::newRow("Deletion. We organized.") << item << changeType << expectedEmailCount << invitationPolicySend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event that we organized, and has attendees. We won't send e-mail notifications.
    changeType = IncidenceChanger::ChangeTypeDelete;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    expectedEmailCount = 0;
    QTest::newRow("Deletion. We organized.2") << item << changeType << expectedEmailCount << invitationPolicyDontSend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event that we organized, and has attendees, who will be notified.
    changeType = IncidenceChanger::ChangeTypeModify;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    expectedEmailCount = 1;
    QTest::newRow("Modification. We organizd.") << item << changeType << expectedEmailCount << invitationPolicySend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event that we organized, and has attendees, who won't be notified.
    changeType = IncidenceChanger::ChangeTypeModify;
    item = generateIncidence(uid, /**organizer=*/ourEmail);
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent); // TODO: test that all attendees got the e-mail
    incidence->addAttendee(jules);
    expectedEmailCount = 0;
    QTest::newRow("Modification. We organizd.2") << item << changeType << expectedEmailCount << invitationPolicyDontSend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event which we're not the organizer of. Organizer gets REPLY with PartState=Declined
    changeType = IncidenceChanger::ChangeTypeDelete;
    item = generateIncidence(uid, /**organizer=*/mia.email());
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    us.setStatus(Attendee::Accepted); // TODO: Test without accepted status
    incidence->addAttendee(us); // TODO: test that attendees didn't receive the REPLY
    expectedEmailCount = 1; // REPLY is always sent, there are no dialogs to control this. Dialogs only control REQUEST and CANCEL. Bug or feature ?
    QTest::newRow("Deletion. We didn't organize.") << item << changeType << expectedEmailCount << invitationPolicyDontSend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We delete an event which we're not the organizer of. Organizer gets REPLY with PartState=Declined
    changeType = IncidenceChanger::ChangeTypeDelete;
    item = generateIncidence(uid, /**organizer=*/mia.email());
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules); // TODO: test that attendees didn't receive the REPLY
    us.setStatus(Attendee::Accepted); // TODO: Test without accepted status
    incidence->addAttendee(us);
    expectedEmailCount = 1;
    QTest::newRow("Deletion. We didn't organize.2") << item << changeType << expectedEmailCount << invitationPolicySend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We modified an event which we're not the organizer of. And, when the "do you really want to modify", we choose "yes".
    changeType = IncidenceChanger::ChangeTypeModify;
    item = generateIncidence(uid, /**organizer=*/mia.email());
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    us.setStatus(Attendee::Accepted);
    incidence->addAttendee(us);
    expectedEmailCount = 0;
    QTest::newRow("Modification. We didn't organize") << item << changeType << expectedEmailCount << invitationPolicySend << userDoesntCancel;
    //----------------------------------------------------------------------------------------------
    // We modified an event which we're not the organizer of. And, when the "do you really want to modify", we choose "no".
    changeType = IncidenceChanger::ChangeTypeModify;
    item = generateIncidence(uid, /**organizer=*/mia.email());
    incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    incidence->addAttendee(vincent);
    incidence->addAttendee(jules);
    us.setStatus(Attendee::Accepted);
    incidence->addAttendee(us);
    expectedEmailCount = 0;
    QTest::newRow("Modification. We didn't organize.2") << item << changeType << expectedEmailCount << invitationPolicyDontSend << userCancels;
    //----------------------------------------------------------------------------------------------
}

void ITIPHandlerTest::testOutgoingInvitations()
{
    QFETCH(Akonadi::Item, item);
    QFETCH(IncidenceChanger::ChangeType, changeType);
    QFETCH(int, expectedEmailCount);
    QFETCH(IncidenceChanger::InvitationPolicy, invitationPolicy);
    QFETCH(bool, userCancels);
    auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();

    m_pendingIncidenceChangerSignal = 1;
    FakeMessageQueueJob::sUnitTestResults.clear();
    m_changer->setInvitationPolicy(invitationPolicy);

    m_cancelExpected = userCancels;

    switch (changeType) {
    case IncidenceChanger::ChangeTypeCreate:
        m_changer->createIncidence(incidence, mCollection);
        waitForIt();
        QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), expectedEmailCount);
        break;
    case IncidenceChanger::ChangeTypeModify: {
        // Create if first, so we have something to modify
        m_changer->setGroupwareCommunication(false); // we disable groupware because creating an incidence which we're not the organizer of is not permitted.
        m_changer->createIncidence(incidence, mCollection);
        waitForIt();
        m_changer->setGroupwareCommunication(true);
        QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), 0);
        QVERIFY(mLastInsertedItem.isValid());

        m_pendingIncidenceChangerSignal = 1;
        Incidence::Ptr oldIncidence = Incidence::Ptr(incidence->clone());
        incidence->setSummary(QStringLiteral("the-new-summary"));
        int changeId = m_changer->modifyIncidence(mLastInsertedItem, oldIncidence);
        QVERIFY(changeId != 1);
        waitForIt();
        QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), expectedEmailCount);
        break;
    }
    case IncidenceChanger::ChangeTypeDelete:
        // Create if first, so we have something to delete
        m_changer->setGroupwareCommunication(false);
        m_changer->createIncidence(incidence, mCollection);
        waitForIt();
        m_changer->setGroupwareCommunication(true);
        QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), 0);

        QVERIFY(mLastInsertedItem.isValid());
        m_pendingIncidenceChangerSignal = 1;
        m_changer->deleteIncidence(mLastInsertedItem);
        waitForIt();
        QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), expectedEmailCount);
        break;
    default:
        Q_ASSERT(false);
    }
}

void ITIPHandlerTest::testIdentity_data()
{
    QTest::addColumn<QString>("email");
    QTest::addColumn<bool>("expectedResult");

    const QString myEmail = QLatin1String(s_ourEmail);
    QString myEmail2 = QStringLiteral("Some name <%1>").arg(myEmail);

    const QString myAlias1 = QStringLiteral("alias1@kde.org"); // hardcoded in emailidentities, do not change
    const QString myIdentity2 = QLatin1String(s_outEmail2);

    QTest::newRow("Me") << myEmail << true;
    QTest::newRow("Also me") << myEmail2 << true;
    QTest::newRow("My identity2") << myIdentity2 << true;
    QTest::newRow("Not me") << QStringLiteral("laura.palmer@twinpeaks.com") << false;

    QTest::newRow("My alias") << myAlias1 << true;
}

void ITIPHandlerTest::testIdentity()
{
    QFETCH(QString, email);
    QFETCH(bool, expectedResult);

    if (CalendarUtils::thatIsMe(email) != expectedResult) {
        qDebug() << email;
        QVERIFY(false);
    }
}

void ITIPHandlerTest::cleanup()
{
    const Akonadi::Item::List items = calendarItems();
    for (const Akonadi::Item &item : items) {
        auto job = new ItemDeleteJob(item);
        AKVERIFYEXEC(job);
    }

    delete m_itipHandler;
    m_itipHandler = nullptr;
}

void ITIPHandlerTest::createITIPHandler()
{
    m_itipHandler = new Akonadi::ITIPHandler(new FakeITIPHandlerComponentFactory(this), this);
    m_itipHandler->setShowDialogsOnError(false);
    connect(m_itipHandler, &ITIPHandler::iTipMessageProcessed, this, &ITIPHandlerTest::oniTipMessageProcessed);
}

QString ITIPHandlerTest::icalData(const QString &data_filename)
{
    QString absolutePath = QFINDTESTDATA(QLatin1String("itip_data/") + data_filename);
    return QString::fromLatin1(readFile(absolutePath));
}

void ITIPHandlerTest::processItip(const QString &icaldata,
                                  const QString &receiver,
                                  const QString &action,
                                  int expectedNumIncidences,
                                  Akonadi::Item::List &items)
{
    items.clear();
    m_pendingItipMessageSignal = 1;
    m_itipHandler->processiTIPMessage(receiver, icaldata, action);
    waitForIt();

    // 0 e-mails are sent because the status update e-mail is sent by
    // kmail's text_calendar.cpp.
    QCOMPARE(FakeMessageQueueJob::sUnitTestResults.count(), 0);

    items = calendarItems();

    if (expectedNumIncidences != -1) {
        QCOMPARE(items.count(), expectedNumIncidences);
    }
}

Attendee ITIPHandlerTest::ourAttendee(const KCalendarCore::Incidence::Ptr &incidence) const
{
    const KCalendarCore::Attendee::List attendees = incidence->attendees();
    KCalendarCore::Attendee me;
    for (const KCalendarCore::Attendee &attendee : attendees) {
        if (attendee.email() == QLatin1String(s_ourEmail)) {
            me = attendee;
            break;
        }
    }

    return me;
}

void ITIPHandlerTest::oniTipMessageProcessed(ITIPHandler::Result result, const QString &errorMessage)
{
    if (result != ITIPHandler::ResultSuccess && result != m_expectedResult) {
        qDebug() << "ITIPHandlerTest::oniTipMessageProcessed() error = " << errorMessage;
    }

    m_pendingItipMessageSignal--;
    QVERIFY(m_pendingItipMessageSignal >= 0);
    if (m_pendingItipMessageSignal == 0) {
        stopWaiting();
    }

    QCOMPARE(m_expectedResult, result);
}

void ITIPHandlerTest::onCreateFinished(int changeId, const Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    Q_UNUSED(changeId)
    Q_UNUSED(errorString)
    mLastInsertedItem = item;
    m_pendingIncidenceChangerSignal--;
    QVERIFY(m_pendingIncidenceChangerSignal >= 0);
    if (m_pendingIncidenceChangerSignal == 0) {
        stopWaiting();
    }
    QCOMPARE(resultCode, IncidenceChanger::ResultCodeSuccess);
}

void ITIPHandlerTest::onDeleteFinished(int changeId,
                                       const QList<Akonadi::Item::Id> &deletedIds,
                                       IncidenceChanger::ResultCode resultCode,
                                       const QString &errorString)
{
    Q_UNUSED(changeId)
    Q_UNUSED(errorString)
    Q_UNUSED(deletedIds)

    m_pendingIncidenceChangerSignal--;
    QVERIFY(m_pendingIncidenceChangerSignal >= 0);
    if (m_pendingIncidenceChangerSignal == 0) {
        stopWaiting();
    }
    QCOMPARE(resultCode, IncidenceChanger::ResultCodeSuccess);
}

void ITIPHandlerTest::onModifyFinished(int changeId, const Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    Q_UNUSED(changeId)
    Q_UNUSED(errorString)
    Q_UNUSED(item)

    m_pendingIncidenceChangerSignal--;
    QVERIFY(m_pendingIncidenceChangerSignal >= 0);
    if (m_pendingIncidenceChangerSignal == 0) {
        stopWaiting();
    }
    QCOMPARE(resultCode, m_cancelExpected ? IncidenceChanger::ResultCodeUserCanceled : IncidenceChanger::ResultCodeSuccess);
}

QTEST_AKONADIMAIN(ITIPHandlerTest)

#include "moc_itiphandlertest.cpp"
