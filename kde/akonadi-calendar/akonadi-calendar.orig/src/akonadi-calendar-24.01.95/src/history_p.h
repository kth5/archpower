/*
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "history.h"
#include "incidencechanger.h"
#include <Akonadi/Collection>
#include <KCalendarCore/Incidence>

#include <QList>
#include <QPointer>
#include <QStack>

using namespace Akonadi;
using namespace KCalendarCore;

namespace Akonadi
{
class History;

enum OperationType { TypeNone, TypeUndo, TypeRedo };

class Entry : public QObject
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<Entry>;
    using List = QList<Entry::Ptr>;
    Entry(const Akonadi::Item &item, const QString &description, History *qq);
    Entry(const Akonadi::Item::List &items, const QString &description, History *qq);
    virtual void updateIds(Item::Id oldId, Item::Id newId);
    void doIt(OperationType);

    Akonadi::Item::List mItems;
    QString mDescription;
Q_SIGNALS:
    void finished(Akonadi::IncidenceChanger::ResultCode, const QString &errorString);

protected:
    virtual bool undo() = 0;
    virtual bool redo() = 0;
    void updateIdsGlobaly(Item::Id oldId, Item::Id newId);
    QWidget *currentParent() const;
    IncidenceChanger *mChanger = nullptr;
    QHash<Akonadi::Item::Id, int> mLatestRevisionByItemId;
    History *q = nullptr;
    QList<int> mChangeIds;

private:
    void init(const QString &description, History *qq);
    Q_DISABLE_COPY(Entry)
};

class AKONADI_CALENDAR_EXPORT HistoryPrivate : public QObject
{
    Q_OBJECT
public:
    explicit HistoryPrivate(History *qq);
    ~HistoryPrivate() override = default;

    void doIt(OperationType);
    void stackEntry(const Entry::Ptr &entry, uint atomicOperationId);
    void updateIds(Item::Id oldId, Item::Id newId);
    QStack<Entry::Ptr> &destinationStack();
    QStack<Entry::Ptr> &stack(OperationType);
    QStack<Entry::Ptr> &stack();
    void undoOrRedo(OperationType, QWidget *parent);

    void emitDone(OperationType, History::ResultCode);
    void setEnabled(bool enabled);

    int redoCount() const;
    int undoCount() const;

    IncidenceChanger *const mChanger;

    QStack<Entry::Ptr> mUndoStack;
    QStack<Entry::Ptr> mRedoStack;

    OperationType mOperationTypeInProgress;

    Entry::Ptr mEntryInProgress;

    QString mLastErrorString;
    bool mUndoAllInProgress = false;

    /**
     * When recordCreation/Deletion/Modification is called and an undo operation is already in progress
     * the entry is added here.
     */
    QList<Entry::Ptr> mQueuedEntries;
    bool mEnabled = true;
    QPointer<QWidget> mCurrentParent;

public Q_SLOTS:
    void handleFinished(Akonadi::IncidenceChanger::ResultCode, const QString &errorString);

private:
    History *const q;
};

class CreationEntry : public Entry
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CreationEntry>;
    CreationEntry(const Akonadi::Item &item, const QString &description, History *q);

    bool undo() override;
    bool redo() override;

private Q_SLOTS:
    void
    onDeleteFinished(int changeId, const QList<Akonadi::Item::Id> &deletedIds, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

    void onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

private:
    Q_DISABLE_COPY(CreationEntry)
};

class DeletionEntry : public Entry
{
    Q_OBJECT
public:
    DeletionEntry(const Akonadi::Item::List &items, const QString &description, History *q);
    bool undo() override;
    bool redo() override;

private Q_SLOTS:
    void
    onDeleteFinished(int changeId, const QList<Akonadi::Item::Id> &deletedIds, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

    void onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

private:
    IncidenceChanger::ResultCode mResultCode;
    QString mErrorString;
    QHash<int, Akonadi::Item::Id> mOldIdByChangeId;
    int mNumPendingCreations;
    Q_DISABLE_COPY(DeletionEntry)
};

class ModificationEntry : public Entry
{
    Q_OBJECT
public:
    ModificationEntry(const Akonadi::Item &item, const Incidence::Ptr &originalPayload, const QString &description, History *q);

    bool undo() override;
    bool redo() override;

private Q_SLOTS:
    void onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

private:
    Q_DISABLE_COPY(ModificationEntry)
    Incidence::Ptr mOriginalPayload;
};

class MultiEntry : public Entry
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<MultiEntry>;
    MultiEntry(int id, const QString &description, History *q);

    void addEntry(const Entry::Ptr &entry);
    void updateIds(Item::Id oldId, Item::Id newId) override;

protected:
    bool undo() override;
    bool redo() override;

private Q_SLOTS:
    void onEntryFinished(Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

public:
    const uint mAtomicOperationId;

private:
    Entry::List mEntries;
    int mFinishedEntries;
    OperationType mOperationInProgress;
    Q_DISABLE_COPY(MultiEntry)
};
}
