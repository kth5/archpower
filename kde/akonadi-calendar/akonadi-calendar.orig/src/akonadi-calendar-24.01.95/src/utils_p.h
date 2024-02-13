/*
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KCalendarCore/Incidence>

#include <QString>
#include <QStringList>

/**
 * Util functions that have no place to live.
 */

class QWidget;

namespace Akonadi
{
namespace CalendarUtils
{
[[nodiscard]] QString fullName();
[[nodiscard]] QString email();
[[nodiscard]] bool thatIsMe(const QString &email);

// faster version, because we know that attendee->email() is only the email address
[[nodiscard]] bool thatIsMe(const KCalendarCore::Attendee &attendee);

[[nodiscard]] QStringList allEmails();

[[nodiscard]] Akonadi::Collection
selectCollection(QWidget *parent, int &dialogCode, const QStringList &mimeTypes, const Akonadi::Collection &defaultCollection = Akonadi::Collection());
}
}
