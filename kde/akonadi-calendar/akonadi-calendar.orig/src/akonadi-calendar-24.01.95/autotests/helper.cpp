/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "helper.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/ItemFetchJob>

#include <QStringList>

using namespace Akonadi;

bool Helper::confirmExists(const Akonadi::Item &item)
{
    auto job = new ItemFetchJob(item);
    return job->exec() != 0;
}

bool Helper::confirmDoesntExist(const Akonadi::Item &item)
{
    auto job = new ItemFetchJob(item);
    return job->exec() == 0;
}

Akonadi::Collection Helper::fetchCollection()
{
    auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive);
    // Get list of collections
    job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
    const bool ret = job->exec();
    Q_ASSERT(ret);
    Q_UNUSED(ret)

    // Find our collection
    const Collection::List collections = job->collections();
    Q_ASSERT(!collections.isEmpty());
    Collection collection = collections.first();

    Q_ASSERT(collection.isValid());

    return collection;
}
