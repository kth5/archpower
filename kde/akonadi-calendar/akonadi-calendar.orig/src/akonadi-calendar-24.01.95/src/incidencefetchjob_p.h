/*
  SPDX-FileCopyrightText: 2011 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Item>
#include <Akonadi/Job>
#include <Akonadi/MimeTypeChecker>
#include <KCalendarCore/Incidence>

namespace Akonadi
{
/**
 * Retrieve all incidences in all calendars.
 * This is a Strigi/Nepomuk-free replacement for an IncidenceSearchJob without a query.
 * @internal
 */
class IncidenceFetchJob : public Akonadi::Job
{
    Q_OBJECT
public:
    explicit IncidenceFetchJob(QObject *parent = nullptr);

    Akonadi::Item::List items() const;

protected:
    void doStart() override;

private:
    void collectionFetchResult(KJob *job);
    void itemFetchResult(KJob *job);
    Akonadi::Item::List m_items;
    Akonadi::MimeTypeChecker m_mimeTypeChecker;
    int m_jobCount = 0;
};
}
