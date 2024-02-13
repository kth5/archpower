/*
    SPDX-FileCopyrightText: 2011 Grégory Oestreicher <greg@kamago.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "freebusyproviderbase.h"

#include <QObject>

class Akonadi__FreeBusyProviderAdaptor;

namespace Akonadi
{
/**
 * @internal
 * This class implements the D-Bus interface of FreeBusyProviderBase
 *
 * @since 4.7
 */
class FreeBusyProviderBasePrivate : public QObject
{
    Q_OBJECT

public:
    explicit FreeBusyProviderBasePrivate(FreeBusyProviderBase *qq);

Q_SIGNALS:
    /**
     * This signal gets emitted when the resource answered
     * the free-busy handling request.
     *
     * @param email The email address of the contact the resource
     *              answered for.
     * @param handles Whether the resource handles free-busy information
     *              (true) or not (false).
     */
    void handlesFreeBusy(const QString &email, bool handles);

    /**
     * This signal gets emitted when the resource answered the
     * free-busy retrieval request.
     *
     * @param email The email address of the contact the resource
     *              answers for.
     * @param freeBusy The free-busy data in iCal format.
     * @param success Whether the retrieval was successful or not.
     * @param errorText A human friendly error message in case something
     *                  went wrong.
     */
    void freeBusyRetrieved(const QString &email, const QString &freeBusy, bool success, const QString &errorText);

private:
    friend class FreeBusyProviderBase;
    friend class ::Akonadi__FreeBusyProviderAdaptor;

    // D-Bus calls
    QDateTime lastCacheUpdate();
    void canHandleFreeBusy(const QString &email);
    void retrieveFreeBusy(const QString &email, const QDateTime &start, const QDateTime &end);

    FreeBusyProviderBase *const q;
};
}
