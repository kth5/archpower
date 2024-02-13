/*
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "history.h"
#include "akonadicalendar_debug.h"
#include "history_p.h"

using namespace KCalendarCore;
using namespace Akonadi;

History::History(QObject *parent)
    : QObject(parent)
    , d(new HistoryPrivate(this))
{
}

History::~History() = default;

HistoryPrivate::HistoryPrivate(History *qq)
    : mChanger(new IncidenceChanger(/*history=*/false, qq))
    , mOperationTypeInProgress(TypeNone)
    , q(qq)
{
    mChanger->setObjectName(QLatin1StringView("changer")); // for auto-connects
}

void History::recordCreation(const Akonadi::Item &item, const QString &description, const uint atomicOperationId)
{
    Q_ASSERT_X(item.isValid(), "History::recordCreation()", "Item must be valid.");

    Q_ASSERT_X(item.hasPayload<KCalendarCore::Incidence::Ptr>(), "History::recordCreation()", "Item must have Incidence::Ptr payload.");

    Entry::Ptr entry(new CreationEntry(item, description, this));

    d->stackEntry(entry, atomicOperationId);
}

void History::recordModification(const Akonadi::Item &oldItem, const Akonadi::Item &newItem, const QString &description, const uint atomicOperationId)
{
    Q_ASSERT_X(oldItem.isValid(), "History::recordModification", "old item must be valid");
    Q_ASSERT_X(newItem.isValid(), "History::recordModification", "newItem item must be valid");
    Q_ASSERT_X(oldItem.hasPayload<KCalendarCore::Incidence::Ptr>(), "History::recordModification", "old item must have Incidence::Ptr payload");
    Q_ASSERT_X(newItem.hasPayload<KCalendarCore::Incidence::Ptr>(), "History::recordModification", "newItem item must have Incidence::Ptr payload");

    Entry::Ptr entry(new ModificationEntry(newItem, oldItem.payload<KCalendarCore::Incidence::Ptr>(), description, this));

    Q_ASSERT(newItem.revision() >= oldItem.revision());

    d->stackEntry(entry, atomicOperationId);
}

void History::recordDeletion(const Akonadi::Item &item, const QString &description, const uint atomicOperationId)
{
    Q_ASSERT_X(item.isValid(), "History::recordDeletion", "Item must be valid");
    Item::List list;
    list.append(item);
    recordDeletions(list, description, atomicOperationId);
}

void History::recordDeletions(const Akonadi::Item::List &items, const QString &description, const uint atomicOperationId)
{
    Entry::Ptr entry(new DeletionEntry(items, description, this));

    for (const Akonadi::Item &item : items) {
        Q_UNUSED(item)
        Q_ASSERT_X(item.isValid(), "History::recordDeletion()", "Item must be valid.");
        Q_ASSERT_X(item.hasPayload<Incidence::Ptr>(), "History::recordDeletion()", "Item must have an Incidence::Ptr payload.");
    }

    d->stackEntry(entry, atomicOperationId);
}

QString History::nextUndoDescription() const
{
    if (!d->mUndoStack.isEmpty()) {
        return d->mUndoStack.top()->mDescription;
    } else {
        return {};
    }
}

QString History::nextRedoDescription() const
{
    if (!d->mRedoStack.isEmpty()) {
        return d->mRedoStack.top()->mDescription;
    } else {
        return {};
    }
}

void History::undo(QWidget *parent)
{
    d->undoOrRedo(TypeUndo, parent);
}

void History::redo(QWidget *parent)
{
    d->undoOrRedo(TypeRedo, parent);
}

void History::undoAll(QWidget *parent)
{
    if (d->mOperationTypeInProgress != TypeNone) {
        qCWarning(AKONADICALENDAR_LOG) << "Don't call History::undoAll() while an undo/redo/undoAll is in progress";
    } else if (d->mEnabled) {
        d->mUndoAllInProgress = true;
        d->mCurrentParent = parent;
        d->doIt(TypeUndo);
    } else {
        qCWarning(AKONADICALENDAR_LOG) << "Don't call undo/redo when History is disabled";
    }
}

bool History::clear()
{
    bool result = true;
    if (d->mOperationTypeInProgress == TypeNone) {
        d->mRedoStack.clear();
        d->mUndoStack.clear();
        d->mLastErrorString.clear();
        d->mQueuedEntries.clear();
    } else {
        result = false;
    }
    Q_EMIT changed();
    return result;
}

QString History::lastErrorString() const
{
    return d->mLastErrorString;
}

bool History::undoAvailable() const
{
    return !d->mUndoStack.isEmpty() && d->mOperationTypeInProgress == TypeNone;
}

bool History::redoAvailable() const
{
    return !d->mRedoStack.isEmpty() && d->mOperationTypeInProgress == TypeNone;
}

void HistoryPrivate::updateIds(Item::Id oldId, Item::Id newId)
{
    mEntryInProgress->updateIds(oldId, newId);

    for (const Entry::Ptr &entry : std::as_const(mUndoStack)) {
        entry->updateIds(oldId, newId);
    }

    for (const Entry::Ptr &entry : std::as_const(mRedoStack)) {
        entry->updateIds(oldId, newId);
    }
}

void HistoryPrivate::doIt(OperationType type)
{
    mOperationTypeInProgress = type;
    Q_EMIT q->changed(); // Application will disable undo/redo buttons because operation is in progress
    Q_ASSERT(!stack().isEmpty());
    mEntryInProgress = stack().pop();

    connect(mEntryInProgress.data(), &Entry::finished, this, &HistoryPrivate::handleFinished, Qt::UniqueConnection);
    mEntryInProgress->doIt(type);
}

void HistoryPrivate::handleFinished(IncidenceChanger::ResultCode changerResult, const QString &errorString)
{
    Q_ASSERT(mOperationTypeInProgress != TypeNone);
    Q_ASSERT(!(mUndoAllInProgress && mOperationTypeInProgress == TypeRedo));

    const bool success = (changerResult == IncidenceChanger::ResultCodeSuccess);
    const History::ResultCode resultCode = success ? History::ResultCodeSuccess : History::ResultCodeError;

    if (success) {
        mLastErrorString.clear();
        destinationStack().push(mEntryInProgress);
    } else {
        mLastErrorString = errorString;
        stack().push(mEntryInProgress);
    }

    mCurrentParent = nullptr;

    // Process recordCreation/Modification/Deletions that came in while an operation
    // was in progress
    if (!mQueuedEntries.isEmpty()) {
        mRedoStack.clear();
        for (const Entry::Ptr &entry : std::as_const(mQueuedEntries)) {
            mUndoStack.push(entry);
        }
        mQueuedEntries.clear();
    }

    emitDone(mOperationTypeInProgress, resultCode);
    mOperationTypeInProgress = TypeNone;
    Q_EMIT q->changed();
}

void HistoryPrivate::stackEntry(const Entry::Ptr &entry, uint atomicOperationId)
{
    const bool useMultiEntry = (atomicOperationId > 0);

    Entry::Ptr entryToPush;

    if (useMultiEntry) {
        Entry::Ptr topEntry = (mOperationTypeInProgress == TypeNone) ? (mUndoStack.isEmpty() ? Entry::Ptr() : mUndoStack.top())
                                                                     : (mQueuedEntries.isEmpty() ? Entry::Ptr() : mQueuedEntries.last());

        const bool topIsMultiEntry = qobject_cast<MultiEntry *>(topEntry.data());

        if (topIsMultiEntry) {
            MultiEntry::Ptr multiEntry = topEntry.staticCast<MultiEntry>();
            if (multiEntry->mAtomicOperationId != atomicOperationId) {
                multiEntry = MultiEntry::Ptr(new MultiEntry(atomicOperationId, entry->mDescription, q));
                entryToPush = multiEntry;
            }
            multiEntry->addEntry(entry);
        } else {
            MultiEntry::Ptr multiEntry = MultiEntry::Ptr(new MultiEntry(atomicOperationId, entry->mDescription, q));
            multiEntry->addEntry(entry);
            entryToPush = multiEntry;
        }
    } else {
        entryToPush = entry;
    }

    if (mOperationTypeInProgress == TypeNone) {
        if (entryToPush) {
            mUndoStack.push(entryToPush);
        }
        mRedoStack.clear();
        Q_EMIT q->changed();
    } else {
        if (entryToPush) {
            mQueuedEntries.append(entryToPush);
        }
    }
}

void HistoryPrivate::undoOrRedo(OperationType type, QWidget *parent)
{
    // Don't call undo() without the previous one finishing
    Q_ASSERT(mOperationTypeInProgress == TypeNone);

    if (!stack(type).isEmpty()) {
        if (mEnabled) {
            mCurrentParent = parent;
            doIt(type);
        } else {
            qCWarning(AKONADICALENDAR_LOG) << "Don't call undo/redo when History is disabled";
        }
    } else {
        qCWarning(AKONADICALENDAR_LOG) << "Don't call undo/redo when the stack is empty.";
    }
}

QStack<Entry::Ptr> &HistoryPrivate::stack(OperationType type)
{
    // Entries from the undo stack go to the redo stack, and vice-versa
    return type == TypeUndo ? mUndoStack : mRedoStack;
}

void HistoryPrivate::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

int HistoryPrivate::redoCount() const
{
    return mRedoStack.count();
}

int HistoryPrivate::undoCount() const
{
    return mUndoStack.count();
}

QStack<Entry::Ptr> &HistoryPrivate::stack()
{
    return stack(mOperationTypeInProgress);
}

QStack<Entry::Ptr> &HistoryPrivate::destinationStack()
{
    // Entries from the undo stack go to the redo stack, and vice-versa
    return mOperationTypeInProgress == TypeRedo ? mUndoStack : mRedoStack;
}

void HistoryPrivate::emitDone(OperationType type, History::ResultCode resultCode)
{
    if (type == TypeUndo) {
        Q_EMIT q->undone(resultCode);
    } else if (type == TypeRedo) {
        Q_EMIT q->redone(resultCode);
    } else {
        Q_ASSERT(false);
    }
}

Akonadi::IncidenceChanger *History::incidenceChanger() const
{
    return d->mChanger;
}

#include "moc_history.cpp"
