/*
    SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// mailclient_p.cpp isn't exported so we include it directly.

#include "mailclient_p.h"

#include <Akonadi/ItemCreateJob>
#include <Akonadi/MessageQueueJob>
#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/Incidence>
#include <KContacts/Addressee>
#include <KIdentityManagementCore/Identity>
#include <MessageComposer/ContactPreference>

#include <gpgme++/context.h>
#include <gpgme++/keylistresult.h>

#include <akonadi/qtest_akonadi.h>

#include <QObject>
#include <QTestEventLoop>

const QString s_ourEmail = QStringLiteral("unittests@dev.nul"); // change also in kdepimlibs/akonadi/calendar/tests/unittestenv/kdehome/share/config
const auto s_ourGpgKey = QByteArray("7E501DEA81F62DB17389393325058D1857FDD0E7");

const QString s_testEmail = QStringLiteral("test@example.com");
const auto s_testGpgKey = QByteArray("D6003D89B2840A1B1888C39E5AB1CE1311F6B1DB");

const QString s_test2Email = QStringLiteral("test2@example.com");
const auto s_test2GpgKey = QByteArray("A9794D762BC67B1DEB161CDD8B3613B451672CB8");

enum class CryptoState { Plain, Signed, Encrypted };

struct ExpectedDialog {
    QString text;
    Akonadi::ITIPHandlerDialogDelegate::DialogAction action;
};

using ContactPreferences = QMap<QString, MessageComposer::ContactPreference>;

using namespace Akonadi;

Q_DECLARE_METATYPE(KCalendarCore::Incidence::Ptr)
Q_DECLARE_METATYPE(CryptoState)
Q_DECLARE_METATYPE(QList<ExpectedDialog>)
Q_DECLARE_METATYPE(ContactPreferences)

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

class FakeITIPHandlerDialogDelegate : public ITIPHandlerDialogDelegate
{
public:
    explicit FakeITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent = nullptr)
        : ITIPHandlerDialogDelegate(incidence, method, parent)
    {
    }

    static QList<ExpectedDialog> expectedWarningTwoActionsCancelDialogs;

protected:
    int warningTwoActionsCancel(const QString &text, const QString &title, const KGuiItem &, const KGuiItem &, const KGuiItem &) override
    {
        if (expectedWarningTwoActionsCancelDialogs.empty()) {
            QTest::qFail("Unexpected dialog - the testcase doesn't expect any dialog", __FILE__, __LINE__);
            qDebug() << "Dialog title:" << title;
            qDebug() << "Dialog text:" << text;
            return ITIPHandlerDialogDelegate::CancelAction;
        }

        const auto expected = expectedWarningTwoActionsCancelDialogs.front();
        expectedWarningTwoActionsCancelDialogs.erase(expectedWarningTwoActionsCancelDialogs.begin());

        if (!title.contains(expected.text) && !text.contains(expected.text)) {
            QTest::qFail("Mismatching dialog - the dialog text doesn't match the expected string", __FILE__, __LINE__);
            qDebug() << "Dialog title:" << title;
            qDebug() << "Dialog text:" << text;
            qDebug() << "Expected substring:" << expected.text;
            return ITIPHandlerDialogDelegate::CancelAction;
        }

        return expected.action;
    }
};

QList<ExpectedDialog> FakeITIPHandlerDialogDelegate::expectedWarningTwoActionsCancelDialogs;

class FakeITIPHandlerComponentFactory : public ITIPHandlerComponentFactory
{
public:
    explicit FakeITIPHandlerComponentFactory(QObject *parent = nullptr)
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

    ITIPHandlerDialogDelegate *
    createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent = nullptr) override
    {
        return new FakeITIPHandlerDialogDelegate(incidence, method, parent);
    }
};

class TestableMailClient : public MailClient
{
public:
    TestableMailClient(QObject *parent)
        : MailClient(new FakeITIPHandlerComponentFactory(parent), parent)
    {
        // Disable Akonadi contacts lookup - we ain't gonna find anything anyway,
        // since there's no indexer in the test env
        setAkonadiLookupEnabled(false);
    }

    static ContactPreferences preferences;

private:
    std::optional<MessageComposer::ContactPreference> contactPreference(const QString &address) override
    {
        auto it = preferences.constFind(address);
        if (it != preferences.cend()) {
            return *it;
        }

        return {};
    }

    bool showKeyApprovalDialog() const override
    {
        return false; // no gui in tests!
    }
};

ContactPreferences TestableMailClient::preferences;

static MessageComposer::ContactPreference
createPreference(const QByteArray &key, Kleo::EncryptionPreference encPref, Kleo::SigningPreference sigPref = Kleo::UnknownSigningPreference)
{
    MessageComposer::ContactPreference preference;
    preference.pgpKeyFingerprints.push_back(QString::fromLatin1(key));
    preference.encryptionPreference = encPref;
    preference.signingPreference = sigPref;
    preference.cryptoMessageFormat = Kleo::AnyOpenPGP;
    return preference;
}

class MailClientTest : public QObject
{
    Q_OBJECT

private:
    TestableMailClient *mMailClient = nullptr;
    int mPendingSignals;
    MailClient::Result mLastResult;
    QString mLastErrorMessage;

private Q_SLOTS:

    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();

        mPendingSignals = 0;
        mMailClient = new TestableMailClient(this);
        mLastResult = MailClient::ResultSuccess;
        connect(mMailClient, &MailClient::finished, this, &MailClientTest::handleFinished);
    }

    void cleanupTestCase()
    {
    }

    void testMailAttendees_data()
    {
        QTest::addColumn<KCalendarCore::Incidence::Ptr>("incidence");
        QTest::addColumn<KIdentityManagementCore::Identity>("identity");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedCcList");
        QTest::addColumn<QStringList>("expectedBccList");
        QTest::addColumn<CryptoState>("expectedCrypto");
        QTest::addColumn<QList<ExpectedDialog>>("expectedDialogs");
        QTest::addColumn<ContactPreferences>("contactPreferences");

        KCalendarCore::Incidence::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagementCore::Identity identity;
        identity.setPrimaryEmailAddress(s_ourEmail);

        KIdentityManagementCore::Identity cryptoIdentity = identity;
        cryptoIdentity.setPGPSigningKey(s_ourGpgKey);
        cryptoIdentity.setPGPEncryptionKey(s_ourGpgKey);
        cryptoIdentity.setPgpAutoSign(true);
        cryptoIdentity.setPgpAutoEncrypt(true);

        bool bccMe = false;
        QString attachment;
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultNoAttendees;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        const QString expectedFrom = s_ourEmail; // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), s_ourEmail);

        QStringList toList;
        QStringList toCcList;
        QStringList toBccList;
        //----------------------------------------------------------------------------------------------
        QTest::newRow("No attendees") << incidence << identity << bccMe << attachment << transport << expectedResult << -1 << QString() << toList << toCcList
                                      << toBccList << CryptoState::Plain << QList<ExpectedDialog>{} << ContactPreferences{};
        //----------------------------------------------------------------------------------------------
        // One attendee, but without e-mail
        KCalendarCore::Attendee attendee(QStringLiteral("name1"), QString());
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->addAttendee(attendee);
        expectedResult = MailClient::ResultReallyNoAttendees;
        QTest::newRow("No attendees with email") << incidence << identity << bccMe << attachment << transport << expectedResult << -1 << QString() << toList
                                                 << toCcList << toBccList << CryptoState::Plain << QList<ExpectedDialog>{} << ContactPreferences{};
        //----------------------------------------------------------------------------------------------
        // One valid attendee
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->addAttendee(attendee);
        incidence->setOrganizer(organizer);
        expectedResult = MailClient::ResultSuccess;
        toList << QStringLiteral("test@foo.org");
        QTest::newRow("One attendee") << incidence << identity << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                                      << toList << toCcList << toBccList << CryptoState::Plain << QList<ExpectedDialog>{} << ContactPreferences{};
        //----------------------------------------------------------------------------------------------
        // One valid attendee
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        QString invalidTransport = QStringLiteral("foo");
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        QTest::newRow("Invalid transport") << incidence << identity << bccMe << attachment << invalidTransport << expectedResult << expectedTransportId
                                           << expectedFrom << toList << toCcList << toBccList << CryptoState::Plain << QList<ExpectedDialog>{}
                                           << ContactPreferences{};

        //----------------------------------------------------------------------------------------------
        // One valid attendee, identity wants to sign
        {
            auto ident = identity;
            ident.setPGPSigningKey(s_ourGpgKey);
            ident.setPgpAutoSign(true);

            QTest::newRow("One attendee, identity wants to sign")
                << incidence << ident << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom << toList << toCcList
                << toBccList << CryptoState::Signed << QList<ExpectedDialog>{} << ContactPreferences{};
        }

        //----------------------------------------------------------------------------------------------
        // One valid attendee, identity wants to encrypt
        {
            KCalendarCore::Incidence::Ptr inc{incidence->clone()};
            inc->clearAttendees();
            inc->addAttendee(KCalendarCore::Attendee({}, s_testEmail));
            // No crypto preference for the attendee

            QTest::newRow("One attendee, identity wants to encrypt")
                << inc << cryptoIdentity << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                << QStringList({s_testEmail}) << toCcList << toBccList << CryptoState::Encrypted << QList<ExpectedDialog>{} << ContactPreferences{};
        }

        //----------------------------------------------------------------------------------------------
        // One valid attendee, attendee wants to encrypt
        {
            auto ident = identity;
            ident.setPGPSigningKey(s_ourGpgKey);
            ident.setPGPEncryptionKey(s_ourGpgKey);

            KCalendarCore::Incidence::Ptr inc{incidence->clone()};
            inc->clearAttendees();
            inc->addAttendee(KCalendarCore::Attendee({}, s_testEmail));

            QTest::newRow("One attendee, attendee wants to encrypt")
                << inc << ident << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom << QStringList({s_testEmail})
                << toCcList << toBccList << CryptoState::Encrypted << QList<ExpectedDialog>{}
                << ContactPreferences{{s_testEmail, createPreference(s_testGpgKey, Kleo::AlwaysEncrypt, Kleo::AlwaysSign)}};
        }

        //----------------------------------------------------------------------------------------------
        // Two attendees, one without key
        {
            auto ident = identity;
            ident.setPGPSigningKey(s_ourGpgKey);
            ident.setPGPEncryptionKey(s_ourGpgKey);

            KCalendarCore::Incidence::Ptr inc{incidence->clone()};
            inc->addAttendee(KCalendarCore::Attendee({}, s_testEmail));

            QTest::newRow("Two attendees, one wants encryption, one has no key")
                << inc << ident << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                << QStringList({QStringLiteral("test@foo.org"), s_testEmail}) << toCcList << toBccList << CryptoState::Plain
                << QList<ExpectedDialog>{{QStringLiteral("conflicting encryption preferences"),
                                          ITIPHandlerDialogDelegate::SecondaryAction /* do not encrypt */}}
                << ContactPreferences{{s_testEmail, createPreference(s_testGpgKey, Kleo::AlwaysEncrypt)}};
        }

        //----------------------------------------------------------------------------------------------
        // Two attendees, both have key but only one explicitly wants encryption - we should encrypt anyway
        {
            auto ident = identity;
            ident.setPGPSigningKey(s_ourGpgKey);
            ident.setPGPEncryptionKey(s_ourGpgKey);

            KCalendarCore::Incidence::Ptr inc{incidence->clone()};
            inc->clearAttendees();
            inc->addAttendee(KCalendarCore::Attendee({}, s_testEmail));
            inc->addAttendee(KCalendarCore::Attendee({}, s_test2Email));

            QTest::newRow("Two attendees, both have keys but only one explicitly wants encryption")
                << inc << ident << bccMe << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                << QStringList({s_testEmail, s_test2Email}) << toCcList << toBccList << CryptoState::Encrypted << QList<ExpectedDialog>{}
                << ContactPreferences{{s_test2Email, createPreference(s_test2GpgKey, Kleo::AlwaysEncrypt)}};
        }

        //----------------------------------------------------------------------------------------------
        // One valid attendee, and bcc me
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        toBccList.clear();
        toBccList << s_ourEmail;
        QTest::newRow("Test bcc") << incidence << identity << /*bccMe*/ true << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                                  << toList << toCcList << toBccList << CryptoState::Plain << QList<ExpectedDialog>{} << ContactPreferences{};
        //----------------------------------------------------------------------------------------------
        // Test CC list
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        KCalendarCore::Attendee optionalAttendee(QStringLiteral("opt"), QStringLiteral("optional@foo.org"));
        KCalendarCore::Attendee nonParticipant(QStringLiteral("non"), QStringLiteral("non@foo.org"));
        optionalAttendee.setRole(KCalendarCore::Attendee::OptParticipant);
        nonParticipant.setRole(KCalendarCore::Attendee::NonParticipant);
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        incidence->addAttendee(optionalAttendee);
        incidence->addAttendee(nonParticipant);
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        toBccList.clear();
        toBccList << s_ourEmail;

        toCcList.clear();
        toCcList << QStringLiteral("optional@foo.org") << QStringLiteral("non@foo.org");
        QTest::newRow("Test cc") << incidence << identity << /*bccMe*/ true << attachment << transport << expectedResult << expectedTransportId << expectedFrom
                                 << toList << toCcList << toBccList << CryptoState::Plain << QList<ExpectedDialog>{} << ContactPreferences{};
    }

    void testMailAttendees()
    {
        QFETCH(KCalendarCore::Incidence::Ptr, incidence);
        QFETCH(KIdentityManagementCore::Identity, identity);
        QFETCH(bool, bccMe);
        QFETCH(QString, attachment);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedCcList);
        QFETCH(QStringList, expectedBccList);
        QFETCH(CryptoState, expectedCrypto);
        QFETCH(QList<ExpectedDialog>, expectedDialogs);
        QFETCH(ContactPreferences, contactPreferences);

        FakeMessageQueueJob::sUnitTestResults.clear();

        FakeITIPHandlerDialogDelegate::expectedWarningTwoActionsCancelDialogs = expectedDialogs;
        TestableMailClient::preferences = contactPreferences;

        mPendingSignals = 1;
        mMailClient->mailAttendees(incidence, identity, KCalendarCore::iTIPRequest, bccMe, attachment, transport);
        waitForSignals();

        if (mLastResult != expectedResult) {
            qDebug() << "Fail1: last=" << mLastResult << "; expected=" << expectedResult << "; error=" << mLastErrorMessage;
            QVERIFY(false);
        }

        if (FakeMessageQueueJob::sUnitTestResults.isEmpty()) {
            qDebug() << "mail results are empty";
        } else {
            const auto unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
            if (expectedTransportId != -1 && unitTestResult.transportId != expectedTransportId) {
                qDebug() << "got " << unitTestResult.transportId << "; expected=" << expectedTransportId;
                QVERIFY(false);
            }

            QCOMPARE(unitTestResult.from, expectedFrom);
            QCOMPARE(unitTestResult.to, expectedToList);
            QCOMPARE(unitTestResult.cc, expectedCcList);
            QCOMPARE(unitTestResult.bcc, expectedBccList);
            switch (expectedCrypto) {
            case CryptoState::Plain:
                QCOMPARE(unitTestResult.message->contentType(false)->mimeType(), "text/plain");
                break;

            case CryptoState::Signed:
                QCOMPARE(unitTestResult.message->contentType(false)->mimeType(), "multipart/signed");
                break;

            case CryptoState::Encrypted:
                QCOMPARE(unitTestResult.message->contentType(false)->mimeType(), "multipart/encrypted");
                break;
            }
        }

        if (!FakeITIPHandlerDialogDelegate::expectedWarningTwoActionsCancelDialogs.empty()) {
            QFAIL("An expected dialog wasn't seen");
        }
    }

    void testMailOrganizer_data()
    {
        QTest::addColumn<KCalendarCore::IncidenceBase::Ptr>("incidence");
        QTest::addColumn<KIdentityManagementCore::Identity>("identity");
        QTest::addColumn<QString>("from");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("subject");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedBccList");
        QTest::addColumn<QString>("expectedSubject");

        KCalendarCore::IncidenceBase::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagementCore::Identity identity;
        const QString from = s_ourEmail;
        bool bccMe = false;
        QString attachment;
        QString subject = QStringLiteral("subject1");
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultSuccess;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        QString expectedFrom = from; // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), s_ourEmail);
        incidence->setOrganizer(organizer);

        QStringList toList;
        toList << s_ourEmail;
        QStringList toBccList;
        QString expectedSubject;
        //----------------------------------------------------------------------------------------------
        expectedSubject = subject;
        QTest::newRow("test1") << incidence << identity << from << bccMe << attachment << subject << transport << expectedResult << expectedTransportId
                               << expectedFrom << toList << toBccList << expectedSubject;
        //----------------------------------------------------------------------------------------------
        expectedSubject = QStringLiteral("Free Busy Message");
        incidence = KCalendarCore::IncidenceBase::Ptr(new KCalendarCore::FreeBusy());
        incidence->setOrganizer(organizer);
        QTest::newRow("FreeBusy") << incidence << identity << from << bccMe << attachment << subject << transport << expectedResult << expectedTransportId
                                  << expectedFrom << toList << toBccList << expectedSubject;
    }

    void testMailOrganizer()
    {
        QFETCH(KCalendarCore::IncidenceBase::Ptr, incidence);
        QFETCH(KIdentityManagementCore::Identity, identity);
        QFETCH(QString, from);
        QFETCH(bool, bccMe);
        QFETCH(QString, attachment);
        QFETCH(QString, subject);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedBccList);
        QFETCH(QString, expectedSubject);
        FakeMessageQueueJob::sUnitTestResults.clear();

        mPendingSignals = 1;
        mMailClient->mailOrganizer(incidence, identity, from, KCalendarCore::iTIPReply, bccMe, attachment, subject, transport);
        waitForSignals();
        QCOMPARE(mLastResult, expectedResult);

        UnitTestResult unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
        if (expectedTransportId != -1) {
            QCOMPARE(unitTestResult.transportId, expectedTransportId);
        }

        QCOMPARE(unitTestResult.from, expectedFrom);
        QCOMPARE(unitTestResult.to, expectedToList);
        QCOMPARE(unitTestResult.bcc, expectedBccList);
        QCOMPARE(unitTestResult.message->subject()->asUnicodeString(), expectedSubject);
    }

    void testMailTo_data()
    {
        QTest::addColumn<KCalendarCore::IncidenceBase::Ptr>("incidence");
        QTest::addColumn<KIdentityManagementCore::Identity>("identity");
        QTest::addColumn<QString>("from");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("recipients");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedBccList");

        KCalendarCore::IncidenceBase::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagementCore::Identity identity;
        const QString from = s_ourEmail;
        bool bccMe = false;
        const QString recipients = s_ourEmail;
        QString attachment;
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultSuccess;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        QString expectedFrom = from; // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), s_ourEmail);
        QStringList toList;
        toList << s_ourEmail;
        QStringList toBccList;
        //----------------------------------------------------------------------------------------------
        QTest::newRow("test1") << incidence << identity << from << bccMe << recipients << attachment << transport << expectedResult << expectedTransportId
                               << expectedFrom << toList << toBccList;
    }

    void testMailTo()
    {
        QFETCH(KCalendarCore::IncidenceBase::Ptr, incidence);
        QFETCH(KIdentityManagementCore::Identity, identity);
        QFETCH(QString, from);
        QFETCH(bool, bccMe);
        QFETCH(QString, recipients);
        QFETCH(QString, attachment);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedBccList);
        FakeMessageQueueJob::sUnitTestResults.clear();

        mPendingSignals = 1;
        mMailClient->mailTo(incidence, identity, from, KCalendarCore::iTIPRequest, bccMe, recipients, attachment, transport);
        waitForSignals();
        QCOMPARE(mLastResult, expectedResult);
        UnitTestResult unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
        if (expectedTransportId != -1) {
            QCOMPARE(unitTestResult.transportId, expectedTransportId);
        }

        QCOMPARE(unitTestResult.from, expectedFrom);
        QCOMPARE(unitTestResult.to, expectedToList);
        QCOMPARE(unitTestResult.bcc, expectedBccList);
    }

    void handleFinished(Akonadi::MailClient::Result result, const QString &errorMessage)
    {
        qDebug() << "handleFinished: " << result << errorMessage;
        mLastResult = result;
        mLastErrorMessage = errorMessage;
        --mPendingSignals;
        QTestEventLoop::instance().exitLoop();
    }

    void waitForSignals()
    {
        if (mPendingSignals > 0) {
            QTestEventLoop::instance().enterLoop(5); // 5 seconds is enough
            QVERIFY(!QTestEventLoop::instance().timeout());
        }
    }
};

QTEST_AKONADIMAIN(MailClientTest)

#include "mailclienttest.moc"
