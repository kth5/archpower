/*
  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "calendarbase.h"

#include <KCalendarCore/IncidenceBase>
#include <KCalendarCore/ScheduleMessage>

#include <QObject>
#include <QString>

#include <memory>

namespace KCalendarCore
{
class ICalFormat;
class FreeBusyCache;
}

namespace Akonadi
{
class SchedulerPrivate;

/**
  This class provides an encapsulation of iTIP transactions (RFC 2446).
  It is an abstract base class for inheritance by implementations of the
  iTIP scheme like iMIP or iRIP.
*/
class Scheduler : public QObject
{
    Q_OBJECT
public:
    enum Result {
        ResultSuccess,
        ResultAssigningDifferentTypes,
        ResultOutatedUpdate,
        ResultErrorDelete,
        ResultIncidenceToDeleteNotFound,
        ResultGenericError,
        ResultNoFreeBusyCache,
        ResultErrorSavingFreeBusy,
        ResultCreatingError,
        ResultModifyingError,
        ResultDeletingError,
        ResultUnsupported,
        ResultUserCancelled
    };

    /**
      Creates a scheduler for calendar specified as argument.
    */
    explicit Scheduler(QObject *parent = nullptr);
    ~Scheduler() override;

    void setShowDialogs(bool enable);

    /**
     * Notify @p recipients about @p incidence
     *
     * @param incidence the incidence to send
     * @param recipients the people to send it to
     */
    virtual void publish(const KCalendarCore::IncidenceBase::Ptr &incidence, const QString &recipients) = 0;
    /**
      Performs iTIP transaction on incidence. The method is specified as the
      method argument and can be any valid iTIP method.

      @param incidence the incidence for the transaction. Must be valid.
      @param method the iTIP transaction method to use.
      @param sender email address of the identity to use for sending - if empty, the identity is chosen based on the incidence organizer.
    */
    virtual void performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::iTIPMethod method, const QString &sender = {}) = 0;

    /**
      Performs iTIP transaction on incidence to specified recipient(s).
      The method is specified as the method argumanet and can be any valid iTIP method.

      @param incidence the incidence for the transaction. Must be valid.
      @param method the iTIP transaction method to use.
      @param recipients the recipients of the transaction.
      @param sender email address of the identity to use for sending - if empty, the identity is chosen based on the incidence organizer.
    */
    virtual void performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence,
                                    KCalendarCore::iTIPMethod method,
                                    const QString &recipients,
                                    const QString &sender) = 0;

    /**
      Accepts the transaction. The incidence argument specifies the iCal
      component on which the transaction acts. The status is the result of
      processing a iTIP message with the current calendar and specifies the
      action to be taken for this incidence.

      @param incidence the incidence for the transaction. Must be valid.
      @param calendar a loaded calendar.
      @param method iTIP transaction method to check.
      @param status scheduling status.
      @param email the email address of the person for whom this
      transaction is to be performed.

      Listen to the acceptTransactionFinished() signal to know the success.
    */
    void acceptTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence,
                           const Akonadi::CalendarBase::Ptr &calendar,
                           KCalendarCore::iTIPMethod method,
                           KCalendarCore::ScheduleMessage::Status status,
                           const QString &email = QString());

    /**
      Returns the directory where the free-busy information is stored.
    */
    virtual QString freeBusyDir() const = 0;

    /**
      Sets the free/busy cache used to store free/busy information.
    */
    void setFreeBusyCache(KCalendarCore::FreeBusyCache *c);

    /**
      Returns the free/busy cache.
    */
    KCalendarCore::FreeBusyCache *freeBusyCache() const;

protected:
    void acceptPublish(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const Akonadi::CalendarBase::Ptr &calendar,
                       KCalendarCore::ScheduleMessage::Status status,
                       KCalendarCore::iTIPMethod method);

    void acceptRequest(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const Akonadi::CalendarBase::Ptr &calendar,
                       KCalendarCore::ScheduleMessage::Status status,
                       const QString &email);

    void acceptAdd(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::ScheduleMessage::Status status);

    void acceptCancel(const KCalendarCore::IncidenceBase::Ptr &incidence,
                      const Akonadi::CalendarBase::Ptr &calendar,
                      KCalendarCore::ScheduleMessage::Status status,
                      const QString &attendee);

    void acceptDeclineCounter(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::ScheduleMessage::Status status);

    void acceptReply(const KCalendarCore::IncidenceBase::Ptr &incidence,
                     const Akonadi::CalendarBase::Ptr &calendar,
                     KCalendarCore::ScheduleMessage::Status status,
                     KCalendarCore::iTIPMethod method);

    void acceptRefresh(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::ScheduleMessage::Status status);

    void acceptCounter(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::ScheduleMessage::Status status);

    void acceptFreeBusy(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::iTIPMethod method);
    KCalendarCore::ICalFormat *mFormat = nullptr;

Q_SIGNALS:
    void transactionFinished(Akonadi::Scheduler::Result, const QString &errorMessage);
private Q_SLOTS:
    void handleCreateFinished(bool success, const QString &errorMessage);
    void handleModifyFinished(bool success, const QString &errorMessage);
    void handleDeleteFinished(bool success, const QString &errorMessage);

private:
    void connectCalendar(const Akonadi::CalendarBase::Ptr &calendar);
    Q_DISABLE_COPY(Scheduler)
    std::unique_ptr<SchedulerPrivate> const d;
};
}
