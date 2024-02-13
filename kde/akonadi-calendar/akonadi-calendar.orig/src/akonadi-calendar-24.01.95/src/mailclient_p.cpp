/*
  SPDX-FileCopyrightText: 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mailclient_p.h"

#include "akonadi-calendar-version.h"

#include <Akonadi/Collection>

#include <KCalUtils/IncidenceFormatter>
#include <KCalendarCore/Attendee>
#include <KCalendarCore/Incidence>
#include <KEmailAddress>
#include <KIdentityManagementCore/Identity>
#include <MessageComposer/ContactPreference>

#include <Akonadi/MessageQueueJob>
#include <MailTransport/Transport>
#include <MailTransport/TransportManager>

#include <KMime/Headers>

#include <QGpgME/ExportJob>
#include <QGpgME/ImportJob>
#include <QGpgME/Protocol>
#include <gpgme++/context.h>
#include <gpgme++/importresult.h>

#include <MessageComposer/Composer>
#include <MessageComposer/GlobalPart>
#include <MessageComposer/InfoPart>
#include <MessageComposer/ItipPart>
#include <MessageComposer/KeyResolver>
#include <MessageComposer/MessageComposerSettings>
#include <MessageComposer/Util>
#include <MessageCore/AutocryptStorage>

#include <Libkleo/Enum>
#include <Libkleo/ExpiryChecker>
#include <Libkleo/ExpiryCheckerSettings>

#include "akonadicalendar_debug.h"
#include <KJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QTemporaryDir>

using namespace Akonadi;

// Crypto-related helpers
static Kleo::chrono::days encryptOwnKeyNearExpiryWarningThresholdInDays()
{
    if (!MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire()) {
        return Kleo::chrono::days{-1};
    }
    const int num = MessageComposer::MessageComposerSettings::self()->cryptoWarnOwnEncrKeyNearExpiryThresholdDays();
    return Kleo::chrono::days{qMax(1, num)};
}

static Kleo::chrono::days encryptKeyNearExpiryWarningThresholdInDays()
{
    if (!MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire()) {
        return Kleo::chrono::days{-1};
    }
    const int num = MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrKeyNearExpiryThresholdDays();
    return Kleo::chrono::days{qMax(1, num)};
}

static Kleo::chrono::days encryptRootCertNearExpiryWarningThresholdInDays()
{
    if (!MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire()) {
        return Kleo::chrono::days{-1};
    }
    const int num = MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrRootNearExpiryThresholdDays();
    return Kleo::chrono::days{qMax(1, num)};
}

static Kleo::chrono::days encryptChainCertNearExpiryWarningThresholdInDays()
{
    if (!MessageComposer::MessageComposerSettings::self()->cryptoWarnWhenNearExpire()) {
        return Kleo::chrono::days{-1};
    }
    const int num = MessageComposer::MessageComposerSettings::self()->cryptoWarnEncrChaincertNearExpiryThresholdDays();
    return Kleo::chrono::days{qMax(1, num)};
}

static bool cryptoWarningUnsigned(const KIdentityManagementCore::Identity &identity)
{
    if (identity.encryptionOverride()) {
        return identity.warnNotSign();
    }
    return MessageComposer::MessageComposerSettings::self()->cryptoWarningUnsigned();
}

static bool cryptoWarningUnencrypted(const KIdentityManagementCore::Identity &identity)
{
    if (identity.encryptionOverride()) {
        return identity.warnNotEncrypt();
    }
    return MessageComposer::MessageComposerSettings::self()->cryptoWarningUnencrypted();
}

static QStringList extractEmailAndNormalize(const QString &email)
{
    const QStringList splittedEmail = KEmailAddress::splitAddressList(email);
    QStringList normalizedEmail;
    normalizedEmail.reserve(splittedEmail.count());
    for (const QString &email : splittedEmail) {
        const QString str = KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(email));
        normalizedEmail << str;
    }
    return normalizedEmail;
}

void MailClient::setAkonadiLookupEnabled(bool enabled)
{
    mAkonadiLookupEnabled = enabled;
}

std::optional<MessageComposer::ContactPreference> MailClient::contactPreference(const QString &address)
{
    Q_UNUSED(address);
    return {};
}

void MailClient::populateKeyResolverContactsPreferences(Kleo::KeyResolver &keyResolver, const QStringList &addresses)
{
    for (const auto &address : addresses) {
        if (const auto &pref = contactPreference(address); pref.has_value()) {
            keyResolver.setContactPreferences(address, *pref);
        }
    }
}

static bool populateKeyResolverEncryptionKeys(Kleo::KeyResolver &keyResolver, const KIdentityManagementCore::Identity &identity)
{
    QStringList encryptToSelfKeys;
    if (!identity.pgpEncryptionKey().isEmpty()) {
        encryptToSelfKeys.push_back(QString::fromLatin1(identity.pgpEncryptionKey()));
    }
    if (!identity.smimeEncryptionKey().isEmpty()) {
        encryptToSelfKeys.push_back(QString::fromLatin1(identity.smimeEncryptionKey()));
    }
    if (const auto result = keyResolver.setEncryptToSelfKeys(encryptToSelfKeys); result != Kleo::Ok) {
        qCWarning(AKONADICALENDAR_LOG) << "KeyResolver - failed to set encrypto-to-self keys, result:" << result;
        return false;
    }

    return true;
}

static bool populateKeyResolverSigningKeys(Kleo::KeyResolver &keyResolver, const KIdentityManagementCore::Identity &identity)
{
    QStringList signingKeys;
    if (!identity.pgpSigningKey().isEmpty()) {
        signingKeys.push_back(QString::fromLatin1(identity.pgpSigningKey()));
    }
    if (!identity.smimeSigningKey().isEmpty()) {
        signingKeys.push_back(QString::fromLatin1(identity.smimeSigningKey()));
    }
    qCDebug(AKONADICALENDAR_LOG) << "Settings signing keys:" << signingKeys;
    if (const auto result = keyResolver.setSigningKeys(signingKeys); result != Kleo::Ok) {
        qCWarning(AKONADICALENDAR_LOG) << "KeyResolver - failed to set signing keys, result:" << result;
        return false;
    }

    return true;
}

std::vector<std::unique_ptr<MessageComposer::Composer>>
MailClient::buildComposers(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagementCore::Identity &identity, const MessageData &msg)
{
    // TODO: Those should be set based on whether the user selects "Sign" or "Encrypt" options
    // in the incidence editor (similar to the "Sign" and "Encrypt" actions in KMail composer).
    // Right now we do NOT have those actions in the UI, so we keep the values to false so that
    // signing/encryption depends purely on current identity and attendee signing and encryption
    // preferences.
    bool signSomething = msg.sign;
    const bool doSignCompletely = msg.sign;
    bool encryptSomething = msg.encrypt;
    const bool doEncryptCompletely = msg.encrypt;

    std::unique_ptr<ITIPHandlerDialogDelegate> dialogDelegate(
        mFactory->createITIPHanderDialogDelegate(qSharedPointerCast<KCalendarCore::Incidence>(incidence), KCalendarCore::iTIPMethod::iTIPNoMethod));

    auto expiryChecker = std::make_shared<Kleo::ExpiryChecker>(Kleo::ExpiryCheckerSettings{encryptOwnKeyNearExpiryWarningThresholdInDays(),
                                                                                           encryptKeyNearExpiryWarningThresholdInDays(),
                                                                                           encryptRootCertNearExpiryWarningThresholdInDays(),
                                                                                           encryptChainCertNearExpiryWarningThresholdInDays()});
    Kleo::KeyResolver keyResolver(/* encToSelf */ true, showKeyApprovalDialog(), identity.pgpAutoEncrypt(), Kleo::AutoFormat, expiryChecker);

    const auto recipients = msg.to + msg.cc;
    populateKeyResolverContactsPreferences(keyResolver, recipients);

    keyResolver.setAkonadiLookupEnabled(mAkonadiLookupEnabled);
    keyResolver.setAutocryptEnabled(identity.autocryptEnabled());
    keyResolver.setPrimaryRecipients(recipients);
    if (msg.bccMe) {
        keyResolver.setSecondaryRecipients({msg.from});
    }

    if (!populateKeyResolverEncryptionKeys(keyResolver, identity)) {
        return {};
    }
    if (!populateKeyResolverSigningKeys(keyResolver, identity)) {
        return {};
    }

    bool result = true;
    bool canceled = false;
    bool signAttachments = false;
    bool encryptAttachments = false;

    signSomething = determineWhetherToSign(doSignCompletely, &keyResolver, dialogDelegate.get(), identity, signSomething, signAttachments, result, canceled);
    if (!result) {
        qCDebug(AKONADICALENDAR_LOG) << "KeyResolver failed to resolve signing keys - " << (canceled ? "operation canceled" : "an error occured");
        return {};
    }

    encryptSomething = determineWhetherToEncrypt(doEncryptCompletely,
                                                 &keyResolver,
                                                 dialogDelegate.get(),
                                                 identity,
                                                 encryptSomething,
                                                 signSomething,
                                                 encryptAttachments,
                                                 result,
                                                 canceled);
    if (!result) {
        qCDebug(AKONADICALENDAR_LOG) << "KeyResolver failed to resolve encryption keys - " << (canceled ? "operation canceled" : "an error occured");
        return {};
    }

    std::vector<std::unique_ptr<MessageComposer::Composer>> composers;

    if (!signSomething && !encryptSomething) {
        auto &composer = composers.emplace_back(std::make_unique<MessageComposer::Composer>());
        const auto preferredCrypto = Kleo::stringToCryptoMessageFormat(identity.preferredCryptoMessageFormat());
        if (preferredCrypto & Kleo::OpenPGPMIMEFormat) {
            composer->setAutocryptEnabled(identity.autocryptEnabled());
            if (keyResolver.encryptToSelfKeysFor(Kleo::OpenPGPMIMEFormat).size() > 0) {
                composer->setSenderEncryptionKey(keyResolver.encryptToSelfKeysFor(Kleo::OpenPGPMIMEFormat)[0]);
            }
        }
        return composers;
    }

    canceled = false;
    const Kleo::Result kpgpResult = keyResolver.resolveAllKeys(signSomething, encryptSomething);
    if (kpgpResult == Kleo::Canceled || canceled) {
        qCDebug(AKONADICALENDAR_LOG) << "resolveAllKeys: one key resolution canceled by user";
        return {};
    } else if (kpgpResult != Kleo::Ok) {
        // TODO handle failure
        qCDebug(AKONADICALENDAR_LOG) << "resolveAllKeys: failed to resolve keys! oh noes";
        return {};
    }

    if (encryptSomething || signSomething) {
        for (auto concreteFormat : {Kleo::OpenPGPMIMEFormat, Kleo::SMIMEFormat, Kleo::SMIMEOpaqueFormat, Kleo::InlineOpenPGPFormat}) {
            const auto encData = keyResolver.encryptionItems(concreteFormat);
            if (encData.empty()) {
                continue;
            }

            if (!(concreteFormat & Kleo::AutoFormat)) {
                continue;
            }

            auto composer = std::make_unique<MessageComposer::Composer>();

            if (encryptSomething || identity.autocryptEnabled()) {
                QList<QPair<QStringList, std::vector<GpgME::Key>>> data;
                data.reserve(encData.size());
                for (const auto &info : encData) {
                    data.push_back(qMakePair(info.recipients, info.keys));
                    qCDebug(AKONADICALENDAR_LOG) << "Resolved keys for:" << info.recipients;
                }
                composer->setEncryptionKeys(data);
                if (concreteFormat & Kleo::OpenPGPMIMEFormat && identity.autocryptEnabled()) {
                    composer->setAutocryptEnabled(true);
                    composer->setSenderEncryptionKey(keyResolver.encryptToSelfKeysFor(concreteFormat)[0]);
                    QTemporaryDir dir;
                    bool specialGnupgHome = addKeysToContext(dir.path(), data, keyResolver.useAutocrypt());
                    if (specialGnupgHome) {
                        dir.setAutoRemove(false);
                        composer->setGnupgHome(dir.path());
                    }
                }
            }

            if (signSomething) {
                // find signing keys for this format
                composer->setSigningKeys(keyResolver.signingKeys(concreteFormat));
            }

            composer->setMessageCryptoFormat(concreteFormat);
            composer->setSignAndEncrypt(signSomething, encryptSomething);

            composers.push_back(std::move(composer));
        }
    } else {
        composers.emplace_back(std::make_unique<MessageComposer::Composer>());
    }

    return composers;
}

void MailClient::queueMessage(const MailTransport::Transport *transport,
                              const MessageComposer::Composer *composer,
                              const KCalendarCore::IncidenceBase::Ptr &incidence,
                              const KIdentityManagementCore::Identity &identity,
                              const MessageData &msg,
                              const KMime::Message::Ptr &message)
{
    Akonadi::MessageQueueJob *qjob = mFactory->createMessageQueueJob(incidence, identity, this);
    qjob->setMessage(message);

    if (identity.disabledFcc()) {
        qjob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::Delete);
    } else {
        const Akonadi::Collection sentCollection(identity.fcc().toLongLong());
        if (sentCollection.isValid()) {
            qjob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::MoveToCollection);
            qjob->sentBehaviourAttribute().setMoveToCollection(sentCollection);
        } else {
            qjob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::MoveToDefaultSentCollection);
        }
    }

    qjob->transportAttribute().setTransportId(transport->id());

    if (transport && transport->specifySenderOverwriteAddress()) {
        qjob->addressAttribute().setFrom(
            KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(transport->senderOverwriteAddress())));
    } else {
        qjob->addressAttribute().setFrom(KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(composer->infoPart()->from())));
    }

    qjob->addressAttribute().setTo(MessageComposer::Util::cleanUpEmailListAndEncoding(composer->infoPart()->to()));
    qjob->addressAttribute().setCc(MessageComposer::Util::cleanUpEmailListAndEncoding(composer->infoPart()->cc()));
    if (msg.bccMe) {
        qjob->addressAttribute().setBcc({qjob->addressAttribute().from()});
    }

    message->assemble();
    connect(qjob, &KJob::finished, this, &MailClient::handleQueueJobFinished);
    qjob->start();
}

MailClient::MailClient(ITIPHandlerComponentFactory *factory, QObject *parent)
    : QObject(parent)
    , mFactory(factory ? factory : new ITIPHandlerComponentFactory(this))
{
}

MailClient::~MailClient() = default;

void MailClient::mailAttendees(const KCalendarCore::IncidenceBase::Ptr &incidence,
                               const KIdentityManagementCore::Identity &identity,
                               KCalendarCore::iTIPMethod method,
                               bool bccMe,
                               const QString &attachment,
                               const QString &mailTransport,
                               MailPrivacyFlags mailPrivacy)
{
    Q_ASSERT(incidence);
    KCalendarCore::Attendee::List attendees = incidence->attendees();
    if (attendees.isEmpty()) {
        qCWarning(AKONADICALENDAR_LOG) << "There are no attendees to e-mail";
        Q_EMIT finished(ResultNoAttendees, i18n("There are no attendees to e-mail"));
        return;
    }

    MessageData msg;
    msg.bccMe = bccMe;
    msg.attachment = attachment;
    msg.from = incidence->organizer().fullName();
    msg.method = KCalendarCore::ScheduleMessage::methodName(method);

    const QString organizerEmail = incidence->organizer().email();

    const int numberOfAttendees = attendees.count();
    for (int i = 0; i < numberOfAttendees; ++i) {
        const KCalendarCore::Attendee a = attendees.at(i);

        const QString email = a.email();
        if (email.isEmpty()) {
            continue;
        }

        // In case we (as one of our identities) are the organizer we are sending
        // this mail. We could also have added ourselves as an attendee, in which
        // case we don't want to send ourselves a notification mail.
        if (organizerEmail == email) {
            continue;
        }

        // Optional Participants and Non-Participants are copied on the email
        if (a.role() == KCalendarCore::Attendee::OptParticipant || a.role() == KCalendarCore::Attendee::NonParticipant) {
            msg.cc.push_back(a.email());
        } else {
            msg.to.push_back(a.email());
        }
    }
    if (msg.cc.isEmpty() && msg.to.isEmpty()) {
        // Not really to be called a groupware meeting, eh
        qCWarning(AKONADICALENDAR_LOG) << "There are really no attendees to e-mail";
        Q_EMIT finished(ResultReallyNoAttendees, i18n("There are no attendees to e-mail"));
        return;
    }

    QString subject;
    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        msg.subject = inc->summary();
    } else {
        msg.subject = i18n("Free Busy Object");
    }

    msg.body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);
    msg.sign = (mailPrivacy & MailPrivacySign) == MailPrivacySign;
    msg.encrypt = (mailPrivacy & MailPrivacyEncrypt) == MailPrivacyEncrypt;

    send(incidence, identity, msg, mailTransport);
}

void MailClient::mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &incidence,
                               const KIdentityManagementCore::Identity &identity,
                               const QString &from,
                               KCalendarCore::iTIPMethod method,
                               bool bccMe,
                               const QString &attachment,
                               const QString &sub,
                               const QString &mailTransport,
                               MailPrivacyFlags mailPrivacy)
{
    MessageData msg;
    msg.from = from;
    msg.to.push_back(incidence->organizer().fullName());
    msg.bccMe = bccMe;
    msg.subject = sub;
    msg.attachment = attachment;
    msg.method = KCalendarCore::ScheduleMessage::methodName(method);

    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        if (msg.subject.isEmpty()) {
            msg.subject = inc->summary();
        }
    } else {
        msg.subject = i18n("Free Busy Message");
    }

    msg.body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);
    msg.sign = (mailPrivacy & MailPrivacySign) == MailPrivacySign;
    msg.encrypt = (mailPrivacy & MailPrivacyEncrypt) == MailPrivacyEncrypt;

    send(incidence, identity, msg, mailTransport);
}

void MailClient::mailTo(const KCalendarCore::IncidenceBase::Ptr &incidence,
                        const KIdentityManagementCore::Identity &identity,
                        const QString &from,
                        KCalendarCore::iTIPMethod method,
                        bool bccMe,
                        const QString &recipients,
                        const QString &attachment,
                        const QString &mailTransport,
                        MailPrivacyFlags mailPrivacy)
{
    MessageData msg;
    msg.to = extractEmailAndNormalize(recipients);
    msg.from = from;
    msg.bccMe = bccMe;
    msg.attachment = attachment;
    msg.method = KCalendarCore::ScheduleMessage::methodName(method);

    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        msg.subject = inc->summary();
    } else {
        msg.subject = i18n("Free Busy Message");
    }

    msg.body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);
    msg.sign = (mailPrivacy & MailPrivacySign) == MailPrivacySign;
    msg.encrypt = (mailPrivacy & MailPrivacyEncrypt) == MailPrivacyEncrypt;

    send(incidence, identity, msg, mailTransport);
}

void MailClient::populateComposer(MessageComposer::Composer *composer, const MessageData &msg)
{
    // gather config values
    KConfig config(QStringLiteral("kmail2rc"));
    KConfigGroup configGroup(&config, QStringLiteral("Invitations"));
    const bool outlookConformInvitation = configGroup.readEntry("LegacyBodyInvites",
#ifdef KDEPIM_ENTERPRISE_BUILD
                                                                true
#else
                                                                false
#endif
    );

    auto *globalPart = composer->globalPart();
    globalPart->setGuiEnabled(false);

    auto *infoPart = composer->infoPart();
    infoPart->setCc(msg.cc);
    infoPart->setTo(msg.to);
    infoPart->setFrom(msg.from);
    if (msg.bccMe) {
        infoPart->setBcc({msg.from});
    }
    infoPart->setSubject(msg.subject);

    // Set User-Agent
    auto *header = new KMime::Headers::Generic("User-Agent");
    header->fromUnicodeString(QStringLiteral("KOrganizer %1").arg(QStringLiteral(AKONADI_CALENDAR_VERSION)), "utf-8");
    KMime::Headers::Base::List extras;
    extras.push_back(header);
    infoPart->setExtraHeaders(extras);

    auto *itipPart = composer->itipPart();
    itipPart->setOutlookConformInvitation(outlookConformInvitation);
    itipPart->setInvitationBody(msg.body);
    itipPart->setInvitation(msg.attachment);
    itipPart->setMethod(msg.method);
}

bool MailClient::addKeysToContext(const QString &gnupgHome,
                                  const QList<QPair<QStringList, std::vector<GpgME::Key>>> &data,
                                  const std::map<QByteArray, QString> &autocryptMap)
{
    bool needSpecialContext = false;

    for (const auto &p : data) {
        for (const auto &k : p.second) {
            const auto it = autocryptMap.find(k.primaryFingerprint());
            if (it != autocryptMap.end()) {
                needSpecialContext = true;
                break;
            }
        }
        if (needSpecialContext) {
            break;
        }
    }

    if (!needSpecialContext) {
        return false;
    }
    const QGpgME::Protocol *proto(QGpgME::openpgp());

    const auto storage = MessageCore::AutocryptStorage::self();
    QEventLoop loop;
    int runningJobs = 0;
    for (const auto &p : data) {
        for (const auto &k : p.second) {
            const auto it = autocryptMap.find(k.primaryFingerprint());
            if (it == autocryptMap.end()) {
                qCDebug(AKONADICALENDAR_LOG) << "Adding " << k.primaryFingerprint() << "via Export/Import";
                auto exportJob = proto->publicKeyExportJob(false);
                connect(exportJob,
                        &QGpgME::ExportJob::result,
                        exportJob,
                        [&gnupgHome, &proto, &runningJobs, &loop, &k](const GpgME::Error &result,
                                                                      const QByteArray &keyData,
                                                                      const QString &auditLogAsHtml,
                                                                      const GpgME::Error &auditLogError) {
                            Q_UNUSED(auditLogAsHtml);
                            Q_UNUSED(auditLogError);
                            if (result) {
                                qCWarning(AKONADICALENDAR_LOG) << "Failed to export " << k.primaryFingerprint() << result.asString();
                                --runningJobs;
                                if (runningJobs < 1) {
                                    loop.quit();
                                }
                            }

                            auto importJob = proto->importJob();
                            QGpgME::Job::context(importJob)->setEngineHomeDirectory(gnupgHome.toUtf8().constData());
                            importJob->exec(keyData);
                            importJob->deleteLater();
                            --runningJobs;
                            if (runningJobs < 1) {
                                loop.quit();
                            }
                        });
                QStringList patterns;
                patterns << QString::fromUtf8(k.primaryFingerprint());
                runningJobs++;
                exportJob->start(patterns);
                exportJob->setExportFlags(GpgME::Context::ExportMinimal);
            } else {
                qCDebug(AKONADICALENDAR_LOG) << "Adding " << k.primaryFingerprint() << "from Autocrypt storage";
                const auto recipient = storage->getRecipient(it->second.toUtf8());
                auto key = recipient->gpgKey();
                auto keydata = recipient->gpgKeydata();
                if (QByteArray(key.primaryFingerprint()) != QByteArray(k.primaryFingerprint())) {
                    qCDebug(AKONADICALENDAR_LOG) << "Using gossipkey";
                    keydata = recipient->gossipKeydata();
                }
                auto importJob = proto->importJob();
                QGpgME::Job::context(importJob)->setEngineHomeDirectory(gnupgHome.toUtf8().constData());
                const auto result = importJob->exec(keydata);
                importJob->deleteLater();
            }
        }
    }
    loop.exec();
    return true;
}

void MailClient::send(const KCalendarCore::IncidenceBase::Ptr &incidence,
                      const KIdentityManagementCore::Identity &identity,
                      const MessageData &_msg,
                      const QString &mailTransport)
{
    if (!MailTransport::TransportManager::self()->showTransportCreationDialog(nullptr, MailTransport::TransportManager::IfNoTransportExists)) {
        qCritical() << "Error while creating transport";
        Q_EMIT finished(ResultErrorCreatingTransport, i18n("Error while creating transport"));
        return;
    }

    MessageData msg = _msg;

    // We must have a recipients list for most MUAs. Thus, if the 'to' list
    // is empty simply use the 'from' address as the recipient.
    if (msg.to.isEmpty()) {
        msg.to.push_back(msg.from);
    }
    qCDebug(AKONADICALENDAR_LOG) << "\nFrom:" << msg.from << "\nTo:" << msg.to << "\nCC:" << msg.cc << "\nSubject:" << msg.subject << "\nBody: \n"
                                 << msg.body << "\nAttachment:\n"
                                 << msg.attachment << "\nmailTransport: " << mailTransport << "\nidentity:" << identity.identityName();

    MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportByName(mailTransport);
    if (!transport) {
        qCritical() << "Error fetching transport; mailTransport" << mailTransport << MailTransport::TransportManager::self()->defaultTransportName();
        Q_EMIT finished(ResultErrorFetchingTransport, i18n("Error fetching transport. Unable to send invitations"));
        return;
    }

    auto composers = buildComposers(incidence, identity, msg);
    for (auto &composerPtr : composers) {
        populateComposer(composerPtr.get(), msg);
        auto *composer = composerPtr.release();
        QObject::connect(composer, &MessageComposer::Composer::result, this, [this, transport, composer, incidence, identity, msg]() {
            for (const auto &message : composer->resultMessages()) {
                queueMessage(transport, composer, incidence, identity, msg, message);
            }
            composer->deleteLater();
        });
        composer->start();
    }
}

void MailClient::handleQueueJobFinished(KJob *job)
{
    if (job->error()) {
        qCritical() << "Error queueing message:" << job->errorText();
        Q_EMIT finished(ResultQueueJobError, i18n("Error queuing message in outbox: %1", job->errorText()));
    } else {
        Q_EMIT finished(ResultSuccess, QString());
    }
}

bool MailClient::determineWhetherToSign(bool doSignCompletely,
                                        Kleo::KeyResolver *keyResolver,
                                        ITIPHandlerDialogDelegate *dialogDelegate,
                                        const KIdentityManagementCore::Identity &identity,
                                        bool signSomething,
                                        bool &signAttachments,
                                        bool &result,
                                        bool &canceled)
{
    bool sign = false;
    switch (keyResolver->checkSigningPreferences(signSomething)) {
    case Kleo::DoIt:
        if (!signSomething) {
            signAttachments = true;
            return true;
        }
        sign = true;
        break;
    case Kleo::DontDoIt:
        sign = false;
        break;
    case Kleo::AskOpportunistic:
        assert(0);
    case Kleo::Ask: {
        // the user wants to be asked or has to be asked
        const QString msg = i18n(
            "Examination of the recipient's signing preferences "
            "yielded that you be asked whether or not to sign "
            "this message.\n"
            "Sign this message?");
        switch (dialogDelegate->warningTwoActionsCancel(msg, i18n("Sign Message?"), KGuiItem(i18nc("to sign", "&Sign")), KGuiItem(i18n("Do &Not Sign")))) {
        case ITIPHandlerDialogDelegate::CancelAction:
            result = false;
            canceled = true;
            return false;
        case ITIPHandlerDialogDelegate::PrimaryAction:
            signAttachments = true;
            return true;
        case ITIPHandlerDialogDelegate::SecondaryAction:
            signAttachments = false;
            return false;
        default:
            qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
            return false;
        }
        break;
    }
    case Kleo::Conflict: {
        // warn the user that there are conflicting signing preferences
        const QString msg = i18n(
            "There are conflicting signing preferences "
            "for these recipients.\n"
            "Sign this message?");
        switch (dialogDelegate->warningTwoActionsCancel(msg, i18n("Sign Message?"), KGuiItem(i18nc("to sign", "&Sign")), KGuiItem(i18n("Do &Not Sign")))) {
        case ITIPHandlerDialogDelegate::CancelAction:
            result = false;
            canceled = true;
            return false;
        case ITIPHandlerDialogDelegate::PrimaryAction:
            signAttachments = true;
            return true;
        case ITIPHandlerDialogDelegate::SecondaryAction:
            signAttachments = false;
            return false;
        default:
            qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
            return false;
        }
        break;
    }
    case Kleo::Impossible: {
        const QString msg = i18n(
            "You have requested to sign this message, "
            "but no valid signing keys have been configured "
            "for this identity.");
        if (dialogDelegate->warningContinueCancel(msg, i18n("Send Unsigned?"), KGuiItem(i18n("Send &Unsigned"))) == KMessageBox::Cancel) {
            result = false;
            return false;
        } else {
            signAttachments = false;
            return false;
        }
    }
    }

    if (!sign || !doSignCompletely) {
        if (cryptoWarningUnsigned(identity)) {
            const QString msg = sign && !doSignCompletely ? i18n(
                                    "Some parts of this message will not be signed.\n"
                                    "Sending only partially signed messages might violate site policy.\n"
                                    "Sign all parts instead?") // oh, I hate this...
                                                          : i18n(
                                                              "This message will not be signed.\n"
                                                              "Sending unsigned message might violate site policy.\n"
                                                              "Sign message instead?"); // oh, I hate this...
            const QString buttonText = sign && !doSignCompletely ? i18n("&Sign All Parts") : i18n("&Sign");
            switch (dialogDelegate->warningTwoActionsCancel(msg, i18n("Unsigned-Message Warning"), KGuiItem(buttonText), KGuiItem(i18n("Send &As Is")))) {
            case ITIPHandlerDialogDelegate::CancelAction:
                result = false;
                canceled = true;
                return false;
            case ITIPHandlerDialogDelegate::PrimaryAction:
                signAttachments = true;
                return true;
            case ITIPHandlerDialogDelegate::SecondaryAction:
                return sign || doSignCompletely;
            default:
                qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
                return false;
            }
        }
    }
    return sign || doSignCompletely;
}

bool MailClient::determineWhetherToEncrypt(bool doEncryptCompletely,
                                           Kleo::KeyResolver *keyResolver,
                                           ITIPHandlerDialogDelegate *dialogDelegate,
                                           const KIdentityManagementCore::Identity &identity,
                                           bool encryptSomething,
                                           bool signSomething,
                                           bool &encryptAttachments,
                                           bool &result,
                                           bool &canceled)
{
    bool encrypt = false;
    bool opportunistic = false;

    const auto encryptionPrefs = keyResolver->checkEncryptionPreferences(encryptSomething);
    qDebug() << "DetermineWhetherToEncrypt:" << encryptionPrefs;

    switch (encryptionPrefs) {
    case Kleo::DoIt:
        if (!encryptSomething) {
            encryptAttachments = true;
            return true;
        }
        encrypt = true;
        break;
    case Kleo::DontDoIt:
        encrypt = false;
        break;
    case Kleo::AskOpportunistic:
        opportunistic = true;
        // fall through...
        [[fallthrough]];
    case Kleo::Ask: {
        // the user wants to be asked or has to be asked
        const QString msg = opportunistic ? i18n(
                                "Valid trusted encryption keys were found for all recipients.\n"
                                "Encrypt this message?")
                                          : i18n(
                                              "Examination of the recipient's encryption preferences "
                                              "yielded that you be asked whether or not to encrypt "
                                              "this message.\n"
                                              "Encrypt this message?");
        switch (dialogDelegate->warningTwoActionsCancel(msg,
                                                        i18n("Encrypt Message?"),
                                                        KGuiItem(signSomething ? i18n("Sign && &Encrypt") : i18n("&Encrypt")),
                                                        KGuiItem(signSomething ? i18n("&Sign Only") : i18n("&Send As-Is")))) {
        case ITIPHandlerDialogDelegate::CancelAction:
            result = false;
            canceled = true;
            return false;
        case ITIPHandlerDialogDelegate::PrimaryAction:
            encryptAttachments = true;
            return true;
        case ITIPHandlerDialogDelegate::SecondaryAction:
            encryptAttachments = false;
            return false;
        default:
            qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
            return false;
        }
        break;
    }
    case Kleo::Conflict: {
        // warn the user that there are conflicting encryption preferences
        const QString msg = i18n(
            "There are conflicting encryption preferences "
            "for these recipients.\n"
            "Encrypt this message?");
        switch (dialogDelegate->warningTwoActionsCancel(msg, i18n("Encrypt Message?"), KGuiItem(i18n("&Encrypt")), KGuiItem(i18n("Do &Not Encrypt")))) {
        case ITIPHandlerDialogDelegate::CancelAction:
            result = false;
            canceled = true;
            return false;
        case ITIPHandlerDialogDelegate::PrimaryAction:
            encryptAttachments = true;
            return true;
        case ITIPHandlerDialogDelegate::SecondaryAction:
            encryptAttachments = false;
            return false;
        default:
            qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
            return false;
        }
        break;
    }
    case Kleo::Impossible: {
        const QString msg = i18n(
            "You have requested to encrypt this message, "
            "and to encrypt a copy to yourself, "
            "but no valid trusted encryption keys have been "
            "configured for this identity.");
        if (dialogDelegate->warningContinueCancel(msg, i18n("Send Unencrypted?"), KGuiItem(i18n("Send &Unencrypted")))
            == ITIPHandlerDialogDelegate::CancelAction) {
            result = false;
            return false;
        } else {
            encryptAttachments = false;
            return false;
        }
    }
    }

    if (!encrypt || !doEncryptCompletely) {
        if (cryptoWarningUnencrypted(identity)) {
            const QString msg = !doEncryptCompletely ? i18n(
                                    "Some parts of this message will not be encrypted.\n"
                                    "Sending only partially encrypted messages might violate "
                                    "site policy and/or leak sensitive information.\n"
                                    "Encrypt all parts instead?") // oh, I hate this...
                                                     : i18n(
                                                         "This message will not be encrypted.\n"
                                                         "Sending unencrypted messages might violate site policy and/or "
                                                         "leak sensitive information.\n"
                                                         "Encrypt messages instead?"); // oh, I hate this...
            const QString buttonText = !doEncryptCompletely ? i18n("&Encrypt All Parts") : i18n("&Encrypt");
            switch (dialogDelegate->warningTwoActionsCancel(msg,
                                                            i18n("Unencrypted Message Warning"),
                                                            KGuiItem(buttonText),
                                                            KGuiItem(signSomething ? i18n("&Sign Only") : i18n("&Send As-Is")))) {
            case ITIPHandlerDialogDelegate::CancelAction:
                result = false;
                canceled = true;
                return false;
            case ITIPHandlerDialogDelegate::PrimaryAction:
                encryptAttachments = true;
                return true;
            case ITIPHandlerDialogDelegate::SecondaryAction:
                return encrypt || doEncryptCompletely;
            default:
                qCWarning(AKONADICALENDAR_LOG) << "Unhandled MessageBox response";
                return false;
            }
        }
    }

    return encrypt || doEncryptCompletely;
}

bool MailClient::showKeyApprovalDialog() const
{
    return MessageComposer::MessageComposerSettings::self()->cryptoShowKeysForApproval();
}

#include "moc_mailclient_p.cpp"
