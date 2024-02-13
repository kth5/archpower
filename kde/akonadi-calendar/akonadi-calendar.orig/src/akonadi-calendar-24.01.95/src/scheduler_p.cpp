/*
  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2012-2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "scheduler_p.h"
#include "calendarbase_p.h"

#include <KCalUtils/Stringify>

#include <KCalendarCore/FreeBusyCache>
#include <KCalendarCore/ICalFormat>

#include "akonadicalendar_debug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <QTimeZone>

using namespace KCalendarCore;
using namespace Akonadi;

class Akonadi::SchedulerPrivate
{
public:
    explicit SchedulerPrivate(Scheduler *qq)
        : q(qq)
    {
    }

    FreeBusyCache *mFreeBusyCache = nullptr;
    bool mShowDialogs = true;
    Scheduler *const q;
};

Scheduler::Scheduler(QObject *parent)
    : QObject(parent)
    , d(new SchedulerPrivate(this))
{
    mFormat = new ICalFormat();
    mFormat->setTimeZone(QTimeZone::systemTimeZone());
}

Scheduler::~Scheduler()
{
    delete mFormat;
}

void Scheduler::setShowDialogs(bool enable)
{
    d->mShowDialogs = enable;
}

void Scheduler::setFreeBusyCache(FreeBusyCache *c)
{
    d->mFreeBusyCache = c;
}

FreeBusyCache *Scheduler::freeBusyCache() const
{
    return d->mFreeBusyCache;
}

void Scheduler::acceptTransaction(const IncidenceBase::Ptr &incidence,
                                  const Akonadi::CalendarBase::Ptr &calendar,
                                  iTIPMethod method,
                                  ScheduleMessage::Status status,
                                  const QString &email)
{
    Q_ASSERT(incidence);
    Q_ASSERT(calendar);
    qCDebug(AKONADICALENDAR_LOG) << "method=" << ScheduleMessage::methodName(method);
    connectCalendar(calendar);
    switch (method) {
    case iTIPPublish:
        acceptPublish(incidence, calendar, status, method);
        break;
    case iTIPRequest:
        acceptRequest(incidence, calendar, status, email);
        break;
    case iTIPAdd:
        acceptAdd(incidence, status);
        break;
    case iTIPCancel:
        acceptCancel(incidence, calendar, status, email);
        break;
    case iTIPDeclineCounter:
        acceptDeclineCounter(incidence, status);
        break;
    case iTIPReply:
        acceptReply(incidence, calendar, status, method);
        break;
    case iTIPRefresh:
        acceptRefresh(incidence, status);
        break;
    case iTIPCounter:
        acceptCounter(incidence, status);
        break;
    default:
        qCWarning(AKONADICALENDAR_LOG) << "Unhandled method: " << method;
    }
}

void Scheduler::acceptPublish(const IncidenceBase::Ptr &newIncBase,
                              const Akonadi::CalendarBase::Ptr &calendar,
                              ScheduleMessage::Status status,
                              iTIPMethod method)
{
    if (newIncBase->type() == IncidenceBase::TypeFreeBusy) {
        acceptFreeBusy(newIncBase, method);
        return;
    }

    QString errorString;
    Result result = ResultSuccess;

    qCDebug(AKONADICALENDAR_LOG) << "status=" << KCalUtils::Stringify::scheduleMessageStatus(status);

    Incidence::Ptr newInc = newIncBase.staticCast<Incidence>();
    Incidence::Ptr calInc = calendar->incidence(newIncBase->uid());
    switch (status) {
    case ScheduleMessage::Unknown:
    case ScheduleMessage::PublishNew:
    case ScheduleMessage::PublishUpdate:
        if (calInc && newInc) {
            if ((newInc->revision() > calInc->revision()) || (newInc->revision() == calInc->revision() && newInc->lastModified() > calInc->lastModified())) {
                const QString oldUid = calInc->uid();

                if (calInc->type() != newInc->type()) {
                    result = ResultAssigningDifferentTypes;
                    errorString = i18n("Error: Assigning different incidence types.");
                    qCritical() << errorString;
                } else {
                    newInc->setSchedulingID(newInc->uid(), oldUid);
                    const bool success = calendar->modifyIncidence(newInc);

                    if (!success) {
                        Q_EMIT transactionFinished(ResultModifyingError, QStringLiteral("Error modifying incidence"));
                    } else {
                        // signal will be emitted in the handleModifyFinished() slot
                    }

                    return;
                }
            }
        }
        break;
    case ScheduleMessage::Obsolete:
        break;
    default:
        break;
    }

    Q_EMIT transactionFinished(result, errorString);
}

void Scheduler::acceptRequest(const IncidenceBase::Ptr &incidenceBase,
                              const Akonadi::CalendarBase::Ptr &calendar,
                              ScheduleMessage::Status status,
                              const QString &email)
{
    Incidence::Ptr incidence = incidenceBase.staticCast<Incidence>();

    if (incidence->type() == IncidenceBase::TypeFreeBusy) {
        // reply to this request is handled in korganizer's incomingdialog
        Q_EMIT transactionFinished(ResultSuccess, QString());
        return;
    }

    const QString schedulingUid = incidence->uid();
    QString errorString;
    Result result = ResultSuccess;

    const Incidence::List existingIncidences = calendar->incidencesFromSchedulingID(schedulingUid);
    qCDebug(AKONADICALENDAR_LOG) << "status=" << KCalUtils::Stringify::scheduleMessageStatus(status) << ": found " << existingIncidences.count()
                                 << " incidences with schedulingID " << incidence->schedulingID() << "; uid was = " << schedulingUid;

    if (existingIncidences.isEmpty()) {
        // Perfectly normal if the incidence doesn't exist. This is probably
        // a new invitation.
        qCDebug(AKONADICALENDAR_LOG) << "incidence not found; calendar = " << calendar.data() << "; incidence count = " << calendar->incidences().count();
    }

    for (const KCalendarCore::Incidence::Ptr &existingIncidence : existingIncidences) {
        qCDebug(AKONADICALENDAR_LOG) << "Considering this found event (" << (existingIncidence->isReadOnly() ? "readonly" : "readwrite")
                                     << ") :" << mFormat->toString(existingIncidence);
        // If it's readonly, we can't possible update it.
        if (existingIncidence->isReadOnly()) {
            continue;
        }

        const QString existingUid = existingIncidence->uid();
        const int existingRevision = existingIncidence->revision();

        if (existingRevision <= incidence->revision()) {
            // The new incidence might be an update for the found one
            bool isUpdate = true;
            // Code for new invitations:
            // If you think we could check the value of "status" to be RequestNew:  we can't.
            // It comes from a similar check inside libical, where the event is compared to
            // other events in the calendar. But if we have another version of the event around
            // (e.g. shared folder for a group), the status could be RequestNew, Obsolete or Updated.
            qCDebug(AKONADICALENDAR_LOG) << "looking in " << existingUid << "'s attendees";
            // This is supposed to be a new request, not an update - however we want to update
            // the existing one to handle the "clicking more than once on the invitation" case.
            // So check the attendee status of the attendee.
            const Attendee::List attendees = existingIncidence->attendees();
            Attendee::List::ConstIterator ait;
            for (ait = attendees.begin(); ait != attendees.end(); ++ait) {
                if ((*ait).email() == email && (*ait).status() == Attendee::NeedsAction) {
                    // This incidence wasn't created by me - it's probably in a shared folder
                    // and meant for someone else, ignore it.
                    qCDebug(AKONADICALENDAR_LOG) << "ignoring " << existingUid << " since I'm still NeedsAction there";
                    isUpdate = false;
                    break;
                }
            }
            if (isUpdate) {
                if (existingRevision == incidence->revision() && existingIncidence->lastModified() > incidence->lastModified()) {
                    // This isn't an update - the found incidence was modified more recently
                    errorString = i18n(
                        "This isn't an update. "
                        "The found incidence was modified more recently.");
// QT5 port
#if 0
                    qCWarning(AKONADICALENDAR_LOG) << errorString
                                                   << "; revision=" << existingIncidence->revision()
                                                   << "; existing->lastModified=" << existingIncidence->lastModified()
                                                   << "; update->lastModified=" << incidence->lastModified();
#endif
                    Q_EMIT transactionFinished(ResultOutatedUpdate, errorString);
                    return;
                }
                qCDebug(AKONADICALENDAR_LOG) << "replacing existing incidence " << existingUid;
                if (existingIncidence->type() != incidence->type()) {
                    qCritical() << "assigning different incidence types";
                    result = ResultAssigningDifferentTypes;
                    errorString = i18n("Error: Assigning different incidence types.");
                    Q_EMIT transactionFinished(result, errorString);
                } else {
                    incidence->setSchedulingID(schedulingUid, existingUid);

                    if (incidence->hasRecurrenceId()) {
                        Incidence::Ptr existingInstance = calendar->incidence(incidence->instanceIdentifier());
                        if (!existingInstance) {
                            // The organizer created an exception, lets create it in our calendar, we don't have it yet
                            const bool success = calendar->addIncidence(incidence);

                            if (!success) {
                                Q_EMIT transactionFinished(ResultCreatingError, QStringLiteral("Error creating incidence"));
                            } else {
                                // Signal emitted in the result slot of addFinished()
                            }

                            return;
                        }
                    }

                    const bool success = calendar->modifyIncidence(incidence);

                    if (!success) {
                        Q_EMIT transactionFinished(ResultModifyingError, i18n("Error modifying incidence"));
                    } else {
                        // handleModifyFinished() will Q_EMIT the final signal.
                    }
                }
                return;
            }
        } else {
            errorString = i18n(
                "This isn't an update. "
                "The found incidence was modified more recently.");
            qCWarning(AKONADICALENDAR_LOG) << errorString;
            // This isn't an update - the found incidence has a bigger revision number
            qCDebug(AKONADICALENDAR_LOG) << "This isn't an update - the found incidence has a bigger revision number";
            Q_EMIT transactionFinished(ResultOutatedUpdate, errorString);
            return;
        }
    }

    // Move the uid to be the schedulingID and make a unique UID
    incidence->setSchedulingID(schedulingUid, CalFormat::createUniqueId());
    // notify the user in case this is an update and we didn't find the to-be-updated incidence
    if (d->mShowDialogs && existingIncidences.isEmpty() && incidence->revision() > 0) {
        KMessageBox::information(nullptr,
                                 xi18nc("@info",
                                        "<para>You added an invitation update, but an earlier version of the "
                                        "item could not be found in your calendar.</para>"
                                        "<para>This may have occurred because:<list>"
                                        "<item>the organizer did not include you in the original invitation</item>"
                                        "<item>you did not accept the original invitation yet</item>"
                                        "<item>you deleted the original invitation from your calendar</item>"
                                        "<item>you no longer have access to the calendar containing the invitation</item>"
                                        "</list></para>"
                                        "<para>This is not a problem, but we thought you should know.</para>"),
                                 i18nc("@title:window", "Cannot Find Invitation to be Updated"),
                                 QStringLiteral("AcceptCantFindIncidence"));
    }
    qCDebug(AKONADICALENDAR_LOG) << "Storing new incidence with scheduling uid=" << schedulingUid << " and uid=" << incidence->uid();

    const bool success = calendar->addIncidence(incidence);
    if (!success) {
        Q_EMIT transactionFinished(ResultCreatingError, i18n("Error adding incidence"));
    } else {
        // The slot will Q_EMIT the result
    }
}

void Scheduler::acceptAdd(const IncidenceBase::Ptr &, ScheduleMessage::Status)
{
    Q_EMIT transactionFinished(ResultSuccess, QString());
}

void Scheduler::acceptCancel(const IncidenceBase::Ptr &incidenceBase,
                             const Akonadi::CalendarBase::Ptr &calendar,
                             ScheduleMessage::Status status,
                             const QString &attendeeEmail)
{
    Incidence::Ptr incidence = incidenceBase.staticCast<Incidence>();

    if (incidence->type() == IncidenceBase::TypeFreeBusy) {
        // reply to this request is handled in korganizer's incomingdialog
        Q_EMIT transactionFinished(ResultSuccess, QString());
        return;
    }

    if (incidence->type() == IncidenceBase::TypeJournal) {
        Q_EMIT transactionFinished(ResultUnsupported, QStringLiteral("Unsupported incidence type"));
        return;
    }

    const Incidence::List existingIncidences = calendar->incidencesFromSchedulingID(incidence->uid());
    qCDebug(AKONADICALENDAR_LOG) << "Scheduler::acceptCancel=" << KCalUtils::Stringify::scheduleMessageStatus(status) << ": found "
                                 << existingIncidences.count() << " incidences with schedulingID " << incidence->schedulingID();

    Result result = ResultIncidenceToDeleteNotFound;
    const QString errorString = i18n("Could not find incidence to delete.");
    for (const KCalendarCore::Incidence::Ptr &existingIncidence : existingIncidences) {
        qCDebug(AKONADICALENDAR_LOG) << "Considering this found event (" << (existingIncidence->isReadOnly() ? "readonly" : "readwrite")
                                     << ") :" << mFormat->toString(existingIncidence);

        // If it's readonly, we can't possible remove it.
        if (existingIncidence->isReadOnly()) {
            continue;
        }

        const QString existingUid = existingIncidence->uid();

        // Code for new invitations:
        // We cannot check the value of "status" to be RequestNew because
        // "status" comes from a similar check inside libical, where the event
        // is compared to other events in the calendar. But if we have another
        // version of the event around (e.g. shared folder for a group), the
        // status could be RequestNew, Obsolete or Updated.
        qCDebug(AKONADICALENDAR_LOG) << "looking in " << existingUid << "'s attendees";

        // This is supposed to be a new request, not an update - however we want
        // to update the existing one to handle the "clicking more than once
        // on the invitation" case. So check the attendee status of the attendee.
        bool isMine = true;
        const Attendee::List attendees = existingIncidence->attendees();
        for (const KCalendarCore::Attendee &attendee : attendees) {
            if (attendee.email() == attendeeEmail && attendee.status() == Attendee::NeedsAction) {
                // This incidence wasn't created by me - it's probably in a shared
                // folder and meant for someone else, ignore it.
                qCDebug(AKONADICALENDAR_LOG) << "ignoring " << existingUid << " since I'm still NeedsAction there";
                isMine = false;
                break;
            }
        }

        if (!isMine) {
            continue;
        }

        qCDebug(AKONADICALENDAR_LOG) << "removing existing incidence " << existingUid;
        if (incidence->hasRecurrenceId()) {
            Incidence::Ptr existingInstance = calendar->incidence(incidence->instanceIdentifier());

            if (existingInstance) {
                existingInstance->setStatus(Incidence::StatusCanceled);
                result = calendar->modifyIncidence(existingInstance) ? ResultSuccess : ResultModifyingError;
            } else {
                incidence->setSchedulingID(incidence->uid(), existingIncidence->uid());
                incidence->setStatus(Incidence::StatusCanceled);
                result = calendar->addIncidence(incidence) ? ResultSuccess : ResultCreatingError;
            }

            if (result != ResultSuccess) {
                Q_EMIT transactionFinished(result, i18n("Error recording exception"));
            }
        } else {
            result = calendar->deleteIncidence(existingIncidence) ? ResultSuccess : ResultErrorDelete;
            if (result != ResultSuccess) {
                Q_EMIT transactionFinished(result, errorString);
            }
        }

        // The success case will be handled in handleDeleteFinished()
        return;
    }

    // in case we didn't find the to-be-removed incidencez
    if (d->mShowDialogs && !existingIncidences.isEmpty() && incidence->revision() > 0) {
        KMessageBox::error(nullptr,
                           i18nc("@info",
                                 "The event or task could not be removed from your calendar. "
                                 "Maybe it has already been deleted or is not owned by you. "
                                 "Or it might belong to a read-only or disabled calendar."));
    }
    Q_EMIT transactionFinished(result, errorString);
}

void Scheduler::acceptDeclineCounter(const IncidenceBase::Ptr &, ScheduleMessage::Status)
{
    // Not sure why KCalUtils::Scheduler returned false here
    Q_EMIT transactionFinished(ResultGenericError, i18n("Generic Error"));
}

void Scheduler::acceptReply(const IncidenceBase::Ptr &incidenceBase,
                            const Akonadi::CalendarBase::Ptr &calendar,
                            ScheduleMessage::Status status,
                            iTIPMethod method)
{
    Q_UNUSED(status)
    if (incidenceBase->type() == IncidenceBase::TypeFreeBusy) {
        acceptFreeBusy(incidenceBase, method);
        return;
    }

    Result result = ResultGenericError;
    QString errorString = i18n("Generic Error");

    Incidence::Ptr incidence = calendar->incidence(incidenceBase->uid());

    // try harder to find the correct incidence
    if (!incidence) {
        const Incidence::List list = calendar->incidences();
        for (Incidence::List::ConstIterator it = list.constBegin(), end = list.constEnd(); it != end; ++it) {
            if ((*it)->schedulingID() == incidenceBase->uid()) {
                incidence = (*it).dynamicCast<Incidence>();
                break;
            }
        }
    }

    if (incidence) {
        // get matching attendee in calendar
        qCDebug(AKONADICALENDAR_LOG) << "match found!";
        const Attendee::List attendeesIn = incidenceBase->attendees();
        Attendee::List attendeesNew;
        Attendee::List attendeesEv = incidence->attendees();
        for (const auto &attIn : attendeesIn) {
            bool found = false;
            for (auto &attEv : attendeesEv) {
                if (attIn.email().toLower() == attEv.email().toLower()) {
                    // update attendee-info
                    qCDebug(AKONADICALENDAR_LOG) << "update attendee";
                    attEv.setStatus(attIn.status());
                    attEv.setDelegate(attIn.delegate());
                    attEv.setDelegator(attIn.delegator());
                    result = ResultSuccess;
                    errorString.clear();
                    found = true;
                }
            }
            if (!found && attIn.status() != Attendee::Declined) {
                attendeesNew.append(attIn);
            }
        }
        incidence->setAttendees(attendeesEv);

        bool attendeeAdded = false;
        for (const auto &attNew : std::as_const(attendeesNew)) {
            QString msg = i18nc("@info", "%1 wants to attend %2 but was not invited.", attNew.fullName(), incidence->summary());
            if (!attNew.delegator().isEmpty()) {
                msg = i18nc("@info", "%1 wants to attend %2 on behalf of %3.", attNew.fullName(), incidence->summary(), attNew.delegator());
            }
            if (KMessageBox::questionTwoActions(nullptr,
                                                msg,
                                                i18nc("@title:window", "Uninvited Attendee"),
                                                KGuiItem(i18nc("@option", "Accept Attendance")),
                                                KGuiItem(i18nc("@option", "Reject Attendance")))
                != KMessageBox::ButtonCode::PrimaryAction) {
                Incidence::Ptr cancel = incidence;
                cancel->addComment(i18nc("@info", "The organizer rejected your attendance at this meeting."));
                performTransaction(incidenceBase, iTIPCancel, attNew.fullName());
                continue;
            }

            Attendee a(attNew.name(), attNew.email(), attNew.RSVP(), attNew.status(), attNew.role(), attNew.uid());

            a.setDelegate(attNew.delegate());
            a.setDelegator(attNew.delegator());
            incidence->addAttendee(a);

            result = ResultSuccess;
            errorString.clear();
            attendeeAdded = true;
        }

        // send update about new participants
        if (attendeeAdded) {
            bool sendMail = false;
            if (KMessageBox::questionTwoActions(nullptr,
                                                i18nc("@info",
                                                      "An attendee was added to the incidence. "
                                                      "Do you want to email the attendees an update message?"),
                                                i18nc("@title:window", "Attendee Added"),
                                                KGuiItem(i18nc("@option", "Send Messages")),
                                                KGuiItem(i18nc("@option", "Do Not Send")))
                == KMessageBox::ButtonCode::PrimaryAction) {
                sendMail = true;
            }

            incidence->setRevision(incidence->revision() + 1);
            if (sendMail) {
                performTransaction(incidence, iTIPRequest);
            }
        }

        if (incidence->type() == Incidence::TypeTodo) {
            // for VTODO a REPLY can be used to update the completion status of
            // a to-do. see RFC2446 3.4.3
            Todo::Ptr update = incidenceBase.dynamicCast<Todo>();
            Todo::Ptr calendarTodo = incidence.staticCast<Todo>();
            Q_ASSERT(update);
            if (update && (calendarTodo->percentComplete() != update->percentComplete())) {
                calendarTodo->setPercentComplete(update->percentComplete());
                calendarTodo->updated();
                const bool success = calendar->modifyIncidence(calendarTodo);
                if (!success) {
                    Q_EMIT transactionFinished(ResultModifyingError, i18n("Error modifying incidence"));
                } else {
                    // success will be emitted in the handleModifyFinished() slot
                }
                return;
            }
        }

        if (result == ResultSuccess) {
            // We set at least one of the attendees, so the incidence changed
            // Note: This should not result in a sequence number bump
            incidence->updated();
            const bool success = calendar->modifyIncidence(incidence);

            if (!success) {
                Q_EMIT transactionFinished(ResultModifyingError, i18n("Error modifying incidence"));
            } else {
                // success will be emitted in the handleModifyFinished() slot
            }

            return;
        }
    } else {
        result = ResultSuccess;
        errorString = i18n("No incidence for scheduling.");
        qCritical() << errorString;
    }
    Q_EMIT transactionFinished(result, errorString);
}

void Scheduler::acceptRefresh(const IncidenceBase::Ptr &, ScheduleMessage::Status)
{
    // handled in korganizer's IncomingDialog
    // Not sure why it returns false here
    Q_EMIT transactionFinished(ResultGenericError, i18n("Generic Error"));
}

void Scheduler::acceptCounter(const IncidenceBase::Ptr &, ScheduleMessage::Status)
{
    // Not sure why it returns false here
    Q_EMIT transactionFinished(ResultGenericError, i18n("Generic Error"));
}

void Scheduler::acceptFreeBusy(const IncidenceBase::Ptr &incidence, iTIPMethod method)
{
    if (!d->mFreeBusyCache) {
        qCritical() << "Scheduler: no FreeBusyCache.";
        Q_EMIT transactionFinished(ResultNoFreeBusyCache, i18n("No Free Busy Cache"));
        return;
    }

    FreeBusy::Ptr freebusy = incidence.staticCast<FreeBusy>();

    qCDebug(AKONADICALENDAR_LOG) << "freeBusyDirName:" << freeBusyDir();

    Person from;
    if (method == iTIPPublish) {
        from = freebusy->organizer();
    }
    if ((method == iTIPReply) && (freebusy->attendeeCount() == 1)) {
        Attendee attendee = freebusy->attendees().at(0);
        from.setName(attendee.name());
        from.setEmail(attendee.email());
    }

    if (!d->mFreeBusyCache->saveFreeBusy(freebusy, from)) {
        Q_EMIT transactionFinished(ResultErrorSavingFreeBusy, i18n("Error saving freebusy object"));
    } else {
        Q_EMIT transactionFinished(ResultNoFreeBusyCache, QString());
    }
}

void Scheduler::handleCreateFinished(bool success, const QString &errorMessage)
{
    auto calendar = qobject_cast<Akonadi::CalendarBase *>(sender());
    const bool cancelled = calendar && calendar->d_ptr->mLastCreationCancelled;

    Q_EMIT transactionFinished(success ? ResultSuccess : (cancelled ? ResultUserCancelled : ResultCreatingError), errorMessage);
}

void Scheduler::handleModifyFinished(bool success, const QString &errorMessage)
{
    qCDebug(AKONADICALENDAR_LOG) << "Modification finished. Success=" << success << errorMessage;
    Q_EMIT transactionFinished(success ? ResultSuccess : ResultModifyingError, errorMessage);
}

void Scheduler::handleDeleteFinished(bool success, const QString &errorMessage)
{
    Q_EMIT transactionFinished(success ? ResultSuccess : ResultDeletingError, errorMessage);
}

void Scheduler::connectCalendar(const Akonadi::CalendarBase::Ptr &calendar)
{
    connect(calendar.data(), &CalendarBase::createFinished, this, &Scheduler::handleCreateFinished, Qt::UniqueConnection);
    connect(calendar.data(), &CalendarBase::modifyFinished, this, &Scheduler::handleModifyFinished, Qt::UniqueConnection);
    connect(calendar.data(), &CalendarBase::deleteFinished, this, &Scheduler::handleDeleteFinished, Qt::UniqueConnection);
}

#include "moc_scheduler_p.cpp"
