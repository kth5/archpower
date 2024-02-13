/*
   SPDX-FileCopyrightText: 2011 Sérgio Martins <sergio.martins@kdab.com>
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarbase.h"
#include "akonadicalendar_debug.h"
#include "calendarbase_p.h"
#include "calendarutils.h"
#include "incidencechanger.h"
#include <Akonadi/CollectionFetchJob>

#include <KLocalizedString>
#include <QTimeZone>

using namespace Akonadi;
using namespace KCalendarCore;

static QString itemToString(const Akonadi::Item &item)
{
    const KCalendarCore::Incidence::Ptr &incidence = CalendarUtils::incidence(item);
    QString str;
    QTextStream stream(&str);
    stream << item.id() << "; summary=" << incidence->summary() << "; uid=" << incidence->uid() << "; type=" << incidence->type()
           << "; recurs=" << incidence->recurs() << "; recurrenceId=" << incidence->recurrenceId().toString() << "; dtStart=" << incidence->dtStart().toString()
           << "; dtEnd=" << incidence->dateTime(Incidence::RoleEnd).toString() << "; parentCollection=" << item.storageCollectionId()
           << item.parentCollection().displayName();

    return str;
}

CalendarBasePrivate::CalendarBasePrivate(CalendarBase *qq)
    : QObject()
    , mIncidenceChanger(new IncidenceChanger())
    , q(qq)
{
    connect(mIncidenceChanger, &IncidenceChanger::createFinished, this, &CalendarBasePrivate::slotCreateFinished);

    connect(mIncidenceChanger, &IncidenceChanger::deleteFinished, this, &CalendarBasePrivate::slotDeleteFinished);

    connect(mIncidenceChanger, &IncidenceChanger::modifyFinished, this, &CalendarBasePrivate::slotModifyFinished);

    mIncidenceChanger->setDestinationPolicy(IncidenceChanger::DestinationPolicyAsk);
    mIncidenceChanger->setGroupwareCommunication(false);
    mIncidenceChanger->setHistoryEnabled(false);
}

CalendarBasePrivate::~CalendarBasePrivate()
{
    delete mIncidenceChanger;
}

void CalendarBasePrivate::internalInsert(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
    KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);

    if (!incidence) {
        qCritical() << "Incidence is null. id=" << item.id() << "; hasPayload()=" << item.hasPayload()
                    << "; has incidence=" << item.hasPayload<KCalendarCore::Incidence::Ptr>() << "; mime type=" << item.mimeType();
        Q_ASSERT(false);
        return;
    }

    // qCDebug(AKONADICALENDAR_LOG) << "Inserting incidence in calendar. id=" << item.id() << "uid=" << incidence->uid();
    const QString uid = incidence->instanceIdentifier();

    if (uid.isEmpty()) {
        // This code path should never happen
        qCritical() << "Incidence has empty UID. id=" << item.id() << "; summary=" << incidence->summary() << "Please fix it. Ignoring this incidence.";
        return;
    }

    if (mItemIdByUid.contains(uid) && mItemIdByUid[uid] != item.id()) {
        // We only allow duplicate UIDs if they have the same item id, for example
        // when using virtual folders.
#if 0
        qCWarning(AKONADICALENDAR_LOG) << "Discarding duplicate incidence with instanceIdentifier=" << uid
                                       << "and summary " << incidence->summary()
                                       << "; recurrenceId() =" << incidence->recurrenceId()
                                       << "; new id=" << item.id()
                                       << "; existing id=" << mItemIdByUid[uid];
#endif
        return;
    }

    if (incidence->type() == KCalendarCore::Incidence::TypeEvent && !incidence->dtStart().isValid()) {
        // TODO: make the parser discard them would also be a good idea
        qCWarning(AKONADICALENDAR_LOG) << "Discarding event with invalid DTSTART. identifier=" << incidence->instanceIdentifier()
                                       << "; summary=" << incidence->summary();
        return;
    }

    Akonadi::Collection collection = item.parentCollection();
    if (collection.isValid()) {
        // Some items don't have collection set
        if (item.storageCollectionId() != collection.id() && item.storageCollectionId() > -1) {
            if (mCollections.contains(item.storageCollectionId())) {
                collection = mCollections.value(item.storageCollectionId());
                incidence->setReadOnly(!(collection.rights() & Akonadi::Collection::CanChangeItem));
            } else if (!mCollectionJobs.key(item.storageCollectionId())) {
                collection = Akonadi::Collection(item.storageCollectionId());
                auto job = new Akonadi::CollectionFetchJob(collection, Akonadi::CollectionFetchJob::Base, this);
                QObject::connect(job, &KJob::result, this, &CalendarBasePrivate::collectionFetchResult);
                mCollectionJobs.insert(job, collection.id());
            }
        } else {
            mCollections.insert(collection.id(), collection);
            incidence->setReadOnly(!(collection.rights() & Akonadi::Collection::CanChangeItem));
        }
    }

    mItemById.insert(item.id(), item);
    mItemIdByUid.insert(uid, item.id());
    mItemsByCollection.insert(item.storageCollectionId(), item);

    if (!incidence->hasRecurrenceId()) {
        // Insert parent relationships
        const QString parentUid = incidence->relatedTo();
        if (!parentUid.isEmpty()) {
            mParentUidToChildrenUid[parentUid].append(incidence->uid());
            mUidToParent.insert(uid, parentUid);
        }
    }

    incidence->setCustomProperty("VOLATILE", "AKONADI-ID", QString::number(item.id()));
    incidence->setCustomProperty("VOLATILE", "COLLECTION-ID", QString::number(item.storageCollectionId()));
    // Must be the last one due to re-entrancy
    const bool result = q->MemoryCalendar::addIncidence(incidence);
    if (!result) {
        qCritical() << "Error adding incidence " << itemToString(item);
        Q_ASSERT(false);
    }
}

void CalendarBasePrivate::collectionFetchResult(KJob *job)
{
    Akonadi::Collection::Id colid = mCollectionJobs.take(job);

    if (job->error()) {
        qWarning() << "Error occurred: " << job->errorString();
        return;
    }

    auto fetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);

    const Akonadi::Collection collection = fetchJob->collections().at(0);
    if (collection.id() != colid) {
        qCritical() << "Fetched the wrong collection,  should fetch: " << colid << "fetched: " << collection.id();
    }

    bool isReadOnly = !(collection.rights() & Akonadi::Collection::CanChangeItem);
    const auto lst = mItemsByCollection.values(collection.id());
    for (const Akonadi::Item &item : lst) {
        KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
        incidence->setReadOnly(isReadOnly);
    }

    mCollections.insert(collection.id(), collection);

    if (mCollectionJobs.isEmpty()) {
        Q_EMIT fetchFinished();
    }
}

void CalendarBasePrivate::internalRemove(const Akonadi::Item &item)
{
    Q_ASSERT(item.isValid());

    Incidence::Ptr tmp = CalendarUtils::incidence(item);
    if (!tmp) {
        qCritical() << "CalendarBase::internalRemove1: incidence is null, item.id=" << item.id();
        return;
    }

    // We want the one stored in the calendar
    Incidence::Ptr incidence = q->incidence(tmp->uid(), tmp->recurrenceId());

    // Null incidence means it was deleted via CalendarBase::deleteIncidence(), but then
    // the ETMCalendar received the monitor notification and tried to delete it again.
    if (incidence) {
        q->Calendar::notifyIncidenceAboutToBeDeleted(incidence);

        mItemById.remove(item.id());
        // qCDebug(AKONADICALENDAR_LOG) << "Deleting incidence from calendar .id=" << item.id() << "uid=" << incidence->uid();
        mItemIdByUid.remove(incidence->instanceIdentifier());

        mItemsByCollection.remove(item.storageCollectionId(), item);

        if (!incidence->hasRecurrenceId()) {
            const QString uid = incidence->uid();
            const QString parentUid = incidence->relatedTo();
            mParentUidToChildrenUid.remove(uid);
            if (!parentUid.isEmpty()) {
                mParentUidToChildrenUid[parentUid].removeAll(uid);
                mUidToParent.remove(uid);
            }
        }

        q->Calendar::setObserversEnabled(false);
        // Must be the last one due to re-entrancy
        const bool result = q->MemoryCalendar::deleteIncidence(incidence);
        q->Calendar::setObserversEnabled(true);
        q->Calendar::notifyIncidenceDeleted(incidence);
        if (!result) {
            qCritical() << "Error removing incidence " << itemToString(item);
            Q_ASSERT(false);
        }
    } else {
        qCWarning(AKONADICALENDAR_LOG) << "CalendarBase::internalRemove2: incidence is null, item.id=" << itemToString(item);
    }
}

void CalendarBasePrivate::slotDeleteFinished(int changeId,
                                             const QList<Akonadi::Item::Id> &itemIds,
                                             IncidenceChanger::ResultCode resultCode,
                                             const QString &errorMessage)
{
    Q_UNUSED(changeId)
    if (resultCode == IncidenceChanger::ResultCodeSuccess) {
        for (const Akonadi::Item::Id &id : itemIds) {
            if (mItemById.contains(id)) {
                internalRemove(mItemById.value(id));
            }
        }
    }

    Q_EMIT q->deleteFinished(resultCode == IncidenceChanger::ResultCodeSuccess, errorMessage);
}

void CalendarBasePrivate::slotCreateFinished(int changeId, const Akonadi::Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    Q_UNUSED(changeId)
    Q_UNUSED(item)
    if (resultCode == IncidenceChanger::ResultCodeSuccess && !mListensForNewItems) {
        Q_ASSERT(item.isValid());
        Q_ASSERT(item.hasPayload<KCalendarCore::Incidence::Ptr>());
        internalInsert(item);
    }

    mLastCreationCancelled = (resultCode == IncidenceChanger::ResultCodeUserCanceled);

    Q_EMIT q->createFinished(resultCode == IncidenceChanger::ResultCodeSuccess, errorMessage);
}

void CalendarBasePrivate::slotModifyFinished(int changeId, const Akonadi::Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    Q_UNUSED(changeId)
    Q_UNUSED(item)
    QString message = errorMessage;
    if (resultCode == IncidenceChanger::ResultCodeSuccess) {
        KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
        Q_ASSERT(incidence);
        KCalendarCore::Incidence::Ptr localIncidence = q->incidence(incidence->instanceIdentifier());

        if (localIncidence) {
            // update our local one
            *(static_cast<KCalendarCore::IncidenceBase *>(localIncidence.data())) = *(incidence.data());
        } else {
            // This shouldn't happen, unless the incidence gets deleted between event loops
            qCWarning(AKONADICALENDAR_LOG) << "CalendarBasePrivate::slotModifyFinished() Incidence was deleted already probably? id=" << item.id();
            message = i18n("Could not find incidence to update, it probably was deleted recently.");
            resultCode = IncidenceChanger::ResultCodeAlreadyDeleted;
        }
    }
    Q_EMIT q->modifyFinished(resultCode == IncidenceChanger::ResultCodeSuccess, message);
}

void CalendarBasePrivate::handleUidChange(const Akonadi::Item &oldItem, const Akonadi::Item &newItem, const QString &newIdentifier)
{
    Q_ASSERT(oldItem.isValid());
    Incidence::Ptr newIncidence = CalendarUtils::incidence(newItem);
    Q_ASSERT(newIncidence);
    Incidence::Ptr oldIncidence = CalendarUtils::incidence(oldItem);
    Q_ASSERT(oldIncidence);

    const QString newUid = newIncidence->uid();
    if (mItemIdByUid.contains(newIdentifier)) {
        Incidence::Ptr oldIncidence = CalendarUtils::incidence(oldItem);
#if 0
        qCWarning(AKONADICALENDAR_LOG) << "New uid shouldn't be known: "  << newIdentifier << "; id="
                                       << newItem.id() << "; oldItem.id=" << mItemIdByUid[newIdentifier]
                                       << "; new summary= " << newIncidence->summary()
                                       << "; new recurrenceId=" << newIncidence->recurrenceId()
                                       << "; oldIncidence" << oldIncidence;
#endif
        if (oldIncidence) {
#if 0
            qCWarning(AKONADICALENDAR_LOG) << "; oldIncidence uid=" << oldIncidence->uid()
                                           << "; oldIncidence recurrenceId = " << oldIncidence->recurrenceId()
                                           << "; oldIncidence summary = " << oldIncidence->summary();
#endif
        }
        Q_ASSERT(false);
        return;
    }

    mItemIdByUid[newIdentifier] = newItem.id();

    // Get the real pointer
    oldIncidence = q->MemoryCalendar::incidence(oldIncidence->uid());

    if (!oldIncidence) {
        // How can this happen ?
        qCWarning(AKONADICALENDAR_LOG) << "Couldn't find old incidence";
        Q_ASSERT(false);
        return;
    }

    if (newIncidence->instanceIdentifier() == oldIncidence->instanceIdentifier()) {
#if 0
        qCWarning(AKONADICALENDAR_LOG) << "New uid=" << newIncidence->uid() << "; old uid=" << oldIncidence->uid()
                                       << "; new recurrenceId="
                                       << newIncidence->recurrenceId()
                                       << "; old recurrenceId=" << oldIncidence->recurrenceId()
                                       << "; new summary = " << newIncidence->summary()
                                       << "; old summary = " << oldIncidence->summary()
                                       << "; id = " << newItem.id();
#endif
        Q_ASSERT(false); // The reason we're here in the first place
        return;
    }

    mItemIdByUid.remove(oldIncidence->instanceIdentifier());
    const QString oldUid = oldIncidence->uid();

    if (mParentUidToChildrenUid.contains(oldUid)) {
        Q_ASSERT(!mParentUidToChildrenUid.contains(newIdentifier));
        QStringList children = mParentUidToChildrenUid.value(oldUid);
        mParentUidToChildrenUid.insert(newIdentifier, children);
        mParentUidToChildrenUid.remove(oldUid);
    }

    // Update internal maps of the base class, MemoryCalendar
    q->setObserversEnabled(false);
    q->MemoryCalendar::deleteIncidence(oldIncidence);
    q->MemoryCalendar::addIncidence(newIncidence);

    newIncidence->setUid(oldUid); // We set and unset just to notify observers of a change.
    q->setObserversEnabled(true);
    newIncidence->setUid(newUid);
}

void CalendarBasePrivate::handleParentChanged(const KCalendarCore::Incidence::Ptr &newIncidence)
{
    Q_ASSERT(newIncidence);

    if (newIncidence->hasRecurrenceId()) { // These ones don't/shouldn't have a parent
        return;
    }

    const QString originalParentUid = mUidToParent.value(newIncidence->uid());
    const QString newParentUid = newIncidence->relatedTo();

    if (originalParentUid == newParentUid) {
        return; // nothing changed
    }

    if (!originalParentUid.isEmpty()) {
        // Remove this child from it's old parent:
        Q_ASSERT(mParentUidToChildrenUid.contains(originalParentUid));
        mParentUidToChildrenUid[originalParentUid].removeAll(newIncidence->uid());
    }

    mUidToParent.remove(newIncidence->uid());

    if (!newParentUid.isEmpty()) {
        // Deliver this child to it's new parent:
        Q_ASSERT(!mParentUidToChildrenUid[newParentUid].contains(newIncidence->uid()));
        mParentUidToChildrenUid[newParentUid].append(newIncidence->uid());
        mUidToParent.insert(newIncidence->uid(), newParentUid);
    }
}

CalendarBase::CalendarBase(QObject *parent)
    : MemoryCalendar(QTimeZone::systemTimeZone())
    , d_ptr(new CalendarBasePrivate(this))
{
    setParent(parent);
}

CalendarBase::CalendarBase(CalendarBasePrivate *const dd, QObject *parent)
    : MemoryCalendar(QTimeZone::systemTimeZone())
    , d_ptr(dd)
{
    setParent(parent);
}

CalendarBase::~CalendarBase() = default;

Akonadi::Item CalendarBase::item(Akonadi::Item::Id id) const
{
    Q_D(const CalendarBase);
    Akonadi::Item i;
    auto it = d->mItemById.constFind(id);
    if (it != d->mItemById.cend()) {
        i = *it;
    } else {
        qCDebug(AKONADICALENDAR_LOG) << "Can't find any item with id " << id;
    }
    return i;
}

Akonadi::Item CalendarBase::item(const QString &uid) const
{
    Q_D(const CalendarBase);
    Akonadi::Item i;

    if (uid.isEmpty()) {
        return i;
    }

    auto it = d->mItemIdByUid.constFind(uid);
    if (it != d->mItemIdByUid.cend()) {
        const Akonadi::Item::Id id = *it;
        auto it2 = d->mItemById.constFind(id);
        if (it2 == d->mItemById.cend()) {
            qCritical() << "Item with id " << id << "(uid=" << uid << ") not found, but in uid map";
            Q_ASSERT_X(false, "CalendarBase::item", "not in mItemById");
        } else {
            i = *it2;
        }
    } else {
        qCDebug(AKONADICALENDAR_LOG) << "Can't find any incidence with uid " << uid;
    }
    return i;
}

Item CalendarBase::item(const Incidence::Ptr &incidence) const
{
    return incidence ? item(incidence->instanceIdentifier()) : Item();
}

Akonadi::Item::List CalendarBase::items(Akonadi::Collection::Id id) const
{
    Q_D(const CalendarBase);

    Akonadi::Item::List result;
    if (id == -1) {
        result.reserve(d->mItemsByCollection.size());
    }

    auto it = id == -1 ? d->mItemsByCollection.cbegin() : d->mItemsByCollection.constFind(id);
    while (it != d->mItemsByCollection.cend() && (id == -1 || it.key() == id)) {
        result.push_back(*it);
        ++it;
    }

    return result;
}

Akonadi::Item::List CalendarBase::itemList(const KCalendarCore::Incidence::List &incidences) const
{
    Akonadi::Item::List items;
    items.reserve(incidences.size());

    for (const KCalendarCore::Incidence::Ptr &incidence : incidences) {
        if (incidence) {
            items << item(incidence->instanceIdentifier());
        } else {
            items << Akonadi::Item();
        }
    }

    return items;
}

KCalendarCore::Incidence::List CalendarBase::childIncidences(Akonadi::Item::Id parentId) const
{
    Q_D(const CalendarBase);
    KCalendarCore::Incidence::List children;

    if (d->mItemById.contains(parentId)) {
        const Akonadi::Item item = d->mItemById.value(parentId);
        Q_ASSERT(item.isValid());
        KCalendarCore::Incidence::Ptr parent = CalendarUtils::incidence(item);

        if (parent) {
            children = childIncidences(parent->uid());
        } else {
            Q_ASSERT(false);
        }
    }

    return children;
}

KCalendarCore::Incidence::List CalendarBase::childIncidences(const QString &parentUid) const
{
    Q_D(const CalendarBase);
    KCalendarCore::Incidence::List children;
    const QStringList uids = d->mParentUidToChildrenUid.value(parentUid);
    for (const QString &uid : uids) {
        Incidence::Ptr child = incidence(uid);
        if (child) {
            children.append(child);
        } else {
            qCWarning(AKONADICALENDAR_LOG) << "Invalid child with uid " << uid;
        }
    }
    return children;
}

Akonadi::Item::List CalendarBase::childItems(Akonadi::Item::Id parentId) const
{
    Q_D(const CalendarBase);
    Akonadi::Item::List children;

    if (d->mItemById.contains(parentId)) {
        const Akonadi::Item item = d->mItemById.value(parentId);
        Q_ASSERT(item.isValid());
        KCalendarCore::Incidence::Ptr parent = CalendarUtils::incidence(item);

        if (parent) {
            children = childItems(parent->uid());
        } else {
            Q_ASSERT(false);
        }
    }

    return children;
}

Akonadi::Item::List CalendarBase::childItems(const QString &parentUid) const
{
    Q_D(const CalendarBase);
    Akonadi::Item::List children;
    const QStringList uids = d->mParentUidToChildrenUid.value(parentUid);
    for (const QString &uid : uids) {
        Akonadi::Item child = item(uid);
        if (child.isValid() && child.hasPayload<KCalendarCore::Incidence::Ptr>()) {
            children.append(child);
        } else {
            qCWarning(AKONADICALENDAR_LOG) << "Invalid child with uid " << uid;
        }
    }
    return children;
}

bool CalendarBase::addEvent(const KCalendarCore::Event::Ptr &event)
{
    return addIncidence(event);
}

bool CalendarBase::deleteEvent(const KCalendarCore::Event::Ptr &event)
{
    return deleteIncidence(event);
}

bool CalendarBase::addTodo(const KCalendarCore::Todo::Ptr &todo)
{
    return addIncidence(todo);
}

bool CalendarBase::deleteTodo(const KCalendarCore::Todo::Ptr &todo)
{
    return deleteIncidence(todo);
}

bool CalendarBase::addJournal(const KCalendarCore::Journal::Ptr &journal)
{
    return addIncidence(journal);
}

bool CalendarBase::deleteJournal(const KCalendarCore::Journal::Ptr &journal)
{
    return deleteIncidence(journal);
}

bool CalendarBase::addIncidence(const KCalendarCore::Incidence::Ptr &incidence)
{
    // TODO: Parent for dialogs
    Q_D(CalendarBase);

    // User canceled on the collection selection dialog
    if (batchAdding() && d->mBatchInsertionCancelled) {
        return false;
    }

    d->mLastCreationCancelled = false;

    Akonadi::Collection collection;

    if (batchAdding() && d->mCollectionForBatchInsertion.isValid()) {
        collection = d->mCollectionForBatchInsertion;
    }

    if (incidence->hasRecurrenceId() && !collection.isValid()) {
        // We are creating an exception, reuse the same collection that the main incidence uses
        Item mainItem = item(incidence->uid());
        if (mainItem.isValid()) {
            collection = Collection(mainItem.storageCollectionId());
        }
    }

    const int changeId = d->mIncidenceChanger->createIncidence(incidence, collection);

    if (batchAdding()) {
        const Akonadi::Collection lastCollection = d->mIncidenceChanger->lastCollectionUsed();
        if (changeId != -1 && !lastCollection.isValid()) {
            d->mBatchInsertionCancelled = true;
        } else if (lastCollection.isValid() && !d->mCollectionForBatchInsertion.isValid()) {
            d->mCollectionForBatchInsertion = d->mIncidenceChanger->lastCollectionUsed();
        }
    }

    return changeId != -1;
}

bool CalendarBase::deleteIncidence(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_D(CalendarBase);
    Q_ASSERT(incidence);
    if (!incidence->hasRecurrenceId() && incidence->recurs()) {
        deleteIncidenceInstances(incidence);
    }
    Akonadi::Item item_ = item(incidence->instanceIdentifier());
    return -1 != d->mIncidenceChanger->deleteIncidence(item_);
}

bool CalendarBase::modifyIncidence(const KCalendarCore::Incidence::Ptr &newIncidence)
{
    Q_D(CalendarBase);
    Q_ASSERT(newIncidence);
    Akonadi::Item item_ = item(newIncidence->instanceIdentifier());
    item_.setPayload<KCalendarCore::Incidence::Ptr>(newIncidence);
    return -1 != d->mIncidenceChanger->modifyIncidence(item_);
}

IncidenceChanger *CalendarBase::incidenceChanger() const
{
    Q_D(const CalendarBase);
    return d->mIncidenceChanger;
}

void CalendarBase::startBatchAdding()
{
    KCalendarCore::MemoryCalendar::startBatchAdding();
}

void CalendarBase::endBatchAdding()
{
    Q_D(CalendarBase);
    d->mCollectionForBatchInsertion = Akonadi::Collection();
    d->mBatchInsertionCancelled = false;
    KCalendarCore::MemoryCalendar::endBatchAdding();
}

#include "moc_calendarbase.cpp"
#include "moc_calendarbase_p.cpp"
