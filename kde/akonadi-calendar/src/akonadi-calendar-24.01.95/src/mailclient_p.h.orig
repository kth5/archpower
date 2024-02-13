/*
  SPDX-FileCopyrightText: 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_tests_export.h"
#include "itiphandler.h"
#include <KCalendarCore/IncidenceBase>
#include <KMime/KMimeMessage>
#include <QObject>

#include <optional>

struct UnitTestResult {
    using List = QList<UnitTestResult>;
    QString from;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    int transportId;
    KMime::Message::Ptr message;
    UnitTestResult()
        : transportId(-1)
    {
    }
};

namespace KIdentityManagementCore
{
class Identity;
}

namespace Kleo
{
class KeyResolver;
}

namespace MessageComposer
{
class Composer;
class ContactPreference;
}

namespace MailTransport
{
class Transport;
}

namespace GpgME
{
class Key;
}

class KJob;

namespace Akonadi
{
class ITIPHandlerComponentFactory;

class AKONADI_CALENDAR_TESTS_EXPORT MailClient : public QObject
{
    Q_OBJECT

    struct MessageData {
        QString from;
        QStringList to;
        QStringList cc;
        QString subject;
        QString body;
        QString attachment;
        QString method;
        bool hidden = false;
        bool bccMe = false;
        bool sign = false;
        bool encrypt = false;
    };

    bool mAkonadiLookupEnabled = true;

public:
    enum Result { ResultSuccess, ResultNoAttendees, ResultReallyNoAttendees, ResultErrorCreatingTransport, ResultErrorFetchingTransport, ResultQueueJobError };

    enum MailPrivacy { MailPrivacyPlain = 0, MailPrivacySign = 1, MailPrivacyEncrypt = 2 };
    Q_DECLARE_FLAGS(MailPrivacyFlags, MailPrivacy)

    explicit MailClient(ITIPHandlerComponentFactory *factory, QObject *parent = nullptr);
    ~MailClient() override;

    void mailAttendees(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const KIdentityManagementCore::Identity &identity,
                       KCalendarCore::iTIPMethod method,
                       bool bccMe,
                       const QString &attachment = QString(),
                       const QString &mailTransport = QString(),
                       MailPrivacyFlags mailPrivacy = MailPrivacyPlain);

    void mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const KIdentityManagementCore::Identity &identity,
                       const QString &from,
                       KCalendarCore::iTIPMethod method,
                       bool bccMe,
                       const QString &attachment = QString(),
                       const QString &sub = QString(),
                       const QString &mailTransport = QString(),
                       MailPrivacyFlags mailPrivacy = MailPrivacyPlain);

    void mailTo(const KCalendarCore::IncidenceBase::Ptr &incidence,
                const KIdentityManagementCore::Identity &identity,
                const QString &from,
                KCalendarCore::iTIPMethod method,
                bool bccMe,
                const QString &recipients,
                const QString &attachment = QString(),
                const QString &mailTransport = QString(),
                MailPrivacyFlags mailPrivacy = MailPrivacyPlain);

    /**
      Sends mail with specified from, to and subject field and body as text.
      If bcc is set, send a blind carbon copy to the sender

      @param incidence is the incidence, that is sended
      @param identity is the Identity of the sender
      @param from is the address of the sender of the message
      @param to a list of addresses to receive the message
      @param cc a list of addresses to receive message carbon copies
      @param subject is the subject of the message
      @param body is the boody of the message
      @param hidden if true and using KMail as the mailer, send the message
      without opening a composer window.
      @param bcc if true, send a blind carbon copy to the message sender
      @param attachment optional attachment (raw data)
      @param mailTransport defines the mail transport method. See here the
      kdepimlibs/mailtransport library.
    */
    void send(const KCalendarCore::IncidenceBase::Ptr &incidence,
              const KIdentityManagementCore::Identity &identity,
              const MessageData &msg,
              const QString &mailTransport = QString());

private:
    void handleQueueJobFinished(KJob *job);

Q_SIGNALS:
    void finished(Akonadi::MailClient::Result result, const QString &errorString);

public:
    // For unit-test usage, since we can't depend on kdepim-runtime on jenkins
    ITIPHandlerComponentFactory *mFactory = nullptr;

protected:
    /** Allows to disable Akonadi lookup in KeyResolver (for tests) */
    void setAkonadiLookupEnabled(bool enabled);
    /** Allows to override KeyResolver contact preferences (useful for tests) */
    virtual std::optional<MessageComposer::ContactPreference> contactPreference(const QString &email);

    /** Whether to show key approval dialog or not. **/
    virtual bool showKeyApprovalDialog() const;

private:
    std::vector<std::unique_ptr<MessageComposer::Composer>>
    buildComposers(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagementCore::Identity &identity, const MessageData &msg);

    void populateComposer(MessageComposer::Composer *composer, const MessageData &msg);

    bool determineWhetherToSign(bool doSignCompletely,
                                Kleo::KeyResolver *keyResolver,
                                ITIPHandlerDialogDelegate *dialogDelegate,
                                const KIdentityManagementCore::Identity &identity,
                                bool signSomething,
                                bool &signAttachments,
                                bool &result,
                                bool &canceled);

    bool determineWhetherToEncrypt(bool doEncryptCompletely,
                                   Kleo::KeyResolver *keyResolver,
                                   ITIPHandlerDialogDelegate *dialogDelegate,
                                   const KIdentityManagementCore::Identity &identity,
                                   bool encryptSomething,
                                   bool signSomething,
                                   bool &encryptAttachments,
                                   bool &result,
                                   bool &canceled);

    bool
    addKeysToContext(const QString &gnupgHome, const QList<QPair<QStringList, std::vector<GpgME::Key>>> &data, const std::map<QByteArray, QString> &autocrypt);

    void queueMessage(const MailTransport::Transport *transport,
                      const MessageComposer::Composer *composer,
                      const KCalendarCore::IncidenceBase::Ptr &incidence,
                      const KIdentityManagementCore::Identity &identity,
                      const MessageData &msg,
                      const KMime::Message::Ptr &message);

    void populateKeyResolverContactsPreferences(Kleo::KeyResolver &keyResolver, const QStringList &addresses);
};
}

Q_DECLARE_METATYPE(Akonadi::MailClient::Result)
