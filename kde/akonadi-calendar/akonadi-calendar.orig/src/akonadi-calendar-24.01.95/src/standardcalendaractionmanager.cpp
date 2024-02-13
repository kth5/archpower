/*
 *  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
 *  SPDX-FileCopyrightText: 2009-2010 Tobias Koenig <tokoe@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "standardcalendaractionmanager.h"

#include <Akonadi/EntityTreeModel>

#include <KActionCollection>
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KLocalizedString>
#include <QAction>

#include <QItemSelectionModel>

using namespace Akonadi;

class Akonadi::StandardCalendarActionManagerPrivate
{
public:
    StandardCalendarActionManagerPrivate(KActionCollection *actionCollection, QWidget *parentWidget, StandardCalendarActionManager *parent)
        : mActionCollection(actionCollection)
        , mParentWidget(parentWidget)
        , mParent(parent)
    {
        mGenericManager = new StandardActionManager(actionCollection, parentWidget);
        mParent->connect(mGenericManager, &StandardActionManager::actionStateUpdated, mParent, &StandardCalendarActionManager::actionStateUpdated);
        mGenericManager->setMimeTypeFilter(QStringList() << QStringLiteral("text/calendar"));
        mGenericManager->setCapabilityFilter(QStringList() << QStringLiteral("Resource"));
    }

    ~StandardCalendarActionManagerPrivate()
    {
        delete mGenericManager;
    }

    void updateGenericAction(StandardActionManager::Type type)
    {
        switch (type) {
        case Akonadi::StandardActionManager::CreateCollection:
            mGenericManager->action(Akonadi::StandardActionManager::CreateCollection)->setText(i18n("Add Folder..."));
            mGenericManager->action(Akonadi::StandardActionManager::CreateCollection)
                ->setWhatsThis(i18n("Add a new calendar folder to the currently selected calendar folder."));
            mGenericManager->setContextText(StandardActionManager::CreateCollection, StandardActionManager::DialogTitle, i18nc("@title:window", "New Folder"));

            mGenericManager->setContextText(StandardActionManager::CreateCollection,
                                            StandardActionManager::ErrorMessageText,
                                            ki18n("Could not create calendar folder: %1"));

            mGenericManager->setContextText(StandardActionManager::CreateCollection,
                                            StandardActionManager::ErrorMessageTitle,
                                            i18nc("@title:window", "Calendar Folder Creation Failed"));
            mGenericManager->action(Akonadi::StandardActionManager::CreateCollection)
                ->setProperty("ContentMimeTypes",
                              QStringList() << QStringLiteral("inode/directory") << QStringLiteral("application/x-vnd.akonadi.calendar.todo")
                                            << QStringLiteral("application/x-vnd.akonadi.calendar.event")
                                            << QStringLiteral("application/x-vnd.akonadi.calendar.journal"));

            break;
        case Akonadi::StandardActionManager::CopyCollections:
            mGenericManager->setActionText(Akonadi::StandardActionManager::CopyCollections, ki18np("Copy Folder", "Copy %1 Folders"));
            mGenericManager->action(Akonadi::StandardActionManager::CopyCollections)
                ->setWhatsThis(i18n("Copy the selected calendar folders to the clipboard."));
            break;
        case Akonadi::StandardActionManager::DeleteCollections:
            mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteCollections, ki18np("Delete Folder", "Delete %1 Folders"));
            mGenericManager->action(Akonadi::StandardActionManager::DeleteCollections)
                ->setWhatsThis(i18n("Delete the selected calendar folders from the calendar."));
            mGenericManager->setContextText(StandardActionManager::DeleteCollections,
                                            StandardActionManager::MessageBoxText,
                                            ki18np("Do you really want to delete this calendar folder and all its sub-folders?",
                                                   "Do you really want to delete %1 calendar folders and all their sub-folders?"));

            mGenericManager->setContextText(StandardActionManager::DeleteCollections,
                                            StandardActionManager::MessageBoxTitle,
                                            ki18ncp("@title:window", "Delete Calendar Folder?", "Delete Calendar Folders?"));

            mGenericManager->setContextText(StandardActionManager::DeleteCollections,
                                            StandardActionManager::ErrorMessageText,
                                            ki18n("Could not delete calendar folder: %1"));

            mGenericManager->setContextText(StandardActionManager::DeleteCollections,
                                            StandardActionManager::ErrorMessageTitle,
                                            i18nc("@title:window", "Calendar Folder Deletion Failed"));

            break;
        case Akonadi::StandardActionManager::SynchronizeCollections:
            mGenericManager->setActionText(Akonadi::StandardActionManager::SynchronizeCollections, ki18np("Update Folder", "Update %1 Folders"));
            mGenericManager->action(Akonadi::StandardActionManager::SynchronizeCollections)
                ->setWhatsThis(i18n("Update the content of the selected calendar folders."));

            break;
        case Akonadi::StandardActionManager::CutCollections:
            mGenericManager->setActionText(Akonadi::StandardActionManager::CutCollections, ki18np("Cut Folder", "Cut %1 Folders"));
            mGenericManager->action(Akonadi::StandardActionManager::CutCollections)->setWhatsThis(i18n("Cut the selected calendar folders from the calendar."));
            break;
        case Akonadi::StandardActionManager::CollectionProperties:
            mGenericManager->action(Akonadi::StandardActionManager::CollectionProperties)->setText(i18n("Folder Properties..."));
            mGenericManager->action(Akonadi::StandardActionManager::CollectionProperties)
                ->setWhatsThis(i18n("Open a dialog to edit the properties of the selected calendar folder."));
            mGenericManager->setContextText(StandardActionManager::CollectionProperties,
                                            StandardActionManager::DialogTitle,
                                            ki18nc("@title:window", "Properties of Calendar Folder %1"));
            break;
        case Akonadi::StandardActionManager::CopyItems:
            mGenericManager->setActionText(Akonadi::StandardActionManager::CopyItems, ki18np("Copy Event", "Copy %1 Events"));
            mGenericManager->action(Akonadi::StandardActionManager::CopyItems)->setWhatsThis(i18n("Copy the selected events to the clipboard."));

            break;
        case Akonadi::StandardActionManager::DeleteItems:
            mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteItems, ki18np("Delete Event", "Delete %1 Events"));
            mGenericManager->action(Akonadi::StandardActionManager::DeleteItems)->setWhatsThis(i18n("Delete the selected events from the calendar."));
            mGenericManager->setContextText(StandardActionManager::DeleteItems,
                                            StandardActionManager::MessageBoxText,
                                            ki18np("Do you really want to delete the selected event?", "Do you really want to delete %1 events?"));

            mGenericManager->setContextText(StandardActionManager::DeleteItems,
                                            StandardActionManager::MessageBoxTitle,
                                            ki18ncp("@title:window", "Delete Event?", "Delete Events?"));

            mGenericManager->setContextText(StandardActionManager::DeleteItems, StandardActionManager::ErrorMessageText, ki18n("Could not delete event: %1"));

            mGenericManager->setContextText(StandardActionManager::DeleteItems,
                                            StandardActionManager::ErrorMessageTitle,
                                            i18nc("@title:window", "Event Deletion Failed"));
            break;

        case Akonadi::StandardActionManager::CutItems:
            mGenericManager->setActionText(Akonadi::StandardActionManager::CutItems, ki18np("Cut Event", "Cut %1 Events"));
            mGenericManager->action(Akonadi::StandardActionManager::CutItems)->setWhatsThis(i18n("Cut the selected events from the calendar."));
            break;
        case Akonadi::StandardActionManager::CreateResource:
            mGenericManager->action(Akonadi::StandardActionManager::CreateResource)->setText(i18n("Add &Calendar..."));
            mGenericManager->action(Akonadi::StandardActionManager::CreateResource)
                ->setWhatsThis(i18n("Add a new calendar<p>"
                                    "You will be presented with a dialog where you can select "
                                    "the type of the calendar that shall be added.</p>"));
            mGenericManager->setContextText(StandardActionManager::CreateResource, StandardActionManager::DialogTitle, i18nc("@title:window", "Add Calendar"));

            mGenericManager->setContextText(StandardActionManager::CreateResource,
                                            StandardActionManager::ErrorMessageText,
                                            ki18n("Could not create calendar: %1"));

            mGenericManager->setContextText(StandardActionManager::CreateResource,
                                            StandardActionManager::ErrorMessageTitle,
                                            i18nc("@title:window", "Calendar Creation Failed"));

            break;
        case Akonadi::StandardActionManager::DeleteResources:

            mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteResources, ki18np("&Delete Calendar", "&Delete %1 Calendars"));
            mGenericManager->action(Akonadi::StandardActionManager::DeleteResources)
                ->setWhatsThis(i18n("Delete the selected calendars<p>"
                                    "The currently selected calendars will be deleted, "
                                    "along with all the events, todos and journals they contain.</p>"));
            mGenericManager->setContextText(StandardActionManager::DeleteResources,
                                            StandardActionManager::MessageBoxText,
                                            ki18np("Do you really want to delete this calendar?", "Do you really want to delete %1 calendars?"));

            mGenericManager->setContextText(StandardActionManager::DeleteResources,
                                            StandardActionManager::MessageBoxTitle,
                                            ki18ncp("@title:window", "Delete Calendar?", "Delete Calendars?"));

            break;
        case Akonadi::StandardActionManager::ResourceProperties:
            mGenericManager->action(Akonadi::StandardActionManager::ResourceProperties)->setText(i18n("Calendar Properties..."));
            mGenericManager->action(Akonadi::StandardActionManager::ResourceProperties)
                ->setWhatsThis(i18n("Open a dialog to edit properties of the selected calendar."));
            break;
        case Akonadi::StandardActionManager::SynchronizeResources:

            mGenericManager->setActionText(Akonadi::StandardActionManager::SynchronizeResources, ki18np("Update Calendar", "Update %1 Calendars"));
            mGenericManager->action(Akonadi::StandardActionManager::SynchronizeResources)
                ->setWhatsThis(i18n("Updates the content of all folders of the selected calendars."));
            break;
        case Akonadi::StandardActionManager::CopyItemToMenu:
            mGenericManager->action(Akonadi::StandardActionManager::CopyItemToMenu)->setText(i18n("&Copy to Calendar"));
            mGenericManager->action(Akonadi::StandardActionManager::CopyItemToMenu)->setWhatsThis(i18n("Copy the selected event to a different calendar."));
            break;
        case Akonadi::StandardActionManager::MoveItemToMenu:
            mGenericManager->action(Akonadi::StandardActionManager::MoveItemToMenu)->setText(i18n("&Move to Calendar"));
            mGenericManager->action(Akonadi::StandardActionManager::MoveItemToMenu)->setWhatsThis(i18n("Move the selected event to a different calendar."));
            break;
        case StandardActionManager::Paste:
            mGenericManager->setContextText(StandardActionManager::Paste, StandardActionManager::ErrorMessageText, ki18n("Could not paste event: %1"));

            mGenericManager->setContextText(StandardActionManager::Paste, StandardActionManager::ErrorMessageTitle, i18nc("@title:window", "Paste Failed"));
            break;
        case Akonadi::StandardActionManager::SynchronizeCollectionTree:
            mGenericManager->action(Akonadi::StandardActionManager::SynchronizeCollectionTree)->setText(i18n("Update Available Calendars"));
            mGenericManager->action(Akonadi::StandardActionManager::SynchronizeCollectionTree)->setWhatsThis(i18n("Updates the tree of available calendars."));

            break;
        default:
            break;
        }
    }

    void updateGenericAllActions()
    {
        updateGenericAction(StandardActionManager::CreateCollection);
        updateGenericAction(StandardActionManager::CopyCollections);
        updateGenericAction(StandardActionManager::DeleteCollections);
        updateGenericAction(StandardActionManager::SynchronizeCollections);
        updateGenericAction(StandardActionManager::CollectionProperties);
        updateGenericAction(StandardActionManager::CopyItems);
        updateGenericAction(StandardActionManager::Paste);
        updateGenericAction(StandardActionManager::DeleteItems);
        updateGenericAction(StandardActionManager::ManageLocalSubscriptions);
        updateGenericAction(StandardActionManager::AddToFavoriteCollections);
        updateGenericAction(StandardActionManager::RemoveFromFavoriteCollections);
        updateGenericAction(StandardActionManager::RenameFavoriteCollection);
        updateGenericAction(StandardActionManager::CopyCollectionToMenu);
        updateGenericAction(StandardActionManager::CopyItemToMenu);
        updateGenericAction(StandardActionManager::MoveItemToMenu);
        updateGenericAction(StandardActionManager::MoveCollectionToMenu);
        updateGenericAction(StandardActionManager::CutItems);
        updateGenericAction(StandardActionManager::CutCollections);
        updateGenericAction(StandardActionManager::CreateResource);
        updateGenericAction(StandardActionManager::DeleteResources);
        updateGenericAction(StandardActionManager::ResourceProperties);
        updateGenericAction(StandardActionManager::SynchronizeResources);
        updateGenericAction(StandardActionManager::ToggleWorkOffline);
        updateGenericAction(StandardActionManager::CopyCollectionToDialog);
        updateGenericAction(StandardActionManager::MoveCollectionToDialog);
        updateGenericAction(StandardActionManager::CopyItemToDialog);
        updateGenericAction(StandardActionManager::MoveItemToDialog);
        updateGenericAction(StandardActionManager::SynchronizeCollectionsRecursive);
        updateGenericAction(StandardActionManager::MoveCollectionsToTrash);
        updateGenericAction(StandardActionManager::MoveItemsToTrash);
        updateGenericAction(StandardActionManager::RestoreCollectionsFromTrash);
        updateGenericAction(StandardActionManager::RestoreItemsFromTrash);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreCollection);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreCollectionAlternative);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreItem);
        updateGenericAction(StandardActionManager::MoveToTrashRestoreItemAlternative);
        updateGenericAction(StandardActionManager::SynchronizeFavoriteCollections);
        updateGenericAction(StandardActionManager::SynchronizeCollectionTree);
    }

    static bool hasWritableCollection(const QModelIndex &index, const QString &mimeType)
    {
        const auto collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        if (collection.isValid()) {
            if (collection.contentMimeTypes().contains(mimeType) && (collection.rights() & Akonadi::Collection::CanCreateItem)) {
                return true;
            }
        }

        const QAbstractItemModel *model = index.model();
        if (!model) {
            return false;
        }

        for (int row = 0; row < model->rowCount(index); ++row) {
            if (hasWritableCollection(model->index(row, 0, index), mimeType)) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] bool hasWritableCollection(const QString &mimeType) const
    {
        if (!mCollectionSelectionModel) {
            return false;
        }

        const QAbstractItemModel *collectionModel = mCollectionSelectionModel->model();
        for (int row = 0; row < collectionModel->rowCount(); ++row) {
            if (hasWritableCollection(collectionModel->index(row, 0, QModelIndex()), mimeType)) {
                return true;
            }
        }

        return false;
    }

    void updateActions()
    {
        if (!mItemSelectionModel) {
            return;
        }

        // update action labels
        const int itemCount = mItemSelectionModel->selectedRows().count();
        if (itemCount == 1) {
            const QModelIndex index = mItemSelectionModel->selectedRows().at(0);
            if (index.isValid()) {
                const QString mimeType = index.data(EntityTreeModel::MimeTypeRole).toString();
                if (mimeType == KCalendarCore::Event::eventMimeType()) {
                    if (mGenericManager->action(Akonadi::StandardActionManager::CopyItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CopyItems, ki18np("Copy Event", "Copy %1 Events"));
                    }
                    QAction *act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToMenu);
                    if (act) {
                        act->setText(i18n("Copy Event To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToDialog);
                    if (act) {
                        act->setText(i18n("Copy Event To"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::DeleteItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteItems, ki18np("Delete Event", "Delete %1 Events"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::CutItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CutItems, ki18np("Cut Event", "Cut %1 Events"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToMenu);
                    if (act) {
                        act->setText(i18n("Move Event To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToDialog);
                    if (act) {
                        act->setText(i18n("Move Event To"));
                    }
                    act = mActions.value(StandardCalendarActionManager::EditIncidence);
                    if (act) {
                        act->setText(i18n("Edit Event..."));
                    }
                } else if (mimeType == KCalendarCore::Todo::todoMimeType()) {
                    if (mGenericManager->action(Akonadi::StandardActionManager::CopyItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CopyItems, ki18np("Copy To-do", "Copy %1 To-dos"));
                    }
                    QAction *act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToMenu);
                    if (act) {
                        act->setText(i18n("Copy To-do To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToDialog);
                    if (act) {
                        act->setText(i18n("Copy To-do To"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::DeleteItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteItems, ki18np("Delete To-do", "Delete %1 To-dos"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::CutItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CutItems, ki18np("Cut To-do", "Cut %1 To-dos"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToMenu);
                    if (act) {
                        act->setText(i18n("Move To-do To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToDialog);
                    if (act) {
                        act->setText(i18n("Move To-do To"));
                    }
                    act = mActions.value(StandardCalendarActionManager::EditIncidence);
                    if (act) {
                        act->setText(i18n("Edit To-do..."));
                    }
                } else if (mimeType == KCalendarCore::Journal::journalMimeType()) {
                    if (mGenericManager->action(Akonadi::StandardActionManager::CopyItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CopyItems, ki18np("Copy Journal", "Copy %1 Journals"));
                    }
                    QAction *act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToMenu);
                    if (act) {
                        act->setText(i18n("Copy Journal To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::CopyItemToDialog);
                    if (act) {
                        act->setText(i18n("Copy Journal To"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::DeleteItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::DeleteItems, ki18np("Delete Journal", "Delete %1 Journals"));
                    }
                    if (mGenericManager->action(Akonadi::StandardActionManager::CutItems)) {
                        mGenericManager->setActionText(Akonadi::StandardActionManager::CutItems, ki18np("Cut Journal", "Cut %1 Journals"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToMenu);
                    if (act) {
                        act->setText(i18n("Move Journal To"));
                    }
                    act = mGenericManager->action(Akonadi::StandardActionManager::MoveItemToDialog);
                    if (act) {
                        act->setText(i18n("Move Journal To"));
                    }
                    act = mActions.value(StandardCalendarActionManager::EditIncidence);
                    if (act) {
                        act->setText(i18n("Edit Journal..."));
                    }
                }
            }
        }

        // update action states
        QAction *act = mActions.value(StandardCalendarActionManager::CreateEvent);
        if (act) {
            act->setEnabled(hasWritableCollection(KCalendarCore::Event::eventMimeType()));
        }
        act = mActions.value(StandardCalendarActionManager::CreateTodo);
        if (act) {
            act->setEnabled(hasWritableCollection(KCalendarCore::Todo::todoMimeType()));
        }
        act = mActions.value(StandardCalendarActionManager::CreateJournal);
        if (act) {
            act->setEnabled(hasWritableCollection(KCalendarCore::Journal::journalMimeType()));
        }

        act = mActions.value(StandardCalendarActionManager::EditIncidence);
        if (act) {
            bool canEditItem = true;

            // only one selected item can be edited
            canEditItem = canEditItem && (itemCount == 1);

            // check whether parent collection allows changing the item
            const QModelIndexList rows = mItemSelectionModel->selectedRows();
            if (rows.count() == 1) {
                const QModelIndex index = rows.first();
                const auto parentCollection = index.data(EntityTreeModel::ParentCollectionRole).value<Collection>();
                if (parentCollection.isValid()) {
                    canEditItem = canEditItem && (parentCollection.rights() & Collection::CanChangeItem);
                }
            }

            act->setEnabled(canEditItem);
        }

        act = mActions.value(StandardCalendarActionManager::CreateSubTodo);
        if (act) {
            act->setEnabled(false);
        }

        if (itemCount == 1) {
            const Akonadi::Item item = mGenericManager->selectedItems().at(0);
            if (item.isValid() && item.hasPayload<KCalendarCore::Todo::Ptr>()) {
                if (QAction *actSubTodo = mActions.value(StandardCalendarActionManager::CreateSubTodo)) {
                    actSubTodo->setEnabled(hasWritableCollection(KCalendarCore::Todo::todoMimeType()));
                }
            }
        }

        Q_EMIT mParent->actionStateUpdated();
    }

    void slotCreateEvent()
    {
        // dummy as long as there are no editors available in kdepimlibs/
    }

    void slotCreateTodo()
    {
        // dummy as long as there are no editors available in kdepimlibs/
    }

    void slotCreateSubTodo()
    {
        // dummy as long as there are no editors available in kdepimlibs/
    }

    void slotCreateJournal()
    {
        // dummy as long as there are no editors available in kdepimlibs/
    }

    void slotEditIncidence()
    {
        // dummy as long as there are no editors available in kdepimlibs/
    }

    KActionCollection *mActionCollection = nullptr;
    QWidget *mParentWidget = nullptr;
    StandardActionManager *mGenericManager = nullptr;
    QItemSelectionModel *mCollectionSelectionModel = nullptr;
    QItemSelectionModel *mItemSelectionModel = nullptr;
    QHash<StandardCalendarActionManager::Type, QAction *> mActions;
    QSet<StandardCalendarActionManager::Type> mInterceptedActions;
    StandardCalendarActionManager *const mParent;
};

Akonadi::StandardCalendarActionManager::StandardCalendarActionManager(KActionCollection *actionCollection, QWidget *parent)
    : QObject(parent)
    , d(new StandardCalendarActionManagerPrivate(actionCollection, parent, this))
{
}

StandardCalendarActionManager::~StandardCalendarActionManager() = default;

void StandardCalendarActionManager::setCollectionSelectionModel(QItemSelectionModel *selectionModel)
{
    d->mCollectionSelectionModel = selectionModel;
    d->mGenericManager->setCollectionSelectionModel(selectionModel);

    connect(selectionModel->model(), &QAbstractItemModel::rowsInserted, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel->model(), &QAbstractItemModel::rowsRemoved, this, [this]() {
        d->updateActions();
    });
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        d->updateActions();
    });
    d->updateActions();
}

void StandardCalendarActionManager::setItemSelectionModel(QItemSelectionModel *selectionModel)
{
    d->mItemSelectionModel = selectionModel;
    d->mGenericManager->setItemSelectionModel(selectionModel);

    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, [this]() {
        d->updateActions();
    });

    d->updateActions();
}

QAction *StandardCalendarActionManager::createAction(StandardCalendarActionManager::Type type)
{
    QAction *action = d->mActions.value(type);
    if (action) {
        return action;
    }

    switch (type) {
    case CreateEvent:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("appointment-new")));
        action->setText(i18n("New E&vent..."));
        action->setWhatsThis(i18n("Create a new event"));
        d->mActions.insert(CreateEvent, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_event_create"), action);
        connect(action, &QAction::triggered, this, [this]() {
            d->slotCreateEvent();
        });
        break;
    case CreateTodo:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("task-new")));
        action->setText(i18n("New &To-do..."));
        action->setWhatsThis(i18n("Create a new To-do"));
        d->mActions.insert(CreateTodo, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_todo_create"), action);
        connect(action, &QAction::triggered, this, [this]() {
            d->slotCreateTodo();
        });
        break;
    case CreateSubTodo:
        action = new QAction(d->mParentWidget);
        action->setText(i18n("New Su&b-to-do..."));
        action->setWhatsThis(i18n("Create a new Sub-to-do"));
        d->mActions.insert(CreateSubTodo, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_subtodo_create"), action);
        connect(action, &QAction::triggered, this, [this]() {
            d->slotCreateSubTodo();
        });
        break;
    case CreateJournal:
        action = new QAction(d->mParentWidget);
        action->setIcon(QIcon::fromTheme(QStringLiteral("journal-new")));
        action->setText(i18n("New &Journal..."));
        action->setWhatsThis(i18n("Create a new Journal"));
        d->mActions.insert(CreateJournal, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_journal_create"), action);
        connect(action, &QAction::triggered, this, [this]() {
            d->slotCreateJournal();
        });
        break;
    case EditIncidence:
        action = new QAction(d->mParentWidget);
        action->setText(i18n("&Edit..."));
        action->setWhatsThis(i18n("Edit the selected incidence."));
        d->mActions.insert(EditIncidence, action);
        d->mActionCollection->addAction(QStringLiteral("akonadi_incidence_edit"), action);
        connect(action, &QAction::triggered, this, [this]() {
            d->slotEditIncidence();
        });
        break;
    default:
        Q_ASSERT(false); // should never happen
        break;
    }

    return action;
}

QAction *StandardCalendarActionManager::createAction(StandardActionManager::Type type)
{
    QAction *act = d->mGenericManager->action(type);
    if (!act) {
        act = d->mGenericManager->createAction(type);
    }
    d->updateGenericAction(type);
    return act;
}

void StandardCalendarActionManager::createAllActions()
{
    createAction(CreateEvent);
    createAction(CreateTodo);
    createAction(CreateSubTodo);
    createAction(CreateJournal);
    createAction(EditIncidence);

    d->mGenericManager->createAllActions();
    d->updateGenericAllActions();
    d->updateActions();
}

QAction *StandardCalendarActionManager::action(StandardCalendarActionManager::Type type) const
{
    return d->mActions.value(type);
}

QAction *StandardCalendarActionManager::action(StandardActionManager::Type type) const
{
    return d->mGenericManager->action(type);
}

void StandardCalendarActionManager::setActionText(StandardActionManager::Type type, const KLocalizedString &text)
{
    d->mGenericManager->setActionText(type, text);
}

void StandardCalendarActionManager::interceptAction(StandardCalendarActionManager::Type type, bool intercept)
{
    if (intercept) {
        d->mInterceptedActions.insert(type);
    } else {
        d->mInterceptedActions.remove(type);
    }
}

void StandardCalendarActionManager::interceptAction(StandardActionManager::Type type, bool intercept)
{
    d->mGenericManager->interceptAction(type, intercept);
}

Akonadi::Collection::List StandardCalendarActionManager::selectedCollections() const
{
    return d->mGenericManager->selectedCollections();
}

Akonadi::Item::List StandardCalendarActionManager::selectedItems() const
{
    return d->mGenericManager->selectedItems();
}

void StandardCalendarActionManager::setContextText(StandardActionManager::Type type, StandardActionManager::TextContext context, const QString &text)
{
    d->mGenericManager->setContextText(type, context, text);
}

void StandardCalendarActionManager::setContextText(StandardActionManager::Type type, StandardActionManager::TextContext context, const KLocalizedString &text)
{
    d->mGenericManager->setContextText(type, context, text);
}

void StandardCalendarActionManager::setCollectionPropertiesPageNames(const QStringList &names)
{
    d->mGenericManager->setCollectionPropertiesPageNames(names);
}

#include "moc_standardcalendaractionmanager.cpp"
