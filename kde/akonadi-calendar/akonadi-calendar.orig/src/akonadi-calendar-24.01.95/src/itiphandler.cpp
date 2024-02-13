/*
  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB,
        <info@klaralvdalens-datakonsult.se>

  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itiphandler.h"
#include "calendarsettings.h"
#include "itiphandler_p.h"
#include "itiphandlerhelper_p.h"
#include "mailclient_p.h"
#include "publishdialog.h"
#include "utils_p.h"

#include <KCalUtils/Stringify>
#include <KCalendarCore/Attendee>
#include <KCalendarCore/ICalFormat>

#include <Akonadi/MessageQueueJob>
#include <KIdentityManagementCore/IdentityManager>

#include "akonadicalendar_debug.h"
#include <KMessageBox>

using namespace Akonadi;

// async emittion
static void emitiTipMessageProcessed(ITIPHandler *handler, ITIPHandler::Result resultCode, const QString &errorString)
{
    QMetaObject::invokeMethod(handler,
                              "iTipMessageProcessed",
                              Qt::QueuedConnection,
                              Q_ARG(Akonadi::ITIPHandler::Result, resultCode),
                              Q_ARG(QString, errorString));
}

GroupwareUiDelegate::GroupwareUiDelegate(QObject *parent)
    : QObject(parent)
{
}

GroupwareUiDelegate::~GroupwareUiDelegate() = default;

ITIPHandlerComponentFactory::ITIPHandlerComponentFactory(QObject *parent)
    : QObject(parent)
{
}

ITIPHandlerComponentFactory::~ITIPHandlerComponentFactory() = default;

Akonadi::MessageQueueJob *ITIPHandlerComponentFactory::createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence,
                                                                             const KIdentityManagementCore::Identity &identity,
                                                                             QObject *parent)
{
    Q_UNUSED(incidence)
    Q_UNUSED(identity)
    return new Akonadi::MessageQueueJob(parent);
}

ITIPHandlerDialogDelegate *
ITIPHandlerComponentFactory::createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent)
{
    return new ITIPHandlerDialogDelegate(incidence, method, parent);
}

ITIPHandler::ITIPHandler(QObject *parent)
    : QObject(parent)
    , d(new ITIPHandlerPrivate(/*factory=*/nullptr, this))
{
    qRegisterMetaType<Akonadi::ITIPHandler::Result>("Akonadi::ITIPHandler::Result");
}

ITIPHandler::ITIPHandler(ITIPHandlerComponentFactory *factory, QObject *parent)
    : QObject(parent)
    , d(new ITIPHandlerPrivate(factory, this))
{
    qRegisterMetaType<Akonadi::ITIPHandler::Result>("Akonadi::ITIPHandler::Result");
}

ITIPHandler::~ITIPHandler() = default;

void ITIPHandler::processiTIPMessage(const QString &receiver, const QString &iCal, const QString &action)
{
    qCDebug(AKONADICALENDAR_LOG) << "processiTIPMessage called with receiver=" << receiver << "; action=" << action;

    if (d->m_currentOperation != OperationNone) {
        d->m_currentOperation = OperationNone;
        qCritical() << "There can't be an operation in progress!" << d->m_currentOperation;
        return;
    }

    d->m_currentOperation = OperationProcessiTIPMessage;

    if (!d->isLoaded()) {
        d->m_queuedInvitation.receiver = receiver;
        d->m_queuedInvitation.iCal = iCal;
        d->m_queuedInvitation.action = action;
        return;
    }

    if (d->m_calendarLoadError) {
        d->m_currentOperation = OperationNone;
        qCritical() << "Error loading calendar";
        emitiTipMessageProcessed(this, ResultError, i18n("Error loading calendar."));
        return;
    }

    KCalendarCore::ICalFormat format;
    KCalendarCore::ScheduleMessage::Ptr message = format.parseScheduleMessage(d->calendar(), iCal);

    if (!message) {
        const QString errorMessage = format.exception() ? i18n("Error message: %1", KCalUtils::Stringify::errorMessage(*format.exception()))
                                                        : i18n("Unknown error while parsing iCal invitation");

        qCritical() << "Error parsing" << errorMessage;

        if (d->m_showDialogsOnError) {
            KMessageBox::detailedError(nullptr, // mParent, TODO
                                       i18n("Error while processing an invitation or update."),
                                       errorMessage);
        }

        d->m_currentOperation = OperationNone;
        emitiTipMessageProcessed(this, ResultError, errorMessage);

        return;
    }

    d->m_method = static_cast<KCalendarCore::iTIPMethod>(message->method());

    KCalendarCore::ScheduleMessage::Status status = message->status();
    d->m_incidence = message->event().dynamicCast<KCalendarCore::Incidence>();
    if (!d->m_incidence) {
        qCritical() << "Invalid incidence";
        d->m_currentOperation = OperationNone;
        emitiTipMessageProcessed(this, ResultError, i18n("Invalid incidence"));
        return;
    }

    if (action.startsWith(QLatin1String("accepted")) || action.startsWith(QLatin1String("tentative")) || action.startsWith(QLatin1String("delegated"))
        || action.startsWith(QLatin1String("counter"))) {
        // Find myself and set my status. This can't be done in the scheduler,
        // since this does not know the choice I made in the KMail bpf
        KCalendarCore::Attendee::List attendees = d->m_incidence->attendees();
        for (auto &attendee : attendees) {
            if (attendee.email() == receiver) {
                if (action.startsWith(QLatin1String("accepted"))) {
                    attendee.setStatus(KCalendarCore::Attendee::Accepted);
                } else if (action.startsWith(QLatin1String("tentative"))) {
                    attendee.setStatus(KCalendarCore::Attendee::Tentative);
                } else if (CalendarSettings::self()->outlookCompatCounterProposals() && action.startsWith(QLatin1String("counter"))) {
                    attendee.setStatus(KCalendarCore::Attendee::Tentative);
                } else if (action.startsWith(QLatin1String("delegated"))) {
                    attendee.setStatus(KCalendarCore::Attendee::Delegated);
                }
                break;
            }
        }
        d->m_incidence->setAttendees(attendees);

        if (CalendarSettings::self()->outlookCompatCounterProposals() || !action.startsWith(QLatin1String("counter"))) {
            d->m_scheduler->acceptTransaction(d->m_incidence, d->calendar(), d->m_method, status, receiver);
            return; // signal emitted in onSchedulerFinished().
        }
        // TODO: what happens here? we must emit a signal
    } else if (action.startsWith(QLatin1String("cancel"))) {
        // Delete the old incidence, if one is present
        KCalendarCore::Incidence::Ptr existingIncidence = d->calendar()->incidenceFromSchedulingID(d->m_incidence->uid());
        if (existingIncidence) {
            d->m_scheduler->acceptTransaction(d->m_incidence, d->calendar(), KCalendarCore::iTIPCancel, status, receiver);
            return; // signal emitted in onSchedulerFinished().
        } else {
            // We don't have the incidence, nothing to cancel
            qCWarning(AKONADICALENDAR_LOG) << "Couldn't find the incidence to delete.\n"
                                           << "You deleted it previously or didn't even accept the invitation it in the first place.\n"
                                           << "; uid=" << d->m_incidence->uid() << "; identifier=" << d->m_incidence->instanceIdentifier()
                                           << "; summary=" << d->m_incidence->summary();

            qCDebug(AKONADICALENDAR_LOG) << "\n Here's what we do have with such a summary:";
            const KCalendarCore::Incidence::List knownIncidences = calendar()->incidences();
            for (const KCalendarCore::Incidence::Ptr &knownIncidence : knownIncidences) {
                if (knownIncidence->summary() == d->m_incidence->summary()) {
                    qCDebug(AKONADICALENDAR_LOG) << "\nFound: uid=" << knownIncidence->uid() << "; identifier=" << knownIncidence->instanceIdentifier()
                                                 << "; schedulingId" << knownIncidence->schedulingID();
                }
            }

            emitiTipMessageProcessed(this, ResultSuccess, QString());
        }
    } else if (action.startsWith(QLatin1String("reply"))) {
        if (d->m_method != KCalendarCore::iTIPCounter) {
            d->m_scheduler->acceptTransaction(d->m_incidence, d->calendar(), d->m_method, status, QString());
        } else {
            d->m_scheduler->acceptCounterProposal(d->m_incidence, d->calendar());
        }
        return; // signal emitted in onSchedulerFinished().
    } else if (action.startsWith(QLatin1String("request"))) {
        d->m_scheduler->acceptTransaction(d->m_incidence, d->calendar(), d->m_method, status, receiver);
        return;
    } else {
        qCritical() << "Unknown incoming action" << action;

        d->m_currentOperation = OperationNone;
        emitiTipMessageProcessed(this, ResultError, i18n("Invalid action: %1", action));
    }

    if (action.startsWith(QLatin1String("counter"))) {
        if (d->m_uiDelegate) {
            Akonadi::Item item;
            item.setMimeType(d->m_incidence->mimeType());
            item.setPayload(KCalendarCore::Incidence::Ptr(d->m_incidence->clone()));

            // TODO_KDE5: This blocks because m_uiDelegate is not a QObject and can't emit a finished()
            // signal. Make async in kde5
            d->m_uiDelegate->requestIncidenceEditor(item);
            KCalendarCore::Incidence::Ptr newIncidence;
            if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                newIncidence = item.payload<KCalendarCore::Incidence::Ptr>();
            }

            if (*newIncidence == *d->m_incidence) {
                emitiTipMessageProcessed(this, ResultCancelled, QString());
            } else {
                ITIPHandlerHelper::SendResult result = d->m_helper->sendCounterProposal(receiver, d->m_incidence, newIncidence);
                if (result != ITIPHandlerHelper::ResultSuccess) {
                    // It gives success in all paths, this never happens
                    emitiTipMessageProcessed(this, ResultError, i18n("Error sending counter proposal"));
                    Q_ASSERT(false);
                }
            }
        } else {
            // This should never happen
            qCWarning(AKONADICALENDAR_LOG) << "No UI delegate is set";
            emitiTipMessageProcessed(this, ResultError, i18n("Could not start editor to edit counter proposal"));
        }
    }
}

void ITIPHandler::sendiTIPMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence, QWidget *parentWidget)
{
    if (!incidence) {
        Q_ASSERT(false);
        qCritical() << "Invalid incidence";
        return;
    }

    d->m_queuedInvitation.method = method;
    d->m_queuedInvitation.incidence = incidence;
    d->m_parentWidget = parentWidget;

    if (!d->isLoaded()) {
        // This method will be called again once the calendar is loaded.
        return;
    }

    Q_ASSERT(d->m_currentOperation == OperationNone);
    if (d->m_currentOperation != OperationNone) {
        qCritical() << "There can't be an operation in progress!" << d->m_currentOperation;
        return;
    }

    if (incidence->attendeeCount() == 0 && method != KCalendarCore::iTIPPublish) {
        if (d->m_showDialogsOnError) {
            KMessageBox::information(parentWidget,
                                     i18n("The item '%1' has no attendees. "
                                          "Therefore no groupware message will be sent.",
                                          incidence->summary()),
                                     i18nc("@title:window", "Message Not Sent"),
                                     QStringLiteral("ScheduleNoAttendees"));
        }

        return;
    }

    d->m_currentOperation = OperationSendiTIPMessage;

    KCalendarCore::Incidence *incidenceCopy = incidence->clone();
    incidenceCopy->registerObserver(nullptr);
    incidenceCopy->clearAttendees();

    d->m_scheduler->performTransaction(incidence, method);
}

void ITIPHandler::publishInformation(const KCalendarCore::Incidence::Ptr &incidence, QWidget *parentWidget)
{
    Q_ASSERT(incidence);
    if (!incidence) {
        qCritical() << "Invalid incidence. Aborting.";
        return;
    }

    Q_ASSERT(d->m_currentOperation == OperationNone);
    if (d->m_currentOperation != OperationNone) {
        qCritical() << "There can't be an operation in progress!" << d->m_currentOperation;
        return;
    }

    d->m_queuedInvitation.incidence = incidence;
    d->m_parentWidget = parentWidget;

    d->m_currentOperation = OperationPublishInformation;

    QPointer<Akonadi::PublishDialog> publishdlg = new Akonadi::PublishDialog();
    if (incidence->attendeeCount() > 0) {
        KCalendarCore::Attendee::List attendees = incidence->attendees();
        KCalendarCore::Attendee::List::ConstIterator it;
        KCalendarCore::Attendee::List::ConstIterator end(attendees.constEnd());
        for (it = attendees.constBegin(); it != end; ++it) {
            publishdlg->addAttendee(*it);
        }
    }
    if (publishdlg->exec() == QDialog::Accepted && publishdlg) {
        d->m_scheduler->publish(incidence, publishdlg->addresses());
    } else {
        d->m_currentOperation = OperationNone;
        Q_EMIT informationPublished(ResultSuccess, QString()); // Canceled.
    }
    delete publishdlg;
}

void ITIPHandler::sendAsICalendar(const KCalendarCore::Incidence::Ptr &originalIncidence, QWidget *parentWidget)
{
    Q_UNUSED(parentWidget)
    Q_ASSERT(originalIncidence);
    if (!originalIncidence) {
        qCritical() << "Invalid incidence";
        return;
    }

    // Clone so we can change organizer and recurid
    KCalendarCore::Incidence::Ptr incidence = KCalendarCore::Incidence::Ptr(originalIncidence->clone());

    QPointer<Akonadi::PublishDialog> publishdlg = new Akonadi::PublishDialog;
    if (publishdlg->exec() == QDialog::Accepted && publishdlg) {
        const QString recipients = publishdlg->addresses();
        if (incidence->organizer().isEmpty()) {
            incidence->setOrganizer(KCalendarCore::Person(Akonadi::CalendarUtils::fullName(), Akonadi::CalendarUtils::email()));
        }

        if (incidence->hasRecurrenceId()) {
            // For an individual occurrence, recur id doesn't make sense, since we're not sending the whole recurrence series.
            incidence->setRecurrenceId({});
        }

        KCalendarCore::ICalFormat format;
        const QString from = Akonadi::CalendarUtils::email();
        const bool bccMe = Akonadi::CalendarSettings::self()->bcc();
        const QString messageText = format.createScheduleMessage(incidence, KCalendarCore::iTIPRequest);
        auto mailer = new MailClient(d->m_factory);
        d->m_queuedInvitation.incidence = incidence;
        connect(mailer, &MailClient::finished, d.get(), [this](Akonadi::MailClient::Result result, const QString &str) {
            d->finishSendAsICalendar(result, str);
        });

        mailer->mailTo(incidence,
                       KIdentityManagementCore::IdentityManager::self()->identityForAddress(from),
                       from,
                       KCalendarCore::iTIPRequest,
                       bccMe,
                       recipients,
                       messageText);
    }
    delete publishdlg;
}

void ITIPHandler::setGroupwareUiDelegate(GroupwareUiDelegate *delegate)
{
    d->m_uiDelegate = delegate;
}

void ITIPHandler::setCalendar(const Akonadi::CalendarBase::Ptr &calendar)
{
    if (d->m_calendar != calendar) {
        d->m_calendar = calendar;
    }
}

void ITIPHandler::setShowDialogsOnError(bool enable)
{
    d->m_showDialogsOnError = enable;
}

Akonadi::CalendarBase::Ptr ITIPHandler::calendar() const
{
    return d->m_calendar;
}

#include "moc_itiphandler.cpp"
