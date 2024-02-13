/*
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include <KCalendarCore/Alarm>

#include <Akonadi/Attribute>

#include <memory>

namespace Akonadi
{
class BlockAlarmsAttributePrivate;

/**
 * @short An Attribute that marks that alarms from a calendar collection are blocked.
 *
 * A calendar collection which has this attribute set won't be evaluated by korgac and
 * therefore it's alarms won't be used, unless explicitly unblocked in blockAlarmType().
 *
 * @author Tobias Koenig <tokoe@kdab.com>
 * @see Akonadi::Attribute
 * @since 4.11
 */
class AKONADI_CALENDAR_EXPORT BlockAlarmsAttribute : public Akonadi::Attribute
{
public:
    /**
     * Creates a new block alarms attribute.
     */
    BlockAlarmsAttribute();

    /**
     * Destroys the block alarms attribute.
     */
    ~BlockAlarmsAttribute() override;

    /**
     * Blocks or unblocks given alarm type.
     *
     * By default, all alarm types are blocked.
     *
     * @since 4.11
     */
    void blockAlarmType(KCalendarCore::Alarm::Type type, bool block = true);

    /**
     * Blocks or unblocks every alarm type.
     *
     * By default, all alarm types are blocked.
     *
     * @since 5.0
     */
    void blockEverything(bool block = true);

    /**
     * Returns whether given alarm type is blocked or not.
     *
     * @since 4.11
     */
    bool isAlarmTypeBlocked(KCalendarCore::Alarm::Type type) const;

    /**
     * Returns whether all alarms are blocked or not.
     *
     * @since 5.0
     */

    bool isEverythingBlocked() const;

    QByteArray type() const override;
    BlockAlarmsAttribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    Q_DISABLE_COPY(BlockAlarmsAttribute)
    std::unique_ptr<BlockAlarmsAttributePrivate> const d;
};
}
