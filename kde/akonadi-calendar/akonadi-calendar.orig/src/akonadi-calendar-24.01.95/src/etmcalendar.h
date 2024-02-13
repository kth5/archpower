/*
   SPDX-FileCopyrightText: 2011 Sérgio Martins <sergio.martins@kdab.com>
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

#include <Akonadi/Collection>

class QAbstractItemModel;
class KCheckableProxyModel;

namespace Akonadi
{
class Monitor;
class ETMCalendarPrivate;
class EntityTreeModel;

/**
 * @short A KCalendarCore::Calendar that uses an EntityTreeModel to populate itself.
 *
 * All non-idempotent KCalendarCore::Calendar methods interact with Akonadi.
 * If you need a need a non-persistent calendar use FetchJobCalendar.
 *
 * ETMCalendar allows to be populated with only a subset of your calendar items,
 * by using a KCheckableProxyModel to specify which collections to be used
 * and/or by setting a KCalendarCore::CalFilter.
 *
 * @see FetchJobCalendar
 * @see CalendarBase
 *
 * @author Sérgio Martins <iamsergio@gmail.com>
 * @since 4.11
 */
class AKONADI_CALENDAR_EXPORT ETMCalendar : public CalendarBase
{
    Q_OBJECT
public:
    enum CollectionColumn { CollectionTitle = 0, CollectionColumnCount };

    using Ptr = QSharedPointer<ETMCalendar>;

    /**
     * Constructs a new ETMCalendar. Loading begins immediately, asynchronously.
     */
    explicit ETMCalendar(QObject *parent = nullptr);

    /**
     * Constructs a new ETMCalendar that will only load the specified mime types.
     * Use this ctor to ignore journals or to-dos for example.
     * If no mime types are specified, all mime types will be used.
     */
    explicit ETMCalendar(const QStringList &mimeTypes, QObject *parent = nullptr);

    /**
     * Constructs a new ETMCalendar.
     *
     * This overload exists for optimization reasons, it allows to share an EntityTreeModel across
     * several ETMCalendars to save memory.
     *
     * Usually when having many ETMCalendars, the only bit that's different is the collection
     * selection. The memory hungry EntityTreeModel is the same, so should be shared.
     *
     * @param calendar an existing ETMCalendar who's EntityTreeModel is to be used.
     *
     * @since 4.13
     */
    explicit ETMCalendar(ETMCalendar *calendar, QObject *parent = nullptr);

    explicit ETMCalendar(Monitor *monitor, QObject *parent = nullptr);
    /**
     * Destroys this ETMCalendar.
     */
    ~ETMCalendar() override;

    /**
     * Returns the collection having @p id.
     * Use this instead of creating a new collection, the returned collection will have
     * the correct right, name, display name, etc all set.
     */
    [[nodiscard]] Akonadi::Collection collection(Akonadi::Collection::Id) const;

    /**
     * Returns true if the collection owning incidence @p has righ @p right
     */
    [[nodiscard]] bool hasRight(const Akonadi::Item &item, Akonadi::Collection::Right right) const;

    /**
     * This is an overloaded function.
     * @param uid the identifier for the incidence to check for rights
     * @param right the access right to check for
     * @see hasRight()
     */
    [[nodiscard]] bool hasRight(const QString &uid, Akonadi::Collection::Right right) const;

    /**
     * Returns the KCheckableProxyModel used to select from which collections should
     * the calendar be populated from.
     */
    KCheckableProxyModel *checkableProxyModel() const;

    /**
     * Convenience method to access the contents of this KCalendarCore::Calendar through
     * a QAIM interface.
     *
     * Like through calendar interface, the model only items of selected collections.
     * To select or unselect collections, see checkableProxyModel().
     *
     * @see checkableProxyModel()
     * @see entityTreeModel()
     */
    QAbstractItemModel *model() const;

    /**
     * Returns the underlying EntityTreeModel.
     *
     * For most uses, you'll want model() or the KCalendarCore::Calendar interface instead.
     *
     * It contains every item and collection with calendar mime type, doesn't have
     * KCalendarCore filtering and doesn't honour any collection selection.
     *
     * This method is exposed for performance reasons only, so you can reuse it,
     * since it's resource savvy.
     *
     * @see model()
     */
    Akonadi::EntityTreeModel *entityTreeModel() const;

    /**
     * Returns all alarms occurring in a specified time interval.
     * @param from start date of interval
     * @param to end data of interval
     * @param excludeBlockedAlarms if true, alarms belonging to blocked collections aren't returned.
     */
    KCalendarCore::Alarm::List alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms = false) const override;

    /**
     * Enable or disable collection filtering.
     * If true, the calendar will only contain items of selected collections.
     * @param enable enables collection filtering if set as @c true
     * @see checkableProxyModel()
     * @see collectionFilteringEnabled()
     */
    void setCollectionFilteringEnabled(bool enable);

    /**
     * Returns whether collection filtering is enabled. Default is true.
     * @see setCollectionFilteringEnabled()
     */
    [[nodiscard]] bool collectionFilteringEnabled() const;

Q_SIGNALS:
    /**
     * This signal is emitted if a collection has been changed (properties or attributes).
     *
     * @param collection The changed collection.
     * @param attributeNames The names of the collection attributes that have been changed.
     */
    void collectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &attributeNames);

    /**
     * This signal is emitted when one or more collections are added to the ETM.
     *
     * @param collection non empty list of collections
     */
    void collectionsAdded(const Akonadi::Collection::List &collection);

    /**
     * This signal is emitted when one or more collections are deleted from the ETM.
     *
     * @param collection non empty list of collections
     */
    void collectionsRemoved(const Akonadi::Collection::List &collection);

    /**
     * Emitted whenever an Item is inserted, removed or modified.
     */
    void calendarChanged();

private:
    Q_DECLARE_PRIVATE(ETMCalendar)
};
}
