/*
 * SPDX-FileCopyrightText: 2019 Dimitris Kardarakos <dimkard@posteo.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <KCalendarCore/Incidence>
#include <KNotification>
#include <QDateTime>
#include <QPointer>
#include <QUrl>

class KalendarAlarmClient;

/**
 * @brief The alarm notification that should be displayed. It is a wrapper of a KNotification enhanced with alarm properties, like uid and remind time
 *
 */
class AlarmNotification
{
public:
    explicit AlarmNotification(const QString &uid);
    ~AlarmNotification();

    /**
     * @brief Sends the notification to be displayed
     */
    void send(KalendarAlarmClient *client, const KCalendarCore::Incidence::Ptr &incidence);

    /**
     * @return The uid of the Incidence of the alarm of the notification
     */
    [[nodiscard]] QString uid() const;

    /**
     * @brief The text of the notification that should be displayed
     */
    [[nodiscard]] QString text() const;

    /**
     * @brief Sets the to-be-displayed text of the notification
     */
    void setText(const QString &alarmText);

    /** Occurrence time in case of recurring incidences. */
    [[nodiscard]] QDateTime occurrence() const;
    void setOccurrence(const QDateTime &occurrence);

    /**
     * @return In case of a suspended notification, the time that the notification should be displayed. Otherwise, it is empty.
     */
    [[nodiscard]] QDateTime remindAt() const;

    /**
     * @brief Sets the time that should be displayed a suspended notification
     */
    void setRemindAt(const QDateTime &remindAtDt);

    /**
     * @return true if we suspended alarm notification.
     */
    [[nodiscard]] bool wasSuspended() const;

    void setWasSuspended(bool newWasSuspended);

private:
    bool hasValidContextAction() const;
    [[nodiscard]] QString determineContextAction(const KCalendarCore::Incidence::Ptr &incidence);

    QPointer<KNotification> m_notification;
    QString m_uid;
    QString m_text;
    QDateTime m_occurrence;
    QDateTime m_remind_at;
    QUrl m_contextAction;
    bool m_wasSuspended = false;
};
