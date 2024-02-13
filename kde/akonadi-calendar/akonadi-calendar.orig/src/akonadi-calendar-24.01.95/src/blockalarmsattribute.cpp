/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "blockalarmsattribute.h"
#include <Akonadi/AttributeFactory>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

using namespace Akonadi;

class Akonadi::BlockAlarmsAttributePrivate
{
public:
    int audio = 1;
    int display = 1;
    int email = 1;
    int procedure = 1;
};

BlockAlarmsAttribute::BlockAlarmsAttribute()
    : d(new BlockAlarmsAttributePrivate)
{
}

BlockAlarmsAttribute::~BlockAlarmsAttribute() = default;

void BlockAlarmsAttribute::blockAlarmType(KCalendarCore::Alarm::Type type, bool block)
{
    switch (type) {
    case KCalendarCore::Alarm::Audio:
        d->audio = block;
        return;
    case KCalendarCore::Alarm::Display:
        d->display = block;
        return;
    case KCalendarCore::Alarm::Email:
        d->email = block;
        return;
    case KCalendarCore::Alarm::Procedure:
        d->procedure = block;
        return;
    default:
        return;
    }
}

void BlockAlarmsAttribute::blockEverything(bool block)
{
    blockAlarmType(KCalendarCore::Alarm::Audio, block);
    blockAlarmType(KCalendarCore::Alarm::Display, block);
    blockAlarmType(KCalendarCore::Alarm::Email, block);
    blockAlarmType(KCalendarCore::Alarm::Procedure, block);
}

bool BlockAlarmsAttribute::isAlarmTypeBlocked(KCalendarCore::Alarm::Type type) const
{
    switch (type) {
    case KCalendarCore::Alarm::Audio:
        return d->audio;
    case KCalendarCore::Alarm::Display:
        return d->display;
    case KCalendarCore::Alarm::Email:
        return d->email;
    case KCalendarCore::Alarm::Procedure:
        return d->procedure;
    default:
        return false;
    }
}

bool BlockAlarmsAttribute::isEverythingBlocked() const
{
    return isAlarmTypeBlocked(KCalendarCore::Alarm::Audio) && isAlarmTypeBlocked(KCalendarCore::Alarm::Display)
        && isAlarmTypeBlocked(KCalendarCore::Alarm::Email) && isAlarmTypeBlocked(KCalendarCore::Alarm::Procedure);
}

QByteArray BlockAlarmsAttribute::type() const
{
    static const QByteArray sType("BlockAlarmsAttribute");
    return sType;
}

BlockAlarmsAttribute *BlockAlarmsAttribute::clone() const
{
    auto copy = new BlockAlarmsAttribute();
    copy->d->audio = d->audio;
    copy->d->display = d->display;
    copy->d->email = d->email;
    copy->d->procedure = d->procedure;

    return copy;
}

QByteArray BlockAlarmsAttribute::serialized() const
{
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << d->audio;
    stream << d->display;
    stream << d->email;
    stream << d->procedure;

    return ba;
}

void BlockAlarmsAttribute::deserialize(const QByteArray &data)
{
    // Pre-4.11, default behavior
    if (data.isEmpty()) {
        d->audio = 1;
        d->display = 1;
        d->email = 1;
        d->procedure = 1;
    } else {
        QByteArray ba = data;
        QDataStream stream(&ba, QIODevice::ReadOnly);
        int i;
        stream >> i;
        d->audio = i;
        stream >> i;
        d->display = i;
        stream >> i;
        d->email = i;
        stream >> i;
        d->procedure = i;
    }
}

namespace
{
// Anonymous namespace; function is invisible outside this file.
bool dummy()
{
    Akonadi::AttributeFactory::registerAttribute<Akonadi::BlockAlarmsAttribute>();

    return true;
}

// Called when this library is loaded.
const bool registered = dummy();
} // namespace
