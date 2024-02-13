/*
  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB,
        <info@klaralvdalens-datakonsult.se>

  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "etmcalendar.h"

#include <KCalendarCore/Incidence>
#include <KCalendarCore/ScheduleMessage>

#include <KGuiItem>
#include <KLocalizedString>

#include <QObject>
#include <QString>
#include <QWidget>

#include <memory>

namespace Akonadi
{
class MessageQueueJob;
}

namespace KIdentityManagementCore
{
class Identity;
}

namespace Akonadi
{
class ITIPHandlerPrivate;

/**
 * @short Ui delegate for editing counter proposals.
 * @since 4.11
 */
class AKONADI_CALENDAR_EXPORT GroupwareUiDelegate : public QObject
{
    Q_OBJECT
public:
    explicit GroupwareUiDelegate(QObject *parent = nullptr);
    ~GroupwareUiDelegate() override;

    virtual void requestIncidenceEditor(const Akonadi::Item &item) = 0;
    virtual void setCalendar(const Akonadi::ETMCalendar::Ptr &calendar) = 0;
    virtual void createCalendar() = 0;
};

/**
 * @short Ui delegate for dialogs raised by the ITIPHandler and IncidenceChanger.
 * @since 4.15
 */
class AKONADI_CALENDAR_EXPORT ITIPHandlerDialogDelegate : public QObject
{
    Q_OBJECT
public:
    // Possible default actions
    enum Action {
        ActionAsk, /**< Ask the user for a decision */
        ActionSendMessage, /**< Answer with Yes */
        ActionDontSendMessage /**< Answer with No */
    };

    // How will receive the mail afterwards
    enum Recipient {
        Organizer, /**< the organizer of the incidence */
        Attendees /**< the attendees of the incidence */
    };

    /**
     * Creates a new AskDelegator
     */
    explicit ITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent = nullptr);

    /*
     * Opens a Dialog, when an incidence is created
     * The function must emit a dialogClosed signal with the user's answer
     *
     * @param recipient: to who the mail will be sent afterwards
     * @param question: dialog's question
     * @param action: Should the dialog been shown or should a default answer be returned
     * @param buttonYes: dialog's yes answer
     * @param buttonNo: dialog's no answer
     */
    virtual void openDialogIncidenceCreated(Recipient recipient,
                                            const QString &question,
                                            Action action = ActionAsk,
                                            const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                            const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

    /*
     * Opens a Dialog, when an incidence is modified
     * The function must emit a dialogClosed signal with the user's answer
     *
     * @param attendeeStatusChanged: Only the status of the own attendeeStatus is changed
     * @param recipient: to who the mail will be sent afterwards
     * @param question: dialog's question
     * @param action: Should the dialog been shown or should a default answer be returned
     * @param buttonYes: dialog's yes answer
     * @param buttonNo: dialog's no answer
     */
    virtual void openDialogIncidenceModified(bool attendeeStatusChanged,
                                             Recipient recipient,
                                             const QString &question,
                                             Action action = ActionAsk,
                                             const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                             const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

    /*
     * Opens a Dialog, when an incidence is deleted
     * The function must emit a dialogClosed signal with the user's answer
     *
     * @param recipient: to who the mail will be sent afterwards
     * @param question: dialog's question
     * @param action: Should the dialog been shown or should a default answer be returned
     * @param buttonYes: dialog's yes answer
     * @param buttonNo: dialog's no answer
     */
    virtual void openDialogIncidenceDeleted(Recipient recipient,
                                            const QString &question,
                                            Action action = ActionAsk,
                                            const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                            const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));
    /*
     * Opens a Dialog, when mail was sended
     * The function must emit a dialogClosed signal with the user's answer
     *
     * @param question: dialog's question
     * @param action: Should the dialog been shown or should a default answer be returned
     * @param buttonYes: dialog's yes answer
     * @param buttonNo: dialog's no answer
     */
    virtual void openDialogSchedulerFinished(const QString &question,
                                             Action action = ActionAsk,
                                             const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                             const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send")));

Q_SIGNALS:
    /*
     * Signal is emitted, when the user has answered the dialog or the defaultAction is used
     * @param answer: answer should be part of KMessageBox:ButtonCode, keep in mind that it is a YesNoDialog so normally it should be KMessageBox::Yes or
     * KMessageBox::No
     * @param method: itip method
     * @param incidence: purpose of the dialog
     */
    void dialogClosed(int answer, KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence);

protected:
    /*
     * Opens a KMessageBox::questionYesNo with the question
     *
     * @return KMessageBox::Yes or KMessageBox::No
     *
     * @param question: dialog's question
     * @param action: Should the dialog been shown or should a default answer be returned
     * @param buttonYes: dialog's yes answer
     * @param buttonNo: dialog's no answer
     */
    int askUserIfNeeded(const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo) const;

    // parent of the dialog
    QWidget *mParent = nullptr;

    // Incidence related to the dialog
    KCalendarCore::Incidence::Ptr mIncidence;

    // ITIPMethod related to the dialog
    KCalendarCore::iTIPMethod mMethod;
};

/**
 * @short Factory to create Akonadi::MessageQueueJob jobs or ITIPHandlerDialogDelegate objects.
 * @since 4.15
 */
class AKONADI_CALENDAR_EXPORT ITIPHandlerComponentFactory : public QObject
{
    Q_OBJECT
public:
    /*
     * Created a new ITIPHandlerComponentFactory object.
     */
    explicit ITIPHandlerComponentFactory(QObject *parent = nullptr);

    /*
     * deletes the object.
     */
    ~ITIPHandlerComponentFactory() override;

    /*
     * @return A new Akonadi::MessageQueueJob object
     * @param incidence related to the mail
     * @param identity that is the mail sender
     * @param parent of the Akonadi::MessageQueueJob object
     */
    virtual Akonadi::MessageQueueJob *
    createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagementCore::Identity &identity, QObject *parent = nullptr);

    /*
     * @return A new ITIPHandlerDialogDelegate object
     * @param incidence the purpose of the dialogs
     * @param method the ITIPMethod
     * @parent parent of the AskDelegator
     *
     */
    virtual ITIPHandlerDialogDelegate *
    createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent = nullptr);
};

/**
 * @short Handles sending of iTip messages as well as processing incoming ones.
 * @since 4.11
 */
class AKONADI_CALENDAR_EXPORT ITIPHandler : public QObject
{
    Q_OBJECT
public:
    enum Result {
        ResultError, /**< An unexpected error occurred */
        ResultSuccess, /**< The invitation was successfully handled. */
        ResultCancelled /**< User cancelled the operation. @since 4.12 */
    };

    /**
     * Creates a new ITIPHandler instance.
     * creates a default ITIPHandlerComponentFactory object.
     */
    explicit ITIPHandler(QObject *parent = nullptr);

    /**
     * Create a new ITIPHandler instance.
     * @param factory is set to 0 a new factory is created.
     * @since 4.15
     */
    explicit ITIPHandler(ITIPHandlerComponentFactory *factory, QObject *parent);
    /**
     * Destroys this instance.
     */
    ~ITIPHandler() override;

    /**
     * Processes a received iTip message.
     *
     * @param receiver
     * @param iCal
     * @param type
     *
     * @see iTipMessageProcessed()
     */
    void processiTIPMessage(const QString &receiver, const QString &iCal, const QString &type);

    /**
     * Sends an iTip message.
     *
     * @param method iTip method
     * @param incidence Incidence for which we're sending the iTip message.
     *                  Should contain a list of attendees.
     * @param parentWidget
     */
    void sendiTIPMessage(KCalendarCore::iTIPMethod method, const KCalendarCore::Incidence::Ptr &incidence, QWidget *parentWidget = nullptr);

    /**
     * Publishes incidence @p incidence.
     * A publish dialog will prompt the user to input recipients.
     * @see rfc2446 3.2.1
     */
    void publishInformation(const KCalendarCore::Incidence::Ptr &incidence, QWidget *parentWidget = nullptr);

    /**
     * Sends an e-mail with the incidence attached as iCalendar source.
     * A dialog will prompt the user to input recipients.
     */
    void sendAsICalendar(const KCalendarCore::Incidence::Ptr &incidence, QWidget *parentWidget = nullptr);

    /**
     * Sets the UI delegate to edit counter proposals.
     */
    void setGroupwareUiDelegate(GroupwareUiDelegate *delegate);

    /**
     * Sets the calendar that the itip handler should use.
     * The calendar should already be loaded.
     *
     * If none is set, a FetchJobCalendar will be created internally.
     */
    void setCalendar(const Akonadi::CalendarBase::Ptr &calendar);

    /**
     * Sets if the ITIP handler should show dialogs on error.
     * Default is true, for compatibility reasons, but this will change in KDE5.
     * TODO_KDE5: use message delegates
     *
     * @since 4.12
     */
    void setShowDialogsOnError(bool enable);

    /**
     * Returns the calendar used by this itip handler.
     */
    Akonadi::CalendarBase::Ptr calendar() const;

Q_SIGNALS:
    /**
     * Sent after processing an incoming iTip message.
     *
     * @param result success of the operation.
     * @param errorMessage translated error message suitable for user dialogs.
     *                     Empty if the operation was successful
     */
    void iTipMessageProcessed(Akonadi::ITIPHandler::Result result, const QString &errorMessage);

    /**
     * Signal emitted after an iTip message was sent through sendiTIPMessage()
     */
    void iTipMessageSent(Akonadi::ITIPHandler::Result, const QString &errorMessage);

    /**
     * Signal emitted after an incidence was published with publishInformation()
     */
    void informationPublished(Akonadi::ITIPHandler::Result, const QString &errorMessage);

    /**
     * Signal emitted after an incidence was sent with sendAsICalendar()
     */
    void sentAsICalendar(Akonadi::ITIPHandler::Result, const QString &errorMessage);

private:
    Q_DISABLE_COPY(ITIPHandler)
    std::unique_ptr<ITIPHandlerPrivate> const d;
};
}
