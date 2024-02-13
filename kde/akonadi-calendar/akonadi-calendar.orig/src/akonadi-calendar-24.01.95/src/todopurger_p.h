/*
   SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidencechanger.h"
#include "todopurger.h"

#include <QObject>
#include <QString>

namespace Akonadi
{
class IncidenceChanger;

class TodoPurgerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit TodoPurgerPrivate(TodoPurger *q);
    IncidenceChanger *m_changer = nullptr;
    QString m_lastError;
    CalendarBase::Ptr m_calendar;
    int m_currentChangeId = -1;
    int m_ignoredItems = 0;
    bool m_calendarOwnership = true; // If false it's not ours.

    void deleteTodos();
    [[nodiscard]] bool treeIsDeletable(const KCalendarCore::Todo::Ptr &todo);

public Q_SLOTS:
    void onCalendarLoaded(bool success, const QString &message);
    void onItemsDeleted(int changeId, const QList<Akonadi::Item::Id> &deletedItems, Akonadi::IncidenceChanger::ResultCode, const QString &message);

private:
    TodoPurger *const q;
};
}
