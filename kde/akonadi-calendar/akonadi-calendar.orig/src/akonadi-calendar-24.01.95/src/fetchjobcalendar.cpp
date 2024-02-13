/*
   SPDX-FileCopyrightText: 2011 Sérgio Martins <sergio.martins@kdab.com>
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fetchjobcalendar.h"
#include "akonadicalendar_debug.h"
#include "fetchjobcalendar_p.h"
#include "incidencefetchjob_p.h"
#include <Akonadi/Collection>

using namespace Akonadi;
using namespace KCalendarCore;

FetchJobCalendarPrivate::FetchJobCalendarPrivate(FetchJobCalendar *qq)
    : CalendarBasePrivate(qq)
    , q(qq)
{
    auto job = new IncidenceFetchJob();
    connect(job, &KJob::result, this, &FetchJobCalendarPrivate::slotSearchJobFinished);
    connect(this, &CalendarBasePrivate::fetchFinished, this, &FetchJobCalendarPrivate::slotFetchJobFinished);
}

FetchJobCalendarPrivate::~FetchJobCalendarPrivate() = default;

void FetchJobCalendarPrivate::slotSearchJobFinished(KJob *job)
{
    auto searchJob = static_cast<Akonadi::IncidenceFetchJob *>(job);
    m_success = true;
    m_errorMessage = QString();
    if (searchJob->error()) {
        m_success = false;
        m_errorMessage = searchJob->errorText();
        qCWarning(AKONADICALENDAR_LOG) << "Unable to fetch incidences:" << searchJob->errorText();
    } else {
        const Akonadi::Item::List lstItem = searchJob->items();
        for (const Akonadi::Item &item : lstItem) {
            if (!item.isValid() || !item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                m_success = false;
                m_errorMessage = QStringLiteral("Invalid item or payload: %1").arg(item.id());
                qCWarning(AKONADICALENDAR_LOG) << "Unable to fetch incidences:" << m_errorMessage;
                continue;
            }
            internalInsert(item);
        }
    }

    if (mCollectionJobs.isEmpty()) {
        slotFetchJobFinished();
    }
}

void FetchJobCalendarPrivate::slotFetchJobFinished()
{
    q->setIsLoading(false);
    // Q_EMIT loadFinished() in a delayed manner, due to freezes because of execs.
    QMetaObject::invokeMethod(q, "loadFinished", Qt::QueuedConnection, Q_ARG(bool, m_success), Q_ARG(QString, m_errorMessage));
}

FetchJobCalendar::FetchJobCalendar(QObject *parent)
    : CalendarBase(new FetchJobCalendarPrivate(this), parent)
{
    setIsLoading(true);
}

FetchJobCalendar::~FetchJobCalendar() = default;

#include "moc_fetchjobcalendar.cpp"
#include "moc_fetchjobcalendar_p.cpp"
