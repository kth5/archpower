/*
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "incidencechanger.h"

#include <Akonadi/Item>
#include <KCalendarCore/Incidence>
#include <QWidget>

#include <memory>

class HistoryTest;

namespace Akonadi
{
class IncidenceChanger;
class HistoryPrivate;

/**
   @short History class for implementing undo/redo of calendar operations

   Whenever you use IncidenceChanger to create, delete or modify incidences,
   this class is used to record those changes onto a stack, so they can be
   undone or redone.

   If needed, groupware invitations will be sent to attendees and organizers when
   undoing or redoing changes.

   This class can't be instantiated directly, use it through IncidenceChanger:

   @code
      Akonadi::IncidenceChanger *myIncidenceChanger = new Akonadi::IncidenceChanger();
      connect(undoAction, &QAction::triggered, myIncidenceChanger->history(), &Akonadi::IncidenceChanger::undo);
      connect(redoAction, &QAction::triggered, myIncidenceChanger->history(), &Akonadi::IncidenceChanger::redo);
   @endcode

   @author Sérgio Martins <iamsergio@gmail.com>
   @since 4.11
*/

class AKONADI_CALENDAR_EXPORT History : public QObject
{
    Q_OBJECT
public:
    /**
     * This enum describes the possible result codes (success/error values) for an
     * undo or redo operation.
     * @see undone()
     * @see redone()
     */
    enum ResultCode {
        ResultCodeSuccess = 0, ///< Success
        ResultCodeError ///< An error occurred. Call lastErrorString() for the error message. This isn't very verbose because IncidenceChanger hasn't been
                        ///< refactored yet.
    };

    /**
     * Destroys the History instance.
     */
    ~History() override;

    /**
     * Pushes an incidence creation onto the undo stack. The creation can be
     * undone calling undo().
     *
     * @param item the item that was created. Must be valid and have a Incidence::Ptr payload
     * @param description text that can be used in the undo/redo menu item to describe the operation
     *        If empty, a default one will be provided.
     * @param atomicOperationId if not 0, specifies which group of changes this change belongs to.
     *        When a change is undone/redone, all other changes which are in the same group are also
     *        undone/redone
     */
    void recordCreation(const Akonadi::Item &item, const QString &description, const uint atomicOperationId = 0);

    /**
     * Pushes an incidence modification onto the undo stack. The modification can be undone calling
     * undo().
     *
     * @param oldItem item containing the payload before the change. Must be valid
     *        and contain an Incidence::Ptr payload.
     * @param newItem item containing the new payload. Must be valid and contain
     *        an Incidence::Ptr payload.
     * @param description text that can be used in the undo/redo menu item to describe the operation
     *        If empty, a default one will be provided.
     * @param atomicOperationId if not 0, specifies which group of changes this change belongs to.
     *        When a change is undone/redone, all other changes which are in the same group are also
     *        undone/redone
     */
    void recordModification(const Akonadi::Item &oldItem, const Akonadi::Item &newItem, const QString &description, const uint atomicOperationId = 0);

    /**
     * Pushes an incidence deletion onto the undo stack. The deletion can be
     * undone calling undo().
     *
     * @param item The item to delete. Must be valid, doesn't need to contain a
     *        payload.
     * @param description text that can be used in the undo/redo menu item to describe the operation
     *        If empty, a default one will be provided.
     * @param atomicOperationId if not 0, specifies which group of changes this change belongs to.
     *        When a change is undone/redone, all other changes which are in the same group are also
     *        undone/redone
     */
    void recordDeletion(const Akonadi::Item &item, const QString &description, const uint atomicOperationId = 0);

    /**
     * Pushes a list of incidence deletions onto the undo stack. The deletions can
     * be undone calling undo() once.
     *
     * @param items The list of items to delete. All items must be valid.
     * @param description text that can be used in the undo/redo menu item to describe the operation
     *        If empty, a default one will be provided.
     * @param atomicOperationId If != 0, specifies which group of changes thischange belongs to.
     *        Will be useful for atomic undoing/redoing, not implemented yet.
     */
    void recordDeletions(const Akonadi::Item::List &items, const QString &description, const uint atomicOperationId = 0);

    /**
     * Returns the last error message.
     *
     * Call this immediately after catching the undone()/redone() signal
     * with an ResultCode != ResultCodeSuccess.
     *
     * The message is translated.
     */
    [[nodiscard]] QString lastErrorString() const;

    /**
     * Reverts every change in the undo stack.
     *
     * @param parent will be passed to dialogs created by IncidenceChanger,
     *        for example those asking if you want to send invitations.
     */
    void undoAll(QWidget *parent = nullptr);

    /**
     * Returns true if there are changes that can be undone.
     */
    [[nodiscard]] bool undoAvailable() const;

    /**
     * Returns true if there are changes that can be redone.
     */
    [[nodiscard]] bool redoAvailable() const;

    /**
     * Returns the description of the next undo.
     *
     * This is the description that was passed when calling recordCreation(),
     * recordDeletion() or recordModification().
     *
     * @see nextRedoDescription()
     */
    [[nodiscard]] QString nextUndoDescription() const;

    /**
     * Returns the description of the next redo.
     *
     * This is the description that was passed when calling recordCreation(),
     * recordDeletion() or recordModification().
     *
     * @see nextUndoDescription()
     */
    [[nodiscard]] QString nextRedoDescription() const;

public Q_SLOTS:
    /**
     * Clears the undo and redo stacks.
     * Won't do anything if there's a undo/redo job currently running.
     *
     * @return true if the stacks were cleared, false if there was a job running
     */
    bool clear();

    /**
     * Reverts the change that's on top of the undo stack.
     * Can't be called if there's an undo/redo operation running, asserts.
     * Can be called if the stack is empty, in this case, nothing happens.
     * This function is async, catch signal undone() to know when the operation
     * finishes.
     *
     * @param parent will be passed to dialogs created by IncidenceChanger,
     *        for example those asking if you want to send invitations.
     *
     * @see redo()
     * @see undone()
     */
    void undo(QWidget *parent = nullptr);

    /**
     * Reverts the change that's on top of the redo stack.
     * Can't be called if there's an undo/redo operation running, asserts.
     * Can be called if the stack is empty, in this case, nothing happens.
     * This function is async, catch signal redone() to know when the operation
     * finishes.
     *
     * @param parent will be passed to dialogs created by IncidenceChanger,
     *        for example those asking if you want to send invitations.
     *
     * @see undo()
     * @see redone()
     */
    void redo(QWidget *parent = nullptr);

Q_SIGNALS:
    /**
     * This signal is emitted when an undo operation finishes.
     * @param resultCode History::ResultCodeSuccess on success.
     * @see lastErrorString()
     */
    void undone(Akonadi::History::ResultCode resultCode);

    /**
     * This signal is emitted when an redo operation finishes.
     * @param resultCode History::ResultCodeSuccess on success.
     * @see lastErrorString()
     */
    void redone(Akonadi::History::ResultCode resultCode);

    /**
     * The redo/undo stacks have changed.
     */
    void changed();

private:
    friend class ::HistoryTest;
    friend class IncidenceChanger;
    friend class IncidenceChangerPrivate;
    friend class Entry;

    // Only IncidenceChanger can create History classes
    explicit History(QObject *parent = nullptr);

    // Used by unit-tests
    Akonadi::IncidenceChanger *incidenceChanger() const;

private:
    //@cond PRIVATE
    std::unique_ptr<HistoryPrivate> const d;
    //@endcond
};
}
