/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "freebusydownloadjob_p.h"

#include <KIO/Job>
#include <KIO/TransferJob>
#include <KJobWidgets>

using namespace Akonadi;

FreeBusyDownloadJob::FreeBusyDownloadJob(const QUrl &url, QWidget *parentWidget)
    : mUrl(url)
    , mParent(parentWidget)
{
    setObjectName(QLatin1StringView("FreeBusyDownloadJob"));
}

FreeBusyDownloadJob::~FreeBusyDownloadJob() = default;

void FreeBusyDownloadJob::start()
{
    KIO::TransferJob *job = KIO::get(mUrl, KIO::NoReload, KIO::HideProgressInfo);
    KJobWidgets::setWindow(job, mParent);
    connect(job, &KIO::TransferJob::result, this, &FreeBusyDownloadJob::slotResult);
    connect(job, &KIO::TransferJob::data, this, &FreeBusyDownloadJob::slotData);
}

QByteArray FreeBusyDownloadJob::rawFreeBusyData() const
{
    return mFreeBusyData;
}

QUrl FreeBusyDownloadJob::url() const
{
    return mUrl;
}

void FreeBusyDownloadJob::slotData(KIO::Job *, const QByteArray &data)
{
    mFreeBusyData += data;
}

void FreeBusyDownloadJob::slotResult(KJob *job)
{
    if (job->error()) {
        setErrorText(job->errorText());
    }

    emitResult();
}

#include "moc_freebusydownloadjob_p.cpp"
