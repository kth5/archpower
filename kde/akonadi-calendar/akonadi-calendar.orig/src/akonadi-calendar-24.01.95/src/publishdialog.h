/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "akonadi-calendar_export.h"

#include <KCalendarCore/Attendee>
#include <QDialog>

#include <memory>

// TODO: documentation
// Uses akonadi-contact, so don't move this class to KCalUtils.
namespace Akonadi
{
class PublishDialogPrivate;

class AKONADI_CALENDAR_EXPORT PublishDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Creates a new PublishDialog
     * @param parent the dialog's parent
     */
    explicit PublishDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~PublishDialog() override;

    /**
     * Adds a new attendee to the dialog
     * @param attendee the attendee to add
     */
    void addAttendee(const KCalendarCore::Attendee &attendee);

    /**
     * Returns a list of e-mail addresses.
     * //TODO: This should be a QStringList, but KCalUtils::Scheduler::publish() accepts a QString.
     */
    [[nodiscard]] QString addresses() const;

public Q_SLOTS:
    void accept() override;

private:
    AKONADI_CALENDAR_NO_EXPORT void slotHelp();
    //@cond PRIVATE
    std::unique_ptr<PublishDialogPrivate> const d;
    //@endcond
};
}
