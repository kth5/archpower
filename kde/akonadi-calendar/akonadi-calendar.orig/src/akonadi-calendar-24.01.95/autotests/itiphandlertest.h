/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidencechanger.h"
#include "itiphandler.h"
#include "unittestbase.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>

class ITIPHandlerTest : public UnitTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testProcessITIPMessages_data();
    void testProcessITIPMessages();

    // Deprecated methods, use testProcessITIPMessages() for new stuff
    void testProcessITIPMessage_data();
    void testProcessITIPMessage();

    // Deprecated methods do test CANCEL.
    void testProcessITIPMessageCancel_data();
    void testProcessITIPMessageCancel();

    // These ones don't have to do with kmail. It's when doing a modification, itip REQUESTs are sent.
    // Also tests cases where we're not the organizer.
    void testOutgoingInvitations_data();
    void testOutgoingInvitations();

    // Tests identity related code, like "thisIsMe()".
    void testIdentity_data();
    void testIdentity();

private:
    void waitForSignals();
    void cleanup();
    void createITIPHandler();
    QString icalData(const QString &filename);
    void processItip(const QString &icaldata, const QString &receiver, const QString &action, int expectedNumIncidences, Akonadi::Item::List &items);
    KCalendarCore::Attendee ourAttendee(const KCalendarCore::Incidence::Ptr &incidence) const;

public Q_SLOTS:
    void oniTipMessageProcessed(Akonadi::ITIPHandler::Result result, const QString &errorMessage);

    void onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

    void
    onDeleteFinished(int changeId, const QList<Akonadi::Item::Id> &deletedIds, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);

    void onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);

private:
    int m_pendingItipMessageSignal;
    int m_pendingIncidenceChangerSignal;
    Akonadi::Item mLastInsertedItem;
    Akonadi::ITIPHandler::Result m_expectedResult;
    Akonadi::ITIPHandler *m_itipHandler = nullptr;
    Akonadi::IncidenceChanger *m_changer = nullptr;
    bool m_cancelExpected = false;
};
