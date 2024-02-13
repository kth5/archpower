/*
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "history_p.h"
#include "akonadicalendar_debug.h"
#include <KCalUtils/Stringify>
#include <KLocalizedString>

Entry::Entry(const Akonadi::Item &item, const QString &description, History *qq)
    : QObject()
{
    mItems << item;
    init(description, qq);
}

Entry::Entry(const Akonadi::Item::List &items, const QString &description, History *qq)
    : QObject()
    , mItems(items)
{
    init(description, qq);
}

void Entry::init(const QString &description, History *qq)
{
    mDescription = description;
    q = qq;
    mChanger = qq->d->mChanger;
}

QWidget *Entry::currentParent() const
{
    return q->d->mCurrentParent;
}

void Entry::updateIds(Item::Id oldId, Item::Id newId)
{
    Q_ASSERT(newId != -1);
    Q_ASSERT(oldId != newId);

    Akonadi::Item::List::iterator it = mItems.begin();
    while (it != mItems.end()) {
        if ((*it).id() == oldId) {
            (*it).setId(newId);
            (*it).setRevision(0);
        }
        ++it;
    }
}

void Entry::updateIdsGlobaly(Item::Id oldId, Item::Id newId)
{
    q->d->updateIds(oldId, newId);
}

void Entry::doIt(OperationType type)
{
    bool result = false;
    mChangeIds.clear();
    if (type == TypeRedo) {
        result = redo();
    } else if (type == TypeUndo) {
        result = undo();
    } else {
        Q_ASSERT(false);
    }

    if (!result) {
        Q_EMIT finished(IncidenceChanger::ResultCodeJobError, i18n("General error"));
    }
}

CreationEntry::CreationEntry(const Akonadi::Item &item, const QString &description, History *q)
    : Entry(item, description, q)
{
    mLatestRevisionByItemId.insert(item.id(), item.revision());
    Q_ASSERT(mItems.count() == 1);
    const auto incidence = mItems.constFirst().payload<KCalendarCore::Incidence::Ptr>();
    if (mDescription.isEmpty()) {
        mDescription = i18nc("%1 is event, todo or journal", "%1 creation", KCalUtils::Stringify::incidenceType(incidence->type()));
    }
    connect(mChanger, &IncidenceChanger::createFinished, this, &CreationEntry::onCreateFinished);
    connect(mChanger, &IncidenceChanger::deleteFinished, this, &CreationEntry::onDeleteFinished);
}

bool CreationEntry::undo()
{
    const int changeId = mChanger->deleteIncidence(mItems.constFirst(), currentParent());
    mChangeIds << changeId;

    if (changeId == -1) {
        qCritical() << "Undo failed";
    }

    return changeId != -1;
}

bool CreationEntry::redo()
{
    const Akonadi::Item item = mItems.constFirst();
    Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
    const int changeId = mChanger->createIncidence(item.payload<KCalendarCore::Incidence::Ptr>(), Collection(item.storageCollectionId()), currentParent());
    mChangeIds << changeId;

    if (changeId == -1) {
        qCritical() << "Redo failed";
    }

    return changeId != -1;
}

void CreationEntry::onDeleteFinished(int changeId,
                                     const QList<Akonadi::Item::Id> &deletedIds,
                                     Akonadi::IncidenceChanger::ResultCode resultCode,
                                     const QString &errorString)
{
    if (mChangeIds.contains(changeId)) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            Q_ASSERT(deletedIds.count() == 1);
            mLatestRevisionByItemId.remove(deletedIds.constFirst()); // TODO
        }
        Q_EMIT finished(resultCode, errorString);
    }
}

void CreationEntry::onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    if (mChangeIds.contains(changeId)) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            mLatestRevisionByItemId.insert(item.id(), item.revision());
            Q_ASSERT(mItems.count() == 1);

            if (mItems.constFirst().id() == item.id()) {
                qCWarning(AKONADICALENDAR_LOG) << "Duplicate id. Old= " << mItems.constFirst().id() << item.id();
                Q_ASSERT(false);
            }
            updateIdsGlobaly(mItems.constFirst().id(), item.id());
        }
        Q_EMIT finished(resultCode, errorString);
    }
}

DeletionEntry::DeletionEntry(const Akonadi::Item::List &items, const QString &description, History *q)
    : Entry(items, description, q)
{
    const auto incidence = items.constFirst().payload<KCalendarCore::Incidence::Ptr>();
    if (mDescription.isEmpty()) {
        mDescription = i18nc("%1 is event, todo or journal", "%1 deletion", KCalUtils::Stringify::incidenceType(incidence->type()));
    }
    connect(mChanger, &IncidenceChanger::createFinished, this, &DeletionEntry::onCreateFinished);
    connect(mChanger, &IncidenceChanger::deleteFinished, this, &DeletionEntry::onDeleteFinished);
}

bool DeletionEntry::undo()
{
    mResultCode = IncidenceChanger::ResultCodeSuccess;
    mErrorString.clear();
    const bool useAtomicOperation = mItems.count() > 1;
    bool success = true;
    for (const Akonadi::Item &item : std::as_const(mItems)) {
        if (useAtomicOperation) {
            mChanger->startAtomicOperation();
        }

        Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
        const int changeId = mChanger->createIncidence(item.payload<KCalendarCore::Incidence::Ptr>(), Collection(item.storageCollectionId()), currentParent());
        success = (changeId != -1) && success;
        mChangeIds << changeId;
        if (useAtomicOperation) {
            mChanger->endAtomicOperation();
        }

        mOldIdByChangeId.insert(changeId, item.id());
    }
    mNumPendingCreations = mItems.count();
    return success;
}

bool DeletionEntry::redo()
{
    const int changeId = mChanger->deleteIncidences(mItems, currentParent());
    mChangeIds << changeId;

    if (changeId == -1) {
        qCritical() << "Redo failed";
    }

    return changeId != -1;
}

void DeletionEntry::onDeleteFinished(int changeId,
                                     const QList<Akonadi::Item::Id> &deletedIds,
                                     Akonadi::IncidenceChanger::ResultCode resultCode,
                                     const QString &errorString)
{
    if (mChangeIds.contains(changeId)) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            for (const Akonadi::Item::Id id : deletedIds) {
                mLatestRevisionByItemId.remove(id); // TODO
            }
        }
        Q_EMIT finished(resultCode, errorString);
    }
}

void DeletionEntry::onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    if (mChangeIds.contains(changeId)) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            updateIdsGlobaly(mOldIdByChangeId.value(changeId), item.id());
            mLatestRevisionByItemId.insert(item.id(), item.revision());
        } else {
            mResultCode = resultCode;
            mErrorString = errorString;
        }
        --mNumPendingCreations;
        mOldIdByChangeId.remove(changeId);
        if (mNumPendingCreations == 0) {
            Q_EMIT finished(mResultCode, mErrorString);
        }
    }
}

ModificationEntry::ModificationEntry(const Akonadi::Item &item, const Incidence::Ptr &originalPayload, const QString &description, History *q)
    : Entry(item, description, q)
    , mOriginalPayload(originalPayload->clone())
{
    const auto incidence = mItems.constFirst().payload<KCalendarCore::Incidence::Ptr>();
    if (mDescription.isEmpty()) {
        mDescription = i18nc("%1 is event, todo or journal", "%1 modification", KCalUtils::Stringify::incidenceType(incidence->type()));
    }

    connect(mChanger, &IncidenceChanger::modifyFinished, this, &ModificationEntry::onModifyFinished);
}

bool ModificationEntry::undo()
{
    Item oldItem = mItems.constFirst();
    oldItem.setPayload<KCalendarCore::Incidence::Ptr>(mOriginalPayload);
    const int changeId = mChanger->modifyIncidence(oldItem, Incidence::Ptr(), currentParent());
    mChangeIds << changeId;

    if (changeId == -1) {
        qCritical() << "Undo failed";
    }

    return changeId != -1;
}

bool ModificationEntry::redo()
{
    const int changeId = mChanger->modifyIncidence(mItems.constFirst(), mOriginalPayload, currentParent());
    mChangeIds << changeId;

    if (changeId == -1) {
        qCritical() << "Redo failed";
    }

    return changeId != -1;
}

void ModificationEntry::onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    if (mChangeIds.contains(changeId)) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            mLatestRevisionByItemId.insert(item.id(), item.revision());
        }
        Q_EMIT finished(resultCode, errorString);
    }
}

MultiEntry::MultiEntry(int id, const QString &description, History *q)
    : Entry(Item(), description, q)
    , mAtomicOperationId(id)
    , mFinishedEntries(0)
    , mOperationInProgress(TypeNone)
{
}

void MultiEntry::addEntry(const Entry::Ptr &entry)
{
    Q_ASSERT(mOperationInProgress == TypeNone);
    mEntries.append(entry);
    connect(entry.data(), &Entry::finished, this, &MultiEntry::onEntryFinished, Qt::UniqueConnection);
}

void MultiEntry::updateIds(Item::Id oldId, Item::Id newId)
{
    const int numberOfEntries(mEntries.count());
    for (int i = 0; i < numberOfEntries; ++i) {
        mEntries.at(i)->updateIds(oldId, newId);
    }
}

bool MultiEntry::undo()
{
    mChanger->startAtomicOperation();
    mOperationInProgress = TypeUndo;
    Q_ASSERT(!mEntries.isEmpty());
    mFinishedEntries = 0;

    const int count = mEntries.count();
    // To undo a batch of changes we iterate in reverse order so we don't violate
    // causality.
    for (int i = count - 1; i >= 0; --i) {
        mEntries[i]->doIt(TypeUndo);
    }

    mChanger->endAtomicOperation();
    return true;
}

bool MultiEntry::redo()
{
    mChanger->startAtomicOperation();
    mOperationInProgress = TypeRedo;
    Q_ASSERT(!mEntries.isEmpty());
    mFinishedEntries = 0;
    for (const Entry::Ptr &entry : std::as_const(mEntries)) {
        entry->doIt(TypeRedo);
    }
    mChanger->endAtomicOperation();
    return true;
}

void MultiEntry::onEntryFinished(Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString)
{
    ++mFinishedEntries;
    if (mFinishedEntries == mEntries.count() || (mFinishedEntries < mEntries.count() && resultCode != IncidenceChanger::ResultCodeSuccess)) {
        mFinishedEntries = mEntries.count(); // we're done
        mOperationInProgress = TypeNone;
        Q_EMIT finished(resultCode, errorString);
    }
}

#include "moc_history_p.cpp"
