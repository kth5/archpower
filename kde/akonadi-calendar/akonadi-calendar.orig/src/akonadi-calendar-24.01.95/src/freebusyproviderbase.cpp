/*
    SPDX-FileCopyrightText: 2011 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "freebusyproviderbase.h"
#include "freebusyproviderbase_p.h"

#include "freebusyprovideradaptor.h"

#include <QDBusConnection>
#include <QDateTime>

using namespace Akonadi;

FreeBusyProviderBasePrivate::FreeBusyProviderBasePrivate(FreeBusyProviderBase *qq)
    : QObject()
    , q(qq)
{
    new Akonadi__FreeBusyProviderAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/FreeBusyProvider"), this, QDBusConnection::ExportAdaptors);
}

QDateTime FreeBusyProviderBasePrivate::lastCacheUpdate()
{
    return q->lastCacheUpdate();
}

void FreeBusyProviderBasePrivate::canHandleFreeBusy(const QString &email)
{
    q->canHandleFreeBusy(email);
}

void FreeBusyProviderBasePrivate::retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end)
{
    q->retrieveFreeBusy(email, start, end);
}

FreeBusyProviderBase::FreeBusyProviderBase()
    : d(new FreeBusyProviderBasePrivate(this))
{
}

FreeBusyProviderBase::~FreeBusyProviderBase() = default;

void FreeBusyProviderBase::handlesFreeBusy(const QString &email, bool handles) const
{
    Q_EMIT d->handlesFreeBusy(email, handles);
}

void FreeBusyProviderBase::freeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText)
{
    Q_EMIT d->freeBusyRetrieved(email, freeBusy, success, errorText);
}

#include "moc_freebusyproviderbase_p.cpp"
