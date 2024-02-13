/*
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "utils_p.h"
#include "akonadicalendar_debug.h"
#include <Akonadi/CollectionDialog>
#include <KEMailSettings>
#include <KEmailAddress>
#include <KIdentityManagementCore/Identity>
#include <KIdentityManagementCore/Utils>
#include <KMime/HeaderParsing>

#include <QPointer>
#include <QWidget>

using namespace Akonadi::CalendarUtils;

Akonadi::Collection
Akonadi::CalendarUtils::selectCollection(QWidget *parent, int &dialogCode, const QStringList &mimeTypes, const Akonadi::Collection &defaultCollection)
{
    QPointer<Akonadi::CollectionDialog> dlg(new Akonadi::CollectionDialog(parent));

    qCDebug(AKONADICALENDAR_LOG) << "selecting collections with mimeType in " << mimeTypes;

    dlg->changeCollectionDialogOptions(Akonadi::CollectionDialog::KeepTreeExpanded);
    dlg->setMimeTypeFilter(mimeTypes);
    dlg->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    if (defaultCollection.isValid()) {
        dlg->setDefaultCollection(defaultCollection);
    }
    Akonadi::Collection collection;

    // FIXME: don't use exec.
    dialogCode = dlg->exec();
    if (dialogCode == QDialog::Accepted) {
        collection = dlg->selectedCollection();

        if (!collection.isValid()) {
            qCWarning(AKONADICALENDAR_LOG) << "An invalid collection was selected!";
        }
    }
    delete dlg;

    return collection;
}

QString Akonadi::CalendarUtils::fullName()
{
    KEMailSettings settings;
    QString tusername = settings.getSetting(KEMailSettings::RealName);

    // Quote the username as it might contain commas and other quotable chars.
    tusername = KEmailAddress::quoteNameIfNecessary(tusername);

    QString tname;
    QString temail;
    // ignore the return value from extractEmailAddressAndName() because
    // it will always be false since tusername does not contain "@domain".
    KEmailAddress::extractEmailAddressAndName(tusername, temail, tname);
    return tname;
}

QString Akonadi::CalendarUtils::email()
{
    KEMailSettings emailSettings;
    return emailSettings.getSetting(KEMailSettings::EmailAddress);
}

bool Akonadi::CalendarUtils::thatIsMe(const KCalendarCore::Attendee &attendee)
{
    return KIdentityManagementCore::thatIsMe(attendee.email());
}

bool Akonadi::CalendarUtils::thatIsMe(const QString &_email)
{
    const QByteArray tmp = _email.toUtf8();
    const char *cursor = tmp.constData();
    const char *end = tmp.data() + tmp.length();
    KMime::Types::Mailbox mbox;
    KMime::HeaderParsing::parseMailbox(cursor, end, mbox);
    const QString email = mbox.addrSpec().asString();

    return KIdentityManagementCore::thatIsMe(email);
}

QStringList Akonadi::CalendarUtils::allEmails()
{
    QStringList emails;
    const QSet<QString> &allEmails = KIdentityManagementCore::allEmails();
    emails.reserve(allEmails.count());
    for (const QString &email : allEmails) {
        emails.append(email);
    }
    return emails;
}
