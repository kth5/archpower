// SPDX-FileCopyrightText: 2021 Claudio Cambra <claudio.cambra@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "kalendaralarmclient.h"
#include "../src/calendarsettings.h"
#include "alarmnotification.h"
#include "calendarinterface.h"
#include "logging.h"

#include <Akonadi/IncidenceChanger>
#include <KIO/ApplicationLauncherJob>
#include <KIdentityManagementCore/Utils>

#include <KCheckableProxyModel>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QFileInfo>

using namespace KCalendarCore;

namespace
{
static const char mySuspensedGroupName[] = "Suspended";
}

KalendarAlarmClient::KalendarAlarmClient(QObject *parent)
    : QObject(parent)
{
    mCheckTimer.setSingleShot(true);
    mCheckTimer.setTimerType(Qt::VeryCoarseTimer);

    // Check if Akonadi is already configured
    const QString akonadiConfigFile = Akonadi::ServerManager::serverConfigFilePath(Akonadi::ServerManager::ReadWrite);
    if (QFileInfo::exists(akonadiConfigFile)) {
        // Akonadi is configured, create ETM and friends, which will start Akonadi
        // if its not running yet
        setupAkonadi();
    } else {
        // Akonadi has not been set up yet, wait for someone else to start it,
        // so that we don't unnecessarily slow session start up
        connect(Akonadi::ServerManager::self(), &Akonadi::ServerManager::stateChanged, this, [this](Akonadi::ServerManager::State state) {
            if (state == Akonadi::ServerManager::Running) {
                setupAkonadi();
            }
        });
    }

    KConfigGroup alarmGroup(KSharedConfig::openConfig(), QStringLiteral("Alarms"));
    mLastChecked = alarmGroup.readEntry("CalendarsLastChecked", QDateTime::currentDateTime().addDays(-9));

    restoreSuspendedFromConfig();
}

KalendarAlarmClient::~KalendarAlarmClient() = default;

void KalendarAlarmClient::setupAkonadi()
{
    const QStringList mimeTypes{Event::eventMimeType(), Todo::todoMimeType()};
    mCalendar = Akonadi::ETMCalendar::Ptr(new Akonadi::ETMCalendar(mimeTypes));
    mCalendar->setObjectName(QLatin1StringView("KalendarAC's calendar"));
    Akonadi::IncidenceChanger *changer = mCalendar->incidenceChanger();
    changer->setShowDialogsOnError(false);
    mETM = mCalendar->entityTreeModel();

    connect(&mCheckTimer, &QTimer::timeout, this, &KalendarAlarmClient::checkAlarms);
    connect(mETM, &Akonadi::EntityTreeModel::collectionPopulated, this, &KalendarAlarmClient::deferredInit);
    connect(mETM, &Akonadi::EntityTreeModel::collectionTreeFetched, this, &KalendarAlarmClient::deferredInit);

    checkAlarms();
}

void checkAllItems(KCheckableProxyModel *model, const QModelIndex &parent = QModelIndex())
{
    const int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        model->setData(index, Qt::Checked, Qt::CheckStateRole);

        if (model->rowCount(index) > 0) {
            checkAllItems(model, index);
        }
    }
}

void KalendarAlarmClient::deferredInit()
{
    if (!collectionsAvailable()) {
        return;
    }

    qCDebug(REMINDER_DAEMON_LOG) << "Performing delayed initialization.";

    KCheckableProxyModel *checkableModel = mCalendar->checkableProxyModel();
    checkAllItems(checkableModel);

    // Now that everything is set up, a first check for reminders can be performed.
    checkAlarms();
}

void KalendarAlarmClient::restoreSuspendedFromConfig()
{
    qCDebug(REMINDER_DAEMON_LOG) << "Restore suspended alarms from config";
    const KConfigGroup suspendedGroup(KSharedConfig::openConfig(), QLatin1String(mySuspensedGroupName));
    const auto suspendedAlarms = suspendedGroup.groupList();

    for (const auto &s : suspendedAlarms) {
        const KConfigGroup suspendedAlarm(&suspendedGroup, s);
        const QString uid = suspendedAlarm.readEntry("UID");
        const QString txt = suspendedAlarm.readEntry("Text");
        const QDateTime occurrence = suspendedAlarm.readEntry("Occurrence", QDateTime());
        const QDateTime remindAt = suspendedAlarm.readEntry("RemindAt", QDateTime());
        const bool wasSuspended = suspendedAlarm.readEntry("WasSuspensed", false);
        qCDebug(REMINDER_DAEMON_LOG) << "restoreSuspendedFromConfig: Restoring alarm" << uid << "," << txt << "," << remindAt;

        if (!uid.isEmpty() && remindAt.isValid()) {
            addNotification(uid, txt, occurrence, remindAt, wasSuspended);
        }
    }
}

void KalendarAlarmClient::dismiss(AlarmNotification *notification)
{
    qCDebug(REMINDER_DAEMON_LOG) << "Alarm" << notification->uid() << "dismissed";
    removeNotification(notification);
    m_notifications.remove(notification->uid());
    delete notification;
}

void KalendarAlarmClient::suspend(AlarmNotification *notification, std::chrono::seconds sec)
{
    qCDebug(REMINDER_DAEMON_LOG) << "Alarm " << notification->uid() << "suspended";
    notification->setRemindAt(QDateTime(QDateTime::currentDateTime()).addSecs(sec.count()));
    notification->setWasSuspended(true);
    storeNotification(notification);
}

void KalendarAlarmClient::showIncidence(const QString &uid, const QDateTime &occurrence, const QString &xdgActivationToken)
{
    KConfig cfg(QStringLiteral("defaultcalendarrc"));
    KConfigGroup grp(&cfg, QStringLiteral("General"));
    const auto appId = grp.readEntry(QStringLiteral("ApplicationId"), QString());
    if (appId.isEmpty()) {
        return;
    }
    const auto kontactPlugin = grp.readEntry(QStringLiteral("KontactPlugin"), QStringLiteral("korganizer"));

    // start the calendar application if it isn't running yet
    const auto service = KService::serviceByDesktopName(appId);
    if (!service) {
        return;
    }
    auto job = new KIO::ApplicationLauncherJob(service, this);
    job->setStartupId(xdgActivationToken.toUtf8());
    connect(job, &KJob::finished, this, [appId, kontactPlugin, uid, occurrence, xdgActivationToken]() {
        // if running inside Kontact, select the right plugin
        if (appId == QLatin1String("org.kde.kontact")) {
            const QString objectName = QLatin1Char('/') + kontactPlugin + QLatin1String("_PimApplication");
            QDBusInterface iface(appId, objectName, QStringLiteral("org.kde.PIMUniqueApplication"), QDBusConnection::sessionBus());
            if (iface.isValid()) {
                QStringList arguments({kontactPlugin});
                iface.call(QStringLiteral("newInstance"), QByteArray(), arguments, QString());
            }
        }

        // select the right incidence/occurrence
        org::kde::calendar::Calendar iface(appId, QStringLiteral("/Calendar"), QDBusConnection::sessionBus());
        iface.showIncidenceByUid(uid, occurrence, xdgActivationToken);
    });
    job->start();
}

void KalendarAlarmClient::storeNotification(AlarmNotification *notification)
{
    // Work around crashing when feeding a QString to the KConfigGroup constructor for the config group name
    // BUG: 456157
    const auto notificationUidUtf8 = notification->uid().toUtf8();
    const auto notificationUidData = notificationUidUtf8.constData();

    KConfigGroup suspendedGroup(KSharedConfig::openConfig(), QLatin1String(mySuspensedGroupName));
    KConfigGroup notificationGroup(&suspendedGroup, QLatin1String(notificationUidData));
    notificationGroup.writeEntry("UID", notificationUidData);
    notificationGroup.writeEntry("Text", notification->text());
    if (notification->occurrence().isValid()) {
        notificationGroup.writeEntry("Occurrence", notification->occurrence());
    }
    notificationGroup.writeEntry("RemindAt", notification->remindAt());
    notificationGroup.writeEntry("WasSuspensed", notification->wasSuspended());
    KSharedConfig::openConfig()->sync();
}

void KalendarAlarmClient::removeNotification(AlarmNotification *notification)
{
    // Work around crashing when feeding a QString to the KConfigGroup constructor for the config group name
    // BUG: 456157
    const auto notificationUidUtf8 = notification->uid().toUtf8();
    const auto notificationUidData = notificationUidUtf8.constData();

    KConfigGroup suspendedGroup(KSharedConfig::openConfig(), QLatin1String(mySuspensedGroupName));
    KConfigGroup notificationGroup(&suspendedGroup, QLatin1String(notificationUidData));
    notificationGroup.deleteGroup();
    KSharedConfig::openConfig()->sync();
}

void KalendarAlarmClient::addNotification(const QString &uid, const QString &text, const QDateTime &occurrence, const QDateTime &remindTime, bool wasSuspended)
{
    AlarmNotification *notification = nullptr;
    const auto it = m_notifications.constFind(uid);
    if (it != m_notifications.constEnd()) {
        notification = it.value();
    } else {
        notification = new AlarmNotification(uid);
    }

    if (notification->remindAt().isValid() && notification->remindAt() < remindTime) {
        // we have a notification for this event already, and it's scheduled earlier than the new one
        return;
    }

    // we either have no notification for this event yet, or one that is scheduled for later and that should be replaced
    qCDebug(REMINDER_DAEMON_LOG) << "Adding notification, uid:" << uid << "text:" << text << "remindTime:" << remindTime;
    notification->setText(text);
    notification->setOccurrence(occurrence);
    notification->setRemindAt(remindTime);
    notification->setWasSuspended(wasSuspended);
    m_notifications[notification->uid()] = notification;
    storeNotification(notification);
}

bool KalendarAlarmClient::collectionsAvailable() const
{
    // The list of collections must be available.
    if (!mETM->isCollectionTreeFetched()) {
        return false;
    }

    // All collections must be populated.
    const int rowCount = mETM->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        static const int column = 0;
        const QModelIndex index = mETM->index(row, column);
        const bool haveData = mETM->data(index, Akonadi::EntityTreeModel::IsPopulatedRole).toBool();
        if (!haveData) {
            return false;
        }
    }

    return true;
}

namespace
{

bool shouldNotify(const KCalendarCore::Incidence::Ptr &incidence, const Akonadi::CalendarSettings *settings)
{
    if (settings->onlyShowRemindersForMyEvents()) {
        const auto isMe = [](const auto &person) -> bool {
            return KIdentityManagementCore::thatIsMe(person.email());
        };

        const auto attendees = incidence->attendees();
        if (std::any_of(attendees.cbegin(), attendees.cend(), isMe)) {
            return true;
        }

        if (isMe(incidence->organizer())) {
            return true;
        }

        return false;
    }

    return true;
}

} // namespace

void KalendarAlarmClient::checkAlarms()
{
    // We do not want to miss any reminders, so don't perform check unless
    // the collections are available and populated.
    if (!collectionsAvailable()) {
        qCDebug(REMINDER_DAEMON_LOG) << "Collections are not available; aborting check.";
        return;
    }

    const QDateTime from = mLastChecked.addSecs(1);
    mLastChecked = QDateTime::currentDateTime();

    qCDebug(REMINDER_DAEMON_LOG) << "Check:" << from.toString() << " -" << mLastChecked.toString();

    auto settings = Akonadi::CalendarSettings::self();
    settings->load(); // make sure we register changes to the config file

    // look for new alarms
    const Alarm::List alarms = mCalendar->alarms(from, mLastChecked, true /* exclude blocked alarms */);
    for (const Alarm::Ptr &alarm : alarms) {
        const QString uid = alarm->parentUid();
        const auto incidence = mCalendar->incidence(uid);
        if (incidence) {
            if (shouldNotify(incidence, settings)) {
                const auto occurrence = occurrenceForAlarm(incidence, alarm, from);
                addNotification(uid, alarm->text(), occurrence, mLastChecked, false);
            } else {
                qCDebug(REMINDER_DAEMON_LOG) << "Alarm for incidence " << uid << "skipped, because we are not an organizer or attendee.";
            }
        } else {
            qCDebug(REMINDER_DAEMON_LOG) << "Alarm points" << alarm << "to an nonexisting incidence" << uid;
        }
    }

    QList<QString> nullAlarmNotificationIds;
    // We need a copy of the notifications hash as some may get dismissed as we iterate over them, causing a crash
    // BUG: 455902
    const auto notificationsCopy = m_notifications;

    // execute or update active alarms
    for (auto it = notificationsCopy.constBegin(); it != notificationsCopy.constEnd(); ++it) {
        const auto notification = it.value();

        // Protect against null ptr
        if (!notification) {
            qCDebug(REMINDER_DAEMON_LOG) << "Found null active alarm with id: " << it.key() << "Skipping.";
            nullAlarmNotificationIds.append(it.key());
            continue;
        }

        if (notification->remindAt() <= mLastChecked) {
            const auto incidence = mCalendar->incidence(notification->uid());
            if (incidence) { // can still be null when we get here during the early stages of loading/restoring
                notification->send(this, incidence);
            }
        }
    }

    // Remove the null alarm notification ptrs from our notifications
    for (const auto &nullAlarmId : nullAlarmNotificationIds) {
        m_notifications.remove(nullAlarmId);
    }

    saveLastCheckTime();

    // schedule next check for the beginning of the next minute
    mCheckTimer.start(std::chrono::seconds(60 - mLastChecked.time().second()));
}

void KalendarAlarmClient::saveLastCheckTime()
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Alarms"));
    cg.writeEntry("CalendarsLastChecked", mLastChecked);
    KSharedConfig::openConfig()->sync();
}

// based on KCalendarCore::Calendar::appendRecurringAlarms()
QDateTime
KalendarAlarmClient::occurrenceForAlarm(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Alarm::Ptr &alarm, const QDateTime &from) const
{
    // recurring alarms not handled here for simplicity
    if (incidence.isNull() || !incidence->recurs() || alarm->repeatCount()) {
        return {};
    }

    // Alarm time is defined by an offset from the event start or end time.
    // Find the offset from the event start time, which is also used as the
    // offset from the recurrence time.
    Duration offset(0), endOffset(0);
    if (alarm->hasStartOffset()) {
        offset = alarm->startOffset();
    } else if (alarm->hasEndOffset()) {
        offset = alarm->endOffset();
        endOffset = Duration(incidence->dtStart(), incidence->dateTime(Incidence::RoleAlarmEndOffset));
    } else {
        // alarms at a fixed time, not handled here for simplicity
        return {};
    }

    // Find the incidence's earliest alarm
    QDateTime alarmStart = offset.end(alarm->hasEndOffset() ? incidence->dateTime(Incidence::RoleAlarmEndOffset) : incidence->dtStart());
    QDateTime baseStart = incidence->dtStart();
    if (from > alarmStart) {
        alarmStart = from; // don't look earlier than the earliest alarm
        baseStart = (-offset).end((-endOffset).end(alarmStart));
    }

    // Find the next occurrence from the earliest possible alarm time
    return incidence->recurrence()->getNextDateTime(baseStart.addSecs(-1));
}

#include "moc_kalendaralarmclient.cpp"
