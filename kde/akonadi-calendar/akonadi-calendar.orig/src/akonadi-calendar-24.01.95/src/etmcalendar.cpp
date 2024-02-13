/*
   SPDX-FileCopyrightText: 2011-2013 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "etmcalendar.h"
#include "blockalarmsattribute.h"
#include "calendarmodel_p.h"
#include "calendarutils.h"
#include "calfilterpartstatusproxymodel_p.h"
#include "calfilterproxymodel_p.h"
#include "etmcalendar_p.h"
#include "kcolumnfilterproxymodel_p.h"

#include <Akonadi/CollectionFilterProxyModel>
#include <Akonadi/CollectionUtils>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <KDescendantsProxyModel>
#include <KSelectionProxyModel>

#include <QItemSelectionModel>
#include <QTreeView>

using namespace Akonadi;
using namespace KCalendarCore;

// TODO: implement batchAdding

ETMCalendarPrivate::ETMCalendarPrivate(ETMCalendar *qq)
    : CalendarBasePrivate(qq)
    , mETM(nullptr)
    , q(qq)
{
    mListensForNewItems = true;
}

void ETMCalendarPrivate::init()
{
    if (!mETM) {
        auto session = new Akonadi::Session("ETMCalendar", q);
        auto monitor = new Akonadi::Monitor(q);
        monitor->setObjectName(QLatin1StringView("ETMCalendarMonitor"));
        connect(monitor,
                qOverload<const Akonadi::Collection &, const QSet<QByteArray> &>(&Monitor::collectionChanged),
                this,
                [this](const Akonadi::Collection &cols, const QSet<QByteArray> &set) {
                    onCollectionChanged(cols, set);
                });

        Akonadi::ItemFetchScope scope;
        scope.fetchFullPayload(true);
        scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

        monitor->setSession(session);
        monitor->setCollectionMonitored(Akonadi::Collection::root());
        monitor->fetchCollection(true);
        monitor->setItemFetchScope(scope);
        monitor->setAllMonitored(true);

        const QStringList allMimeTypes = {KCalendarCore::Event::eventMimeType(),
                                          KCalendarCore::Todo::todoMimeType(),
                                          KCalendarCore::Journal::journalMimeType()};

        for (const QString &mimetype : allMimeTypes) {
            monitor->setMimeTypeMonitored(mimetype, mMimeTypes.isEmpty() || mMimeTypes.contains(mimetype));
        }

        mETM = CalendarModel::create(monitor);
        mETM->setObjectName(QLatin1StringView("ETM"));
        mETM->setListFilter(Akonadi::CollectionFetchScope::Display);
    }

    setupFilteredETM();

    connect(q, &Calendar::filterChanged, this, &ETMCalendarPrivate::onFilterChanged);

    connect(mETM.data(), &EntityTreeModel::collectionPopulated, this, &ETMCalendarPrivate::onCollectionPopulated);
    connect(mETM.data(), &QAbstractItemModel::rowsInserted, this, &ETMCalendarPrivate::onRowsInserted);
    connect(mETM.data(), &QAbstractItemModel::dataChanged, this, &ETMCalendarPrivate::onDataChanged);
    connect(mETM.data(), &QAbstractItemModel::rowsMoved, this, &ETMCalendarPrivate::onRowsMoved);
    connect(mETM.data(), &QAbstractItemModel::rowsRemoved, this, &ETMCalendarPrivate::onRowsRemoved);

    connect(mFilteredETM, &QAbstractItemModel::dataChanged, this, &ETMCalendarPrivate::onDataChangedInFilteredModel);
    connect(mFilteredETM, &QAbstractItemModel::layoutChanged, this, &ETMCalendarPrivate::onLayoutChangedInFilteredModel);
    connect(mFilteredETM, &QAbstractItemModel::modelReset, this, &ETMCalendarPrivate::onModelResetInFilteredModel);
    connect(mFilteredETM, &QAbstractItemModel::rowsInserted, this, &ETMCalendarPrivate::onRowsInsertedInFilteredModel);
    connect(mFilteredETM, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ETMCalendarPrivate::onRowsAboutToBeRemovedInFilteredModel);

    loadFromETM();
    updateLoadingState();
}

void ETMCalendarPrivate::onCollectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &attributeNames)
{
    Q_ASSERT(collection.isValid());
    // Is the collection changed to read-only, we update all Incidences
    if (attributeNames.contains("AccessRights")) {
        const Akonadi::Item::List items = q->items();
        for (const Akonadi::Item &item : items) {
            if (item.storageCollectionId() == collection.id()) {
                KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
                if (incidence) {
                    incidence->setReadOnly(!(collection.rights() & Akonadi::Collection::CanChangeItem));
                }
            }
        }
    }

    Q_EMIT q->collectionChanged(collection, attributeNames);
}

void ETMCalendarPrivate::setupFilteredETM()
{
    // We're only interested in the CollectionTitle column
    auto columnFilterProxy = new KColumnFilterProxyModel(this);
    columnFilterProxy->setSourceModel(mETM.data());
    columnFilterProxy->setVisibleColumn(CalendarModel::CollectionTitle);
    columnFilterProxy->setObjectName(QLatin1StringView("Remove columns"));

    mCollectionProxyModel = new Akonadi::CollectionFilterProxyModel(this);
    mCollectionProxyModel->setObjectName(QLatin1StringView("Only show collections"));
    mCollectionProxyModel->setDynamicSortFilter(true);
    mCollectionProxyModel->addMimeTypeFilter(QStringLiteral("text/calendar"));
    mCollectionProxyModel->setExcludeVirtualCollections(false);
    mCollectionProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mCollectionProxyModel->setSourceModel(columnFilterProxy);

    // Keep track of selected items.
    auto selectionModel = new QItemSelectionModel(mCollectionProxyModel);
    selectionModel->setObjectName(QLatin1StringView("Calendar Selection Model"));

    // Make item selection work by means of checkboxes.
    mCheckableProxyModel = new CheckableProxyModel(this);
    mCheckableProxyModel->setSelectionModel(selectionModel);
    mCheckableProxyModel->setSourceModel(mCollectionProxyModel);
    mCheckableProxyModel->setObjectName(QLatin1StringView("Add checkboxes"));

    mSelectionProxy = new KSelectionProxyModel(selectionModel, /**parent=*/this);
    mSelectionProxy->setObjectName(QLatin1StringView("Only show items of selected collection"));
    mSelectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
    mSelectionProxy->setSourceModel(mETM.data());

    mCalFilterProxyModel = new CalFilterProxyModel(this);
    mCalFilterProxyModel->setFilter(q->filter());
    mCalFilterProxyModel->setSourceModel(mSelectionProxy);
    mCalFilterProxyModel->setObjectName(QLatin1StringView("KCalendarCore::CalFilter filtering"));

    mCalFilterPartStatusProxyModel = new CalFilterPartStatusProxyModel(this);
    mCalFilterPartStatusProxyModel->setFilterVirtual(false);
    QList<KCalendarCore::Attendee::PartStat> blockedStatusList;
    blockedStatusList << KCalendarCore::Attendee::NeedsAction;
    blockedStatusList << KCalendarCore::Attendee::Declined;
    mCalFilterPartStatusProxyModel->setDynamicSortFilter(true);
    mCalFilterPartStatusProxyModel->setBlockedStatusList(blockedStatusList);
    mCalFilterPartStatusProxyModel->setSourceModel(mCalFilterProxyModel);
    mCalFilterPartStatusProxyModel->setObjectName(QLatin1StringView("PartStatus filtering"));

    mFilteredETM = new Akonadi::EntityMimeTypeFilterModel(this);
    mFilteredETM->setSourceModel(mCalFilterPartStatusProxyModel);
    mFilteredETM->setHeaderGroup(Akonadi::EntityTreeModel::ItemListHeaders);
    mFilteredETM->setSortRole(CalendarModel::SortRole);
    mFilteredETM->setObjectName(QLatin1StringView("Show headers"));

#ifdef AKONADI_CALENDAR_DEBUG_MODEL
    QTreeView *view = new QTreeView;
    view->setModel(mFilteredETM);
    view->show();
#endif
}

ETMCalendarPrivate::~ETMCalendarPrivate() = default;

void ETMCalendarPrivate::loadFromETM()
{
    itemsAdded(itemsFromModel(mFilteredETM));
}

void ETMCalendarPrivate::clear()
{
    mCollectionMap.clear();
    mItemsByCollection.clear();

    Akonadi::Item::List removedItems;
    removedItems.reserve(mItemById.size());
    for (auto it = mItemById.cbegin(), end = mItemById.cend(); it != end; ++it) {
        removedItems.push_back(it.value());
    }

    itemsRemoved(removedItems);

    if (!mItemById.isEmpty()) {
        mItemById.clear();
        // Q_ASSERT(false); // TODO: discover why this happens
    }

    if (!mItemIdByUid.isEmpty()) {
        mItemIdByUid.clear();
        // Q_ASSERT(false);
    }
    mParentUidToChildrenUid.clear();
    // m_virtualItems.clear();
}

Akonadi::Item::List ETMCalendarPrivate::itemsFromModel(const QAbstractItemModel *model, const QModelIndex &parentIndex, int start, int end)
{
    const int endRow = end >= 0 ? end : model->rowCount(parentIndex) - 1;
    Akonadi::Item::List items;
    int row = start;
    QModelIndex i = model->index(row, 0, parentIndex);
    while (row <= endRow) {
        const Akonadi::Item item = itemFromIndex(i);
        if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
            items << item;
        } else {
            const QModelIndex childIndex = model->index(0, 0, i);
            if (childIndex.isValid()) {
                items << itemsFromModel(model, i);
            }
        }
        ++row;
        i = i.sibling(row, 0);
    }
    return items;
}

Akonadi::Collection::List ETMCalendarPrivate::collectionsFromModel(const QAbstractItemModel *model, const QModelIndex &parentIndex, int start, int end)
{
    const int endRow = end >= 0 ? end : model->rowCount(parentIndex) - 1;
    Akonadi::Collection::List collections;
    int row = start;
    QModelIndex i = model->index(row, 0, parentIndex);
    while (row <= endRow) {
        const Akonadi::Collection collection = CollectionUtils::fromIndex(i);
        if (collection.isValid()) {
            collections << collection;
            QModelIndex childIndex = model->index(0, 0, i);
            if (childIndex.isValid()) {
                collections << collectionsFromModel(model, i);
            }
        }
        ++row;
        i = i.sibling(row, 0);
    }
    return collections;
}

Akonadi::Item ETMCalendarPrivate::itemFromIndex(const QModelIndex &idx)
{
    auto item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    item.setParentCollection(idx.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>());
    return item;
}

void ETMCalendarPrivate::itemsAdded(const Akonadi::Item::List &items)
{
    if (!items.isEmpty()) {
        for (const Akonadi::Item &item : items) {
            internalInsert(item);
        }

        Akonadi::Collection::Id id = items.first().storageCollectionId();
        if (mPopulatedCollectionIds.contains(id)) {
            // If the collection isn't populated yet, it will be sent later
            // Saves some cpu cycles
            Q_EMIT q->calendarChanged();
        }
    }
}

void ETMCalendarPrivate::itemsRemoved(const Akonadi::Item::List &items)
{
    for (const Akonadi::Item &item : items) {
        internalRemove(item);
    }
    Q_EMIT q->calendarChanged();
}

void ETMCalendarPrivate::onRowsInserted(const QModelIndex &index, int start, int end)
{
    const Akonadi::Collection::List collections = collectionsFromModel(mETM.data(), index, start, end);

    for (const Akonadi::Collection &collection : collections) {
        mCollectionMap[collection.id()] = collection;
    }

    if (!collections.isEmpty()) {
        Q_EMIT q->collectionsAdded(collections);
    }
}

void ETMCalendarPrivate::onCollectionPopulated(Akonadi::Collection::Id id)
{
    mPopulatedCollectionIds.insert(id);
    Q_EMIT q->calendarChanged();
    updateLoadingState();
}

void ETMCalendarPrivate::onRowsRemoved(const QModelIndex &index, int start, int end)
{
    const Akonadi::Collection::List collections = collectionsFromModel(mETM.data(), index, start, end);
    for (const Akonadi::Collection &collection : collections) {
        mCollectionMap.remove(collection.id());
    }

    if (!collections.isEmpty()) {
        Q_EMIT q->collectionsRemoved(collections);
    }
}

void ETMCalendarPrivate::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // We only update collections, because items are handled in the filtered model
    Q_ASSERT(topLeft.row() <= bottomRight.row());
    const int endRow = bottomRight.row();
    for (int row = topLeft.row(); row <= endRow; ++row) {
        const Akonadi::Collection col = CollectionUtils::fromIndex(topLeft.sibling(row, 0));
        if (col.isValid()) {
            // Attributes might have changed, store the new collection and discard the old one
            mCollectionMap.insert(col.id(), col);
        }
    }
}

void ETMCalendarPrivate::onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    // TODO
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)
}

void ETMCalendarPrivate::onLayoutChangedInFilteredModel()
{
    clear();
    loadFromETM();
}

void ETMCalendarPrivate::onModelResetInFilteredModel()
{
    clear();
    loadFromETM();
}

void ETMCalendarPrivate::onDataChangedInFilteredModel(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_ASSERT(topLeft.row() <= bottomRight.row());
    const int endRow = bottomRight.row();
    QModelIndex i(topLeft);
    int row = i.row();
    while (row <= endRow) {
        const Akonadi::Item item = itemFromIndex(i);
        if (item.isValid() && item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
            updateItem(item);
        }

        ++row;
        i = i.sibling(row, topLeft.column());
    }

    Q_EMIT q->calendarChanged();
}

void ETMCalendarPrivate::updateItem(const Akonadi::Item &item)
{
    Incidence::Ptr newIncidence = CalendarUtils::incidence(item);
    Q_ASSERT(newIncidence);
    Q_ASSERT(!newIncidence->uid().isEmpty());
    newIncidence->setCustomProperty("VOLATILE", "AKONADI-ID", QString::number(item.id()));
    IncidenceBase::Ptr existingIncidence = q->incidence(newIncidence->uid(), newIncidence->recurrenceId());

    if (!existingIncidence && !mItemById.contains(item.id())) {
        // We don't know about this one because it was discarded, for example because of not having DTSTART
        return;
    }

    mItemsByCollection.insert(item.storageCollectionId(), item);
    Akonadi::Item oldItem = mItemById.value(item.id());

    if (existingIncidence) {
        // We set the payload so that the internal incidence pointer and the one in mItemById stay the same
        Akonadi::Item updatedItem = item;
        updatedItem.setPayload<KCalendarCore::Incidence::Ptr>(existingIncidence.staticCast<KCalendarCore::Incidence>());
        mItemById.insert(item.id(), updatedItem); // The item needs updating too, revision changed.

        // Check if RELATED-TO changed, updating parenting information
        handleParentChanged(newIncidence);
        *(existingIncidence.data()) = *(newIncidence.data());
    } else {
        mItemById.insert(item.id(), item); // The item needs updating too, revision changed.
        // The item changed it's UID, update our maps, the Google resource changes the UID when we create incidences.
        handleUidChange(oldItem, item, newIncidence->instanceIdentifier());
    }
}

void ETMCalendarPrivate::onRowsInsertedInFilteredModel(const QModelIndex &index, int start, int end)
{
    itemsAdded(itemsFromModel(mFilteredETM, index, start, end));
}

void ETMCalendarPrivate::onRowsAboutToBeRemovedInFilteredModel(const QModelIndex &index, int start, int end)
{
    itemsRemoved(itemsFromModel(mFilteredETM, index, start, end));
}

void ETMCalendarPrivate::onFilterChanged()
{
    mCalFilterProxyModel->setFilter(q->filter());
}

ETMCalendar::ETMCalendar(QObject *parent)
    : CalendarBase(new ETMCalendarPrivate(this), parent)
{
    Q_D(ETMCalendar);
    d->init();
}

ETMCalendar::ETMCalendar(const QStringList &mimeTypes, QObject *parent)
    : CalendarBase(new ETMCalendarPrivate(this), parent)
{
    Q_D(ETMCalendar);
    d->mMimeTypes = mimeTypes;
    d->init();
}

ETMCalendar::ETMCalendar(ETMCalendar *other, QObject *parent)
    : CalendarBase(new ETMCalendarPrivate(this), parent)
{
    Q_D(ETMCalendar);

    auto model = qobject_cast<Akonadi::CalendarModel *>(other->entityTreeModel());
    if (model) {
        d->mETM = model->weakPointer().toStrongRef();
    }

    d->init();
}

ETMCalendar::ETMCalendar(Monitor *monitor, QObject *parent)
    : CalendarBase(new ETMCalendarPrivate(this), parent)
{
    Q_D(ETMCalendar);

    if (monitor) {
        QObject::connect(monitor,
                         qOverload<const Akonadi::Collection &, const QSet<QByteArray> &>(&Akonadi::Monitor::collectionChanged),
                         d,
                         &ETMCalendarPrivate::onCollectionChanged);
        d->mETM = CalendarModel::create(monitor);
        d->mETM->setObjectName(QLatin1StringView("ETM"));
        d->mETM->setListFilter(Akonadi::CollectionFetchScope::Display);
    }

    d->init();
}

ETMCalendar::~ETMCalendar() = default;

// TODO: move this up?
Akonadi::Collection ETMCalendar::collection(Akonadi::Collection::Id id) const
{
    Q_D(const ETMCalendar);
    return d->mCollectionMap.value(id);
}

bool ETMCalendar::hasRight(const QString &uid, Akonadi::Collection::Right right) const
{
    return hasRight(item(uid), right);
}

bool ETMCalendar::hasRight(const Akonadi::Item &item, Akonadi::Collection::Right right) const
{
    // if the users changes the rights, item.parentCollection()
    // can still have the old rights, so we use call collection()
    // which returns the updated one
    const Akonadi::Collection col = collection(item.storageCollectionId());
    return col.rights() & right;
}

QAbstractItemModel *ETMCalendar::model() const
{
    Q_D(const ETMCalendar);
    return d->mFilteredETM;
}

KCheckableProxyModel *ETMCalendar::checkableProxyModel() const
{
    Q_D(const ETMCalendar);
    return d->mCheckableProxyModel;
}

KCalendarCore::Alarm::List ETMCalendar::alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms) const
{
    Q_D(const ETMCalendar);
    KCalendarCore::Alarm::List alarmList;
    QHashIterator<Akonadi::Item::Id, Akonadi::Item> i(d->mItemById);
    while (i.hasNext()) {
        const Akonadi::Item item = i.next().value();

        Akonadi::Collection parentCollection; // must have same lifetime as blockedAttr
        BlockAlarmsAttribute *blockedAttr = nullptr;

        if (excludeBlockedAlarms) {
            // take the collection from m_collectionMap, because we need the up-to-date collection attrs
            parentCollection = d->mCollectionMap.value(item.storageCollectionId());
            if (parentCollection.isValid() && parentCollection.hasAttribute<BlockAlarmsAttribute>()) {
                blockedAttr = parentCollection.attribute<BlockAlarmsAttribute>();
                if (blockedAttr->isEverythingBlocked()) {
                    continue;
                }
            }
        }

        KCalendarCore::Incidence::Ptr incidence;
        if (item.isValid() && item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
            incidence = item.payload<KCalendarCore::Incidence::Ptr>();
        }
        if (!incidence || incidence->alarms().isEmpty()) {
            continue;
        }

        Alarm::List tmpList;
        if (incidence->recurs()) {
            appendRecurringAlarms(tmpList, incidence, from, to);
        } else {
            appendAlarms(tmpList, incidence, from, to);
        }

        if (blockedAttr) {
            tmpList.erase(std::remove_if(tmpList.begin(),
                                         tmpList.end(),
                                         [blockedAttr](const auto &alarm) {
                                             return blockedAttr->isAlarmTypeBlocked(alarm->type());
                                         }),
                          tmpList.end());
        }

        alarmList += tmpList;
    }
    return alarmList;
}

Akonadi::EntityTreeModel *ETMCalendar::entityTreeModel() const
{
    Q_D(const ETMCalendar);
    return d->mETM.data();
}

void ETMCalendar::setCollectionFilteringEnabled(bool enable)
{
    Q_D(ETMCalendar);
    if (d->mCollectionFilteringEnabled != enable) {
        d->mCollectionFilteringEnabled = enable;
        if (enable) {
            d->mSelectionProxy->setSourceModel(d->mETM.data());
            QAbstractItemModel *oldModel = d->mCalFilterProxyModel->sourceModel();
            d->mCalFilterProxyModel->setSourceModel(d->mSelectionProxy);
            delete qobject_cast<KDescendantsProxyModel *>(oldModel);
        } else {
            auto flatner = new KDescendantsProxyModel(this);
            flatner->setSourceModel(d->mETM.data());
            d->mCalFilterProxyModel->setSourceModel(flatner);
        }
    }
}

bool ETMCalendar::collectionFilteringEnabled() const
{
    Q_D(const ETMCalendar);
    return d->mCollectionFilteringEnabled;
}

void ETMCalendarPrivate::updateLoadingState()
{
    if (!q->entityTreeModel()->isCollectionTreeFetched()) {
        return q->setIsLoading(true);
    }

    for (const Akonadi::Collection &collection : std::as_const(mCollectionMap)) {
        if (!q->entityTreeModel()->isCollectionPopulated(collection.id())) {
            return q->setIsLoading(true);
        }
    }

    q->setIsLoading(false);
}

#include "moc_etmcalendar.cpp"
#include "moc_etmcalendar_p.cpp"
