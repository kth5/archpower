/*
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarclipboard_p.h"
#include <KCalUtils/DndFactory>
#include <KCalUtils/ICalDrag>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QClipboard>

using namespace Akonadi;

CalendarClipboardPrivate::CalendarClipboardPrivate(const Akonadi::CalendarBase::Ptr &calendar, Akonadi::IncidenceChanger *changer, CalendarClipboard *qq)
    : QObject()
    , m_calendar(calendar)
    , m_changer(changer)
    , q(qq)
{
    Q_ASSERT(m_calendar);
    if (!m_changer) {
        m_changer = new Akonadi::IncidenceChanger(this);
        m_changer->setHistoryEnabled(false);
        m_changer->setGroupwareCommunication(false);
    }

    m_dndfactory = new KCalUtils::DndFactory(m_calendar);

    connect(m_changer, &IncidenceChanger::modifyFinished, this, &CalendarClipboardPrivate::slotModifyFinished);

    connect(m_changer, &IncidenceChanger::deleteFinished, this, &CalendarClipboardPrivate::slotDeleteFinished);
}

CalendarClipboardPrivate::~CalendarClipboardPrivate()
{
    delete m_dndfactory;
}

void CalendarClipboardPrivate::getIncidenceHierarchy(const KCalendarCore::Incidence::Ptr &incidence, QStringList &uids)
{
    // protection against looping hierarchies
    if (incidence && !uids.contains(incidence->uid())) {
        const KCalendarCore::Incidence::List immediateChildren = m_calendar->childIncidences(incidence->uid());

        for (const KCalendarCore::Incidence::Ptr &child : immediateChildren) {
            getIncidenceHierarchy(child, uids);
        }
        uids.append(incidence->uid());
    }
}

void CalendarClipboardPrivate::cut(const KCalendarCore::Incidence::List &incidences)
{
    const bool result = m_dndfactory->copyIncidences(incidences);
    m_pendingChangeIds.clear();
    // Note: Don't use DndFactory::cutIncidences(), it doesn't use IncidenceChanger for deletion
    // we would loose async error handling and redo/undo features
    if (result) {
        Akonadi::Item::List items = m_calendar->itemList(incidences);
        const int result = m_changer->deleteIncidences(items);
        if (result == -1) {
            Q_EMIT q->cutFinished(/**success=*/false, i18n("Error performing deletion."));
        } else {
            m_pendingChangeIds << result;
        }
    } else {
        Q_EMIT q->cutFinished(/**success=*/false, i18n("Error performing copy."));
    }
}

void CalendarClipboardPrivate::cut(const KCalendarCore::Incidence::Ptr &incidence)
{
    KCalendarCore::Incidence::List incidences;
    incidences << incidence;
    cut(incidences);
}

void CalendarClipboardPrivate::makeChildsIndependent(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    const KCalendarCore::Incidence::List children = m_calendar->childIncidences(incidence->uid());

    if (children.isEmpty()) {
        cut(incidence);
    } else {
        m_pendingChangeIds.clear();
        m_abortCurrentOperation = false;
        for (const KCalendarCore::Incidence::Ptr &child : children) {
            Akonadi::Item childItem = m_calendar->item(incidence);
            if (!childItem.isValid()) {
                Q_EMIT q->cutFinished(/**success=*/false, i18n("Can't find item: %1", childItem.id()));
                return;
            }

            KCalendarCore::Incidence::Ptr newIncidence(child->clone());
            newIncidence->setRelatedTo(QString());
            childItem.setPayload<KCalendarCore::Incidence::Ptr>(newIncidence);
            const int changeId = m_changer->modifyIncidence(childItem, /*originalPayload*/ child);
            if (changeId == -1) {
                m_abortCurrentOperation = true;
                break;
            } else {
                m_pendingChangeIds << changeId;
            }
        }
        if (m_pendingChangeIds.isEmpty() && m_abortCurrentOperation) {
            Q_EMIT q->cutFinished(/**success=*/false, i18n("Error while removing relations."));
        } // if m_pendingChangeIds isn't empty, we wait for all jobs to finish first.
    }
}

void CalendarClipboardPrivate::slotModifyFinished(int changeId, const Akonadi::Item &item, IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    if (!m_pendingChangeIds.contains(changeId)) {
        return; // Not ours, someone else deleted something, not our business.
    }

    m_pendingChangeIds.remove(changeId);
    const bool isLastChange = m_pendingChangeIds.isEmpty();

    Q_UNUSED(item)
    Q_UNUSED(errorMessage)
    if (m_abortCurrentOperation && isLastChange) {
        Q_EMIT q->cutFinished(/**success=*/false, i18n("Error while removing relations."));
    } else if (!m_abortCurrentOperation) {
        if (resultCode == IncidenceChanger::ResultCodeSuccess) {
            if (isLastChange) {
                // All children are unparented, lets cut.
                Q_ASSERT(item.isValid() && item.hasPayload());
                cut(item.payload<KCalendarCore::Incidence::Ptr>());
            }
        } else {
            m_abortCurrentOperation = true;
        }
    }
}

void CalendarClipboardPrivate::slotDeleteFinished(int changeId,
                                                  const QList<Akonadi::Item::Id> &ids,
                                                  Akonadi::IncidenceChanger::ResultCode result,
                                                  const QString &errorMessage)
{
    if (!m_pendingChangeIds.contains(changeId)) {
        return; // Not ours, someone else deleted something, not our business.
    }

    m_pendingChangeIds.remove(changeId);

    Q_UNUSED(ids)
    if (result == IncidenceChanger::ResultCodeSuccess) {
        Q_EMIT q->cutFinished(/**success=*/true, QString());
    } else {
        Q_EMIT q->cutFinished(/**success=*/false, i18n("Error while deleting incidences: %1", errorMessage));
    }
}

CalendarClipboard::CalendarClipboard(const Akonadi::CalendarBase::Ptr &calendar, Akonadi::IncidenceChanger *changer, QObject *parent)
    : QObject(parent)
    , d(new CalendarClipboardPrivate(calendar, changer, this))
{
}

CalendarClipboard::~CalendarClipboard() = default;

void CalendarClipboard::cutIncidence(const KCalendarCore::Incidence::Ptr &incidence, CalendarClipboard::Mode mode)
{
    const bool hasChildren = !d->m_calendar->childIncidences(incidence->uid()).isEmpty();
    if (mode == AskMode && hasChildren) {
        const int km = KMessageBox::questionTwoActionsCancel(nullptr,
                                                             i18n("The item \"%1\" has sub-to-dos. "
                                                                  "Do you want to cut just this item and "
                                                                  "make all its sub-to-dos independent, or "
                                                                  "cut the to-do with all its sub-to-dos?",
                                                                  incidence->summary()),
                                                             i18nc("@title:window", "KOrganizer Confirmation"),
                                                             KGuiItem(i18n("Cut Only This")),
                                                             KGuiItem(i18n("Cut All")));

        if (km == KMessageBox::Cancel) {
            Q_EMIT cutFinished(/*success=*/true, QString());
            return;
        }
        mode = km == KMessageBox::ButtonCode::PrimaryAction ? SingleMode : RecursiveMode;
    } else if (mode == AskMode) {
        mode = SingleMode; // Doesn't have children, don't ask
    }

    if (mode == SingleMode) {
        d->makeChildsIndependent(incidence); // Will call d->cut(incidence) when it finishes.
    } else {
        QStringList uids;
        d->getIncidenceHierarchy(incidence, uids);
        Q_ASSERT(!uids.isEmpty());
        KCalendarCore::Incidence::List incidencesToCut;
        for (const QString &uid : std::as_const(uids)) {
            KCalendarCore::Incidence::Ptr child = d->m_calendar->incidence(uid);
            if (child) {
                incidencesToCut << child;
            }
        }
        d->cut(incidencesToCut);
    }
}

bool CalendarClipboard::copyIncidence(const KCalendarCore::Incidence::Ptr &incidence, CalendarClipboard::Mode mode)
{
    const bool hasChildren = !d->m_calendar->childIncidences(incidence->uid()).isEmpty();
    if (mode == AskMode && hasChildren) {
        const int km = KMessageBox::questionTwoActionsCancel(nullptr,
                                                             i18n("The item \"%1\" has sub-to-dos. "
                                                                  "Do you want to copy just this item or "
                                                                  "copy the to-do with all its sub-to-dos?",
                                                                  incidence->summary()),
                                                             i18nc("@title:window", "KOrganizer Confirmation"),
                                                             KGuiItem(i18n("Copy Only This")),
                                                             KGuiItem(i18n("Copy All")));
        if (km == KMessageBox::Cancel) {
            return true;
        }
        mode = km == KMessageBox::ButtonCode::PrimaryAction ? SingleMode : RecursiveMode;
    } else if (mode == AskMode) {
        mode = SingleMode; // Doesn't have children, don't ask
    }

    KCalendarCore::Incidence::List incidencesToCopy;
    if (mode == SingleMode) {
        incidencesToCopy << incidence;
    } else {
        QStringList uids;
        d->getIncidenceHierarchy(incidence, uids);
        Q_ASSERT(!uids.isEmpty());
        for (const QString &uid : std::as_const(uids)) {
            KCalendarCore::Incidence::Ptr child = d->m_calendar->incidence(uid);
            if (child) {
                incidencesToCopy << child;
            }
        }
    }

    return d->m_dndfactory->copyIncidences(incidencesToCopy);
}

bool CalendarClipboard::pasteAvailable() const
{
    return KCalUtils::ICalDrag::canDecode(QApplication::clipboard()->mimeData());
}

#include "moc_calendarclipboard.cpp"

#include "moc_calendarclipboard_p.cpp"
