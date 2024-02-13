/*
 *  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *  SPDX-FileCopyrightText: 2009-2010 Tobias Koenig <tokoe@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/StandardActionManager>
#include <KCalendarCore/Todo>

#include <QObject>

#include <memory>

// needed for windows ce, its defined somewhere
#undef CreateEvent

class QAction;
class KActionCollection;
class QItemSelectionModel;
class QWidget;

namespace Akonadi
{
class Item;
class StandardCalendarActionManagerPrivate;

/**
 * @short Manages calendar specific actions for collection and item views.
 *
 * @author Casey Link <unnamedrambler@gmail.com>
 * @since 4.6
 */
class AKONADI_CALENDAR_EXPORT StandardCalendarActionManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Describes the supported actions.
     */
    enum Type {
        CreateEvent = StandardActionManager::LastType + 1, ///< Creates a new event
        CreateTodo, ///< Creates a new todo
        CreateSubTodo, ///< Creates a new sub-todo
        CreateJournal, ///< Creates a new journal
        EditIncidence, ///< Edit currently selected event/todo/journal
        LastType ///< Marks last action
    };

    /**
     * Creates a new standard calendar action manager.
     *
     * @param actionCollection The action collection to operate on.
     * @param parent The parent widget.
     */
    explicit StandardCalendarActionManager(KActionCollection *actionCollection, QWidget *parent = nullptr);

    /**
     * Destroys the standard calendar action manager.
     */
    ~StandardCalendarActionManager() override;

    /**
     * Sets the collection selection model based on which the collection
     * related actions should operate. If none is set, all collection actions
     * will be disabled.
     * @param selectionModel the selection model for collections
     */
    void setCollectionSelectionModel(QItemSelectionModel *selectionModel);

    /**
     * Sets the item selection model based on which the item related actions
     * should operate. If none is set, all item actions will be disabled.
     * @param selectionModel the selection model for items
     */
    void setItemSelectionModel(QItemSelectionModel *selectionModel);

    /**
     * Creates the action of the given type and adds it to the action collection
     * specified in the constructor if it does not exist yet. The action is
     * connected to its default implementation provided by this class.
     * @param type the type of action to create
     */
    QAction *createAction(Type type);

    /**
     * Creates the action of the given type and adds it to the action collection
     * specified in the constructor if it does not exist yet. The action is
     * connected to its default implementation provided by this class.
     * @param type the type of action to create
     */
    QAction *createAction(StandardActionManager::Type type);

    /**
     * Convenience method to create all standard actions.
     * @see createAction()
     */
    void createAllActions();

    /**
     * Returns the action of the given type, 0 if it has not been created (yet).
     */
    QAction *action(Type type) const;

    /**
     * Returns the action of the given type, 0 if it has not been created (yet).
     * @param type the type of action to return
     */
    QAction *action(StandardActionManager::Type type) const;

    /**
     * Sets the label of the action @p type to @p text, which is used during
     * updating the action state and substituted according to the number of
     * selected objects. This is mainly useful to customize the label of actions
     * that can operate on multiple objects.
     *
     * Example:
     * @code
     * acctMgr->setActionText( Akonadi::StandardActionManager::CopyItems,
     *                         ki18np( "Copy Item", "Copy %1 Items" ) );
     * @endcode
     */
    void setActionText(StandardActionManager::Type type, const KLocalizedString &text);

    /**
     * Sets whether the default implementation for the given action @p type
     * shall be executed when the action is triggered.
     *
     * @param intercept If @c false, the default implementation will be executed,
     *                  if @c true no action is taken.
     */
    void interceptAction(Type type, bool intercept = true);

    /**
     * Sets whether the default implementation for the given action @p type
     * shall be executed when the action is triggered.
     *
     * @param intercept If @c false, the default implementation will be executed,
     *                  if @c true no action is taken.
     */
    void interceptAction(StandardActionManager::Type type, bool intercept = true);

    /**
     * Returns the list of collections that are currently selected.
     * The list is empty if no collection is currently selected.
     */
    [[nodiscard]] Akonadi::Collection::List selectedCollections() const;

    /**
     * Returns the list of items that are currently selected.
     * The list is empty if no item is currently selected.
     */
    [[nodiscard]] Akonadi::Item::List selectedItems() const;

    /**
     * Sets the @p text of the action @p type for the given @p context.
     */
    void setContextText(StandardActionManager::Type type, StandardActionManager::TextContext context, const QString &text);

    /**
     * Sets the @p text of the action @p type for the given @p context.
     */
    void setContextText(StandardActionManager::Type type, StandardActionManager::TextContext context, const KLocalizedString &text);

    void setCollectionPropertiesPageNames(const QStringList &names);

Q_SIGNALS:
    /**
     * This signal is emitted whenever the action state has been updated.
     * In case you have special needs for changing the state of some actions,
     * connect to this signal and adjust the action state.
     */
    void actionStateUpdated();

private:
    //@cond PRIVATE
    std::unique_ptr<StandardCalendarActionManagerPrivate> const d;
    //@endcond
};
}
