/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>
#include <QUrl>

namespace KIO
{
class Job;
}

namespace Akonadi
{
class FreeBusyDownloadJob : public KJob
{
    Q_OBJECT
public:
    explicit FreeBusyDownloadJob(const QUrl &url, QWidget *parentWidget = nullptr);
    ~FreeBusyDownloadJob() override;

    void start() override;

    [[nodiscard]] QUrl url() const;
    [[nodiscard]] QByteArray rawFreeBusyData() const;

private Q_SLOTS:
    void slotData(KIO::Job *, const QByteArray &data);
    void slotResult(KJob *job);

private:
    const QUrl mUrl;
    QByteArray mFreeBusyData;
    QWidget *const mParent;
};
}
