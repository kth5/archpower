/*
   SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

#include <QObject>

#include <memory>

namespace Akonadi
{
class IncidenceChanger;
class TodoPurgerPrivate;

/**
 * @short Class to delete completed to-dos.
 *
 * @author Sérgio Martins <iamsergio@gmail.com>
 * @since 4.12
 */
class AKONADI_CALENDAR_EXPORT TodoPurger : public QObject
{
    Q_OBJECT
public:
    explicit TodoPurger(QObject *parent = nullptr);
    ~TodoPurger() override;

    /**
     * Sets an IncidenceChanger.
     * If you don't call this method, an internal IncidenceChanger will be created.
     * Use this if you want more control over the deletion operations, like iTip management, ACL, undo/redo support.
     */
    void setIncidenceChager(IncidenceChanger *changer);

    /**
     * Sets the calendar to be used for retrieving the to-do hierarchy.
     * If you don't call this method, an internal FetchJobCalendar will be created.
     * Use this if you want to reuse an existing calendar, for performance reasons for example.
     */
    void setCalendar(const CalendarBase::Ptr &calendar);

    /**
     * Deletes completed to-dos. A to-do with incomplete children won't be deleted.
     * @see purgeCompletedTodos()
     */
    void purgeCompletedTodos();

    /**
     * Use this after receiving the an unsuccessful todosPurged() signal to get a i18n error message.
     */
    [[nodiscard]] QString lastError() const;

Q_SIGNALS:
    /**
     * Emitted when purging completed to-dos finished.
     * @param success    True if the operation could be completed. @see lastError()
     * @param numDeleted Number of to-dos that were deleted.
     * @param numIgnored Number of completed to-dos that weren't deleted because they are read-only
     *                   or have incomplete or read-only children.
     */
    void todosPurged(bool success, int numDeleted, int numIgnored);

private:
    std::unique_ptr<TodoPurgerPrivate> const d;
};
}
