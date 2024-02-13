/*
  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "etmcalendar.h"
#include "mailscheduler_p.h"

#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/ICalFormat>

#include <QDBusInterface>
#include <QPointer>

#include <KJob>
#include <QUrl>

namespace KIO
{
class Job;
}

namespace Akonadi
{
class FreeBusyManager;

class FreeBusyManagerPrivate : public QObject
{
    Q_OBJECT

    FreeBusyManager *const q_ptr;
    Q_DECLARE_PUBLIC(FreeBusyManager)

public: /// Structs
    struct FreeBusyProviderRequest {
        FreeBusyProviderRequest(const QString &provider);

        enum Status { NotStarted, HandlingRequested, FreeBusyRequested };

        Status mRequestStatus;
        QSharedPointer<QDBusInterface> mInterface;
    };

    struct FreeBusyProvidersRequestsQueue {
        FreeBusyProvidersRequestsQueue();
        FreeBusyProvidersRequestsQueue(const QDateTime &start, const QDateTime &end);

        QDateTime mStartTime;
        QDateTime mEndTime;
        QList<FreeBusyProviderRequest> mRequests;
        int mHandlersCount = 0;
        KCalendarCore::FreeBusy::Ptr mResultingFreeBusy;
    };

public:
    Akonadi::ETMCalendar::Ptr mCalendar;
    KCalendarCore::ICalFormat mFormat;

    QStringList mRetrieveQueue;
    QMap<QUrl, QString> mFreeBusyUrlEmailMap;
    QMap<QString, FreeBusyProvidersRequestsQueue> mProvidersRequestsByEmail;

    // Free/Busy uploading
    QDateTime mNextUploadTime;
    int mTimerID = 0;
    bool mUploadingFreeBusy = false;
    bool mBrokenUrl = false;

    QPointer<QWidget> mParentWidgetForMailling;

    // the parentWidget to use while doing our "recursive" retrieval
    QPointer<QWidget> mParentWidgetForRetrieval;

public: /// Functions
    FreeBusyManagerPrivate(FreeBusyManager *q);
    void checkFreeBusyUrl();
    QString freeBusyDir() const;
    void fetchFreeBusyUrl(const QString &email);
    QString freeBusyToIcal(const KCalendarCore::FreeBusy::Ptr &freebusy);
    KCalendarCore::FreeBusy::Ptr iCalToFreeBusy(const QByteArray &freeBusyData);
    KCalendarCore::FreeBusy::Ptr ownerFreeBusy();
    QString ownerFreeBusyAsString();
    void processFreeBusyDownloadResult(KJob *_job);
    void processFreeBusyUploadResult(KJob *_job);
    void uploadFreeBusy();
    QStringList getFreeBusyProviders() const;
    void queryFreeBusyProviders(const QStringList &providers, const QString &email);
    void queryFreeBusyProviders(const QStringList &providers, const QString &email, const QDateTime &start, const QDateTime &end);

public Q_SLOTS:
    void processRetrieveQueue();
    void contactSearchJobFinished(KJob *_job);
    void finishProcessRetrieveQueue(const QString &email, const QUrl &url);
    void onHandlesFreeBusy(const QString &email, bool handles);
    void onFreeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText);
    void processMailSchedulerResult(Akonadi::Scheduler::Result result, const QString &errorMsg);
    void fbCheckerJobFinished(KJob *job);

Q_SIGNALS:
    void freeBusyUrlRetrieved(const QString &email, const QUrl &url);
};

class FbCheckerJob : public KJob
{
    Q_OBJECT
public:
    explicit FbCheckerJob(const QList<QUrl> &urlsToCheck, QObject *parent = nullptr);
    void start() override;

    QUrl validUrl() const;

private Q_SLOTS:
    void onGetJobFinished(KJob *job);
    void dataReceived(KIO::Job *, const QByteArray &data);

private:
    void checkNextUrl();
    QList<QUrl> mUrlsToCheck;
    QByteArray mData;
    QUrl mValidUrl;
};
}
