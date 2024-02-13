/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonadicalendarplugin.h"
#include "akonadicalendarplugin_debug.h"
#include "singlecollectioncalendar.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Monitor>
#include <Akonadi/ServerManager>

static bool filterCollection(const Akonadi::Collection &col)
{
    return !col.isVirtual();
}

AkonadiCalendarPlugin::AkonadiCalendarPlugin(QObject *parent, const QVariantList &args)
    : KCalendarCore::CalendarPlugin(parent, args)
{
    // don't automatically start Akonadi if that's explicitly forbidden
    // (useful in e.g. the CI environment)
    if (qEnvironmentVariableIsSet("AKONADI_CALENDAR_KCALENDARCORE_PLUGIN_NO_AUTO_LAUNCH") && !Akonadi::ServerManager::isRunning()) {
        qCWarning(AKONADICALENDARPLUGIN_LOG) << "Akonadi is not running, but auto-launch is disabled!";
        return;
    }

    auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this);
    job->fetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    connect(job, &Akonadi::CollectionFetchJob::finished, this, [this, job]() {
        const auto results = job->collections();
        for (const auto &col : results) {
            if (!filterCollection(col)) {
                continue;
            }
            KCalendarCore::Calendar::Ptr cal(new SingleCollectionCalendar(col));
            m_calendars.push_back(cal);
        }
    });

    auto monitor = new Akonadi::Monitor(this);
    monitor->setCollectionFetchScope(job->fetchScope());
    connect(monitor, &Akonadi::Monitor::collectionAdded, this, [this](const Akonadi::Collection &c) {
        if (!filterCollection(c)) {
            return;
        }
        m_calendars.push_back(KCalendarCore::Calendar::Ptr(new SingleCollectionCalendar(c)));
        Q_EMIT calendarsChanged();
    });
    connect(monitor, &Akonadi::Monitor::collectionRemoved, this, [this](const Akonadi::Collection &c) {
        m_calendars.erase(std::remove_if(m_calendars.begin(),
                                         m_calendars.end(),
                                         [c](const KCalendarCore::Calendar::Ptr &cal) {
                                             return cal.staticCast<SingleCollectionCalendar>()->collection().id() == c.id();
                                         }),
                          m_calendars.end());
        Q_EMIT calendarsChanged();
    });
    connect(monitor, qOverload<const Akonadi::Collection &>(&Akonadi::Monitor::collectionChanged), this, [this](const Akonadi::Collection &col) {
        for (const auto &c : m_calendars) {
            const auto cal = c.staticCast<SingleCollectionCalendar>();
            if (cal->collection().id() == col.id()) {
                cal->setCollection(col);
                Q_EMIT calendarsChanged();
                return;
            }
        }
    });
}

AkonadiCalendarPlugin::~AkonadiCalendarPlugin() = default;

QList<KCalendarCore::Calendar::Ptr> AkonadiCalendarPlugin::calendars() const
{
    return m_calendars;
}

#include "moc_akonadicalendarplugin.cpp"
