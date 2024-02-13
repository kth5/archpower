/*
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itiphandler_p.h"
#include "fetchjobcalendar.h"
#include <KCalendarCore/Incidence>
#include <KLocalizedString>
#include <KMessageBox>
using namespace Akonadi;

ITIPHandlerPrivate::ITIPHandlerPrivate(ITIPHandlerComponentFactory *factory, ITIPHandler *qq)
    : m_factory(factory ? factory : new ITIPHandlerComponentFactory(this))
    , m_scheduler(new MailScheduler(m_factory, qq))
    , m_method(KCalendarCore::iTIPNoMethod)
    , m_helper(new ITIPHandlerHelper(m_factory))
    , m_currentOperation(OperationNone)
    , m_uiDelegate(nullptr)
    , q(qq)
{
    m_helper->setParent(this);
    connect(m_scheduler, &Scheduler::transactionFinished, this, &ITIPHandlerPrivate::onSchedulerFinished);

    connect(m_helper, &ITIPHandlerHelper::finished, this, &ITIPHandlerPrivate::onHelperFinished);

    connect(m_helper, &ITIPHandlerHelper::sendIncidenceModifiedMessageFinished, this, &ITIPHandlerPrivate::onHelperModifyDialogClosed);
}

void ITIPHandlerPrivate::onSchedulerFinished(Akonadi::Scheduler::Result result, const QString &errorMessage)
{
    if (m_currentOperation == OperationNone) {
        qCritical() << "Operation can't be none!" << result << errorMessage;
        return;
    }

    if (m_currentOperation == OperationProcessiTIPMessage) {
        m_currentOperation = OperationNone;
        finishProcessiTIPMessage(result, errorMessage);
    } else if (m_currentOperation == OperationSendiTIPMessage) {
        m_currentOperation = OperationNone;
        finishSendiTIPMessage(result, errorMessage);
    } else if (m_currentOperation == OperationPublishInformation) {
        m_currentOperation = OperationNone;
        finishPublishInformation(result, errorMessage);
    } else {
        Q_ASSERT(false);
        qCritical() << "Unknown operation" << m_currentOperation;
    }
}

void ITIPHandlerPrivate::onHelperFinished(Akonadi::ITIPHandlerHelper::SendResult result, const QString &errorMessage)
{
    const bool success = result == ITIPHandlerHelper::ResultSuccess;

    if (m_currentOperation == OperationProcessiTIPMessage) {
        MailScheduler::Result result2 = success ? MailScheduler::ResultSuccess : MailScheduler::ResultGenericError;
        finishProcessiTIPMessage(result2, i18n("Error: %1", errorMessage));
    } else {
        Q_EMIT q->iTipMessageSent(success ? ITIPHandler::ResultSuccess : ITIPHandler::ResultError, success ? QString() : i18n("Error: %1", errorMessage));
    }
}

void ITIPHandlerPrivate::onCounterProposalDelegateFinished(bool success, const QString &errorMessage)
{
    Q_UNUSED(success)
    Q_UNUSED(errorMessage)
    // This will be used when we make editing counter proposals async.
}

void ITIPHandlerPrivate::onLoadFinished(bool success, const QString &errorMessage)
{
    if (m_currentOperation == OperationProcessiTIPMessage) {
        if (success) {
            // Harmless hack, processiTIPMessage() asserts that there's not current operation running
            // to prevent users from calling it twice.
            m_currentOperation = OperationNone;
            q->processiTIPMessage(m_queuedInvitation.receiver, m_queuedInvitation.iCal, m_queuedInvitation.action);
        } else {
            Q_EMIT q->iTipMessageProcessed(ITIPHandler::ResultError, i18n("Error loading calendar: %1", errorMessage));
        }
    } else if (m_currentOperation == OperationSendiTIPMessage) {
        q->sendiTIPMessage(m_queuedInvitation.method, m_queuedInvitation.incidence, m_parentWidget);
    } else if (!success) { // TODO
        m_calendarLoadError = true;
    }
}

void ITIPHandlerPrivate::finishProcessiTIPMessage(Akonadi::MailScheduler::Result result, const QString &errorMessage)
{
    // Handle when user cancels on the collection selection dialog
    if (result == MailScheduler::ResultUserCancelled) {
        Q_EMIT q->iTipMessageProcessed(ITIPHandler::ResultCancelled, QString());
        return;
    }

    const bool success = result == MailScheduler::ResultSuccess;

    if (m_method == KCalendarCore::iTIPCounter) {
        // Here we're processing a counter-proposal that someone sent us and we're the organizer.
        // TODO: Shouldn't there be a test to see if we're the organizer?
        if (success) {
            // send update to all attendees
            Q_ASSERT(m_incidence);
            m_helper->setDialogParent(m_parentWidget);
            m_helper->sendIncidenceModifiedMessage(KCalendarCore::iTIPRequest, KCalendarCore::Incidence::Ptr(m_incidence->clone()), false);
            m_incidence.clear();
            return;
        } else {
            // fall through
        }
    }

    Q_EMIT q->iTipMessageProcessed(success ? ITIPHandler::ResultSuccess : ITIPHandler::ResultError, success ? QString() : i18n("Error: %1", errorMessage));
}

void ITIPHandlerPrivate::onHelperModifyDialogClosed(ITIPHandlerHelper::SendResult sendResult,
                                                    KCalendarCore::iTIPMethod /*method*/,
                                                    const KCalendarCore::Incidence::Ptr &)
{
    if (sendResult == ITIPHandlerHelper::ResultNoSendingNeeded || sendResult == ITIPHandlerHelper::ResultCanceled) {
        Q_EMIT q->iTipMessageSent(ITIPHandler::ResultSuccess, QString());
    }
}

void ITIPHandlerPrivate::finishSendiTIPMessage(Akonadi::MailScheduler::Result result, const QString &errorMessage)
{
    if (result == Scheduler::ResultSuccess) {
        if (m_parentWidget) {
            KMessageBox::information(m_parentWidget,
                                     i18n("The groupware message for item '%1' "
                                          "was successfully sent.\nMethod: %2",
                                          m_queuedInvitation.incidence->summary(),
                                          KCalendarCore::ScheduleMessage::methodName(m_queuedInvitation.method)),
                                     i18nc("@title:window", "Sending Free/Busy"),
                                     QStringLiteral("FreeBusyPublishSuccess"));
        }
        Q_EMIT q->iTipMessageSent(ITIPHandler::ResultSuccess, QString());
    } else {
        const QString error = i18nc(
            "Groupware message sending failed. "
            "%2 is request/reply/add/cancel/counter/etc.",
            "Unable to send the item '%1'.\nMethod: %2",
            m_queuedInvitation.incidence->summary(),
            KCalendarCore::ScheduleMessage::methodName(m_queuedInvitation.method));
        if (m_parentWidget) {
            KMessageBox::error(m_parentWidget, error);
        }
        qCritical() << "Groupware message sending failed." << error << errorMessage;
        Q_EMIT q->iTipMessageSent(ITIPHandler::ResultError, error + errorMessage);
    }
}

void ITIPHandlerPrivate::finishPublishInformation(Akonadi::MailScheduler::Result result, const QString &errorMessage)
{
    if (result == Scheduler::ResultSuccess) {
        if (m_parentWidget) {
            KMessageBox::information(m_parentWidget,
                                     i18n("The item information was successfully sent."),
                                     i18nc("@title:window", "Publishing"),
                                     QStringLiteral("IncidencePublishSuccess"));
        }
        Q_EMIT q->informationPublished(ITIPHandler::ResultSuccess, QString());
    } else {
        const QString error = i18n("Unable to publish the item '%1'", m_queuedInvitation.incidence->summary());
        if (m_parentWidget) {
            KMessageBox::error(m_parentWidget, error);
        }
        qCritical() << "Publish failed." << error << errorMessage;
        Q_EMIT q->informationPublished(ITIPHandler::ResultError, error + errorMessage);
    }
}

void ITIPHandlerPrivate::finishSendAsICalendar(Akonadi::MailClient::Result result, const QString &errorMessage)
{
    if (result == MailClient::ResultSuccess) {
        if (m_parentWidget) {
            KMessageBox::information(m_parentWidget,
                                     i18n("The item information was successfully sent."),
                                     i18nc("@title:window", "Forwarding"),
                                     QStringLiteral("IncidenceForwardSuccess"));
        }
        Q_EMIT q->sentAsICalendar(ITIPHandler::ResultSuccess, QString());
    } else {
        if (m_parentWidget) {
            KMessageBox::error(m_parentWidget,
                               i18n("Unable to forward the item '%1'", m_queuedInvitation.incidence->summary()),
                               i18nc("@title:window", "Forwarding Error"));
        }
        qCritical() << "Sent as iCalendar failed." << errorMessage;
        Q_EMIT q->sentAsICalendar(ITIPHandler::ResultError, errorMessage);
    }

    sender()->deleteLater(); // Delete the mailer
}

CalendarBase::Ptr ITIPHandlerPrivate::calendar()
{
    if (!m_calendar) {
        FetchJobCalendar::Ptr fetchJobCalendar = FetchJobCalendar::Ptr(new FetchJobCalendar());
        connect(fetchJobCalendar.data(), &FetchJobCalendar::loadFinished, this, &ITIPHandlerPrivate::onLoadFinished);

        m_calendar = fetchJobCalendar;
    }

    return m_calendar;
}

bool ITIPHandlerPrivate::isLoaded()
{
    FetchJobCalendar::Ptr fetchJobCalendar = calendar().dynamicCast<Akonadi::FetchJobCalendar>();
    if (fetchJobCalendar) {
        return !fetchJobCalendar->isLoading();
    }

    // If it's an ETMCalendar, set through setCalendar(), then it's already loaded, it's a requirement of setCalendar().
    // ETM doesn't have any way to check if it's already populated, so we have to require loaded calendars.
    return true;
}

#include "moc_itiphandler_p.cpp"
