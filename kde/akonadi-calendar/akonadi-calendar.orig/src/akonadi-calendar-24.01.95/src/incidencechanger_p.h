/*
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "history.h"
#include "incidencechanger.h"
#include "itiphandlerhelper_p.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/TransactionSequence>

#include <QList>
#include <QObject>
#include <QPointer>
#include <QSet>

class KJob;
class QWidget;

namespace Akonadi
{
class TransactionSequence;
class CollectionFetchJob;

class Change : public QObject
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<Change>;
    using List = QList<Ptr>;
    Change(IncidenceChanger *incidenceChanger, int changeId, IncidenceChanger::ChangeType changeType, uint operationId, QWidget *parent)
        : id(changeId)
        , type(changeType)
        , recordToHistory(incidenceChanger->historyEnabled())
        , parentWidget(parent)
        , atomicOperationId(operationId)
        , resultCode(Akonadi::IncidenceChanger::ResultCodeSuccess)
        , completed(false)
        , queuedModification(false)
        , useGroupwareCommunication(incidenceChanger->groupwareCommunication())
        , changer(incidenceChanger)
    {
    }

    ~Change() override
    {
        if (parentChange) {
            parentChange->childAboutToDie(this);
        }
    }

    virtual void childAboutToDie(Change *child)
    {
        Q_UNUSED(child)
    }

    virtual void emitCompletionSignal() = 0;

    const int id;
    const IncidenceChanger::ChangeType type;
    const bool recordToHistory;
    const QPointer<QWidget> parentWidget;
    uint atomicOperationId;

    // If this change is internal, i.e. not initiated by the user, mParentChange will
    // contain the non-internal change.
    QSharedPointer<Change> parentChange;

    Akonadi::Item::List originalItems;
    Akonadi::Item newItem;

    QString errorString;
    IncidenceChanger::ResultCode resultCode;
    bool completed;
    bool queuedModification;
    bool useGroupwareCommunication;

Q_SIGNALS:
    void dialogClosedBeforeChange(int id, ITIPHandlerHelper::SendResult status);
    void dialogClosedAfterChange(int id, ITIPHandlerHelper::SendResult status);

public Q_SLOTS:
    void emitUserDialogClosedAfterChange(Akonadi::ITIPHandlerHelper::SendResult status);
    void emitUserDialogClosedBeforeChange(Akonadi::ITIPHandlerHelper::SendResult status);

protected:
    IncidenceChanger *const changer;
};

class ModificationChange : public Change
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<ModificationChange>;
    ModificationChange(IncidenceChanger *changer, int id, uint atomicOperationId, QWidget *parent)
        : Change(changer, id, IncidenceChanger::ChangeTypeModify, atomicOperationId, parent)
    {
    }

    ~ModificationChange() override
    {
        if (!parentChange) {
            emitCompletionSignal();
        }
    }

    void emitCompletionSignal() override;
};

class CreationChange : public Change
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CreationChange>;
    CreationChange(IncidenceChanger *changer, int id, uint atomicOperationId, QWidget *parent)
        : Change(changer, id, IncidenceChanger::ChangeTypeCreate, atomicOperationId, parent)
    {
    }

    ~CreationChange() override
    {
        // qCDebug(AKONADICALENDAR_LOG) << "CreationChange::~ will emit signal with " << resultCode;
        if (!parentChange) {
            emitCompletionSignal();
        }
    }

    void emitCompletionSignal() override;

    Akonadi::Collection mUsedCol1lection;
};

class DeletionChange : public Change
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<DeletionChange>;
    DeletionChange(IncidenceChanger *changer, int id, uint atomicOperationId, QWidget *parent)
        : Change(changer, id, IncidenceChanger::ChangeTypeDelete, atomicOperationId, parent)
    {
    }

    ~DeletionChange() override
    {
        // qCDebug(AKONADICALENDAR_LOG) << "DeletionChange::~ will emit signal with " << resultCode;
        if (!parentChange) {
            emitCompletionSignal();
        }
    }

    void emitCompletionSignal() override;

    QList<Akonadi::Item::Id> mItemIds;
};

class AtomicOperation
{
public:
    uint m_id;

    // To make sure they are not repeated
    QSet<Akonadi::Item::Id> m_itemIdsInOperation;

    // After endAtomicOperation() is called we don't accept more changes
    bool m_endCalled;

    // Number of completed changes(jobs)
    int m_numCompletedChanges;
    QString m_description;
    bool m_transactionCompleted;

    AtomicOperation(IncidenceChangerPrivate *icp, uint ident);

    ~AtomicOperation()
    {
        // qCDebug(AKONADICALENDAR_LOG) << "AtomicOperation::~ " << wasRolledback << changes.count();
        if (m_wasRolledback) {
            for (int i = 0; i < m_changes.count(); ++i) {
                // When a job that can finish successfully is aborted because the transaction failed
                // because of some other job, akonadi is returning an Unknown error
                // which isn't very specific
                if (m_changes[i]->completed
                    && (m_changes[i]->resultCode == IncidenceChanger::ResultCodeSuccess
                        || (m_changes[i]->resultCode == IncidenceChanger::ResultCodeJobError
                            && m_changes[i]->errorString == QLatin1String("Unknown error.")))) {
                    m_changes[i]->resultCode = IncidenceChanger::ResultCodeRolledback;
                }
            }
        }
    }

    // Did all jobs return ?
    bool pendingJobs() const
    {
        return m_changes.count() > m_numCompletedChanges;
    }

    void setRolledback()
    {
        // qCDebug(AKONADICALENDAR_LOG) << "AtomicOperation::setRolledBack()";
        m_wasRolledback = true;
        transaction()->rollback();
    }

    bool rolledback() const
    {
        return m_wasRolledback;
    }

    void addChange(const Change::Ptr &change)
    {
        if (change->type == IncidenceChanger::ChangeTypeDelete) {
            DeletionChange::Ptr deletion = change.staticCast<DeletionChange>();
            for (Akonadi::Item::Id id : std::as_const(deletion->mItemIds)) {
                Q_ASSERT(!m_itemIdsInOperation.contains(id));
                m_itemIdsInOperation.insert(id);
            }
        } else if (change->type == IncidenceChanger::ChangeTypeModify) {
            Q_ASSERT(!m_itemIdsInOperation.contains(change->newItem.id()));
            m_itemIdsInOperation.insert(change->newItem.id());
        }

        m_changes << change;
    }

    Akonadi::TransactionSequence *transaction();

private:
    Q_DISABLE_COPY(AtomicOperation)
    QList<Change::Ptr> m_changes;
    bool m_wasRolledback = false;
    Akonadi::TransactionSequence *m_transaction = nullptr; // constructed in first use
    IncidenceChangerPrivate *m_incidenceChangerPrivate = nullptr;
};

class IncidenceChangerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit IncidenceChangerPrivate(bool enableHistory, ITIPHandlerComponentFactory *factory, IncidenceChanger *mIncidenceChanger);
    ~IncidenceChangerPrivate() override;

    void loadCollections(); // async-loading of list of writable collections
    bool isLoadingCollections() const;
    Collection::List collectionsForMimeType(const QString &mimeType, const Collection::List &collections);

    // steps for the async operation:
    void step1DetermineDestinationCollection(const Change::Ptr &change, const Collection &collection);
    void step2CreateIncidence(const Change::Ptr &change, const Collection &collection);

    /**
        Returns true if, for a specific item, an ItemDeleteJob is already running,
        or if one already run successfully.
    */
    bool deleteAlreadyCalled(Akonadi::Item::Id id) const;

    QString showErrorDialog(Akonadi::IncidenceChanger::ResultCode, QWidget *parent);

    void adjustRecurrence(const KCalendarCore::Incidence::Ptr &originalIncidence, const KCalendarCore::Incidence::Ptr &incidence);

    bool hasRights(const Akonadi::Collection &collection, IncidenceChanger::ChangeType) const;
    void queueModification(const Change::Ptr &);
    void performModification(const Change::Ptr &);
    bool atomicOperationIsValid(uint atomicOperationId) const;
    Akonadi::Job *parentJob(const Change::Ptr &change) const;
    void cancelTransaction();
    void cleanupTransaction();
    bool allowAtomicOperation(int atomicOperationId, const Change::Ptr &change) const;

    void handleInvitationsBeforeChange(const Change::Ptr &change);
    void handleInvitationsAfterChange(const Change::Ptr &change);
    static bool
    myAttendeeStatusChanged(const KCalendarCore::Incidence::Ptr &newIncidence, const KCalendarCore::Incidence::Ptr &oldIncidence, const QStringList &myEmails);

public Q_SLOTS:
    void handleCreateJobResult(KJob *job);
    void handleModifyJobResult(KJob *job);
    void handleDeleteJobResult(KJob *job);
    void handleTransactionJobResult(KJob *job);
    void performNextModification(Akonadi::Item::Id id);
    void onCollectionsLoaded(KJob *job);

    void handleCreateJobResult2(int changeId, ITIPHandlerHelper::SendResult);
    void handleDeleteJobResult2(int changeId, ITIPHandlerHelper::SendResult);
    void handleModifyJobResult2(int changeId, ITIPHandlerHelper::SendResult);
    void performModification2(int changeId, ITIPHandlerHelper::SendResult);
    void deleteIncidences2(int changeId, ITIPHandlerHelper::SendResult);

public:
    int mLatestChangeId;
    QHash<const KJob *, Change::Ptr> mChangeForJob;
    bool mShowDialogsOnError = false;
    Akonadi::Collection mDefaultCollection;
    Akonadi::EntityTreeModel *mEntityTreeModel = nullptr;
    IncidenceChanger::DestinationPolicy mDestinationPolicy;
    QList<Akonadi::Item::Id> mDeletedItemIds;
    Change::List mPendingCreations; // Creations waiting for collections to be loaded

    History *mHistory = nullptr;
    bool mUseHistory = false;

    /**
      Queue modifications by ID. We can only send a modification to akonadi when the previous
      one ended.

      The container doesn't look like a queue because of an optimization: if there's a modification
      A in progress, a modification B waiting (queued), and then a new one C comes in,
      we just discard B, and queue C. The queue always has 1 element max.
    */
    QHash<Akonadi::Item::Id, Change::Ptr> mQueuedModifications;

    /**
        So we know if there's already a modification in progress
      */
    QHash<Akonadi::Item::Id, Change::Ptr> mModificationsInProgress;

    QHash<int, Change::Ptr> mChangeById;

    /**
        Indexed by atomic operation id.
    */
    QHash<uint, AtomicOperation *> mAtomicOperations;

    bool mRespectsCollectionRights = false;
    bool mGroupwareCommunication = false;

    QHash<Akonadi::TransactionSequence *, uint> mAtomicOperationByTransaction;
    QHash<uint, ITIPHandlerHelper::SendResult> mInvitationStatusByAtomicOperation;

    uint mLatestAtomicOperationId;
    bool mBatchOperationInProgress;
    Akonadi::Collection mLastCollectionUsed;
    bool mAutoAdjustRecurrence;

    Akonadi::CollectionFetchJob *m_collectionFetchJob = nullptr;

    QMap<KJob *, QSet<KCalendarCore::IncidenceBase::Field>> mDirtyFieldsByJob;

    IncidenceChanger::InvitationPolicy m_invitationPolicy;
    IncidenceChanger::InvitationPrivacyFlags m_invitationPrivacy = IncidenceChanger::InvitationPrivacyPlain;

    ITIPHandlerComponentFactory *mFactory = nullptr;

private:
    IncidenceChanger *q = nullptr;
};
}
