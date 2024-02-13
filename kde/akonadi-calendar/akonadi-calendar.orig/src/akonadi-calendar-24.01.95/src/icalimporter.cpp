/**
  This file is part of the akonadi-calendar library.

  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icalimporter.h"
#include "icalimporter_p.h"
#include "utils_p.h"

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/ServerManager>

#include <KCalendarCore/FileStorage>

#include <KIO/Job>
#include <KIO/StoredTransferJob>

#include <QDBusInterface>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTimeZone>

using namespace KCalendarCore;
using namespace Akonadi;

ICalImporterPrivate::ICalImporterPrivate(IncidenceChanger *changer, ICalImporter *qq)
    : QObject()
    , q(qq)
    , m_changer(changer)
{
    if (!changer) {
        m_changer = new IncidenceChanger(q);
    }
    connect(m_changer, &IncidenceChanger::createFinished, this, &ICalImporterPrivate::onIncidenceCreated);
}

ICalImporterPrivate::~ICalImporterPrivate()
{
    delete m_temporaryFile;
}

void ICalImporterPrivate::onIncidenceCreated(int changeId,
                                             const Akonadi::Item &item,
                                             Akonadi::IncidenceChanger::ResultCode resultCode,
                                             const QString &errorString)
{
    Q_UNUSED(item)

    if (!m_pendingRequests.contains(changeId)) {
        return; // Not ours
    }

    m_pendingRequests.removeAll(changeId);

    if (resultCode != IncidenceChanger::ResultCodeSuccess) {
        m_working = false;
        setErrorMessage(errorString);
        m_pendingRequests.clear();
        Q_EMIT q->importIntoExistingFinished(false, m_numIncidences);
    } else if (m_pendingRequests.isEmpty()) {
        m_working = false;
        Q_EMIT q->importIntoExistingFinished(true, m_numIncidences);
    }
}

void ICalImporterPrivate::setErrorMessage(const QString &message)
{
    m_lastErrorMessage = message;
    qCritical() << message;
}

void ICalImporterPrivate::resourceCreated(KJob *job)
{
    auto createjob = qobject_cast<Akonadi::AgentInstanceCreateJob *>(job);

    Q_ASSERT(createjob);
    m_working = false;
    if (createjob->error()) {
        setErrorMessage(i18n("Error creating ical resource: %1", createjob->errorString()));
        Q_EMIT q->importIntoNewFinished(false);
        return;
    }

    Akonadi::AgentInstance instance = createjob->instance();
    const QString service = Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, instance.identifier());

    QDBusInterface iface(service, QStringLiteral("/Settings"));
    if (!iface.isValid()) {
        setErrorMessage(i18n("Failed to obtain D-Bus interface for remote configuration."));
        Q_EMIT q->importIntoNewFinished(false);
        return;
    }

    const QString path = createjob->property("path").toString();
    Q_ASSERT(!path.isEmpty());

    iface.call(QStringLiteral("setPath"), path);
    iface.call(QStringLiteral("save"));
    instance.reconfigure();

    Q_EMIT q->importIntoNewFinished(true);
}

void ICalImporterPrivate::remoteDownloadFinished(KIO::Job *job, const QByteArray &data)
{
    const bool success = job->error() == 0;
    m_working = false;
    if (success) {
        delete m_temporaryFile;
        m_temporaryFile = new QTemporaryFile();
        m_temporaryFile->write(data.constData(), data.size());
        q->importIntoExistingResource(QUrl(m_temporaryFile->fileName()), m_collection);
    } else {
        setErrorMessage(i18n("Could not download remote file."));
        Q_EMIT q->importIntoExistingFinished(false, 0);
    }
}

ICalImporter::ICalImporter(Akonadi::IncidenceChanger *changer, QObject *parent)
    : QObject(parent)
    , d(new ICalImporterPrivate(changer, this))
{
}

ICalImporter::~ICalImporter() = default;

QString ICalImporter::errorMessage() const
{
    return d->m_lastErrorMessage;
}

bool ICalImporter::importIntoNewResource(const QString &filename)
{
    d->m_lastErrorMessage.clear();

    if (d->m_working) {
        d->setErrorMessage(i18n("An import task is already in progress."));
        return false;
    }

    d->m_working = true;

    Akonadi::AgentType type = Akonadi::AgentManager::self()->type(QStringLiteral("akonadi_ical_resource"));

    auto job = new Akonadi::AgentInstanceCreateJob(type, this);
    job->setProperty("path", filename);
    connect(job, &KJob::result, d.get(), &ICalImporterPrivate::resourceCreated);
    job->start();

    return true;
}

bool ICalImporter::importIntoExistingResource(const QUrl &url, Collection collection)
{
    d->m_lastErrorMessage.clear();

    if (d->m_working) {
        d->setErrorMessage(i18n("An import task is already in progress."));
        return false;
    }

    if (url.isEmpty()) {
        d->setErrorMessage(i18n("Empty filename. Will not import ical file."));
        return false;
    }

    if (!url.isValid()) {
        d->setErrorMessage(i18n("Url to import is malformed."));
        return false;
    }

    if (url.isLocalFile()) {
        QFileInfo f{url.path()};
        if (!f.exists() || !f.isFile() || !f.isReadable()) {
            d->setErrorMessage(i18n("The selected file is not a readable file."));
            return false;
        }
        MemoryCalendar::Ptr temporaryCalendar(new MemoryCalendar(QTimeZone::systemTimeZone()));
        FileStorage storage(temporaryCalendar);
        storage.setFileName(url.path());
        bool success = storage.load();
        if (!success) {
            d->setErrorMessage(i18n("The selected file is not properly formatted, or in a format not supported by this software."));
            return false;
        }

        d->m_pendingRequests.clear();
        const Incidence::List incidences = temporaryCalendar->incidences();

        if (incidences.isEmpty()) {
            d->setErrorMessage(i18n("The selected file is empty."));
            return false;
        }

        if (!collection.isValid()) {
            int dialogCode;
            const QStringList mimeTypes = QStringList()
                << KCalendarCore::Event::eventMimeType() << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType();
            collection = CalendarUtils::selectCollection(nullptr, dialogCode /*by-ref*/, mimeTypes);
        }

        if (!collection.isValid()) {
            // user canceled
            d->setErrorMessage(QString());
            return false;
        }

        const IncidenceChanger::DestinationPolicy policySaved = d->m_changer->destinationPolicy();
        d->m_changer->startAtomicOperation(i18n("Merge ical file into existing calendar."));
        d->m_changer->setDestinationPolicy(IncidenceChanger::DestinationPolicyNeverAsk);
        for (const Incidence::Ptr &incidence : std::as_const(incidences)) {
            Q_ASSERT(incidence);
            if (!incidence) {
                continue;
            }
            const int requestId = d->m_changer->createIncidence(incidence, collection);
            Q_ASSERT(requestId != -1); // -1 only happens with invalid incidences
            if (requestId != -1) {
                d->m_pendingRequests << requestId;
            }
        }
        d->m_changer->endAtomicOperation();

        d->m_changer->setDestinationPolicy(policySaved); // restore
        d->m_numIncidences = incidences.count();
    } else {
        d->m_collection = collection;
        KIO::StoredTransferJob *job = KIO::storedGet(url);
        connect(job, qOverload<KIO::Job *, const QByteArray &>(&KIO::TransferJob::data), d.get(), [this](KIO::Job *job, const QByteArray &ba) {
            d->remoteDownloadFinished(job, ba);
        });
    }

    d->m_working = true;
    return true;
}

#include "moc_icalimporter_p.cpp"

#include "moc_icalimporter.cpp"
