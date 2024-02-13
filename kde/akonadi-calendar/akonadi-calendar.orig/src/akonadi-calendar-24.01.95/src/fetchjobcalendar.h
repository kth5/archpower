/*
   SPDX-FileCopyrightText: 2011 Sérgio Martins <sergio.martins@kdab.com>
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

#include <Akonadi/Item>

namespace Akonadi
{
class FetchJobCalendarPrivate;
/**
 * @short A KCalendarCore::Calendar that uses a one time IncidenceFetchJob to populate itself.
 *
 * If you want a persistent calendar ( which monitors Akonadi for changes )
 * use an ETMCalendar.
 *
 * @see ETMCalendar
 * @see CalendarBase
 *
 * @author Sérgio Martins <sergio.martins@kdab.com>
 * @since 4.11
 */
class AKONADI_CALENDAR_EXPORT FetchJobCalendar : public Akonadi::CalendarBase
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<FetchJobCalendar>;

    /**
     * Creates a new FetchJobCalendar. Loading begins asynchronously.
     * @see loadFinished()
     */
    explicit FetchJobCalendar(QObject *parent = nullptr);

    /**
     * Destroys this FetchJobCalendar.
     */
    ~FetchJobCalendar() override;

Q_SIGNALS:
    /**
     * This signal is emitted when the IncidenceFetchJob finishes.
     * @param success the success of the operation
     * @param errorMessage if @p success is false, contains the error message
     */
    void loadFinished(bool success, const QString &errorMessage);

private:
    Q_DECLARE_PRIVATE(FetchJobCalendar)
};
}
