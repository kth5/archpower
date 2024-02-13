/*
  SPDX-FileCopyrightText: 2011-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "calendarbase_p.h"
#include "fetchjobcalendar.h"

class KJob;

namespace Akonadi
{
class FetchJobCalendarPrivate : public CalendarBasePrivate
{
    Q_OBJECT
public:
    explicit FetchJobCalendarPrivate(FetchJobCalendar *qq);
    ~FetchJobCalendarPrivate() override;

public Q_SLOTS:
    void slotSearchJobFinished(KJob *job);
    void slotFetchJobFinished();

private:
    FetchJobCalendar *const q;
    QString m_errorMessage;
    bool m_success;
};
}
