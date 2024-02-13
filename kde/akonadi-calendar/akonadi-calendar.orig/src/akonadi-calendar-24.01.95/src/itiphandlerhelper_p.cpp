/*
  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB,
        <info@klaralvdalens-datakonsult.se>

  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itiphandlerhelper_p.h"
#include "calendarsettings.h"
#include "utils_p.h"

#include "akonadicalendar_debug.h"
#include <KCalUtils/IncidenceFormatter>
#include <KCalendarCore/Calendar>
#include <KLocalizedString>
#include <KMessageBox>

#include <QWidget>

using namespace Akonadi;

QString proposalComment(const KCalendarCore::Incidence::Ptr &incidence)
{
    QString comment;

    // TODO: doesn't KCalUtils/IncidenceFormater already provide this?
    // if not, it should go there.

    switch (incidence->type()) {
    case KCalendarCore::IncidenceBase::TypeEvent: {
        const QDateTime dtEnd = incidence->dateTime(KCalendarCore::Incidence::RoleDisplayEnd);
        comment = i18n("Proposed new meeting time: %1 - %2",
                       KCalUtils::IncidenceFormatter::dateToString(incidence->dtStart().toLocalTime().date()),
                       KCalUtils::IncidenceFormatter::dateToString(dtEnd.toLocalTime().date()));
        break;
    }
    case KCalendarCore::IncidenceBase::TypeTodo:
        qCWarning(AKONADICALENDAR_LOG) << "NOT IMPLEMENTED: proposalComment called for to-do.";
        break;
    default:
        qCWarning(AKONADICALENDAR_LOG) << "NOT IMPLEMENTED: proposalComment called for " << incidence->typeStr();
    }

    return comment;
}

ITIPHandlerDialogDelegate::ITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent)
    : mParent(parent)
    , mIncidence(incidence)
    , mMethod(method)
{
}

int ITIPHandlerDialogDelegate::askUserIfNeeded(const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo) const
{
    Q_ASSERT_X(!question.isEmpty(), "ITIPHandlerHelper::askUser", "ask what?");

    switch (action) {
    case ActionSendMessage:
        return KMessageBox::ButtonCode::PrimaryAction;
    case ActionDontSendMessage:
        return KMessageBox::ButtonCode::SecondaryAction;
    default:
        return KMessageBox::questionTwoActionsCancel(mParent, question, i18nc("@title:window", "Group Scheduling Email"), buttonYes, buttonNo);
    }
}

void ITIPHandlerDialogDelegate::openDialogIncidenceCreated(Recipient recipient,
                                                           const QString &question,
                                                           Action action,
                                                           const KGuiItem &buttonYes,
                                                           const KGuiItem &buttonNo)
{
    Q_UNUSED(recipient)
    const int messageBoxReturnCode = askUserIfNeeded(question, action, buttonYes, buttonNo);
    Q_EMIT dialogClosed(messageBoxReturnCode, mMethod, mIncidence);
}

void ITIPHandlerDialogDelegate::openDialogIncidenceModified(bool attendeeStatusChanged,
                                                            Recipient recipient,
                                                            const QString &question,
                                                            Action action,
                                                            const KGuiItem &buttonYes,
                                                            const KGuiItem &buttonNo)
{
    Q_UNUSED(attendeeStatusChanged)
    Q_UNUSED(recipient)
    const int messageBoxReturnCode = askUserIfNeeded(question, action, buttonYes, buttonNo);
    Q_EMIT dialogClosed(messageBoxReturnCode, mMethod, mIncidence);
}

void ITIPHandlerDialogDelegate::openDialogIncidenceDeleted(Recipient recipient,
                                                           const QString &question,
                                                           Action action,
                                                           const KGuiItem &buttonYes,
                                                           const KGuiItem &buttonNo)
{
    Q_UNUSED(recipient)
    const int messageBoxReturnCode = askUserIfNeeded(question, action, buttonYes, buttonNo);
    Q_EMIT dialogClosed(messageBoxReturnCode, mMethod, mIncidence);
}

void ITIPHandlerDialogDelegate::openDialogSchedulerFinished(const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    const int messageBoxReturnCode = askUserIfNeeded(question, action, buttonYes, buttonNo);
    Q_EMIT dialogClosed(messageBoxReturnCode, mMethod, mIncidence);
}

ITIPHandlerHelper::SendResult ITIPHandlerHelper::sentInvitation(int messageBoxReturnCode,
                                                                const KCalendarCore::Incidence::Ptr &incidence,
                                                                KCalendarCore::iTIPMethod method,
                                                                const QString &sender)
{
    // The value represented by messageBoxReturnCode is the answer on a question
    // which is a variant of: Do you want to send an email to the attendees?
    //
    // Where the email contains an invitation, modification notification or
    // deletion notification.

    if (messageBoxReturnCode == KMessageBox::ButtonCode::PrimaryAction) {
        // We will be sending out a message here. Now make sure there is some summary
        // Yes, we do modify the incidence here, but we still keep the Ptr
        // semantics, because this change is only for sending and not stored int the
        // local calendar.
        KCalendarCore::Incidence::Ptr _incidence(incidence->clone());
        if (_incidence->summary().isEmpty()) {
            _incidence->setSummary(xi18n("<placeholder>No summary given</placeholder>"));
        }

        // Send the mail
        m_status = StatusSendingInvitation;
        m_scheduler->performTransaction(_incidence, method, sender);
        return ITIPHandlerHelper::ResultSuccess;
    } else if (messageBoxReturnCode == KMessageBox::ButtonCode::SecondaryAction) {
        Q_EMIT finished(ITIPHandlerHelper::ResultCanceled, QString());
        return ITIPHandlerHelper::ResultCanceled;
    } else {
        Q_ASSERT(false); // TODO: Figure out if/how this can happen (by closing the dialog with x??)
        Q_EMIT finished(ITIPHandlerHelper::ResultCanceled, QString());
        return ITIPHandlerHelper::ResultCanceled;
    }
}

bool ITIPHandlerHelper::weAreOrganizerOf(const KCalendarCore::Incidence::Ptr &incidence)
{
    const QString email = incidence->organizer().email();
    return Akonadi::CalendarUtils::thatIsMe(email) || email.isEmpty() || email == QLatin1String("invalid@email.address");
}

bool ITIPHandlerHelper::weNeedToSendMailFor(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (!weAreOrganizerOf(incidence)) {
        qCritical() << "We should be the organizer of this incidence."
                    << "; email= " << incidence->organizer().email() << "; thatIsMe() = " << Akonadi::CalendarUtils::thatIsMe(incidence->organizer().email());
        Q_ASSERT(false);
        return false;
    }

    if (incidence->attendees().isEmpty()) {
        return false;
    }

    // At least one attendee
    return incidence->attendees().count() > 1 || incidence->attendees().at(0).email() != incidence->organizer().email();
}

ITIPHandlerHelper::ITIPHandlerHelper(ITIPHandlerComponentFactory *factory, QWidget *parent)
    : QObject(parent)
    , mDefaultAction(ITIPHandlerDialogDelegate::ActionAsk)
    , mParent(parent)
    , m_factory(factory ? factory : new ITIPHandlerComponentFactory(this))
    , m_scheduler(new MailScheduler(m_factory, this))
    , m_status(StatusNone)
{
    connect(m_scheduler, &MailScheduler::transactionFinished, this, &ITIPHandlerHelper::onSchedulerFinished);
}

ITIPHandlerHelper::~ITIPHandlerHelper() = default;

void ITIPHandlerHelper::setDialogParent(QWidget *parent)
{
    mParent = parent;
}

void ITIPHandlerHelper::setDefaultAction(ITIPHandlerDialogDelegate::Action action)
{
    mDefaultAction = action;
}

void ITIPHandlerHelper::setMessagePrivacy(MessagePrivacyFlags messagePrivacy)
{
    m_scheduler->setSign((messagePrivacy & ITIPHandlerHelper::MessagePrivacySign) == ITIPHandlerHelper::MessagePrivacySign);
    m_scheduler->setEncrypt((messagePrivacy & ITIPHandlerHelper::MessagePrivacyEncrypt) == ITIPHandlerHelper::MessagePrivacyEncrypt);
}

void ITIPHandlerHelper::sendIncidenceCreatedMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence)
{
    /// When we created the incidence, we *must* be the organizer.

    if (!weAreOrganizerOf(incidence)) {
        qCWarning(AKONADICALENDAR_LOG) << "Creating incidence which has another organizer! Will skip sending invitations."
                                       << "; email= " << incidence->organizer().email()
                                       << "; thatIsMe() = " << Akonadi::CalendarUtils::thatIsMe(incidence->organizer().email());
        Q_EMIT sendIncidenceCreatedMessageFinished(ITIPHandlerHelper::ResultFailAbortUpdate, method, incidence);
        Q_EMIT finished(ITIPHandlerHelper::ResultFailAbortUpdate, QString());
        return;
    }

    if (!weNeedToSendMailFor(incidence)) {
        Q_EMIT sendIncidenceCreatedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
        Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
        return;
    }

    QString question;
    if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        question = i18n(
            "The event \"%1\" includes other people.\n"
            "Do you want to email the invitation to the attendees?",
            incidence->summary());
    } else if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        question = i18n(
            "The todo \"%1\" includes other people.\n"
            "Do you want to email the invitation to the attendees?",
            incidence->summary());
    } else {
        question = i18n(
            "This incidence includes other people. "
            "Should an email be sent to the attendees?");
    }

    ITIPHandlerDialogDelegate *askDelegator = m_factory->createITIPHanderDialogDelegate(incidence, method, mParent);
    connect(askDelegator, &ITIPHandlerDialogDelegate::dialogClosed, this, &ITIPHandlerHelper::slotIncidenceCreatedDialogClosed);
    askDelegator->openDialogIncidenceCreated(ITIPHandlerDialogDelegate::Attendees, question, mDefaultAction);
}

void ITIPHandlerHelper::slotIncidenceCreatedDialogClosed(int messageBoxReturnCode,
                                                         KCalendarCore::iTIPMethod method,
                                                         const KCalendarCore::Incidence::Ptr &incidence)
{
    ITIPHandlerHelper::SendResult status = sentInvitation(messageBoxReturnCode, incidence, method);
    Q_EMIT sendIncidenceCreatedMessageFinished(status, method, incidence);
}

bool ITIPHandlerHelper::handleIncidenceAboutToBeModified(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    if (!weAreOrganizerOf(incidence)) {
        switch (incidence->type()) {
        case KCalendarCore::Incidence::TypeEvent: {
            const QString question = i18n(
                "You are not the organizer of this event. Editing it will "
                "bring your calendar out of sync with the organizer's calendar. "
                "Do you really want to edit it?");
            const int messageBoxReturnCode = KMessageBox::warningTwoActions(mParent, question, {}, KStandardGuiItem::ok(), KStandardGuiItem::cancel());
            return messageBoxReturnCode != KMessageBox::ButtonCode::SecondaryAction;
            break;
        }
        case KCalendarCore::Incidence::TypeJournal:
        case KCalendarCore::Incidence::TypeTodo:
            // Not sure why we handle to-dos differently regarding this
            return true;
            break;
        default:
            qCritical() << "Unknown incidence type: " << incidence->type() << incidence->typeStr();
            Q_ASSERT_X(false, "ITIPHandlerHelper::handleIncidenceAboutToBeModified()", "Unknown incidence type");
            return false;
        }
    } else {
        return true;
    }
}

void ITIPHandlerHelper::sendIncidenceModifiedMessage(KCalendarCore::iTIPMethod method,
                                                     const KCalendarCore::Incidence::Ptr &incidence,
                                                     bool attendeeStatusChanged)
{
    ITIPHandlerDialogDelegate *askDelegator = m_factory->createITIPHanderDialogDelegate(incidence, method, mParent);

    connect(askDelegator, &ITIPHandlerDialogDelegate::dialogClosed, this, &ITIPHandlerHelper::slotIncidenceModifiedDialogClosed);
    // For a modified incidence, either we are the organizer or someone else.
    if (weAreOrganizerOf(incidence)) {
        if (weNeedToSendMailFor(incidence)) {
            const QString question = i18n(
                "You changed the invitation \"%1\".\n"
                "Do you want to email the attendees an update message?",
                incidence->summary());

            askDelegator->openDialogIncidenceModified(attendeeStatusChanged,
                                                      ITIPHandlerDialogDelegate::Attendees,
                                                      question,
                                                      mDefaultAction,
                                                      KGuiItem(i18n("Send Update")));
            return;
        } else {
            Q_EMIT sendIncidenceModifiedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
            Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
            delete askDelegator;
            return;
        }
    } else if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        if (method == KCalendarCore::iTIPRequest) {
            // This is an update to be sent to the organizer
            method = KCalendarCore::iTIPReply;
        }

        const QString question = i18n(
            "Do you want to send a status update to the "
            "organizer of this task?");
        askDelegator->openDialogIncidenceModified(attendeeStatusChanged,
                                                  ITIPHandlerDialogDelegate::Organizer,
                                                  question,
                                                  mDefaultAction,
                                                  KGuiItem(i18n("Send Update")));
        return;
    } else if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        if (attendeeStatusChanged && method == KCalendarCore::iTIPRequest) {
            method = KCalendarCore::iTIPReply;
            const QString question = i18n(
                "Your status as an attendee of this event changed. "
                "Do you want to send a status update to the event organizer?");

            askDelegator->openDialogIncidenceModified(attendeeStatusChanged,
                                                      ITIPHandlerDialogDelegate::Organizer,
                                                      question,
                                                      mDefaultAction,
                                                      KGuiItem(i18n("Send Update")));
            return;
        } else {
            slotIncidenceModifiedDialogClosed(KMessageBox::ButtonCode::PrimaryAction, method, incidence);
            delete askDelegator;
            return;
        }
    }
    Q_ASSERT(false); // Shouldn't happen.
    Q_EMIT sendIncidenceModifiedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
    Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
    delete askDelegator;
}

void ITIPHandlerHelper::slotIncidenceModifiedDialogClosed(int messageBoxReturnCode,
                                                          KCalendarCore::iTIPMethod method,
                                                          const KCalendarCore::Incidence::Ptr &incidence)
{
    ITIPHandlerHelper::SendResult status = sentInvitation(messageBoxReturnCode, incidence, method);
    Q_EMIT sendIncidenceModifiedMessageFinished(status, method, incidence);
}

void ITIPHandlerHelper::sendIncidenceDeletedMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);

    ITIPHandlerDialogDelegate *askDelegator = m_factory->createITIPHanderDialogDelegate(incidence, method, mParent);

    connect(askDelegator, &ITIPHandlerDialogDelegate::dialogClosed, this, &ITIPHandlerHelper::slotIncidenceDeletedDialogClosed);

    // For a modified incidence, either we are the organizer or someone else.
    if (weAreOrganizerOf(incidence)) {
        if (weNeedToSendMailFor(incidence)) {
            QString question;
            if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
                question = i18n(
                    "You removed the invitation \"%1\".\n"
                    "Do you want to email the attendees that the event is canceled?",
                    incidence->summary());
            } else if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
                question = i18n(
                    "You removed the invitation \"%1\".\n"
                    "Do you want to email the attendees that the todo is canceled?",
                    incidence->summary());
            } else if (incidence->type() == KCalendarCore::Incidence::TypeJournal) {
                question = i18n(
                    "You removed the invitation \"%1\".\n"
                    "Do you want to email the attendees that the journal is canceled?",
                    incidence->summary());
            }
            askDelegator->openDialogIncidenceDeleted(ITIPHandlerDialogDelegate::Attendees, question, mDefaultAction);
            return;
        } else {
            Q_EMIT sendIncidenceDeletedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
            Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
            delete askDelegator;
            return;
        }
    } else if (incidence->type() != KCalendarCore::Incidence::TypeEvent) {
        if (method == KCalendarCore::iTIPRequest) {
            // This is an update to be sent to the organizer
            method = KCalendarCore::iTIPReply;
        }

        const QString question = (incidence->type() == KCalendarCore::Incidence::TypeTodo) ? i18n(
                                     "Do you want to send a status update to the "
                                     "organizer of this task?")
                                                                                           : i18n(
                                                                                               "Do you want to send a status update to the "
                                                                                               "organizer of this journal?");
        askDelegator->openDialogIncidenceDeleted(ITIPHandlerDialogDelegate::Organizer,
                                                 question,
                                                 mDefaultAction,
                                                 KGuiItem(i18nc("@action:button dialog positive answer", "Send Update")));
        return;
    } else {
        const QStringList myEmails = Akonadi::CalendarUtils::allEmails();
        bool incidenceAcceptedBefore = false;
        for (const QString &email : myEmails) {
            const KCalendarCore::Attendee me = incidence->attendeeByMail(email);
            if (!me.isNull() && (me.status() == KCalendarCore::Attendee::Accepted || me.status() == KCalendarCore::Attendee::Delegated)) {
                incidenceAcceptedBefore = true;
                break;
            }
        }

        if (incidenceAcceptedBefore) {
            QString question = i18n(
                "You had previously accepted an invitation to this event. "
                "Do you want to send an updated response to the organizer "
                "declining the invitation?");
            askDelegator->openDialogIncidenceDeleted(ITIPHandlerDialogDelegate::Organizer,
                                                     question,
                                                     mDefaultAction,
                                                     KGuiItem(i18nc("@action:button dialog positive answer", "Send Update")));
            return;
        } else {
            // We did not accept the event before and delete it from our calendar again,
            // so there is no need to notify people.
            Q_EMIT sendIncidenceDeletedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
            Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
            delete askDelegator;
            return;
        }
    }

    Q_ASSERT(false); // Shouldn't happen.
    Q_EMIT sendIncidenceDeletedMessageFinished(ITIPHandlerHelper::ResultNoSendingNeeded, method, incidence);
    Q_EMIT finished(ITIPHandlerHelper::ResultNoSendingNeeded, QString());
    delete askDelegator;
}

void ITIPHandlerHelper::slotIncidenceDeletedDialogClosed(const int messageBoxReturnCode,
                                                         KCalendarCore::iTIPMethod method,
                                                         const KCalendarCore::Incidence::Ptr &incidence)
{
    ITIPHandlerHelper::SendResult status = sentInvitation(messageBoxReturnCode, incidence, method);
    Q_EMIT sendIncidenceDeletedMessageFinished(status, method, incidence);
}

ITIPHandlerHelper::SendResult
ITIPHandlerHelper::sendCounterProposal(const QString &receiver, const KCalendarCore::Incidence::Ptr &oldEvent, const KCalendarCore::Incidence::Ptr &newEvent)
{
    Q_ASSERT(oldEvent);
    Q_ASSERT(newEvent);

    if (!oldEvent || !newEvent || *oldEvent == *newEvent) {
        return ITIPHandlerHelper::ResultNoSendingNeeded;
    }

    if (CalendarSettings::self()->outlookCompatCounterProposals()) {
        KCalendarCore::Incidence::Ptr tmp(oldEvent->clone());
        tmp->setSummary(i18n("Counter proposal: %1", newEvent->summary()));
        tmp->setDescription(newEvent->description());
        tmp->addComment(proposalComment(newEvent));

        return sentInvitation(KMessageBox::ButtonCode::PrimaryAction, tmp, KCalendarCore::iTIPReply, receiver);
    } else {
        return sentInvitation(KMessageBox::ButtonCode::PrimaryAction, newEvent, KCalendarCore::iTIPCounter, receiver);
    }
}

void ITIPHandlerHelper::onSchedulerFinished(Akonadi::Scheduler::Result result, const QString &errorMsg)
{
    const bool success = result == MailScheduler::ResultSuccess;

    if (m_status == StatusSendingInvitation) {
        m_status = StatusNone;
        if (!success) {
            const QString question(i18n("Sending group scheduling email failed."));

            ITIPHandlerDialogDelegate *askDelegator =
                m_factory->createITIPHanderDialogDelegate(KCalendarCore::Incidence::Ptr(), KCalendarCore::iTIPNoMethod, mParent);

            connect(askDelegator, &ITIPHandlerDialogDelegate::dialogClosed, this, &ITIPHandlerHelper::slotSchedulerFinishDialog);
            askDelegator->openDialogSchedulerFinished(question,
                                                      ITIPHandlerDialogDelegate::ActionAsk,
                                                      KGuiItem(i18nc("@action:button dialog positive answer to abort question", "Abort Update")));
            return;
        } else {
            // fall through
        }
    }

    Q_EMIT finished(success ? ResultSuccess : ResultError, success ? QString() : i18n("Error: %1", errorMsg));
}

void ITIPHandlerHelper::slotSchedulerFinishDialog(const int result, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(method)
    Q_UNUSED(incidence)
    if (result == KMessageBox::ButtonCode::PrimaryAction) {
        Q_EMIT finished(ResultFailAbortUpdate, QString());
    } else {
        Q_EMIT finished(ResultFailKeepUpdate, QString());
    }
}

#include "moc_itiphandlerhelper_p.cpp"
