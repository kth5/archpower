/*
  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2010, 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "mailclient_p.h"
#include "scheduler_p.h"

#include <Akonadi/Item>
#include <KCalendarCore/Incidence>

#include <memory>

namespace Akonadi
{
class MailSchedulerPrivate;

/*
  This class implements the iTIP interface using the email interface specified
  as Mail.
*/
class MailScheduler : public Akonadi::Scheduler
{
    Q_OBJECT
public:
    /**
     * @param calendar Must be a valid and loaded calendar.
     */
    explicit MailScheduler(ITIPHandlerComponentFactory *factory, QObject *parent = nullptr);
    ~MailScheduler() override;

    void setEncrypt(bool encrypt);

    void setSign(bool sign);

    void publish(const KCalendarCore::IncidenceBase::Ptr &incidence, const QString &recipients) override;

    void performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::iTIPMethod method, const QString &sender = {}) override;

    void performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence,
                            KCalendarCore::iTIPMethod method,
                            const QString &recipients,
                            const QString &sender) override;

    /** Returns the directory where the free-busy information is stored */
    [[nodiscard]] QString freeBusyDir() const override;

    /**
     * Accepts a counter proposal.
     * @param incidence A non-null incidence.
     * @param calendar A loaded calendar. Try not to use an ETMCalendar here, due to it's
     *                 async loading.
     */
    void acceptCounterProposal(const KCalendarCore::Incidence::Ptr &incidence, const Akonadi::CalendarBase::Ptr &calendar);

private:
    /**
     * @brief onMailerFinished Handles the result of the MailClient operation.
     * @param result Error code.
     * @param errorMsg Error message if @p result is not success.
     */
    void onMailerFinished(Akonadi::MailClient::Result result, const QString &errorMsg);

private:
    //@cond PRIVATE
    Q_DISABLE_COPY(MailScheduler)
    std::unique_ptr<MailSchedulerPrivate> const d;
    //@endcond
};
}
