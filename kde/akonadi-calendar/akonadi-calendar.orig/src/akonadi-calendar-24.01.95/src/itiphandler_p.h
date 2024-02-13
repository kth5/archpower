/*
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "calendarbase.h"
#include "itiphandler.h"
#include "itiphandlerhelper_p.h"
#include "mailscheduler_p.h"

#include <KCalendarCore/ScheduleMessage>

#include <QObject>
#include <QPointer>
#include <QString>

namespace Akonadi
{
struct Invitation {
    QString receiver;
    QString iCal;
    QString action;
    KCalendarCore::iTIPMethod method;
    KCalendarCore::Incidence::Ptr incidence;
};

/**
 * Our API has two methods, one to process received invitations and another one to send them.
 * These operations are async and we don't want them to be called before the other has finished.
 * This enum is just to Q_ASSERT that.
 */
enum Operation { OperationNone, OperationProcessiTIPMessage, OperationSendiTIPMessage, OperationPublishInformation, OperationSendAsICalendar };

class ITIPHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    ITIPHandlerPrivate(ITIPHandlerComponentFactory *factory, ITIPHandler *q);

    void finishProcessiTIPMessage(Akonadi::MailScheduler::Result, const QString &errorMessage);
    void finishSendiTIPMessage(Akonadi::MailScheduler::Result, const QString &errorMessage);
    void finishPublishInformation(Akonadi::MailScheduler::Result, const QString &errorMessage);

    /**
     * Returns the calendar.
     * Creates a new one, if none is set.
     */
    CalendarBase::Ptr calendar();
    bool isLoaded(); // don't make const

    Invitation m_queuedInvitation;
    bool m_calendarLoadError = false;
    CalendarBase::Ptr m_calendar;
    ITIPHandlerComponentFactory *m_factory = nullptr;
    MailScheduler *m_scheduler = nullptr;
    KCalendarCore::Incidence::Ptr m_incidence;
    KCalendarCore::iTIPMethod m_method; // METHOD field of ical rfc of incoming invitation
    ITIPHandlerHelper *m_helper = nullptr;
    Operation m_currentOperation;
    QPointer<QWidget> m_parentWidget; // To be used for KMessageBoxes
    GroupwareUiDelegate *m_uiDelegate = nullptr;
    bool m_showDialogsOnError = true;
    ITIPHandler *const q;

    void finishSendAsICalendar(Akonadi::MailClient::Result, const QString &errorMessage);

private:
    void onLoadFinished(bool success, const QString &errorMessage);
    void onSchedulerFinished(Akonadi::Scheduler::Result, const QString &errorMessage);
    void onHelperFinished(Akonadi::ITIPHandlerHelper::SendResult result, const QString &errorMessage);

    void onHelperModifyDialogClosed(ITIPHandlerHelper::SendResult result, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

    void onCounterProposalDelegateFinished(bool success, const QString &errorMessage);
};
}
