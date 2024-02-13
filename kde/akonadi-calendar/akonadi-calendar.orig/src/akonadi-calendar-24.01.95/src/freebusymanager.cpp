/*
  SPDX-FileCopyrightText: 2011 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "freebusymanager.h"
#include "calendarsettings.h"
#include "freebusydownloadjob_p.h"
#include "freebusymanager_p.h"
#include "mailscheduler_p.h"
#include "publishdialog.h"
#include "utils_p.h"

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>
#include <Akonadi/ContactSearchJob>

#include <KCalendarCore/Event>
#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/Person>

#include "akonadicalendar_debug.h"
#include <KMessageBox>

#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KIO/TransferJob>
#include <KLocalizedString>
#include <QUrl>

#include <KJobWidgets>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>
#include <QTimerEvent>

using namespace Akonadi;
using namespace KCalendarCore;

/// Free helper functions

QUrl replaceVariablesUrl(const QUrl &url, const QString &email)
{
    QString emailName;
    QString emailHost;

    const int atPos = email.indexOf(QLatin1Char('@'));
    if (atPos >= 0) {
        emailName = email.left(atPos);
        emailHost = email.mid(atPos + 1);
    }

    QString saveStr = url.path();
    saveStr.replace(QStringLiteral("%email%"), email, Qt::CaseInsensitive);
    saveStr.replace(QStringLiteral("%name%"), emailName, Qt::CaseInsensitive);
    saveStr.replace(QStringLiteral("%server%"), emailHost, Qt::CaseInsensitive);

    QUrl retUrl(url);
    retUrl.setPath(saveStr);
    return retUrl;
}

// We need this function because using KIO::NetAccess::exists()
// is useless for the http and https protocols. And getting back
// arbitrary data is also useless because a server can respond back
// with a "no such document" page.  So we need smart checking.
FbCheckerJob::FbCheckerJob(const QList<QUrl> &urlsToCheck, QObject *parent)
    : KJob(parent)
    , mUrlsToCheck(urlsToCheck)
{
}

void FbCheckerJob::start()
{
    checkNextUrl();
}

void FbCheckerJob::checkNextUrl()
{
    if (mUrlsToCheck.isEmpty()) {
        qCDebug(AKONADICALENDAR_LOG) << "No fb file found";
        setError(KJob::UserDefinedError);
        emitResult();
        return;
    }
    const QUrl url = mUrlsToCheck.takeFirst();

    mData.clear();
    KIO::TransferJob *job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::TransferJob::data, this, &FbCheckerJob::dataReceived);
    connect(job, &KIO::TransferJob::result, this, &FbCheckerJob::onGetJobFinished);
}

void FbCheckerJob::dataReceived(KIO::Job *, const QByteArray &data)
{
    mData.append(data);
}

void FbCheckerJob::onGetJobFinished(KJob *job)
{
    auto transferJob = static_cast<KIO::TransferJob *>(job);
    if (mData.contains("BEGIN:VCALENDAR")) {
        qCDebug(AKONADICALENDAR_LOG) << "found freebusy";
        mValidUrl = transferJob->url();
        emitResult();
    } else {
        checkNextUrl();
    }
}

QUrl FbCheckerJob::validUrl() const
{
    return mValidUrl;
}

/// FreeBusyManagerPrivate::FreeBusyProviderRequest

FreeBusyManagerPrivate::FreeBusyProviderRequest::FreeBusyProviderRequest(const QString &provider)
    : mRequestStatus(NotStarted)
    , mInterface(nullptr)
{
    mInterface = QSharedPointer<QDBusInterface>(new QDBusInterface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + provider,
                                                                   QStringLiteral("/FreeBusyProvider"),
                                                                   QStringLiteral("org.freedesktop.Akonadi.Resource.FreeBusyProvider")));
}

/// FreeBusyManagerPrivate::FreeBusyProvidersRequestsQueue

FreeBusyManagerPrivate::FreeBusyProvidersRequestsQueue::FreeBusyProvidersRequestsQueue()
    : mResultingFreeBusy(nullptr)
{
    // Set the start of the period to today 00:00:00
    mStartTime = QDateTime(QDate::currentDate(), QTime());
    mEndTime = mStartTime.addDays(14);
    mResultingFreeBusy = KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(mStartTime, mEndTime));
}

FreeBusyManagerPrivate::FreeBusyProvidersRequestsQueue::FreeBusyProvidersRequestsQueue(const QDateTime &start, const QDateTime &end)
    : mResultingFreeBusy(nullptr)
{
    mStartTime = start;
    mEndTime = end;
    mResultingFreeBusy = KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(start, end));
}

/// FreeBusyManagerPrivate

FreeBusyManagerPrivate::FreeBusyManagerPrivate(FreeBusyManager *q)
    : QObject()
    , q_ptr(q)
    , mParentWidgetForRetrieval(nullptr)
{
    connect(this, &FreeBusyManagerPrivate::freeBusyUrlRetrieved, this, &FreeBusyManagerPrivate::finishProcessRetrieveQueue);
}

QString FreeBusyManagerPrivate::freeBusyDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/korganizer/freebusy");
}

void FreeBusyManagerPrivate::checkFreeBusyUrl()
{
    const QUrl targetURL(CalendarSettings::self()->freeBusyPublishUrl());
    mBrokenUrl = targetURL.isEmpty() || !targetURL.isValid();
}

static QString configFile()
{
    static QString file = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/korganizer/freebusyurls");
    return file;
}

void FreeBusyManagerPrivate::fetchFreeBusyUrl(const QString &email)
{
    // First check if there is a specific FB url for this email
    KConfig cfg(configFile());
    KConfigGroup group = cfg.group(email);
    QString url = group.readEntry(QStringLiteral("url"));
    if (!url.isEmpty()) {
        qCDebug(AKONADICALENDAR_LOG) << "Found cached url:" << url;
        QUrl cachedUrl(url);
        if (Akonadi::CalendarUtils::thatIsMe(email)) {
            cachedUrl.setUserName(CalendarSettings::self()->freeBusyRetrieveUser());
            cachedUrl.setPassword(CalendarSettings::self()->freeBusyRetrievePassword());
        }
        Q_EMIT freeBusyUrlRetrieved(email, replaceVariablesUrl(cachedUrl, email));
        return;
    }
    // Try with the url configured by preferred email in kcontactmanager
    auto job = new Akonadi::ContactSearchJob();
    job->setQuery(Akonadi::ContactSearchJob::Email, email);
    job->setProperty("contactEmail", QVariant::fromValue(email));
    connect(job, &Akonadi::ContactSearchJob::result, this, &FreeBusyManagerPrivate::contactSearchJobFinished);
    job->start();
}

void FreeBusyManagerPrivate::contactSearchJobFinished(KJob *_job)
{
    const QString email = _job->property("contactEmail").toString();

    if (_job->error()) {
        qCritical() << "Error while searching for contact: " << _job->errorString() << ", email = " << email;
        Q_EMIT freeBusyUrlRetrieved(email, QUrl());
        return;
    }

    auto job = qobject_cast<Akonadi::ContactSearchJob *>(_job);
    KConfig cfg(configFile());
    KConfigGroup group = cfg.group(email);
    QString url = group.readEntry(QStringLiteral("url"));

    const KContacts::Addressee::List contacts = job->contacts();
    for (const KContacts::Addressee &contact : contacts) {
        const QString pref = contact.preferredEmail();
        if (!pref.isEmpty() && pref != email) {
            group = cfg.group(pref);
            url = group.readEntry("url");
            qCDebug(AKONADICALENDAR_LOG) << "Preferred email of" << email << "is" << pref;
            if (!url.isEmpty()) {
                qCDebug(AKONADICALENDAR_LOG) << "Taken url from preferred email:" << url;
                Q_EMIT freeBusyUrlRetrieved(email, replaceVariablesUrl(QUrl(url), email));
                return;
            }
        }
    }
    // None found. Check if we do automatic FB retrieving then
    if (!CalendarSettings::self()->freeBusyRetrieveAuto()) {
        // No, so no FB list here
        qCDebug(AKONADICALENDAR_LOG) << "No automatic retrieving";
        Q_EMIT freeBusyUrlRetrieved(email, QUrl());
        return;
    }

    // Sanity check: Don't download if it's not a correct email
    // address (this also avoids downloading for "(empty email)").
    int emailpos = email.indexOf(QLatin1Char('@'));
    if (emailpos == -1) {
        qCWarning(AKONADICALENDAR_LOG) << "No '@' found in" << email;
        Q_EMIT freeBusyUrlRetrieved(email, QUrl());
        return;
    }

    const QString emailHost = email.mid(emailpos + 1);

    // Build the URL
    if (CalendarSettings::self()->freeBusyCheckHostname()) {
        // Don't try to fetch free/busy data for users not on the specified servers
        // This tests if the hostnames match, or one is a subset of the other
        const QString hostDomain = QUrl(CalendarSettings::self()->freeBusyRetrieveUrl()).host();
        if (hostDomain != emailHost && !hostDomain.endsWith(QLatin1Char('.') + emailHost) && !emailHost.endsWith(QLatin1Char('.') + hostDomain)) {
            // Host names do not match
            qCDebug(AKONADICALENDAR_LOG) << "Host '" << hostDomain << "' doesn't match email '" << email << '\'';
            Q_EMIT freeBusyUrlRetrieved(email, QUrl());
            return;
        }
    }

    if (CalendarSettings::self()->freeBusyRetrieveUrl().contains(QRegularExpression(QStringLiteral("\\.[xiv]fb$")))) {
        // user specified a fullpath
        // do variable string replacements to the URL (MS Outlook style)
        const QUrl sourceUrl(CalendarSettings::self()->freeBusyRetrieveUrl());
        QUrl fullpathURL = replaceVariablesUrl(sourceUrl, email);

        // set the User and Password part of the URL
        fullpathURL.setUserName(CalendarSettings::self()->freeBusyRetrieveUser());
        fullpathURL.setPassword(CalendarSettings::self()->freeBusyRetrievePassword());

        // no need to cache this URL as this is pretty fast to get from the config value.
        // return the fullpath URL
        qCDebug(AKONADICALENDAR_LOG) << "Found url. email=" << email << "; url=" << fullpathURL;
        Q_EMIT freeBusyUrlRetrieved(email, fullpathURL);
        return;
    }

    // else we search for a fb file in the specified URL with known possible extensions
    QStringList extensions;
    extensions.reserve(3);
    extensions << QStringLiteral("xfb");
    extensions << QStringLiteral("ifb");
    extensions << QStringLiteral("vfb");

    QStringList::ConstIterator ext;
    QList<QUrl> urlsToCheck;
    urlsToCheck.reserve(extensions.count());
    QStringList::ConstIterator extEnd(extensions.constEnd());
    for (ext = extensions.constBegin(); ext != extEnd; ++ext) {
        // build a url for this extension
        const QUrl sourceUrl = QUrl(CalendarSettings::self()->freeBusyRetrieveUrl());
        QUrl dirURL = replaceVariablesUrl(sourceUrl, email);
        if (CalendarSettings::self()->freeBusyFullDomainRetrieval()) {
            dirURL = dirURL.adjusted(QUrl::StripTrailingSlash);
            dirURL.setPath(QString(dirURL.path() + QLatin1Char('/') + email + QLatin1Char('.') + (*ext)));
        } else {
            // Cut off everything left of the @ sign to get the user name.
            const QString emailName = email.left(emailpos);
            dirURL = dirURL.adjusted(QUrl::StripTrailingSlash);
            dirURL.setPath(QString(dirURL.path() + QLatin1Char('/') + emailName + QLatin1Char('.') + (*ext)));
        }
        dirURL.setUserName(CalendarSettings::self()->freeBusyRetrieveUser());
        dirURL.setPassword(CalendarSettings::self()->freeBusyRetrievePassword());
        urlsToCheck << dirURL;
    }
    KJob *checkerJob = new FbCheckerJob(urlsToCheck, this);
    checkerJob->setProperty("email", email);
    connect(checkerJob, &KJob::result, this, &FreeBusyManagerPrivate::fbCheckerJobFinished);
    checkerJob->start();
}

void FreeBusyManagerPrivate::fbCheckerJobFinished(KJob *job)
{
    const QString email = job->property("email").toString();
    if (!job->error()) {
        auto checkerJob = static_cast<FbCheckerJob *>(job);
        const QUrl dirURL = checkerJob->validUrl();
        if (dirURL.isValid()) {
            // write the URL to the cache
            KConfig cfg(configFile());
            KConfigGroup group = cfg.group(email);
            group.writeEntry("url", dirURL.toDisplayString()); // prettyURL() does not write user nor password
            qCDebug(AKONADICALENDAR_LOG) << "Found url email=" << email << "; url=" << dirURL;
        }
        Q_EMIT freeBusyUrlRetrieved(email, dirURL);
    } else {
        qCDebug(AKONADICALENDAR_LOG) << "Returning invalid url";
        Q_EMIT freeBusyUrlRetrieved(email, QUrl());
    }
}

QString FreeBusyManagerPrivate::freeBusyToIcal(const KCalendarCore::FreeBusy::Ptr &freebusy)
{
    return mFormat.createScheduleMessage(freebusy, KCalendarCore::iTIPPublish);
}

KCalendarCore::FreeBusy::Ptr FreeBusyManagerPrivate::iCalToFreeBusy(const QByteArray &freeBusyData)
{
    const QString freeBusyVCal(QString::fromUtf8(freeBusyData));
    KCalendarCore::FreeBusy::Ptr fb = mFormat.parseFreeBusy(freeBusyVCal);

    if (!fb) {
        qCDebug(AKONADICALENDAR_LOG) << "Error parsing free/busy";
        qCDebug(AKONADICALENDAR_LOG) << freeBusyVCal;
    }

    return fb;
}

KCalendarCore::FreeBusy::Ptr FreeBusyManagerPrivate::ownerFreeBusy()
{
    const QDateTime start = QDateTime::currentDateTimeUtc();
    const QDateTime end = start.addDays(CalendarSettings::self()->freeBusyPublishDays());

    KCalendarCore::Event::List events = mCalendar ? mCalendar->rawEvents(start.date(), end.date()) : KCalendarCore::Event::List();
    KCalendarCore::FreeBusy::Ptr freebusy(new KCalendarCore::FreeBusy(events, start, end));
    freebusy->setOrganizer(KCalendarCore::Person(Akonadi::CalendarUtils::fullName(), Akonadi::CalendarUtils::email()));
    return freebusy;
}

QString FreeBusyManagerPrivate::ownerFreeBusyAsString()
{
    return freeBusyToIcal(ownerFreeBusy());
}

void FreeBusyManagerPrivate::processFreeBusyDownloadResult(KJob *_job)
{
    Q_Q(FreeBusyManager);

    auto job = qobject_cast<FreeBusyDownloadJob *>(_job);
    Q_ASSERT(job);
    if (job->error()) {
        qCritical() << "Error downloading freebusy" << _job->errorString();
        KMessageBox::error(mParentWidgetForRetrieval,
                           i18n("Failed to download free/busy data from: %1\nReason: %2", job->url().toDisplayString(), job->errorText()),
                           i18nc("@title:window", "Free/Busy Retrieval Error"));

        // TODO: Ask for a retry? (i.e. queue  the email again when the user wants it).

        // Make sure we don't fill up the map with unneeded data on failures.
        mFreeBusyUrlEmailMap.take(job->url());
    } else {
        KCalendarCore::FreeBusy::Ptr fb = iCalToFreeBusy(job->rawFreeBusyData());

        Q_ASSERT(mFreeBusyUrlEmailMap.contains(job->url()));
        const QString email = mFreeBusyUrlEmailMap.take(job->url());

        if (fb) {
            KCalendarCore::Person p = fb->organizer();
            p.setEmail(email);
            fb->setOrganizer(p);
            q->saveFreeBusy(fb, p);
            qCDebug(AKONADICALENDAR_LOG) << "Freebusy retrieved for " << email;
            Q_EMIT q->freeBusyRetrieved(fb, email);
        } else {
            qCritical() << "Error downloading freebusy, invalid fb.";
            KMessageBox::error(mParentWidgetForRetrieval,
                               i18n("Failed to parse free/busy information that was retrieved from: %1", job->url().toDisplayString()),
                               i18nc("@title:window", "Free/Busy Retrieval Error"));
        }
    }

    // When downloading failed or finished, start a job for the next one in the
    // queue if needed.
    processRetrieveQueue();
}

void FreeBusyManagerPrivate::processFreeBusyUploadResult(KJob *_job)
{
    auto job = static_cast<KIO::FileCopyJob *>(_job);
    if (job->error()) {
        KMessageBox::error(nullptr,
                           i18n("<qt><p>The software could not upload your free/busy list to "
                                "the URL '%1'. There might be a problem with the access "
                                "rights, or you specified an incorrect URL. The system said: "
                                "<em>%2</em>.</p>"
                                "<p>Please check the URL or contact your system administrator."
                                "</p></qt>",
                                job->destUrl().toString(),
                                job->errorString()));
    }
    // Delete temp file
    QUrl src = job->srcUrl();
    Q_ASSERT(src.isLocalFile());
    if (src.isLocalFile()) {
        QFile::remove(src.toLocalFile());
    }
    mUploadingFreeBusy = false;
}

void FreeBusyManagerPrivate::processRetrieveQueue()
{
    if (mRetrieveQueue.isEmpty()) {
        return;
    }

    const QString email = mRetrieveQueue.takeFirst();

    // First, try to find all agents that are free-busy providers
    const QStringList providers = getFreeBusyProviders();
    qCDebug(AKONADICALENDAR_LOG) << "Got the following FreeBusy providers: " << providers;

    // If some free-busy providers were found let's query them first and ask them
    // if they manage the free-busy information for the email address we have.
    if (!providers.isEmpty()) {
        queryFreeBusyProviders(providers, email);
    } else {
        fetchFreeBusyUrl(email);
    }
}

void FreeBusyManagerPrivate::finishProcessRetrieveQueue(const QString &email, const QUrl &freeBusyUrlForEmail)
{
    Q_Q(FreeBusyManager);

    if (!freeBusyUrlForEmail.isValid()) {
        qCDebug(AKONADICALENDAR_LOG) << "Invalid FreeBusy URL" << freeBusyUrlForEmail.toDisplayString() << email;
        return;
    }

    if (mFreeBusyUrlEmailMap.contains(freeBusyUrlForEmail)) {
        qCDebug(AKONADICALENDAR_LOG) << "Download already in progress for " << freeBusyUrlForEmail;
        return;
    }

    mFreeBusyUrlEmailMap.insert(freeBusyUrlForEmail, email);

    auto job = new FreeBusyDownloadJob(freeBusyUrlForEmail, mParentWidgetForRetrieval);
    q->connect(job, &FreeBusyDownloadJob::result, this, [this](KJob *job) {
        processFreeBusyDownloadResult(job);
    });
    job->start();
}

void FreeBusyManagerPrivate::uploadFreeBusy()
{
    Q_Q(FreeBusyManager);

    // user has automatic uploading disabled, bail out
    if (!CalendarSettings::self()->freeBusyPublishAuto() || CalendarSettings::self()->freeBusyPublishUrl().isEmpty()) {
        return;
    }

    if (mTimerID != 0) {
        // A timer is already running, so we don't need to do anything
        return;
    }

    const QDateTime currentDateTime = QDateTime::currentDateTime();
    int now = static_cast<int>(currentDateTime.toSecsSinceEpoch());
    int eta = static_cast<int>(mNextUploadTime.toSecsSinceEpoch()) - now;

    if (!mUploadingFreeBusy) {
        // Not currently uploading
        if (mNextUploadTime.isNull() || currentDateTime > mNextUploadTime) {
            // No uploading have been done in this session, or delay time is over
            q->publishFreeBusy();
            return;
        }

        // We're in the delay time and no timer is running. Start one
        if (eta <= 0) {
            // Sanity check failed - better do the upload
            q->publishFreeBusy();
            return;
        }
    } else {
        // We are currently uploading the FB list. Start the timer
        if (eta <= 0) {
            qCDebug(AKONADICALENDAR_LOG) << "This shouldn't happen! eta <= 0";
            eta = 10; // whatever
        }
    }

    // Start the timer
    mTimerID = q->startTimer(eta * 1000);

    if (mTimerID == 0) {
        // startTimer failed - better do the upload
        q->publishFreeBusy();
    }
}

QStringList FreeBusyManagerPrivate::getFreeBusyProviders() const
{
    QStringList providers;
    const Akonadi::AgentInstance::List agents = Akonadi::AgentManager::self()->instances();
    for (const Akonadi::AgentInstance &agent : agents) {
        if (agent.type().capabilities().contains(QLatin1String("FreeBusyProvider"))) {
            providers << agent.identifier();
        }
    }
    return providers;
}

void FreeBusyManagerPrivate::queryFreeBusyProviders(const QStringList &providers, const QString &email)
{
    if (!mProvidersRequestsByEmail.contains(email)) {
        mProvidersRequestsByEmail[email] = FreeBusyProvidersRequestsQueue();
    }

    for (const QString &provider : providers) {
        FreeBusyProviderRequest request(provider);

        // clang-format off
        connect(request.mInterface.data(), SIGNAL(handlesFreeBusy(QString,bool)), this, SLOT(onHandlesFreeBusy(QString,bool)));
        // clang-format on
        request.mInterface->call(QStringLiteral("canHandleFreeBusy"), email);
        request.mRequestStatus = FreeBusyProviderRequest::HandlingRequested;
        mProvidersRequestsByEmail[email].mRequests << request;
    }
}

void FreeBusyManagerPrivate::queryFreeBusyProviders(const QStringList &providers, const QString &email, const QDateTime &start, const QDateTime &end)
{
    if (!mProvidersRequestsByEmail.contains(email)) {
        mProvidersRequestsByEmail[email] = FreeBusyProvidersRequestsQueue(start, end);
    }

    queryFreeBusyProviders(providers, email);
}

void FreeBusyManagerPrivate::onHandlesFreeBusy(const QString &email, bool handles)
{
    if (!mProvidersRequestsByEmail.contains(email)) {
        return;
    }

    auto iface = qobject_cast<QDBusInterface *>(sender());
    if (!iface) {
        return;
    }

    FreeBusyProvidersRequestsQueue *queue = &mProvidersRequestsByEmail[email];
    QString respondingService = iface->service();
    qCDebug(AKONADICALENDAR_LOG) << respondingService << "responded to our FreeBusy request:" << handles;
    int requestIndex = -1;

    const int requestsSize(queue->mRequests.size());
    for (int i = 0; i < requestsSize; ++i) {
        if (queue->mRequests.at(i).mInterface->service() == respondingService) {
            requestIndex = i;
        }
    }

    if (requestIndex == -1) {
        return;
    }
    // clang-format off
    disconnect(iface, SIGNAL(handlesFreeBusy(QString,bool)), this, SLOT(onHandlesFreeBusy(QString,bool)));
    // clang-format on
    if (!handles) {
        queue->mRequests.removeAt(requestIndex);
        // If no more requests are left and no handler responded
        // then fall back to the URL mechanism
        if (queue->mRequests.isEmpty() && queue->mHandlersCount == 0) {
            mProvidersRequestsByEmail.remove(email);
            fetchFreeBusyUrl(email);
        }
    } else {
        ++queue->mHandlersCount;
        // clang-format off
        connect(iface, SIGNAL(freeBusyRetrieved(QString,QString,bool,QString)), this, SLOT(onFreeBusyRetrieved(QString,QString,bool,QString)));
        // clang-format on
        iface->call(QStringLiteral("retrieveFreeBusy"), email, queue->mStartTime, queue->mEndTime);
        queue->mRequests[requestIndex].mRequestStatus = FreeBusyProviderRequest::FreeBusyRequested;
    }
}

void FreeBusyManagerPrivate::processMailSchedulerResult(Akonadi::Scheduler::Result result, const QString &errorMsg)
{
    if (result == Scheduler::ResultSuccess) {
        KMessageBox::information(mParentWidgetForMailling,
                                 i18n("The free/busy information was successfully sent."),
                                 i18nc("@title:window", "Sending Free/Busy"),
                                 QStringLiteral("FreeBusyPublishSuccess"));
    } else {
        KMessageBox::error(mParentWidgetForMailling, i18n("Unable to publish the free/busy data: %1", errorMsg));
    }

    sender()->deleteLater();
}

void FreeBusyManagerPrivate::onFreeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText)
{
    Q_Q(FreeBusyManager);
    Q_UNUSED(errorText)

    if (!mProvidersRequestsByEmail.contains(email)) {
        return;
    }

    auto iface = dynamic_cast<QDBusInterface *>(sender());
    if (!iface) {
        return;
    }

    FreeBusyProvidersRequestsQueue *queue = &mProvidersRequestsByEmail[email];
    QString respondingService = iface->service();
    int requestIndex = -1;

    const int requestsSize(queue->mRequests.size());
    for (int i = 0; i < requestsSize; ++i) {
        if (queue->mRequests.at(i).mInterface->service() == respondingService) {
            requestIndex = i;
        }
    }

    if (requestIndex == -1) {
        return;
    }
    // clang-format off
    disconnect(iface, SIGNAL(freeBusyRetrieved(QString,QString,bool,QString)), this, SLOT(onFreeBusyRetrieved(QString,QString,bool,QString)));
    // clang-format on
    queue->mRequests.removeAt(requestIndex);

    if (success) {
        KCalendarCore::FreeBusy::Ptr fb = iCalToFreeBusy(freeBusy.toUtf8());
        if (!fb) {
            --queue->mHandlersCount;
        } else {
            queue->mResultingFreeBusy->merge(fb);
        }
    }

    if (queue->mRequests.isEmpty()) {
        if (queue->mHandlersCount == 0) {
            fetchFreeBusyUrl(email);
        } else {
            Q_EMIT q->freeBusyRetrieved(queue->mResultingFreeBusy, email);
        }
        mProvidersRequestsByEmail.remove(email);
    }
}

/// FreeBusyManager::Singleton

namespace Akonadi
{
class FreeBusyManagerStatic
{
public:
    FreeBusyManager instance;
};
}

Q_GLOBAL_STATIC(FreeBusyManagerStatic, sManagerInstance)

FreeBusyManager::FreeBusyManager()
    : d_ptr(new FreeBusyManagerPrivate(this))
{
    setObjectName(QLatin1StringView("FreeBusyManager"));
    connect(CalendarSettings::self(), SIGNAL(configChanged()), SLOT(checkFreeBusyUrl()));
}

FreeBusyManager::~FreeBusyManager() = default;

FreeBusyManager *FreeBusyManager::self()
{
    return &sManagerInstance->instance;
}

void FreeBusyManager::setCalendar(const Akonadi::ETMCalendar::Ptr &c)
{
    Q_D(FreeBusyManager);

    if (d->mCalendar) {
        disconnect(d->mCalendar.data(), SIGNAL(calendarChanged()));
    }

    d->mCalendar = c;
    if (d->mCalendar) {
        d->mFormat.setTimeZone(d->mCalendar->timeZone());
        connect(d->mCalendar.data(), SIGNAL(calendarChanged()), SLOT(uploadFreeBusy()));
    }

    // Lets see if we need to update our published
    QTimer::singleShot(0, this, SLOT(uploadFreeBusy()));
}

/*!
  This method is called when the user has selected to publish its
  free/busy list or when the delay have passed.
*/
void FreeBusyManager::publishFreeBusy(QWidget *parentWidget)
{
    Q_D(FreeBusyManager);
    // Already uploading? Skip this one then.
    if (d->mUploadingFreeBusy) {
        return;
    }

    // No calendar set yet? Don't upload to prevent losing published information that
    // might still be valid.
    if (!d->mCalendar) {
        return;
    }

    QUrl targetURL(CalendarSettings::self()->freeBusyPublishUrl());
    if (targetURL.isEmpty()) {
        KMessageBox::error(parentWidget,
                           i18n("<qt><p>No URL configured for uploading your free/busy list. "
                                "Please set it in KOrganizer's configuration dialog, on the "
                                "\"Free/Busy\" page.</p>"
                                "<p>Contact your system administrator for the exact URL and the "
                                "account details.</p></qt>"),
                           i18nc("@title:window", "No Free/Busy Upload URL"));
        return;
    }

    if (d->mBrokenUrl) {
        // Url is invalid, don't try again
        return;
    }
    if (!targetURL.isValid()) {
        KMessageBox::error(parentWidget,
                           i18n("<qt>The target URL '%1' provided is invalid.</qt>", targetURL.toDisplayString()),
                           i18nc("@title:window", "Invalid URL"));
        d->mBrokenUrl = true;
        return;
    }
    targetURL.setUserName(CalendarSettings::self()->freeBusyPublishUser());
    targetURL.setPassword(CalendarSettings::self()->freeBusyPublishPassword());

    d->mUploadingFreeBusy = true;

    // If we have a timer running, it should be stopped now
    if (d->mTimerID != 0) {
        killTimer(d->mTimerID);
        d->mTimerID = 0;
    }

    // Save the time of the next free/busy uploading
    d->mNextUploadTime = QDateTime::currentDateTime();
    if (CalendarSettings::self()->freeBusyPublishDelay() > 0) {
        d->mNextUploadTime = d->mNextUploadTime.addSecs(CalendarSettings::self()->freeBusyPublishDelay() * 60);
    }

    QString messageText = d->ownerFreeBusyAsString();

    // We need to massage the list a bit so that Outlook understands
    // it.
    messageText.replace(QRegularExpression(QStringLiteral("ORGANIZER\\s*:MAILTO:")), QStringLiteral("ORGANIZER:"));

    // Create a local temp file and save the message to it
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    if (tempFile.open()) {
        QTextStream textStream(&tempFile);
        textStream << messageText;
        textStream.flush();

#if 0
        QString defaultEmail = KOCore()
                               ::self()->email();
        QString emailHost = defaultEmail.mid(defaultEmail.indexOf('@') + 1);

        // Put target string together
        QUrl targetURL;
        if (CalendarSettings::self()->publishKolab()) {
            // we use Kolab
            QString server;
            if (CalendarSettings::self()->publishKolabServer() == QLatin1String("%SERVER%")
                || CalendarSettings::self()->publishKolabServer().isEmpty()) {
                server = emailHost;
            } else {
                server = CalendarSettings::self()->publishKolabServer();
            }

            targetURL.setScheme("webdavs");
            targetURL.setHost(server);

            QString fbname = CalendarSettings::self()->publishUserName();
            int at = fbname.indexOf('@');
            if (at > 1 && fbname.length() > (uint)at) {
                fbname.truncate(at);
            }
            targetURL.setPath("/freebusy/" + fbname + ".ifb");
            targetURL.setUserName(CalendarSettings::self()->publishUserName());
            targetURL.setPassword(CalendarSettings::self()->publishPassword());
        } else {
            // we use something else
            targetURL = CalendarSettings::self()->+publishAnyURL().replace("%SERVER%", emailHost);
            targetURL.setUserName(CalendarSettings::self()->publishUserName());
            targetURL.setPassword(CalendarSettings::self()->publishPassword());
        }
#endif

        QUrl src;
        src.setPath(tempFile.fileName());

        qCDebug(AKONADICALENDAR_LOG) << targetURL;

        KIO::Job *job = KIO::file_copy(src, targetURL, -1, KIO::Overwrite | KIO::HideProgressInfo);

        KJobWidgets::setWindow(job, parentWidget);

        // FIXME slot doesn't exist
        // connect(job, SIGNAL(result(KJob*)), SLOT(slotUploadFreeBusyResult(KJob*)));
    }
}

void FreeBusyManager::mailFreeBusy(int daysToPublish, QWidget *parentWidget)
{
    Q_D(FreeBusyManager);
    // No calendar set yet?
    if (!d->mCalendar) {
        return;
    }

    QDateTime start = QDateTime::currentDateTimeUtc().toTimeZone(d->mCalendar->timeZone());
    QDateTime end = start.addDays(daysToPublish);

    KCalendarCore::Event::List events = d->mCalendar->rawEvents(start.date(), end.date());

    FreeBusy::Ptr freebusy(new FreeBusy(events, start, end));
    freebusy->setOrganizer(Person(Akonadi::CalendarUtils::fullName(), Akonadi::CalendarUtils::email()));

    QPointer<PublishDialog> publishdlg = new PublishDialog();
    if (publishdlg->exec() == QDialog::Accepted) {
        // Send the mail
        auto scheduler = new MailScheduler(/*factory=*/nullptr, this);
        connect(scheduler, &Scheduler::transactionFinished, d, &FreeBusyManagerPrivate::processMailSchedulerResult);
        d->mParentWidgetForMailling = parentWidget;

        scheduler->publish(freebusy, publishdlg->addresses());
    }
    delete publishdlg;
}

bool FreeBusyManager::retrieveFreeBusy(const QString &email, bool forceDownload, QWidget *parentWidget)
{
    Q_D(FreeBusyManager);

    qCDebug(AKONADICALENDAR_LOG) << email;
    if (email.isEmpty()) {
        qCDebug(AKONADICALENDAR_LOG) << "Email is empty";
        return false;
    }

    d->mParentWidgetForRetrieval = parentWidget;

    if (Akonadi::CalendarUtils::thatIsMe(email)) {
        // Don't download our own free-busy list from the net
        qCDebug(AKONADICALENDAR_LOG) << "freebusy of owner, not downloading";
        Q_EMIT freeBusyRetrieved(d->ownerFreeBusy(), email);
        return true;
    }

    // Check for cached copy of free/busy list
    KCalendarCore::FreeBusy::Ptr fb = loadFreeBusy(email);
    if (fb) {
        qCDebug(AKONADICALENDAR_LOG) << "Found a cached copy for " << email;
        Q_EMIT freeBusyRetrieved(fb, email);
        return true;
    }

    // Don't download free/busy if the user does not want it.
    if (!CalendarSettings::self()->freeBusyRetrieveAuto() && !forceDownload) {
        qCDebug(AKONADICALENDAR_LOG) << "Not downloading freebusy";
        return false;
    }

    d->mRetrieveQueue.append(email);

    if (d->mRetrieveQueue.count() > 1) {
        // TODO: true should always emit
        qCWarning(AKONADICALENDAR_LOG) << "Returning true without Q_EMIT, is this correct?";
        return true;
    }

    // queued, because "true" means the download was initiated. So lets
    // return before starting stuff
    QMetaObject::invokeMethod(
        d,
        [d] {
            d->processRetrieveQueue();
        },
        Qt::QueuedConnection);
    return true;
}

void FreeBusyManager::cancelRetrieval()
{
    Q_D(FreeBusyManager);
    d->mRetrieveQueue.clear();
}

KCalendarCore::FreeBusy::Ptr FreeBusyManager::loadFreeBusy(const QString &email)
{
    Q_D(FreeBusyManager);
    const QString fbd = d->freeBusyDir();

    QFile f(fbd + QLatin1Char('/') + email + QStringLiteral(".ifb"));
    if (!f.exists()) {
        qCDebug(AKONADICALENDAR_LOG) << f.fileName() << "doesn't exist.";
        return {};
    }

    if (!f.open(QIODevice::ReadOnly)) {
        qCDebug(AKONADICALENDAR_LOG) << "Unable to open file" << f.fileName();
        return {};
    }

    QTextStream ts(&f);
    QString str = ts.readAll();

    return d->iCalToFreeBusy(str.toUtf8());
}

bool FreeBusyManager::saveFreeBusy(const KCalendarCore::FreeBusy::Ptr &freebusy, const KCalendarCore::Person &person)
{
    Q_D(FreeBusyManager);
    qCDebug(AKONADICALENDAR_LOG) << person.fullName();

    QString fbd = d->freeBusyDir();

    QDir freeBusyDirectory(fbd);
    if (!freeBusyDirectory.exists()) {
        qCDebug(AKONADICALENDAR_LOG) << "Directory" << fbd << " does not exist!";
        qCDebug(AKONADICALENDAR_LOG) << "Creating directory:" << fbd;

        if (!freeBusyDirectory.mkpath(fbd)) {
            qCDebug(AKONADICALENDAR_LOG) << "Could not create directory:" << fbd;
            return false;
        }
    }

    QString filename(fbd);
    filename += QLatin1Char('/');
    filename += person.email();
    filename += QStringLiteral(".ifb");
    QFile f(filename);

    qCDebug(AKONADICALENDAR_LOG) << "filename:" << filename;

    freebusy->clearAttendees();
    freebusy->setOrganizer(person);

    QString messageText = d->mFormat.createScheduleMessage(freebusy, KCalendarCore::iTIPPublish);

    if (!f.open(QIODevice::ReadWrite)) {
        qCDebug(AKONADICALENDAR_LOG) << "acceptFreeBusy: Can't open:" << filename << "for writing";
        return false;
    }
    QTextStream t(&f);
    t << messageText;
    f.close();

    return true;
}

void FreeBusyManager::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    publishFreeBusy();
}

#include "moc_freebusymanager.cpp"
#include "moc_freebusymanager_p.cpp"
