/*
  SPDX-FileCopyrightText: 2011-2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "calendarbase.h"
#include "incidencechanger.h"

#include <QList>
#include <QMultiHash>

namespace Akonadi
{
class CalendarBasePrivate : public QObject
{
    Q_OBJECT
public:
    explicit CalendarBasePrivate(CalendarBase *qq);
    ~CalendarBasePrivate() override;

    AKONADI_CALENDAR_EXPORT void internalInsert(const Akonadi::Item &item);
    AKONADI_CALENDAR_EXPORT void internalRemove(const Akonadi::Item &item);

    void deleteAllIncidencesOfType(const QString &mimeType);

    void handleUidChange(const Akonadi::Item &oldItem, const Akonadi::Item &newItem, const QString &newUid);

    // Checks if parent changed and adjust internal hierarchy info
    void handleParentChanged(const KCalendarCore::Incidence::Ptr &incidence);

public Q_SLOTS:
    void slotDeleteFinished(int changeId, const QList<Akonadi::Item::Id> &itemIds, Akonadi::IncidenceChanger::ResultCode, const QString &errorMessage);

    void slotCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode, const QString &errorMessage);

    void slotModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode, const QString &errorMessage);

    void collectionFetchResult(KJob *job);

Q_SIGNALS:
    void fetchFinished();

public:
    QMultiHash<Akonadi::Collection::Id, Akonadi::Item> mItemsByCollection;
    QHash<Akonadi::Collection::Id, Akonadi::Collection> mCollections;
    QHash<KJob *, Akonadi::Collection::Id> mCollectionJobs;
    QHash<QString, Akonadi::Item::Id> mItemIdByUid;
    QHash<Akonadi::Item::Id, Akonadi::Item> mItemById;
    Akonadi::IncidenceChanger *const mIncidenceChanger;
    QHash<QString, QStringList> mParentUidToChildrenUid;
    Akonadi::Collection mCollectionForBatchInsertion;
    bool mBatchInsertionCancelled = false;
    bool mListensForNewItems = false; // does this model detect new item creations ?
    bool mLastCreationCancelled = false; // User pressed cancel in the collection selection dialog

    // Hash with uid->parentUid. When receiving onDataChanged() we need a way
    // to obtain the original RELATED-TO. Because RELATED-TO might have been modified
    // we can't trust the incidence stored in the calendar. ( Users of this class don't
    // operate on a incidence clone, they change the same incidence that's inside the calendar )
    QHash<QString, QString> mUidToParent;

private:
    CalendarBase *const q;
};
}
