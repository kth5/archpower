/*
    SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "../src/history.h"
#include "../src/history_p.h"
#include "../src/incidencechanger.h"
#include "unittestbase.h"

enum SignalType { DeletionSignal, CreationSignal, ModificationSignal, UndoSignal, RedoSignal, NumSignals };

class HistoryTest : public UnitTestBase
{
    Q_OBJECT
    History *mHistory = nullptr;
    QHash<SignalType, int> mPendingSignals;
    QHash<int, Akonadi::Item> mItemByChangeId;
    QList<int> mKnownChangeIds;

private Q_SLOTS:
    void initTestCase();

    void testCreation_data();
    void testCreation();

    void testDeletion_data();
    void testDeletion();

    void testModification_data();
    void testModification();

    void testAtomicOperations_data();
    void testAtomicOperations();

    // Tests a sequence of various create/delete/modify operations that are not part
    // of an atomic-operation.
    void testMix_data();
    void testMix();

private:
    void waitForSignals();

public Q_SLOTS:
    void
    deleteFinished(int changeId, const QList<Akonadi::Item::Id> &deletedIds, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);

    void createFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

    void modifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

    void handleRedone(Akonadi::History::ResultCode result);
    void handleUndone(Akonadi::History::ResultCode result);
    void maybeQuitEventLoop();
};
