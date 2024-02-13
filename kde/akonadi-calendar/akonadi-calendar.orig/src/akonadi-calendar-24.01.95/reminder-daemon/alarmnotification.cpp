/*
 * SPDX-FileCopyrightText: 2019 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "alarmnotification.h"
#include "kalendaralarmclient.h"
#include <KLocalizedString>

#include <QDesktopServices>
#include <QRegularExpression>
#include <QUrlQuery>

using namespace std::chrono_literals;

AlarmNotification::AlarmNotification(const QString &uid)
    : m_uid{uid}
{
}

AlarmNotification::~AlarmNotification()
{
    if (m_notification) {
        // don't delete immediately, in case we end up here as a result
        // of a signal from m_notification itself
        m_notification->deleteLater();
    }
}

void AlarmNotification::send(KalendarAlarmClient *client, const KCalendarCore::Incidence::Ptr &incidence)
{
    const QDateTime startTime = m_occurrence.isValid() ? m_occurrence.toLocalTime() : incidence->dtStart().toLocalTime();
    const bool notificationExists = m_notification;
    if (!notificationExists) {
        m_notification = new KNotification(QStringLiteral("alarm"));

        // dismiss both with the explicit action and just closing the notification
        // there is no signal for explicit closing though, we only can observe that
        // indirectly from not having received a different signal before closed()
        QObject::connect(m_notification, &KNotification::closed, client, [this, client]() {
            client->dismiss(this);
        });
    }

    // change the content unconditionally, that will also update already existing notifications
    m_notification->setTitle(incidence->summary());

    auto defaultAction = m_notification->addDefaultAction(i18n("View"));
    QObject::connect(defaultAction, &KNotificationAction::activated, client, [this, client, startTime] {
        client->showIncidence(uid(), startTime, m_notification->xdgActivationToken());
    });

    QString text;
    const auto now = QDateTime::currentDateTime();
    const auto incidenceType = incidence->type() == KCalendarCore::Incidence::TypeTodo ? i18n("Task") : i18n("Event");
    if (incidence->type() == KCalendarCore::Incidence::TypeTodo && !incidence->dtStart().isValid()) {
        const auto todo = incidence.staticCast<KCalendarCore::Todo>();
        text = i18n("Task due at %1", QLocale().toString(todo->dtDue().time(), QLocale::NarrowFormat));
    } else if (!incidence->allDay()) {
        const int startOffset = qRound(now.secsTo(startTime) / 60.0);
        if (startOffset > 0 && startOffset < 60) {
            text = i18ncp("Event starts in 5 minutes", "%2 starts in %1 minute", "%2 starts in %1 minutes", startOffset, incidenceType);
        } else if (startTime.date() == now.date()) {
            // event is/was today
            if (startTime >= now) {
                text = i18nc("Event starts at 10:00", "%1 starts at %2", incidenceType, QLocale().toString(startTime.time(), QLocale::NarrowFormat));
            } else {
                text = i18nc("Event started at 10:00", "%1 started at %2", incidenceType, QLocale().toString(startTime.time(), QLocale::NarrowFormat));
            }
        } else {
            // start time on a different day
            if (startTime >= now) {
                text = i18nc("Event starts on <DATE> at <TIME>",
                             "%1 starts on %2 at %3",
                             incidenceType,
                             QLocale().toString(startTime.date(), QLocale::NarrowFormat),
                             QLocale().toString(startTime.time(), QLocale::NarrowFormat));
            } else {
                text = i18nc("Event started on <DATE> at <TIME>",
                             "%1 started on %2 at %3",
                             incidenceType,
                             QLocale().toString(startTime.date(), QLocale::NarrowFormat),
                             QLocale().toString(startTime.time(), QLocale::NarrowFormat));
            }
        }
    } else {
        // all day events
        if (startTime >= now) {
            text = i18nc("Event starts on <DATE>", "%1 starts on %2", incidenceType, QLocale().toString(startTime.date(), QLocale::NarrowFormat));
        } else {
            text = i18nc("Event started on <DATE>", "%1 started on %2", incidenceType, QLocale().toString(startTime.date(), QLocale::NarrowFormat));
        }
    }

    bool eventHasEnded = false;
    if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        const auto event = incidence.staticCast<KCalendarCore::Event>();
        const auto eventEndTime = startTime.addSecs(event->dtStart().secsTo(event->dtEnd()));
        eventHasEnded = eventEndTime < QDateTime::currentDateTime();
    }
    if (m_wasSuspended) {
        m_notification->setFlags(KNotification::Persistent);
    } else {
        m_notification->setFlags(eventHasEnded ? KNotification::CloseOnTimeout : KNotification::Persistent);
    }

    if (!m_text.isEmpty() && m_text != incidence->summary()) { // MS Teams sometimes repeats the summary as the alarm text, we don't need that
        text = m_text + QLatin1Char('\n') + text;
    }
    m_notification->setText(text);

    m_notification->setIconName(incidence->type() == KCalendarCore::Incidence::TypeTodo ? QStringLiteral("view-task")
                                                                                        : QStringLiteral("view-calendar-upcoming"));

    if (!notificationExists) {
        auto remindIn5MAction = m_notification->addAction(i18n("Remind in 5 mins"));
        QObject::connect(remindIn5MAction, &KNotificationAction::activated, remindIn5MAction, [this, client] {
            QObject::disconnect(m_notification, &KNotification::closed, client, nullptr);
            client->suspend(this, 5min);
        });

        auto remindIn1hAction = m_notification->addAction(i18n("Remind in 1 hour"));
        QObject::connect(remindIn1hAction, &KNotificationAction::activated, remindIn1hAction, [this, client] {
            QObject::disconnect(m_notification, &KNotification::closed, client, nullptr);
            client->suspend(this, 5min);
        });

        auto dismissAction = m_notification->addAction(i18nc("dismiss a reminder notification for an event", "Dismiss"));
        QObject::connect(dismissAction, &KNotificationAction::activated, dismissAction, [this, client] {
            QObject::disconnect(m_notification, &KNotification::closed, client, nullptr);
            client->dismiss(this);
        });

        const auto contextActionString = determineContextAction(incidence);
        if (!contextActionString.isEmpty()) {
            auto contextAction = m_notification->addAction(contextActionString);
            QObject::connect(contextAction, &KNotificationAction::activated, contextAction, [this] {
                QDesktopServices::openUrl(m_contextAction);
            });
        }
        m_notification->sendEvent();
    }
}

QString AlarmNotification::uid() const
{
    return m_uid;
}

QString AlarmNotification::text() const
{
    return m_text;
}

void AlarmNotification::setText(const QString &alarmText)
{
    m_text = alarmText;
}

QDateTime AlarmNotification::occurrence() const
{
    return m_occurrence;
}

void AlarmNotification::setOccurrence(const QDateTime &occurrence)
{
    m_occurrence = occurrence;
}

QDateTime AlarmNotification::remindAt() const
{
    return m_remind_at;
}

void AlarmNotification::setRemindAt(const QDateTime &remindAtDt)
{
    m_remind_at = remindAtDt;
}

bool AlarmNotification::hasValidContextAction() const
{
    return m_contextAction.isValid() && (m_contextAction.scheme() == QLatin1String("https") || m_contextAction.scheme() == QLatin1String("geo"));
}

QString AlarmNotification::determineContextAction(const KCalendarCore::Incidence::Ptr &incidence)
{
    // look for possible (meeting) URLs
    m_contextAction = incidence->url();
    if (!hasValidContextAction()) {
        m_contextAction = QUrl(incidence->location());
    }
    if (!hasValidContextAction()) {
        m_contextAction = QUrl(incidence->customProperty("MICROSOFT", "SKYPETEAMSMEETINGURL"));
    }
    if (!hasValidContextAction()) {
        static QRegularExpression urlFinder(QStringLiteral(R"(https://[^\s>]*)"));
        const auto match = urlFinder.match(incidence->description());
        if (match.hasMatch()) {
            m_contextAction = QUrl(match.captured());
        }
    }

    if (hasValidContextAction()) {
        return i18n("Open URL");
    }

    // navigate to location
    if (incidence->hasGeo()) {
        m_contextAction.clear();
        m_contextAction.setScheme(QStringLiteral("geo"));
        m_contextAction.setPath(QString::number(incidence->geoLatitude()) + QLatin1Char(',') + QString::number(incidence->geoLongitude()));
    } else if (!incidence->location().isEmpty()) {
        m_contextAction.clear();
        m_contextAction.setScheme(QStringLiteral("geo"));
        m_contextAction.setPath(QStringLiteral("0,0"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), incidence->location());
        m_contextAction.setQuery(query);
    }

    if (hasValidContextAction()) {
        return i18n("Map");
    }

    return QString();
}

bool AlarmNotification::wasSuspended() const
{
    return m_wasSuspended;
}

void AlarmNotification::setWasSuspended(bool newWasSuspended)
{
    m_wasSuspended = newWasSuspended;
}
