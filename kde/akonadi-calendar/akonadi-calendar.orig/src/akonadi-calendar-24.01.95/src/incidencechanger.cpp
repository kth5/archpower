/*
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "incidencechanger.h"
#include "akonadicalendar_debug.h"
#include "calendarutils.h"
#include "incidencechanger_p.h"
#include "mailscheduler_p.h"
#include "utils_p.h"
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/TransactionSequence>

#include <KGuiItem>
#include <KJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QBitArray>

using namespace Akonadi;
using namespace KCalendarCore;

AKONADI_CALENDAR_TESTS_EXPORT bool akonadi_calendar_running_unittests = false;

static ITIPHandlerDialogDelegate::Action actionFromStatus(ITIPHandlerHelper::SendResult result)
{
    // enum SendResult {
    //      Canceled,        /**< Sending was canceled by the user, meaning there are
    //                          local changes of which other attendees are not aware. */
    //      FailKeepUpdate,  /**< Sending failed, the changes to the incidence must be kept. */
    //      FailAbortUpdate, /**< Sending failed, the changes to the incidence must be undone. */
    //      NoSendingNeeded, /**< In some cases it is not needed to send an invitation
    //                          (e.g. when we are the only attendee) */
    //      Success
    switch (result) {
    case ITIPHandlerHelper::ResultCanceled:
        return ITIPHandlerDialogDelegate::ActionDontSendMessage;
    case ITIPHandlerHelper::ResultSuccess:
        return ITIPHandlerDialogDelegate::ActionSendMessage;
    default:
        return ITIPHandlerDialogDelegate::ActionAsk;
    }
}

static bool weAreOrganizer(const Incidence::Ptr &incidence)
{
    const QString email = incidence->organizer().email();
    return Akonadi::CalendarUtils::thatIsMe(email);
}

static bool allowedModificationsWithoutRevisionUpdate(const Incidence::Ptr &incidence)
{
    // Modifications that are per user allowed without getting outofsync with organisator
    // * if only alarm settings are modified.
    const QSet<KCalendarCore::IncidenceBase::Field> dirtyFields = incidence->dirtyFields();
    QSet<KCalendarCore::IncidenceBase::Field> alarmOnlyModify;
    alarmOnlyModify << IncidenceBase::FieldAlarms << IncidenceBase::FieldLastModified;
    return dirtyFields == alarmOnlyModify;
}

static void updateHandlerPrivacyPolicy(ITIPHandlerHelper *helper, IncidenceChanger::InvitationPrivacyFlags flags)
{
    ITIPHandlerHelper::MessagePrivacyFlags helperFlags;
    helperFlags.setFlag(ITIPHandlerHelper::MessagePrivacySign, (flags & IncidenceChanger::InvitationPrivacySign) == IncidenceChanger::InvitationPrivacySign);
    helperFlags.setFlag(ITIPHandlerHelper::MessagePrivacyEncrypt,
                        (flags & IncidenceChanger::InvitationPrivacyEncrypt) == IncidenceChanger::InvitationPrivacyEncrypt);
    helper->setMessagePrivacy(helperFlags);
}

namespace Akonadi
{
// Does a queued emit, with QMetaObject::invokeMethod
static void emitCreateFinished(IncidenceChanger *changer,
                               int changeId,
                               const Akonadi::Item &item,
                               Akonadi::IncidenceChanger::ResultCode resultCode,
                               const QString &errorString)
{
    QMetaObject::invokeMethod(changer,
                              "createFinished",
                              Qt::QueuedConnection,
                              Q_ARG(int, changeId),
                              Q_ARG(Akonadi::Item, item),
                              Q_ARG(Akonadi::IncidenceChanger::ResultCode, resultCode),
                              Q_ARG(QString, errorString));
}

// Does a queued emit, with QMetaObject::invokeMethod
static void
emitModifyFinished(IncidenceChanger *changer, int changeId, const Akonadi::Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    QMetaObject::invokeMethod(changer,
                              "modifyFinished",
                              Qt::QueuedConnection,
                              Q_ARG(int, changeId),
                              Q_ARG(Akonadi::Item, item),
                              Q_ARG(Akonadi::IncidenceChanger::ResultCode, resultCode),
                              Q_ARG(QString, errorString));
}

// Does a queued emit, with QMetaObject::invokeMethod
static void emitDeleteFinished(IncidenceChanger *changer,
                               int changeId,
                               const QList<Akonadi::Item::Id> &itemIdList,
                               IncidenceChanger::ResultCode resultCode,
                               const QString &errorString)
{
    QMetaObject::invokeMethod(changer,
                              "deleteFinished",
                              Qt::QueuedConnection,
                              Q_ARG(int, changeId),
                              Q_ARG(QList<Akonadi::Item::Id>, itemIdList),
                              Q_ARG(Akonadi::IncidenceChanger::ResultCode, resultCode),
                              Q_ARG(QString, errorString));
}
}

using IdToRevisionHash = QHash<Akonadi::Item::Id, int>;
Q_GLOBAL_STATIC(IdToRevisionHash, s_latestRevisionByItemId)

IncidenceChangerPrivate::IncidenceChangerPrivate(bool enableHistory, ITIPHandlerComponentFactory *factory, IncidenceChanger *qq)
    : q(qq)
{
    mLatestChangeId = 0;
    mShowDialogsOnError = true;
    mFactory = factory ? factory : new ITIPHandlerComponentFactory(this);
    mHistory = enableHistory ? new History(this) : nullptr;
    mUseHistory = enableHistory;
    mDestinationPolicy = IncidenceChanger::DestinationPolicyDefault;
    mRespectsCollectionRights = false;
    mGroupwareCommunication = false;
    mLatestAtomicOperationId = 0;
    mBatchOperationInProgress = false;
    mAutoAdjustRecurrence = true;
    m_collectionFetchJob = nullptr;
    m_invitationPolicy = IncidenceChanger::InvitationPolicyAsk;

    qRegisterMetaType<QList<Akonadi::Item::Id>>("QList<Akonadi::Item::Id>");
    qRegisterMetaType<Akonadi::Item::Id>("Akonadi::Item::Id");
    qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    qRegisterMetaType<Akonadi::IncidenceChanger::ResultCode>("Akonadi::IncidenceChanger::ResultCode");
    qRegisterMetaType<ITIPHandlerHelper::SendResult>("ITIPHandlerHelper::SendResult");
}

IncidenceChangerPrivate::~IncidenceChangerPrivate()
{
    if (!mAtomicOperations.isEmpty() || !mQueuedModifications.isEmpty() || !mModificationsInProgress.isEmpty()) {
        qCDebug(AKONADICALENDAR_LOG) << "Normal if the application was being used. "
                                        "But might indicate a memory leak if it wasn't";
    }
}

bool IncidenceChangerPrivate::atomicOperationIsValid(uint atomicOperationId) const
{
    // Changes must be done between startAtomicOperation() and endAtomicOperation()
    return mAtomicOperations.contains(atomicOperationId) && !mAtomicOperations[atomicOperationId]->m_endCalled;
}

bool IncidenceChangerPrivate::hasRights(const Collection &collection, IncidenceChanger::ChangeType changeType) const
{
    bool result = false;
    switch (changeType) {
    case IncidenceChanger::ChangeTypeCreate:
        result = collection.rights() & Akonadi::Collection::CanCreateItem;
        break;
    case IncidenceChanger::ChangeTypeModify:
        result = collection.rights() & Akonadi::Collection::CanChangeItem;
        break;
    case IncidenceChanger::ChangeTypeDelete:
        result = collection.rights() & Akonadi::Collection::CanDeleteItem;
        break;
    default:
        Q_ASSERT_X(false, "hasRights", "invalid type");
    }

    return !collection.isValid() || !mRespectsCollectionRights || result;
}

Akonadi::Job *IncidenceChangerPrivate::parentJob(const Change::Ptr &change) const
{
    return (mBatchOperationInProgress && !change->queuedModification) ? mAtomicOperations[mLatestAtomicOperationId]->transaction() : nullptr;
}

void IncidenceChangerPrivate::queueModification(const Change::Ptr &change)
{
    // If there's already a change queued we just discard it
    // and send the newer change, which already includes
    // previous modifications
    const Akonadi::Item::Id id = change->newItem.id();
    if (mQueuedModifications.contains(id)) {
        Change::Ptr toBeDiscarded = mQueuedModifications.take(id);
        toBeDiscarded->resultCode = IncidenceChanger::ResultCodeModificationDiscarded;
        toBeDiscarded->completed = true;
        mChangeById.remove(toBeDiscarded->id);
    }

    change->queuedModification = true;
    mQueuedModifications[id] = change;
}

void IncidenceChangerPrivate::performNextModification(Akonadi::Item::Id id)
{
    mModificationsInProgress.remove(id);

    if (mQueuedModifications.contains(id)) {
        const Change::Ptr change = mQueuedModifications.take(id);
        performModification(change);
    }
}

void IncidenceChangerPrivate::handleTransactionJobResult(KJob *job)
{
    auto transaction = qobject_cast<TransactionSequence *>(job);
    Q_ASSERT(transaction);
    Q_ASSERT(mAtomicOperationByTransaction.contains(transaction));

    const uint atomicOperationId = mAtomicOperationByTransaction.take(transaction);

    Q_ASSERT(mAtomicOperations.contains(atomicOperationId));
    AtomicOperation *operation = mAtomicOperations[atomicOperationId];
    Q_ASSERT(operation);
    Q_ASSERT(operation->m_id == atomicOperationId);
    if (job->error()) {
        if (!operation->rolledback()) {
            operation->setRolledback();
        }
        qCritical() << "Transaction failed, everything was rolledback. " << job->errorString();
    } else {
        Q_ASSERT(operation->m_endCalled);
        Q_ASSERT(!operation->pendingJobs());
    }

    if (!operation->pendingJobs() && operation->m_endCalled) {
        delete mAtomicOperations.take(atomicOperationId);
        mBatchOperationInProgress = false;
    } else {
        operation->m_transactionCompleted = true;
    }
}

void IncidenceChangerPrivate::handleCreateJobResult(KJob *job)
{
    Change::Ptr change = mChangeForJob.take(job);

    const auto j = qobject_cast<const ItemCreateJob *>(job);
    Q_ASSERT(j);
    Akonadi::Item item = j->item();

    if (j->error()) {
        const QString errorString = j->errorString();
        IncidenceChanger::ResultCode resultCode = IncidenceChanger::ResultCodeJobError;
        item = change->newItem;
        qCritical() << errorString;
        if (mShowDialogsOnError) {
            KMessageBox::error(change->parentWidget, i18n("Error while trying to create calendar item. Error was: %1", errorString));
        }
        mChangeById.remove(change->id);
        change->errorString = errorString;
        change->resultCode = resultCode;
        // puff, change finally goes out of scope, and emits the incidenceCreated signal.
    } else {
        Q_ASSERT(item.isValid());
        Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
        change->newItem = item;

        if (change->useGroupwareCommunication) {
            connect(change.data(), &Change::dialogClosedAfterChange, this, &IncidenceChangerPrivate::handleCreateJobResult2);
            handleInvitationsAfterChange(change);
        } else {
            handleCreateJobResult2(change->id, ITIPHandlerHelper::ResultSuccess);
        }
    }
}

void IncidenceChangerPrivate::handleCreateJobResult2(int changeId, ITIPHandlerHelper::SendResult status)
{
    Change::Ptr change = mChangeById[changeId];
    Akonadi::Item item = change->newItem;

    mChangeById.remove(changeId);

    if (status == ITIPHandlerHelper::ResultFailAbortUpdate) {
        qCritical() << "Sending invitations failed, but did not delete the incidence";
    }

    const uint atomicOperationId = change->atomicOperationId;
    if (atomicOperationId != 0) {
        mInvitationStatusByAtomicOperation.insert(atomicOperationId, status);
    }

    QString description;
    if (change->atomicOperationId != 0) {
        AtomicOperation *a = mAtomicOperations[change->atomicOperationId];
        ++a->m_numCompletedChanges;
        change->completed = true;
        description = a->m_description;
    }

    // for user undo/redo
    if (change->recordToHistory) {
        mHistory->recordCreation(item, description, change->atomicOperationId);
    }

    change->errorString = QString();
    change->resultCode = IncidenceChanger::ResultCodeSuccess;
    // puff, change finally goes out of scope, and emits the incidenceCreated signal.
}

void IncidenceChangerPrivate::handleDeleteJobResult(KJob *job)
{
    Change::Ptr change = mChangeForJob.take(job);

    const auto j = qobject_cast<const ItemDeleteJob *>(job);
    const Item::List items = j->deletedItems();

    QSharedPointer<DeletionChange> deletionChange = change.staticCast<DeletionChange>();

    deletionChange->mItemIds.reserve(deletionChange->mItemIds.count() + items.count());
    for (const Akonadi::Item &item : items) {
        deletionChange->mItemIds.append(item.id());
    }
    QString description;
    if (change->atomicOperationId != 0) {
        AtomicOperation *a = mAtomicOperations[change->atomicOperationId];
        a->m_numCompletedChanges++;
        change->completed = true;
        description = a->m_description;
    }
    if (j->error()) {
        const QString errorString = j->errorString();
        qCritical() << errorString;

        if (mShowDialogsOnError) {
            KMessageBox::error(change->parentWidget, i18n("Error while trying to delete calendar item. Error was: %1", errorString));
        }

        for (const Item &item : items) {
            // Weren't deleted due to error
            mDeletedItemIds.remove(mDeletedItemIds.indexOf(item.id()));
        }
        mChangeById.remove(change->id);
        change->resultCode = IncidenceChanger::ResultCodeJobError;
        change->errorString = errorString;
        change->emitCompletionSignal();
    } else { // success
        if (change->recordToHistory) {
            Q_ASSERT(mHistory);
            mHistory->recordDeletions(items, description, change->atomicOperationId);
        }

        if (change->useGroupwareCommunication) {
            connect(change.data(), &Change::dialogClosedAfterChange, this, &IncidenceChangerPrivate::handleDeleteJobResult2);
            handleInvitationsAfterChange(change);
        } else {
            handleDeleteJobResult2(change->id, ITIPHandlerHelper::ResultSuccess);
        }
    }
}

void IncidenceChangerPrivate::handleDeleteJobResult2(int changeId, ITIPHandlerHelper::SendResult status)
{
    Change::Ptr change = mChangeById[changeId];
    mChangeById.remove(change->id);

    if (status == ITIPHandlerHelper::ResultSuccess) {
        change->errorString = QString();
        change->resultCode = IncidenceChanger::ResultCodeSuccess;
    } else {
        change->errorString = i18nc("errormessage for a job ended with an unexpected result", "An unknown error occurred");
        change->resultCode = IncidenceChanger::ResultCodeJobError;
    }

    // puff, change finally goes out of scope, and emits the incidenceDeleted signal.
}

void IncidenceChangerPrivate::handleModifyJobResult(KJob *job)
{
    Change::Ptr change = mChangeForJob.take(job);

    const auto j = qobject_cast<const ItemModifyJob *>(job);
    const Item item = j->item();
    Q_ASSERT(mDirtyFieldsByJob.contains(job));
    Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
    const QSet<KCalendarCore::IncidenceBase::Field> dirtyFields = mDirtyFieldsByJob.value(job);
    item.payload<KCalendarCore::Incidence::Ptr>()->setDirtyFields(dirtyFields);
    QString description;
    if (change->atomicOperationId != 0) {
        AtomicOperation *a = mAtomicOperations[change->atomicOperationId];
        a->m_numCompletedChanges++;
        change->completed = true;
        description = a->m_description;
    }
    if (j->error()) {
        const QString errorString = j->errorString();
        IncidenceChanger::ResultCode resultCode = IncidenceChanger::ResultCodeJobError;
        if (deleteAlreadyCalled(item.id())) {
            // User deleted the item almost at the same time he changed it. We could just return success
            // but the delete is probably already recorded to History, and that would make undo not work
            // in the proper order.
            resultCode = IncidenceChanger::ResultCodeAlreadyDeleted;
            qCWarning(AKONADICALENDAR_LOG) << "Trying to change item " << item.id() << " while deletion is in progress.";
        } else {
            qCritical() << errorString;
        }
        if (mShowDialogsOnError) {
            KMessageBox::error(change->parentWidget, i18n("Error while trying to modify calendar item. Error was: %1", errorString));
        }
        mChangeById.remove(change->id);
        change->errorString = errorString;
        change->resultCode = resultCode;
        // puff, change finally goes out of scope, and emits the incidenceModified signal.

        QMetaObject::invokeMethod(this, "performNextModification", Qt::QueuedConnection, Q_ARG(Akonadi::Item::Id, item.id()));
    } else { // success
        (*(s_latestRevisionByItemId()))[item.id()] = item.revision();
        change->newItem = item;
        if (change->recordToHistory && !change->originalItems.isEmpty()) {
            Q_ASSERT(change->originalItems.count() == 1);
            mHistory->recordModification(change->originalItems.constFirst(), item, description, change->atomicOperationId);
        }

        if (change->useGroupwareCommunication) {
            connect(change.data(), &Change::dialogClosedAfterChange, this, &IncidenceChangerPrivate::handleModifyJobResult2);
            handleInvitationsAfterChange(change);
        } else {
            handleModifyJobResult2(change->id, ITIPHandlerHelper::ResultSuccess);
        }
    }
}

void IncidenceChangerPrivate::handleModifyJobResult2(int changeId, ITIPHandlerHelper::SendResult status)
{
    Change::Ptr change = mChangeById[changeId];

    mChangeById.remove(changeId);
    if (change->atomicOperationId != 0) {
        mInvitationStatusByAtomicOperation.insert(change->atomicOperationId, status);
    }
    change->errorString = QString();
    change->resultCode = IncidenceChanger::ResultCodeSuccess;
    // puff, change finally goes out of scope, and emits the incidenceModified signal.

    QMetaObject::invokeMethod(this, "performNextModification", Qt::QueuedConnection, Q_ARG(Akonadi::Item::Id, change->newItem.id()));
}

bool IncidenceChangerPrivate::deleteAlreadyCalled(Akonadi::Item::Id id) const
{
    return mDeletedItemIds.contains(id);
}

void IncidenceChangerPrivate::handleInvitationsBeforeChange(const Change::Ptr &change)
{
    if (mGroupwareCommunication) {
        ITIPHandlerHelper::SendResult result = ITIPHandlerHelper::ResultSuccess;
        switch (change->type) {
        case IncidenceChanger::ChangeTypeCreate:
            // nothing needs to be done
            break;
        case IncidenceChanger::ChangeTypeDelete: {
            ITIPHandlerHelper::SendResult status;
            bool sendOk = true;
            Q_ASSERT(!change->originalItems.isEmpty());

            auto handler = new ITIPHandlerHelper(mFactory, change->parentWidget);
            handler->setParent(this);
            updateHandlerPrivacyPolicy(handler, m_invitationPrivacy);

            if (m_invitationPolicy == IncidenceChanger::InvitationPolicySend) {
                handler->setDefaultAction(ITIPHandlerDialogDelegate::ActionSendMessage);
            } else if (m_invitationPolicy == IncidenceChanger::InvitationPolicyDontSend) {
                handler->setDefaultAction(ITIPHandlerDialogDelegate::ActionDontSendMessage);
            } else if (mInvitationStatusByAtomicOperation.contains(change->atomicOperationId)) {
                handler->setDefaultAction(actionFromStatus(mInvitationStatusByAtomicOperation.value(change->atomicOperationId)));
            }

            connect(handler, &ITIPHandlerHelper::finished, change.data(), &Change::emitUserDialogClosedBeforeChange);

            for (const Akonadi::Item &item : std::as_const(change->originalItems)) {
                Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
                Incidence::Ptr incidence = CalendarUtils::incidence(item);
                if (!incidence->supportsGroupwareCommunication()) {
                    continue;
                }
                // We only send CANCEL if we're the organizer.
                // If we're not, then we send REPLY with PartStat=Declined in handleInvitationsAfterChange()
                if (Akonadi::CalendarUtils::thatIsMe(incidence->organizer().email())) {
                    // TODO: not to popup all delete message dialogs at once :(
                    sendOk = false;
                    handler->sendIncidenceDeletedMessage(KCalendarCore::iTIPCancel, incidence);
                    if (change->atomicOperationId) {
                        mInvitationStatusByAtomicOperation.insert(change->atomicOperationId, status);
                    }
                    // TODO: with some status we want to break immediately
                }
            }

            if (sendOk) {
                change->emitUserDialogClosedBeforeChange(result);
            }
            return;
        }
        case IncidenceChanger::ChangeTypeModify: {
            if (change->originalItems.isEmpty()) {
                break;
            }

            Q_ASSERT(change->originalItems.count() == 1);
            Incidence::Ptr oldIncidence = CalendarUtils::incidence(change->originalItems.first());
            Incidence::Ptr newIncidence = CalendarUtils::incidence(change->newItem);

            if (!oldIncidence->supportsGroupwareCommunication()) {
                break;
            }

            if (allowedModificationsWithoutRevisionUpdate(newIncidence)) {
                change->emitUserDialogClosedBeforeChange(ITIPHandlerHelper::ResultSuccess);
                return;
            }

            if (akonadi_calendar_running_unittests && !weAreOrganizer(newIncidence)) {
                // This is a bit of a workaround when running tests. I don't want to show the
                // "You're not organizer, do you want to modify event?" dialog in unit-tests, but want
                // to emulate a "yes" and a "no" press.
                if (m_invitationPolicy == IncidenceChanger::InvitationPolicySend) {
                    change->emitUserDialogClosedBeforeChange(ITIPHandlerHelper::ResultSuccess);
                    return;
                } else if (m_invitationPolicy == IncidenceChanger::InvitationPolicyDontSend) {
                    change->emitUserDialogClosedBeforeChange(ITIPHandlerHelper::ResultCanceled);
                    return;
                }
            }

            ITIPHandlerHelper handler(mFactory, change->parentWidget);
            const bool modify = handler.handleIncidenceAboutToBeModified(newIncidence);
            if (modify) {
                break;
            } else {
                result = ITIPHandlerHelper::ResultCanceled;
            }

            if (newIncidence->type() == oldIncidence->type()) {
                IncidenceBase *i1 = newIncidence.data();
                IncidenceBase *i2 = oldIncidence.data();
                *i1 = *i2;
            }
            break;
        }
        default:
            Q_ASSERT(false);
            result = ITIPHandlerHelper::ResultCanceled;
        }
        change->emitUserDialogClosedBeforeChange(result);
    } else {
        change->emitUserDialogClosedBeforeChange(ITIPHandlerHelper::ResultSuccess);
    }
}

void IncidenceChangerPrivate::handleInvitationsAfterChange(const Change::Ptr &change)
{
    if (change->useGroupwareCommunication) {
        auto handler = new ITIPHandlerHelper(mFactory, change->parentWidget);
        connect(handler, &ITIPHandlerHelper::finished, change.data(), &Change::emitUserDialogClosedAfterChange);
        handler->setParent(this);
        updateHandlerPrivacyPolicy(handler, m_invitationPrivacy);

        const bool alwaysSend = (m_invitationPolicy == IncidenceChanger::InvitationPolicySend);
        const bool neverSend = (m_invitationPolicy == IncidenceChanger::InvitationPolicyDontSend);
        if (alwaysSend) {
            handler->setDefaultAction(ITIPHandlerDialogDelegate::ActionSendMessage);
        }

        if (neverSend) {
            handler->setDefaultAction(ITIPHandlerDialogDelegate::ActionDontSendMessage);
        }

        switch (change->type) {
        case IncidenceChanger::ChangeTypeCreate: {
            Incidence::Ptr incidence = CalendarUtils::incidence(change->newItem);
            if (incidence->supportsGroupwareCommunication()) {
                handler->sendIncidenceCreatedMessage(KCalendarCore::iTIPRequest, incidence);
                return;
            }
            break;
        }
        case IncidenceChanger::ChangeTypeDelete:
            handler->deleteLater();
            handler = nullptr;
            Q_ASSERT(!change->originalItems.isEmpty());
            for (const Akonadi::Item &item : std::as_const(change->originalItems)) {
                Q_ASSERT(item.hasPayload());
                Incidence::Ptr incidence = CalendarUtils::incidence(item);
                Q_ASSERT(incidence);
                if (!incidence->supportsGroupwareCommunication()) {
                    continue;
                }

                if (!Akonadi::CalendarUtils::thatIsMe(incidence->organizer().email())) {
                    const QStringList myEmails = Akonadi::CalendarUtils::allEmails();
                    bool notifyOrganizer = false;
                    const KCalendarCore::Attendee me(incidence->attendeeByMails(myEmails));
                    if (!me.isNull()) {
                        if (me.status() == KCalendarCore::Attendee::Accepted || me.status() == KCalendarCore::Attendee::Delegated) {
                            notifyOrganizer = true;
                        }
                        KCalendarCore::Attendee newMe(me);
                        newMe.setStatus(KCalendarCore::Attendee::Declined);
                        incidence->clearAttendees();
                        incidence->addAttendee(newMe);
                        // break;
                    }

                    if (notifyOrganizer) {
                        MailScheduler scheduler(mFactory, change->parentWidget); // TODO make async
                        scheduler.performTransaction(incidence, KCalendarCore::iTIPReply);
                    }
                }
            }
            break;
        case IncidenceChanger::ChangeTypeModify: {
            if (change->originalItems.isEmpty()) {
                break;
            }

            Q_ASSERT(change->originalItems.count() == 1);
            Incidence::Ptr oldIncidence = CalendarUtils::incidence(change->originalItems.first());
            Incidence::Ptr newIncidence = CalendarUtils::incidence(change->newItem);

            if (!newIncidence->supportsGroupwareCommunication() || !Akonadi::CalendarUtils::thatIsMe(newIncidence->organizer().email())) {
                // If we're not the organizer, the user already saw the "Do you really want to do this, incidence will become out of sync"
                break;
            }

            if (allowedModificationsWithoutRevisionUpdate(newIncidence)) {
                break;
            }

            if (!neverSend && !alwaysSend && mInvitationStatusByAtomicOperation.contains(change->atomicOperationId)) {
                handler->setDefaultAction(actionFromStatus(mInvitationStatusByAtomicOperation.value(change->atomicOperationId)));
            }

            const bool attendeeStatusChanged = myAttendeeStatusChanged(newIncidence, oldIncidence, Akonadi::CalendarUtils::allEmails());

            handler->sendIncidenceModifiedMessage(KCalendarCore::iTIPRequest, newIncidence, attendeeStatusChanged);
            return;
        }
        default:
            handler->deleteLater();
            handler = nullptr;
            Q_ASSERT(false);
            change->emitUserDialogClosedAfterChange(ITIPHandlerHelper::ResultCanceled);
            return;
        }
        handler->deleteLater();
        handler = nullptr;
        change->emitUserDialogClosedAfterChange(ITIPHandlerHelper::ResultSuccess);
    } else {
        change->emitUserDialogClosedAfterChange(ITIPHandlerHelper::ResultSuccess);
    }
}

/** static */
bool IncidenceChangerPrivate::myAttendeeStatusChanged(const Incidence::Ptr &newInc, const Incidence::Ptr &oldInc, const QStringList &myEmails)
{
    Q_ASSERT(newInc);
    Q_ASSERT(oldInc);
    const Attendee oldMe = oldInc->attendeeByMails(myEmails);
    const Attendee newMe = newInc->attendeeByMails(myEmails);

    return !oldMe.isNull() && !newMe.isNull() && oldMe.status() != newMe.status();
}

IncidenceChanger::IncidenceChanger(QObject *parent)
    : QObject(parent)
    , d(new IncidenceChangerPrivate(/**history=*/true, /*factory=*/nullptr, this))
{
}

IncidenceChanger::IncidenceChanger(ITIPHandlerComponentFactory *factory, QObject *parent)
    : QObject(parent)
    , d(new IncidenceChangerPrivate(/**history=*/true, factory, this))
{
}

IncidenceChanger::IncidenceChanger(bool enableHistory, QObject *parent)
    : QObject(parent)
    , d(new IncidenceChangerPrivate(enableHistory, /*factory=*/nullptr, this))
{
}

IncidenceChanger::~IncidenceChanger() = default;

int IncidenceChanger::createFromItem(const Item &item, const Collection &collection, QWidget *parent)
{
    const uint atomicOperationId = d->mBatchOperationInProgress ? d->mLatestAtomicOperationId : 0;

    const Change::Ptr change(new CreationChange(this, ++d->mLatestChangeId, atomicOperationId, parent));
    const int changeId = change->id;
    Q_ASSERT(!(d->mBatchOperationInProgress && !d->mAtomicOperations.contains(atomicOperationId)));
    if (d->mBatchOperationInProgress && d->mAtomicOperations[atomicOperationId]->rolledback()) {
        const QString errorMessage = d->showErrorDialog(ResultCodeRolledback, parent);
        qCWarning(AKONADICALENDAR_LOG) << errorMessage;

        change->resultCode = ResultCodeRolledback;
        change->errorString = errorMessage;
        d->cleanupTransaction();
        return changeId;
    }

    change->newItem = item;

    d->step1DetermineDestinationCollection(change, collection);

    return change->id;
}

int IncidenceChanger::createIncidence(const Incidence::Ptr &incidence, const Collection &collection, QWidget *parent)
{
    if (!incidence) {
        qCWarning(AKONADICALENDAR_LOG) << "An invalid payload is not allowed.";
        d->cancelTransaction();
        return -1;
    }

    Item item;
    item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
    item.setMimeType(incidence->mimeType());

    return createFromItem(item, collection, parent);
}

int IncidenceChanger::deleteIncidence(const Item &item, QWidget *parent)
{
    Item::List list;
    list.append(item);

    return deleteIncidences(list, parent);
}

int IncidenceChanger::deleteIncidences(const Item::List &items, QWidget *parent)
{
    if (items.isEmpty()) {
        qCritical() << "Delete what?";
        d->cancelTransaction();
        return -1;
    }

    for (const Item &item : items) {
        if (!item.isValid()) {
            qCritical() << "Items must be valid!";
            d->cancelTransaction();
            return -1;
        }
    }

    const uint atomicOperationId = d->mBatchOperationInProgress ? d->mLatestAtomicOperationId : 0;
    const int changeId = ++d->mLatestChangeId;
    const Change::Ptr change(new DeletionChange(this, changeId, atomicOperationId, parent));

    for (const Item &item : items) {
        if (!d->hasRights(item.parentCollection(), ChangeTypeDelete)) {
            qCWarning(AKONADICALENDAR_LOG) << "Item " << item.id() << " can't be deleted due to ACL restrictions";
            const QString errorString = d->showErrorDialog(ResultCodePermissions, parent);
            change->resultCode = ResultCodePermissions;
            change->errorString = errorString;
            d->cancelTransaction();
            return changeId;
        }
    }

    if (!d->allowAtomicOperation(atomicOperationId, change)) {
        const QString errorString = d->showErrorDialog(ResultCodeDuplicateId, parent);
        change->resultCode = ResultCodeDuplicateId;
        change->errorString = errorString;
        qCWarning(AKONADICALENDAR_LOG) << errorString;
        d->cancelTransaction();
        return changeId;
    }

    Item::List itemsToDelete;
    for (const Item &item : items) {
        if (d->deleteAlreadyCalled(item.id())) {
            // IncidenceChanger::deleteIncidence() called twice, ignore this one.
            qCDebug(AKONADICALENDAR_LOG) << "Item " << item.id() << " already deleted or being deleted, skipping";
        } else {
            itemsToDelete.append(item);
        }
    }

    if (d->mBatchOperationInProgress && d->mAtomicOperations[atomicOperationId]->rolledback()) {
        const QString errorMessage = d->showErrorDialog(ResultCodeRolledback, parent);
        change->resultCode = ResultCodeRolledback;
        change->errorString = errorMessage;
        qCritical() << errorMessage;
        d->cleanupTransaction();
        return changeId;
    }

    if (itemsToDelete.isEmpty()) {
        QList<Akonadi::Item::Id> itemIdList;
        itemIdList.append(Item().id());
        qCDebug(AKONADICALENDAR_LOG) << "Items already deleted or being deleted, skipping";
        const QString errorMessage = i18n("That calendar item was already deleted, or currently being deleted.");
        // Queued emit because return must be executed first, otherwise caller won't know this workId
        change->resultCode = ResultCodeAlreadyDeleted;
        change->errorString = errorMessage;
        d->cancelTransaction();
        qCWarning(AKONADICALENDAR_LOG) << errorMessage;
        return changeId;
    }
    change->originalItems = itemsToDelete;

    d->mChangeById.insert(changeId, change);

    if (d->mGroupwareCommunication) {
        connect(change.data(), &Change::dialogClosedBeforeChange, d.get(), &IncidenceChangerPrivate::deleteIncidences2);
        d->handleInvitationsBeforeChange(change);
    } else {
        d->deleteIncidences2(changeId, ITIPHandlerHelper::ResultSuccess);
    }
    return changeId;
}

void IncidenceChangerPrivate::deleteIncidences2(int changeId, ITIPHandlerHelper::SendResult status)
{
    Q_UNUSED(status)
    Change::Ptr change = mChangeById[changeId];
    const uint atomicOperationId = change->atomicOperationId;
    auto deleteJob = new ItemDeleteJob(change->originalItems, parentJob(change));
    mChangeForJob.insert(deleteJob, change);

    if (mBatchOperationInProgress) {
        AtomicOperation *atomic = mAtomicOperations[atomicOperationId];
        Q_ASSERT(atomic);
        atomic->addChange(change);
    }

    mDeletedItemIds.reserve(mDeletedItemIds.count() + change->originalItems.count());
    for (const Item &item : std::as_const(change->originalItems)) {
        mDeletedItemIds << item.id();
    }

    // Do some cleanup
    if (mDeletedItemIds.count() > 100) {
        mDeletedItemIds.remove(0, 50);
    }

    // QueuedConnection because of possible sync exec calls.
    connect(deleteJob, &KJob::result, this, &IncidenceChangerPrivate::handleDeleteJobResult, Qt::QueuedConnection);
}

int IncidenceChanger::modifyIncidence(const Item &changedItem, const KCalendarCore::Incidence::Ptr &originalPayload, QWidget *parent)
{
    if (!changedItem.isValid() || !changedItem.hasPayload<Incidence::Ptr>()) {
        qCWarning(AKONADICALENDAR_LOG) << "An invalid item or payload is not allowed.";
        d->cancelTransaction();
        return -1;
    }

    if (!d->hasRights(changedItem.parentCollection(), ChangeTypeModify)) {
        qCWarning(AKONADICALENDAR_LOG) << "Item " << changedItem.id() << " can't be deleted due to ACL restrictions";
        const int changeId = ++d->mLatestChangeId;
        const QString errorString = d->showErrorDialog(ResultCodePermissions, parent);
        emitModifyFinished(this, changeId, changedItem, ResultCodePermissions, errorString);
        d->cancelTransaction();
        return changeId;
    }

    // TODO also update revision here instead of in the editor
    changedItem.payload<Incidence::Ptr>()->setLastModified(QDateTime::currentDateTimeUtc());

    const uint atomicOperationId = d->mBatchOperationInProgress ? d->mLatestAtomicOperationId : 0;
    const int changeId = ++d->mLatestChangeId;
    auto modificationChange = new ModificationChange(this, changeId, atomicOperationId, parent);
    Change::Ptr change(modificationChange);

    if (originalPayload) {
        Item originalItem(changedItem);
        originalItem.setPayload<KCalendarCore::Incidence::Ptr>(originalPayload);
        modificationChange->originalItems << originalItem;
    }

    modificationChange->newItem = changedItem;
    d->mChangeById.insert(changeId, change);

    if (!d->allowAtomicOperation(atomicOperationId, change)) {
        const QString errorString = d->showErrorDialog(ResultCodeDuplicateId, parent);

        change->resultCode = ResultCodeDuplicateId;
        change->errorString = errorString;
        d->cancelTransaction();
        qCWarning(AKONADICALENDAR_LOG) << "Atomic operation now allowed";
        return changeId;
    }

    if (d->mBatchOperationInProgress && d->mAtomicOperations[atomicOperationId]->rolledback()) {
        const QString errorMessage = d->showErrorDialog(ResultCodeRolledback, parent);
        qCritical() << errorMessage;
        d->cleanupTransaction();
        emitModifyFinished(this, changeId, changedItem, ResultCodeRolledback, errorMessage);
    } else {
        d->adjustRecurrence(originalPayload, CalendarUtils::incidence(modificationChange->newItem));
        d->performModification(change);
    }

    return changeId;
}

void IncidenceChangerPrivate::performModification(const Change::Ptr &change)
{
    const Item::Id id = change->newItem.id();
    Akonadi::Item &newItem = change->newItem;
    Q_ASSERT(newItem.isValid());
    Q_ASSERT(newItem.hasPayload<Incidence::Ptr>());

    const int changeId = change->id;

    if (deleteAlreadyCalled(id)) {
        // IncidenceChanger::deleteIncidence() called twice, ignore this one.
        qCDebug(AKONADICALENDAR_LOG) << "Item " << id << " already deleted or being deleted, skipping";

        // Queued emit because return must be executed first, otherwise caller won't know this workId
        emitModifyFinished(q,
                           change->id,
                           newItem,
                           IncidenceChanger::ResultCodeAlreadyDeleted,
                           i18n("That calendar item was already deleted, or currently being deleted."));
        return;
    }

    const uint atomicOperationId = change->atomicOperationId;
    const bool hasAtomicOperationId = atomicOperationId != 0;
    if (hasAtomicOperationId && mAtomicOperations[atomicOperationId]->rolledback()) {
        const QString errorMessage = showErrorDialog(IncidenceChanger::ResultCodeRolledback, nullptr);
        qCritical() << errorMessage;
        emitModifyFinished(q, changeId, newItem, IncidenceChanger::ResultCodeRolledback, errorMessage);
        return;
    }
    if (mGroupwareCommunication) {
        connect(change.data(), &Change::dialogClosedBeforeChange, this, &IncidenceChangerPrivate::performModification2);
        handleInvitationsBeforeChange(change);
    } else {
        performModification2(change->id, ITIPHandlerHelper::ResultSuccess);
    }
}

void IncidenceChangerPrivate::performModification2(int changeId, ITIPHandlerHelper::SendResult status)
{
    Change::Ptr change = mChangeById[changeId];
    const Item::Id id = change->newItem.id();
    Akonadi::Item &newItem = change->newItem;
    Q_ASSERT(newItem.isValid());
    Q_ASSERT(newItem.hasPayload<Incidence::Ptr>());
    if (status == ITIPHandlerHelper::ResultCanceled) { // TODO:fireout what is right here:)
        // User got a "You're not the organizer, do you really want to send" dialog, and said "no"
        qCDebug(AKONADICALENDAR_LOG) << "User cancelled, giving up";
        emitModifyFinished(q, change->id, newItem, IncidenceChanger::ResultCodeUserCanceled, QString());
        return;
    }

    const uint atomicOperationId = change->atomicOperationId;
    const bool hasAtomicOperationId = atomicOperationId != 0;

    QHash<Akonadi::Item::Id, int> &latestRevisionByItemId = *(s_latestRevisionByItemId());
    if (latestRevisionByItemId.contains(id) && latestRevisionByItemId[id] > newItem.revision()) {
        /* When a ItemModifyJob ends, the application can still modify the old items if the user
         * is quick because the ETM wasn't updated yet, and we'll get a STORE error, because
         * we are not modifying the latest revision.
         *
         * When a job ends, we keep the new revision in s_latestRevisionByItemId
         * so we can update the item's revision
         */
        newItem.setRevision(latestRevisionByItemId[id]);
    }

    Incidence::Ptr incidence = CalendarUtils::incidence(newItem);
    {
        if (!allowedModificationsWithoutRevisionUpdate(incidence)) { // increment revision ( KCalendarCore revision, not akonadi )
            const int revision = incidence->revision();
            incidence->setRevision(revision + 1);
        }

        // Reset attendee status, when resceduling
        QSet<IncidenceBase::Field> resetPartStatus;
        resetPartStatus << IncidenceBase::FieldDtStart << IncidenceBase::FieldDtEnd << IncidenceBase::FieldDtStart << IncidenceBase::FieldLocation
                        << IncidenceBase::FieldDtDue << IncidenceBase::FieldDuration << IncidenceBase::FieldRecurrence;
        if (!(incidence->dirtyFields() & resetPartStatus).isEmpty() && weAreOrganizer(incidence)) {
            auto attendees = incidence->attendees();
            for (auto &attendee : attendees) {
                if (attendee.role() != Attendee::NonParticipant && attendee.status() != Attendee::Delegated && !Akonadi::CalendarUtils::thatIsMe(attendee)) {
                    attendee.setStatus(Attendee::NeedsAction);
                    attendee.setRSVP(true);
                }
            }
            incidence->setAttendees(attendees);
        }
    }

    // Dav Fix
    // Don't write back remote revision since we can't make sure it is the current one
    newItem.setRemoteRevision(QString());

    if (mModificationsInProgress.contains(newItem.id())) {
        // There's already a ItemModifyJob running for this item ID
        // Let's wait for it to end.
        queueModification(change);
    } else {
        auto modifyJob = new ItemModifyJob(newItem, parentJob(change));
        mChangeForJob.insert(modifyJob, change);
        mDirtyFieldsByJob.insert(modifyJob, incidence->dirtyFields());

        if (hasAtomicOperationId) {
            AtomicOperation *atomic = mAtomicOperations[atomicOperationId];
            Q_ASSERT(atomic);
            atomic->addChange(change);
        }

        mModificationsInProgress[newItem.id()] = change;
        // QueuedConnection because of possible sync exec calls.
        connect(modifyJob, &KJob::result, this, &IncidenceChangerPrivate::handleModifyJobResult, Qt::QueuedConnection);
    }
}

void IncidenceChanger::startAtomicOperation(const QString &operationDescription)
{
    if (d->mBatchOperationInProgress) {
        qCDebug(AKONADICALENDAR_LOG) << "An atomic operation is already in progress.";
        return;
    }

    ++d->mLatestAtomicOperationId;
    d->mBatchOperationInProgress = true;

    auto atomicOperation = new AtomicOperation(d.get(), d->mLatestAtomicOperationId);
    atomicOperation->m_description = operationDescription;
    d->mAtomicOperations.insert(d->mLatestAtomicOperationId, atomicOperation);
}

void IncidenceChanger::endAtomicOperation()
{
    if (!d->mBatchOperationInProgress) {
        qCDebug(AKONADICALENDAR_LOG) << "No atomic operation is in progress.";
        return;
    }

    Q_ASSERT_X(d->mLatestAtomicOperationId != 0, "IncidenceChanger::endAtomicOperation()", "Call startAtomicOperation() first.");

    Q_ASSERT(d->mAtomicOperations.contains(d->mLatestAtomicOperationId));
    AtomicOperation *atomicOperation = d->mAtomicOperations[d->mLatestAtomicOperationId];
    Q_ASSERT(atomicOperation);
    atomicOperation->m_endCalled = true;

    const bool allJobsCompleted = !atomicOperation->pendingJobs();

    if (allJobsCompleted && atomicOperation->rolledback() && atomicOperation->m_transactionCompleted) {
        // The transaction job already completed, we can cleanup:
        delete d->mAtomicOperations.take(d->mLatestAtomicOperationId);
        d->mBatchOperationInProgress = false;
    } /* else if ( allJobsCompleted ) {
     Q_ASSERT( atomicOperation->transaction );
     atomicOperation->transaction->commit(); we using autocommit now
   }*/
}

void IncidenceChanger::setShowDialogsOnError(bool enable)
{
    d->mShowDialogsOnError = enable;
    if (d->mHistory) {
        d->mHistory->incidenceChanger()->setShowDialogsOnError(enable);
    }
}

bool IncidenceChanger::showDialogsOnError() const
{
    return d->mShowDialogsOnError;
}

void IncidenceChanger::setRespectsCollectionRights(bool respects)
{
    d->mRespectsCollectionRights = respects;
}

bool IncidenceChanger::respectsCollectionRights() const
{
    return d->mRespectsCollectionRights;
}

void IncidenceChanger::setDestinationPolicy(IncidenceChanger::DestinationPolicy destinationPolicy)
{
    d->mDestinationPolicy = destinationPolicy;
}

IncidenceChanger::DestinationPolicy IncidenceChanger::destinationPolicy() const
{
    return d->mDestinationPolicy;
}

void IncidenceChanger::setEntityTreeModel(Akonadi::EntityTreeModel *entityTreeModel)
{
    d->mEntityTreeModel = entityTreeModel;
}

Akonadi::EntityTreeModel *IncidenceChanger::entityTreeModel() const
{
    return d->mEntityTreeModel;
}

void IncidenceChanger::setDefaultCollection(const Akonadi::Collection &collection)
{
    d->mDefaultCollection = collection;
}

Collection IncidenceChanger::defaultCollection() const
{
    return d->mDefaultCollection;
}

bool IncidenceChanger::historyEnabled() const
{
    return d->mUseHistory;
}

void IncidenceChanger::setHistoryEnabled(bool enable)
{
    if (d->mUseHistory != enable) {
        d->mUseHistory = enable;
        if (enable && !d->mHistory) {
            d->mHistory = new History(d.get());
        }
    }
}

History *IncidenceChanger::history() const
{
    return d->mHistory;
}

bool IncidenceChanger::deletedRecently(Akonadi::Item::Id id) const
{
    return d->deleteAlreadyCalled(id);
}

void IncidenceChanger::setGroupwareCommunication(bool enabled)
{
    d->mGroupwareCommunication = enabled;
}

bool IncidenceChanger::groupwareCommunication() const
{
    return d->mGroupwareCommunication;
}

void IncidenceChanger::setAutoAdjustRecurrence(bool enable)
{
    d->mAutoAdjustRecurrence = enable;
}

bool IncidenceChanger::autoAdjustRecurrence() const
{
    return d->mAutoAdjustRecurrence;
}

void IncidenceChanger::setInvitationPolicy(IncidenceChanger::InvitationPolicy policy)
{
    d->m_invitationPolicy = policy;
}

IncidenceChanger::InvitationPolicy IncidenceChanger::invitationPolicy() const
{
    return d->m_invitationPolicy;
}

Akonadi::Collection IncidenceChanger::lastCollectionUsed() const
{
    return d->mLastCollectionUsed;
}

void IncidenceChanger::setInvitationPrivacy(IncidenceChanger::InvitationPrivacyFlags invitationPrivacy)
{
    d->m_invitationPrivacy = invitationPrivacy;
}

IncidenceChanger::InvitationPrivacyFlags IncidenceChanger::invitationPrivacy() const
{
    return d->m_invitationPrivacy;
}

QString IncidenceChangerPrivate::showErrorDialog(IncidenceChanger::ResultCode resultCode, QWidget *parent)
{
    QString errorString;
    switch (resultCode) {
    case IncidenceChanger::ResultCodePermissions:
        errorString = i18n("Operation can not be performed due to ACL restrictions");
        break;
    case IncidenceChanger::ResultCodeInvalidUserCollection:
        errorString = i18n("The chosen collection is invalid");
        break;
    case IncidenceChanger::ResultCodeInvalidDefaultCollection:
        errorString = i18n(
            "Default collection is invalid or doesn't have proper ACLs"
            " and DestinationPolicyNeverAsk was used");
        break;
    case IncidenceChanger::ResultCodeDuplicateId:
        errorString = i18n("Duplicate item id in a group operation");
        break;
    case IncidenceChanger::ResultCodeRolledback:
        errorString = i18n(
            "One change belonging to a group of changes failed. "
            "All changes are being rolled back.");
        break;
    default:
        Q_ASSERT(false);
        return QString(i18n("Unknown error"));
    }

    if (mShowDialogsOnError) {
        KMessageBox::error(parent, errorString);
    }

    return errorString;
}

void IncidenceChangerPrivate::adjustRecurrence(const KCalendarCore::Incidence::Ptr &originalIncidence, const KCalendarCore::Incidence::Ptr &incidence)
{
    if (!originalIncidence || !incidence->recurs() || incidence->hasRecurrenceId() || !mAutoAdjustRecurrence
        || !incidence->dirtyFields().contains(KCalendarCore::Incidence::FieldDtStart)) {
        return;
    }

    const QDate originalDate = originalIncidence->dtStart().date();
    const QDate newStartDate = incidence->dtStart().date();

    if (!originalDate.isValid() || !newStartDate.isValid() || originalDate == newStartDate) {
        return;
    }

    KCalendarCore::Recurrence *recurrence = incidence->recurrence();
    switch (recurrence->recurrenceType()) {
    case KCalendarCore::Recurrence::rWeekly: {
        QBitArray days = recurrence->days();
        const int oldIndex = originalDate.dayOfWeek() - 1; // QDate returns [1-7];
        const int newIndex = newStartDate.dayOfWeek() - 1;
        if (oldIndex != newIndex) {
            days.clearBit(oldIndex);
            days.setBit(newIndex);
            recurrence->setWeekly(recurrence->frequency(), days);
        }
    }
    default:
        break; // Other types not implemented
    }

    // Now fix cases where dtstart would be bigger than the recurrence end rendering it impossible for a view to show it:
    // To retrieve the recurrence end don't trust Recurrence::endDt() since it returns dtStart if the rrule's end is < than dtstart,
    // it seems someone made Recurrence::endDt() more robust, but getNextOccurrences() still craps out. So lets fix it here
    // there's no reason to write bogus ical to disk.
    const QDate recurrenceEndDate = recurrence->defaultRRule() ? recurrence->defaultRRule()->endDt().date() : QDate();
    if (recurrenceEndDate.isValid() && recurrenceEndDate < newStartDate) {
        recurrence->setEndDate(newStartDate);
    }
}

void IncidenceChangerPrivate::cancelTransaction()
{
    if (mBatchOperationInProgress) {
        mAtomicOperations[mLatestAtomicOperationId]->setRolledback();
    }
}

void IncidenceChangerPrivate::cleanupTransaction()
{
    Q_ASSERT(mAtomicOperations.contains(mLatestAtomicOperationId));
    AtomicOperation *operation = mAtomicOperations[mLatestAtomicOperationId];
    Q_ASSERT(operation);
    Q_ASSERT(operation->rolledback());
    if (!operation->pendingJobs() && operation->m_endCalled && operation->m_transactionCompleted) {
        delete mAtomicOperations.take(mLatestAtomicOperationId);
        mBatchOperationInProgress = false;
    }
}

bool IncidenceChangerPrivate::allowAtomicOperation(int atomicOperationId, const Change::Ptr &change) const
{
    bool allow = true;
    if (atomicOperationId > 0) {
        Q_ASSERT(mAtomicOperations.contains(atomicOperationId));
        AtomicOperation *operation = mAtomicOperations.value(atomicOperationId);

        if (change->type == IncidenceChanger::ChangeTypeCreate) {
            allow = true;
        } else if (change->type == IncidenceChanger::ChangeTypeModify) {
            allow = !operation->m_itemIdsInOperation.contains(change->newItem.id());
        } else if (change->type == IncidenceChanger::ChangeTypeDelete) {
            DeletionChange::Ptr deletion = change.staticCast<DeletionChange>();
            for (Akonadi::Item::Id id : std::as_const(deletion->mItemIds)) {
                if (operation->m_itemIdsInOperation.contains(id)) {
                    allow = false;
                    break;
                }
            }
        }
    }

    if (!allow) {
        qCWarning(AKONADICALENDAR_LOG) << "Each change belonging to a group operation"
                                       << "must have a different Akonadi::Item::Id";
    }

    return allow;
}

/**reimp*/
void ModificationChange::emitCompletionSignal()
{
    emitModifyFinished(changer, id, newItem, resultCode, errorString);
}

/**reimp*/
void CreationChange::emitCompletionSignal()
{
    // Does a queued emit, with QMetaObject::invokeMethod
    emitCreateFinished(changer, id, newItem, resultCode, errorString);
}

/**reimp*/
void DeletionChange::emitCompletionSignal()
{
    emitDeleteFinished(changer, id, mItemIds, resultCode, errorString);
}

/**
Lost code from KDE 4.4 that was never called/used with incidenceeditors-ng.

      Attendees were removed from this incidence. Only the removed attendees
      are present in the incidence, so we just need to send a cancel messages
      to all attendees groupware messages are enabled at all.

void IncidenceChanger::cancelAttendees( const Akonadi::Item &aitem )
{
  const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  Q_ASSERT( incidence );
  if ( KCalPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo(
           0,
           i18n( "Some attendees were removed from the incidence. "
                 "Shall cancel messages be sent to these attendees?" ),
           i18nc("@title:window", "Attendees Removed" ), KGuiItem( i18n( "Send Messages" ) ),
           KGuiItem( i18n( "Do Not Send" ) ) ) == KMessageBox::ButtonCode::PrimaryAction) {
      // don't use Akonadi::Groupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the Akonadi::MailScheduler::performTransaction, so do this
      // manually.
      CalendarSupport::MailScheduler scheduler(
        static_cast<CalendarSupport::Calendar*>(d->mCalendar) );
      scheduler.performTransaction( incidence, KCalendarCore::iTIPCancel );
    }
  }
}

*/

AtomicOperation::AtomicOperation(IncidenceChangerPrivate *icp, uint ident)
    : m_id(ident)
    , m_endCalled(false)
    , m_numCompletedChanges(0)
    , m_transactionCompleted(false)
    , m_wasRolledback(false)
    , m_transaction(nullptr)
    , m_incidenceChangerPrivate(icp)
{
    Q_ASSERT(m_id != 0);
}

Akonadi::TransactionSequence *AtomicOperation::transaction()
{
    if (!m_transaction) {
        m_transaction = new Akonadi::TransactionSequence;
        m_transaction->setAutomaticCommittingEnabled(true);

        m_incidenceChangerPrivate->mAtomicOperationByTransaction.insert(m_transaction, m_id);

        QObject::connect(m_transaction, &KJob::result, m_incidenceChangerPrivate, &IncidenceChangerPrivate::handleTransactionJobResult);
    }

    return m_transaction;
}

#include "moc_incidencechanger.cpp"
