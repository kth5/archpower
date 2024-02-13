/*
   SPDX-FileCopyrightText: 2023-2024 Laurent Montel <montel.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "notificationobject.h"
#include <QDebug>

NotificationObject::NotificationObject(QObject *parent)
    : QObject{parent}
{
}

NotificationObject::~NotificationObject() = default;

void NotificationObject::sendNotification(const QString &title, const QString &summary)
{
    const bool notificationExists = m_notification;
    if (!notificationExists) {
        m_notification = new KNotification(QStringLiteral("alarm"));
    }
    m_notification->setTitle(title);

    m_notification->setFlags(KNotification::Persistent);
    m_notification->setText(summary);

    if (!notificationExists) {
        auto action = m_notification->addDefaultAction(QStringLiteral("View"));
        auto remindIn5MAction = m_notification->addAction(QStringLiteral("Remind in 5 mins"));
        auto remindIn1hAction = m_notification->addAction(QStringLiteral("Remind in 1 hour"));

        auto dismissAction = m_notification->addAction(QStringLiteral("Dismiss"));
        qDebug() << " send event ";
        m_notification->sendEvent();
    }
}
