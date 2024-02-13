/*
  SPDX-FileCopyrightText: 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>
  SPDX-FileContributor: Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
  SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/Item>

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

class QMimeData;

namespace Akonadi
{

class Collection;
class ETMCalendar;
class EntityTreeModel;
class Item;

/** Utility methods for dealing with calendar content in Akonadi items.
 *  @since 5.20.42
 */
namespace CalendarUtils
{
/**
 * Returns the incidence from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Incidence::Ptr incidence(const Akonadi::Item &item);

/**
 * Returns the event from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Event::Ptr event(const Akonadi::Item &item);

/**
 * Returns the todo from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Todo::Ptr todo(const Akonadi::Item &item);

/**
 * Returns the journal from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Journal::Ptr journal(const Akonadi::Item &item);

/**
 * Returns a suitable display name for the calendar (or calendar folder) @p collection.
 * This takes backend-specific special cases into account.
 */
AKONADI_CALENDAR_EXPORT QString displayName(Akonadi::ETMCalendar *calendar, const Akonadi::Collection &collection);
AKONADI_CALENDAR_EXPORT QString displayName(const Akonadi::EntityTreeModel *model, const Akonadi::Collection &collection);
AKONADI_CALENDAR_EXPORT QString displayName(const Akonadi::Collection &collection);

/**
 * Creates a MIME data object for dragging Akonadi items containing calendar incidences.
 * @since 5.23.41
 */
AKONADI_CALENDAR_EXPORT QMimeData *createMimeData(const Akonadi::Item::List &items);
}

}
