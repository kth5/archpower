/*
  SPDX-FileCopyrightText: 2008 Bruno Virlet <bvirlet@kdemail.net>
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarmodel_p.h"

#include "calendarutils.h"
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <KLocalizedString>

#include <QIcon>

using namespace Akonadi;

class Akonadi::CalendarModelPrivate
{
public:
    CalendarModelPrivate() = default;

    QWeakPointer<CalendarModel> m_weakPointer;
};

CalendarModel::CalendarModel(Akonadi::Monitor *monitor)
    : EntityTreeModel(monitor)
    , d(new CalendarModelPrivate())
{
    monitor->itemFetchScope().fetchAllAttributes(true);
}

CalendarModel::Ptr CalendarModel::create(Monitor *monitor)
{
    auto model = new CalendarModel(monitor);
    CalendarModel::Ptr modelPtr = CalendarModel::Ptr(model);
    model->setWeakPointer(modelPtr.toWeakRef());
    return modelPtr;
}

CalendarModel::~CalendarModel() = default;

QWeakPointer<CalendarModel> CalendarModel::weakPointer() const
{
    return d->m_weakPointer;
}

void CalendarModel::setWeakPointer(const QWeakPointer<CalendarModel> &weakPointer)
{
    d->m_weakPointer = weakPointer;
}

QVariant CalendarModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    const KCalendarCore::Incidence::Ptr inc = CalendarUtils::incidence(item);
    if (!inc) {
        return {};
    }

    switch (role) {
    case Qt::DecorationRole: {
        if (column != Summary) {
            return {};
        }
        const auto incType{inc->type()};
        if (incType == KCalendarCore::IncidenceBase::TypeTodo) {
            return QIcon::fromTheme(QStringLiteral("view-pim-tasks"));
        } else if (incType == KCalendarCore::IncidenceBase::TypeJournal) {
            return QIcon::fromTheme(QStringLiteral("view-pim-journal"));
        } else if (incType == KCalendarCore::IncidenceBase::TypeEvent) {
            return QIcon::fromTheme(QStringLiteral("view-calendar"));
        }
        return QIcon::fromTheme(QStringLiteral("network-wired"));
    }
    case Qt::DisplayRole:
        switch (column) {
        case Summary:
            return inc->summary();

        case DateTimeStart:
            return inc->dtStart().toString();

        case DateTimeEnd:
            return inc->dateTime(KCalendarCore::Incidence::RoleEndTimeZone).toString();

        case DateTimeDue:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->dtDue().toString();
            } else {
                return {};
            }

        case Priority:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->priority();
            } else {
                return {};
            }

        case PercentComplete:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->percentComplete();
            } else {
                return {};
            }

        case Type:
            return inc->typeStr();
        default:
            break;
        }

        return {};
    case SortRole:
        switch (column) {
        case Summary:
            return inc->summary();

        case DateTimeStart:
            return inc->dtStart().toUTC();

        case DateTimeEnd:
            return inc->dateTime(KCalendarCore::Incidence::RoleEndTimeZone).toUTC();

        case DateTimeDue:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->dtDue().toUTC();
            } else {
                return {};
            }

        case Priority:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->priority();
            } else {
                return {};
            }

        case PercentComplete:
            if (KCalendarCore::Todo::Ptr t = CalendarUtils::todo(item)) {
                return t->percentComplete();
            } else {
                return {};
            }

        case Type:
            return inc->type();

        default:
            break;
        }

        return {};

    case RecursRole:
        return inc->recurs();

    default:
        return {};
    }
}

QVariant CalendarModel::entityData(const Akonadi::Collection &collection, int column, int role) const
{
    return EntityTreeModel::entityData(collection, column, role);
}

int CalendarModel::entityColumnCount(EntityTreeModel::HeaderGroup headerSet) const
{
    if (headerSet == EntityTreeModel::ItemListHeaders) {
        return ItemColumnCount;
    } else {
        return CollectionColumnCount;
    }
}

QVariant CalendarModel::entityHeaderData(int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    if (headerSet == EntityTreeModel::ItemListHeaders) {
        switch (section) {
        case Summary:
            return i18nc("@title:column calendar event summary", "Summary");
        case DateTimeStart:
            return i18nc("@title:column calendar event start date and time", "Start Date and Time");
        case DateTimeEnd:
            return i18nc("@title:column calendar event end date and time", "End Date and Time");
        case Type:
            return i18nc("@title:column calendar event type", "Type");
        case DateTimeDue:
            return i18nc("@title:column todo item due date and time", "Due Date and Time");
        case Priority:
            return i18nc("@title:column todo item priority", "Priority");
        case PercentComplete:
            return i18nc("@title:column todo item completion in percent", "Complete");
        default:
            return {};
        }
    }

    if (headerSet == EntityTreeModel::CollectionTreeHeaders) {
        switch (section) {
        case CollectionTitle:
            return i18nc("@title:column calendar title", "Calendar");
        default:
            return {};
        }
    }
    return {};
}

#include "moc_calendarmodel_p.cpp"
