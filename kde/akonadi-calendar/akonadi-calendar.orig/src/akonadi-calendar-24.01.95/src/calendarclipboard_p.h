/*
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "calendarbase.h"
#include "calendarclipboard.h"
#include "incidencechanger.h"
#include <KCalendarCore/Incidence>
#include <QList>
#include <QSet>

namespace KCalUtils
{
class DndFactory;
}

namespace Akonadi
{
class IncidenceChanger;

class CalendarClipboardPrivate : public QObject
{
    Q_OBJECT
public:
    CalendarClipboardPrivate(const Akonadi::CalendarBase::Ptr &calendar, Akonadi::IncidenceChanger *changer, CalendarClipboard *qq);

    ~CalendarClipboardPrivate() override;

    /**
     * Returns all uids of incidenes having @p incidence has their parent (or grand parent, etc.)
     * @p incidence's uid is included in the list too.
     */
    void getIncidenceHierarchy(const KCalendarCore::Incidence::Ptr &incidence, QStringList &uids);

    /**
     * Copies all these incidences to clipboard. Deletes them.
     * This function assumes the caller already unparented all children ( made them independent ).
     */
    void cut(const KCalendarCore::Incidence::List &incidences);

    /**
     * Overload.
     */
    void cut(const KCalendarCore::Incidence::Ptr &incidence);

    /**
     * All immediate children of @p incidence are made independent.
     * Their RELATED-TO field is cleared.
     *
     * After it's done, signal makeChildsIndependentFinished() is emitted.
     */
    void makeChildsIndependent(const KCalendarCore::Incidence::Ptr &incidence);

public Q_SLOTS:
    void slotModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);

    void slotDeleteFinished(int changeId, const QList<Akonadi::Item::Id> &ids, Akonadi::IncidenceChanger::ResultCode result, const QString &errorMessage);

public:
    Akonadi::CalendarBase::Ptr m_calendar;
    Akonadi::IncidenceChanger *m_changer = nullptr;
    KCalUtils::DndFactory *m_dndfactory = nullptr;
    bool m_abortCurrentOperation = false;
    QSet<int> m_pendingChangeIds;
    CalendarClipboard *const q;
};
}
