/*
  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB,
        <info@klaralvdalens-datakonsult.se>

  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "itiphandler.h"
#include "mailscheduler_p.h"

#include <KCalendarCore/Incidence>
#include <KCalendarCore/ScheduleMessage>

#include <QObject>

class QWidget;

namespace Akonadi
{
enum Status { StatusNone, StatusSendingInvitation };

/**
  This class handles sending of invitations to attendees when Incidences (e.g.
  events or todos) are created/modified/deleted.

  There are two scenarios:
  o "we" are the organizer, where "we" means any of the identities or mail
    addresses known to Kontact/PIM. If there are attendees, we need to mail
    them all, even if one or more of them are also "us". Otherwise there
    would be no way to invite a resource or our boss, other identities we
    also manage.
  o "we: are not the organizer, which means we changed the completion status
    of a todo, or we changed our attendee status from, say, tentative to
    accepted. In both cases we only mail the organizer. All other changes
    bring us out of sync with the organizer, so we won't mail, if the user
    insists on applying them.

  NOTE: Currently only events and todos are support, meaning Incidence::type()
        should either return "Event" or "Todo"
 */
class ITIPHandlerHelper : public QObject
{
    Q_OBJECT
public:
    explicit ITIPHandlerHelper(ITIPHandlerComponentFactory *factory, QWidget *parent = nullptr);
    ~ITIPHandlerHelper() override;

    enum SendResult {
        ResultCanceled, /**< Sending was canceled by the user, meaning there are
                           local changes of which other attendees are not aware. */
        ResultFailKeepUpdate, /**< Sending failed, the changes to the incidence must be kept. */
        ResultFailAbortUpdate, /**< Sending failed, the changes to the incidence must be undone. */
        ResultNoSendingNeeded, /**< In some cases it is not needed to send an invitation
                                (e.g. when we are the only attendee) */
        ResultError, /**< An unexpected error occurred */
        ResultSuccess, /**< The invitation was sent to all attendees. */
        ResultPending /**< The user has been asked about one detail, waiting for the answer */
    };

    enum MessagePrivacy { MessagePrivacyPlain = 0, MessagePrivacySign = 1, MessagePrivacyEncrypt = 2 };
    Q_DECLARE_FLAGS(MessagePrivacyFlags, MessagePrivacy)

    /**
      When an Incidence is created/modified/deleted the user can choose to send
      an ICal message to the other participants. By default the user will be asked
      if he wants to send a message to other participants. In some cases it is
      preferably though to not bother the user with this question. This method
      allows to change the default behavior. This method applies to the
      sendIncidence*Message() methods.
      @param action the action to set as default
     */
    void setDefaultAction(ITIPHandlerDialogDelegate::Action action);

    /**
     * Sets the default privacy rules for the message.
     */
    void setMessagePrivacy(MessagePrivacyFlags messagePrivacy);

    /**
      Before an invitation is sent the user is asked for confirmation by means of
      an dialog.
      @param parent The parent widget used for the dialogs.
     */
    void setDialogParent(QWidget *parent);

    /**
      Handles sending of invitations for newly created incidences. This method
      asserts that we (as in any of the identities or mail addresses known to
      Kontact/PIM) are the organizer.
      @param incidence The new incidence.
     */
    void sendIncidenceCreatedMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

    /**
       Checks if the incidence should really be modified.

       If the user is not the organizer of this incidence, he will be asked if he really
       wants to proceed.

       Only create the ItemModifyJob if this method returns true.

       @param incidence The modified incidence. It may not be null.
     */
    bool handleIncidenceAboutToBeModified(const KCalendarCore::Incidence::Ptr &incidence);

    /**
      Handles sending of invitations for modified incidences.
      @param incidence The modified incidence.
      @param attendeeStatusChanged if @c true and @p method is #iTIPRequest ask the user whether to send a status update as well
     */
    void sendIncidenceModifiedMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence, bool attendeeStatusChanged);

    /**
      Handles sending of ivitations for deleted incidences.
      @param incidence The deleted incidence.
     */
    void sendIncidenceDeletedMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

    /**
      Send counter proposal message.
      @param receiver Recipient email of the original invitation.
      @param oldIncidence The original event provided in the invitations.
      @param newIncidence The new event as edited by the user.
    */
    ITIPHandlerHelper::SendResult
    sendCounterProposal(const QString &receiver, const KCalendarCore::Incidence::Ptr &oldIncidence, const KCalendarCore::Incidence::Ptr &newIncidence);

Q_SIGNALS:
    void finished(Akonadi::ITIPHandlerHelper::SendResult result, const QString &errorMessage);

    void sendIncidenceDeletedMessageFinished(ITIPHandlerHelper::SendResult, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);
    void sendIncidenceModifiedMessageFinished(ITIPHandlerHelper::SendResult, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);
    void sendIncidenceCreatedMessageFinished(ITIPHandlerHelper::SendResult, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

private Q_SLOTS:
    void onSchedulerFinished(Akonadi::Scheduler::Result result, const QString &errorMsg);

    void slotIncidenceDeletedDialogClosed(const int, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);
    void slotIncidenceModifiedDialogClosed(const int, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);
    void slotIncidenceCreatedDialogClosed(const int, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

    void slotSchedulerFinishDialog(const int result, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

private:
    ITIPHandlerHelper::SendResult
    sentInvitation(int messageBoxReturnCode, const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, const QString &sender = {});

    /**
      We are the organizer. If there is more than one attendee, or if there is
      only one, and it's not the same as the organizer, ask the user to send
      mail.
    */
    bool weAreOrganizerOf(const KCalendarCore::Incidence::Ptr &incidence);

    /**
      Assumes that we are the organizer. If there is more than one attendee, or if
      there is only one, and it's not the same as the organizer, ask the user to send
      mail.
     */
    bool weNeedToSendMailFor(const KCalendarCore::Incidence::Ptr &incidence);

    ITIPHandlerDialogDelegate::Action mDefaultAction;
    QWidget *mParent = nullptr;
    ITIPHandlerComponentFactory *m_factory = nullptr;
    MailScheduler *m_scheduler = nullptr;
    Status m_status;
    MessagePrivacyFlags m_messagePrivacy = MessagePrivacyPlain;
};
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Akonadi::ITIPHandlerHelper::MessagePrivacyFlags)