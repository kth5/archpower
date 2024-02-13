/*
  SPDX-FileCopyrightText: 2011-2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "calendarbase_p.h"
#include "calendarmodel_p.h"
#include "etmcalendar.h"
#include "incidencechanger.h"

#include <Akonadi/EntityTreeModel>
#include <KCheckableProxyModel>

#include <QModelIndex>
#include <QSet>

class QAbstractItemModel;
class CheckableProxyModel;
class KSelectionProxyModel;

namespace Akonadi
{
class EntityTreeModel;
class EntityMimeTypeFilterModel;
class CollectionFilterProxyModel;
class CalFilterProxyModel;
class CalFilterPartStatusProxyModel;

static bool isStructuralCollection(const Akonadi::Collection &collection)
{
    const QStringList mimeTypes = QStringList() << QStringLiteral("text/calendar") << KCalendarCore::Event::eventMimeType()
                                                << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType();
    const QStringList collectionMimeTypes = collection.contentMimeTypes();
    for (const QString &mimeType : mimeTypes) {
        if (collectionMimeTypes.contains(mimeType)) {
            return false;
        }
    }

    return true;
}

class CheckableProxyModel : public KCheckableProxyModel
{
    Q_OBJECT
public:
    explicit CheckableProxyModel(QObject *parent = nullptr)
        : KCheckableProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::CheckStateRole) {
            // Don't show the checkbox if the collection can't contain incidences
            const auto collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (isStructuralCollection(collection)) {
                return {};
            }
        }
        return KCheckableProxyModel::data(index, role);
    }
};

class ETMCalendarPrivate : public CalendarBasePrivate
{
    Q_OBJECT
public:
    explicit ETMCalendarPrivate(ETMCalendar *qq);
    ~ETMCalendarPrivate() override;

    void init();
    void setupFilteredETM();
    void loadFromETM();

public Q_SLOTS:
    Akonadi::Item::List itemsFromModel(const QAbstractItemModel *model, const QModelIndex &parentIndex = QModelIndex(), int start = 0, int end = -1);

    Akonadi::Collection::List
    collectionsFromModel(const QAbstractItemModel *model, const QModelIndex &parentIndex = QModelIndex(), int start = 0, int end = -1);

    // KCalendarCore::CalFilter has changed.
    void onFilterChanged();

    void clear();
    void updateItem(const Akonadi::Item &item);
    Akonadi::Item itemFromIndex(const QModelIndex &idx);
    void itemsAdded(const Akonadi::Item::List &items);
    void itemsRemoved(const Akonadi::Item::List &items);

    void onRowsInserted(const QModelIndex &index, int start, int end);
    void onRowsRemoved(const QModelIndex &index, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);

    void onLayoutChangedInFilteredModel();
    void onModelResetInFilteredModel();
    void onDataChangedInFilteredModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onRowsInsertedInFilteredModel(const QModelIndex &index, int start, int end);
    void onRowsAboutToBeRemovedInFilteredModel(const QModelIndex &index, int start, int end);
    void onCollectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &attributeNames);
    void onCollectionPopulated(Akonadi::Collection::Id);

    void updateLoadingState();

public:
    Akonadi::CalendarModel::Ptr mETM;
    Akonadi::EntityMimeTypeFilterModel *mFilteredETM = nullptr;

    // akonadi id to collections
    QHash<Akonadi::Collection::Id, Akonadi::Collection> mCollectionMap;
    CheckableProxyModel *mCheckableProxyModel = nullptr;
    Akonadi::CollectionFilterProxyModel *mCollectionProxyModel = nullptr;
    Akonadi::CalFilterProxyModel *mCalFilterProxyModel = nullptr; // KCalendarCore::CalFilter stuff
    // filter out all invitations and declined events
    Akonadi::CalFilterPartStatusProxyModel *mCalFilterPartStatusProxyModel = nullptr;
    KSelectionProxyModel *mSelectionProxy = nullptr;
    bool mCollectionFilteringEnabled = true;
    QSet<Akonadi::Collection::Id> mPopulatedCollectionIds;
    QStringList mMimeTypes;

private:
    ETMCalendar *const q;
};
}
