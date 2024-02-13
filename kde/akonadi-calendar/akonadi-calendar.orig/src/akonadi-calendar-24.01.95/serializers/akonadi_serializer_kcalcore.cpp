/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonadi_serializer_kcalcore.h"

#include <Akonadi/AbstractDifferencesReporter>
#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <KCalendarCore/Event>
#include <KCalendarCore/Todo>

#include <KCalUtils/Stringify>

#include <KLocalizedString>

#include "serializer_debug.h"
#include <QDate>
#include <QIODevice>

using namespace KCalendarCore;
using namespace KCalUtils;
using namespace Akonadi;

SerializerPluginKCalCore::SerializerPluginKCalCore() = default;

//// ItemSerializerPlugin interface

namespace
{
bool isSerializedBinary(QIODevice &data)
{
    const QByteArray buffer = data.peek(4);
    // Use QDataStream because of endianness
    quint32 magic;
    QDataStream input(buffer);
    input >> magic;
    return magic == IncidenceBase::magicSerializationIdentifier();
}

struct Header {
    quint32 magic;
    quint32 incidenceVersion;
    qint32 type;
};

} // namespace

bool SerializerPluginKCalCore::deserialize(Item &item, const QByteArray &label, QIODevice &data, int version)
{
    Q_UNUSED(version)

    if (label != Item::FullPayload) {
        return false;
    }

    Incidence::Ptr incidence;

    if (isSerializedBinary(data)) {
        const auto buffer = data.peek(sizeof(Header));
        QDataStream input(buffer);
        Header header;
        input >> header.magic;
        input >> header.incidenceVersion;
        input >> header.type;

        IncidenceBase::Ptr base;
        switch (static_cast<KCalendarCore::Incidence::IncidenceType>(header.type)) {
        case KCalendarCore::Incidence::TypeEvent:
            base = Event::Ptr(new Event());
            break;
        case KCalendarCore::Incidence::TypeTodo:
            base = Todo::Ptr(new Todo());
            break;
        case KCalendarCore::Incidence::TypeJournal:
            base = Journal::Ptr(new Journal());
            break;
        case KCalendarCore::Incidence::TypeFreeBusy:
            base = FreeBusy::Ptr(new FreeBusy());
            break;
        case KCalendarCore::Incidence::TypeUnknown:
            return false;
        }
        QDataStream stream(&data);
        stream >> base;
        incidence = base.staticCast<KCalendarCore::Incidence>();
    } else {
        // Use the old format
        incidence = mFormat.readIncidence(data.readAll());
    }

    if (!incidence) {
        qCWarning(AKONADI_SERIALIZER_CALENDAR_LOG) << "Failed to parse incidence! Item id = " << item.id() << "Storage collection id "
                                                   << item.storageCollectionId() << "parentCollectionId = " << item.parentCollection().id();
        if (!data.isSequential()) {
            data.seek(0);
            qCWarning(AKONADI_SERIALIZER_CALENDAR_LOG) << QString::fromUtf8(data.readAll());
        }
        return false;
    }

    item.setPayload(incidence);
    return true;
}

void SerializerPluginKCalCore::serialize(const Item &item, const QByteArray &label, QIODevice &data, int &version)
{
    Q_UNUSED(version)

    if (label != Item::FullPayload || !item.hasPayload<Incidence::Ptr>()) {
        return;
    }
    auto i = item.payload<Incidence::Ptr>();

    // Using an env variable for now while testing
    if (qgetenv("KCALCORE_BINARY_SERIALIZER") == QByteArray("1")) {
        QDataStream output(&data);
        IncidenceBase::Ptr base = i;
        output << base;
    } else {
        // ### I guess this can be done without hardcoding stuff
        data.write("BEGIN:VCALENDAR\nPRODID:-//K Desktop Environment//NONSGML libkcal 4.3//EN\nVERSION:2.0\nX-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\n");
        data.write(mFormat.toRawString(i));
        data.write("\nEND:VCALENDAR");
    }
}

//// DifferencesAlgorithmInterface

static bool compareString(const QString &left, const QString &right)
{
    if (left.isEmpty() && right.isEmpty()) {
        return true;
    } else {
        return left == right;
    }
}

static QString toString(const Attendee &attendee)
{
    return attendee.name() + QLatin1Char('<') + attendee.email() + QLatin1Char('>');
}

static QString toString(const Alarm::Ptr &)
{
    return {};
}

static QString toString(const Attachment &)
{
    return {};
}

static QString toString(QDate date)
{
    return date.toString();
}

static QString toString(const QDateTime &dateTime)
{
    return dateTime.toString();
}

static QString toString(const QString &str)
{
    return str;
}

static QString toString(bool value)
{
    if (value) {
        return i18n("Yes");
    } else {
        return i18n("No");
    }
}

template<class C>
static void compareList(AbstractDifferencesReporter *reporter, const QString &id, const C &left, const C &right)
{
    for (typename C::const_iterator it = left.begin(), end = left.end(); it != end; ++it) {
        if (!right.contains(*it)) {
            reporter->addProperty(AbstractDifferencesReporter::AdditionalLeftMode, id, toString(*it), QString());
        }
    }

    for (typename C::const_iterator it = right.begin(), end = right.end(); it != end; ++it) {
        if (!left.contains(*it)) {
            reporter->addProperty(AbstractDifferencesReporter::AdditionalRightMode, id, QString(), toString(*it));
        }
    }
}

static void compareIncidenceBase(AbstractDifferencesReporter *reporter, const IncidenceBase::Ptr &left, const IncidenceBase::Ptr &right)
{
    compareList(reporter, i18n("Attendees"), left->attendees(), right->attendees());

    if (!compareString(left->organizer().fullName(), right->organizer().fullName())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Organizer"), left->organizer().fullName(), right->organizer().fullName());
    }

    if (!compareString(left->uid(), right->uid())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("UID"), left->uid(), right->uid());
    }

    if (left->allDay() != right->allDay()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Is all-day"), toString(left->allDay()), toString(right->allDay()));
    }

    if (left->hasDuration() != right->hasDuration()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Has duration"), toString(left->hasDuration()), toString(right->hasDuration()));
    }

    if (left->duration() != right->duration()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode,
                              i18n("Duration"),
                              QString::number(left->duration().asSeconds()),
                              QString::number(right->duration().asSeconds()));
    }
}

static void compareIncidence(AbstractDifferencesReporter *reporter, const Incidence::Ptr &left, const Incidence::Ptr &right)
{
    if (!compareString(left->description(), right->description())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Description"), left->description(), right->description());
    }

    if (!compareString(left->summary(), right->summary())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Summary"), left->summary(), right->summary());
    }

    if (left->status() != right->status()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Status"), Stringify::incidenceStatus(left), Stringify::incidenceStatus(right));
    }

    if (left->secrecy() != right->secrecy()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Secrecy"), toString(left->secrecy()), toString(right->secrecy()));
    }

    if (left->priority() != right->priority()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Priority"), toString(left->priority()), toString(right->priority()));
    }

    if (!compareString(left->location(), right->location())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Location"), left->location(), right->location());
    }

    compareList(reporter, i18n("Categories"), left->categories(), right->categories());
    compareList(reporter, i18n("Alarms"), left->alarms(), right->alarms());
    compareList(reporter, i18n("Resources"), left->resources(), right->resources());
    compareList(reporter, i18n("Attachments"), left->attachments(), right->attachments());
    compareList(reporter, i18n("Exception Dates"), left->recurrence()->exDates(), right->recurrence()->exDates());
    compareList(reporter, i18n("Exception Times"), left->recurrence()->exDateTimes(), right->recurrence()->exDateTimes());
    // TODO: recurrence dates and date/times, exrules, rrules

    if (left->created() != right->created()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Created"), left->created().toString(), right->created().toString());
    }

    if (!compareString(left->relatedTo(), right->relatedTo())) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Related Uid"), left->relatedTo(), right->relatedTo());
    }
}

static void compareEvent(AbstractDifferencesReporter *reporter, const Event::Ptr &left, const Event::Ptr &right)
{
    if (left->dtStart() != right->dtStart()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Start time"), left->dtStart().toString(), right->dtStart().toString());
    }

    if (left->hasEndDate() != right->hasEndDate()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Has End Date"), toString(left->hasEndDate()), toString(right->hasEndDate()));
    }

    if (left->dtEnd() != right->dtEnd()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("End Date"), left->dtEnd().toString(), right->dtEnd().toString());
    }

    // TODO: check transparency
}

static void compareTodo(AbstractDifferencesReporter *reporter, const Todo::Ptr &left, const Todo::Ptr &right)
{
    if (left->hasStartDate() != right->hasStartDate()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode,
                              i18n("Has Start Date"),
                              toString(left->hasStartDate()),
                              toString(right->hasStartDate()));
    }

    if (left->hasDueDate() != right->hasDueDate()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Has Due Date"), toString(left->hasDueDate()), toString(right->hasDueDate()));
    }

    if (left->dtDue() != right->dtDue()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Due Date"), left->dtDue().toString(), right->dtDue().toString());
    }

    if (left->hasCompletedDate() != right->hasCompletedDate()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode,
                              i18n("Has Complete Date"),
                              toString(left->hasCompletedDate()),
                              toString(right->hasCompletedDate()));
    }

    if (left->percentComplete() != right->percentComplete()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode,
                              i18n("Complete"),
                              QString::number(left->percentComplete()),
                              QString::number(right->percentComplete()));
    }

    if (left->completed() != right->completed()) {
        reporter->addProperty(AbstractDifferencesReporter::ConflictMode, i18n("Completed"), toString(left->completed()), toString(right->completed()));
    }
}

void SerializerPluginKCalCore::compare(Akonadi::AbstractDifferencesReporter *reporter, const Akonadi::Item &leftItem, const Akonadi::Item &rightItem)
{
    Q_ASSERT(reporter);
    Q_ASSERT(leftItem.hasPayload<Incidence::Ptr>());
    Q_ASSERT(rightItem.hasPayload<Incidence::Ptr>());

    const auto leftIncidencePtr = leftItem.payload<Incidence::Ptr>();
    const auto rightIncidencePtr = rightItem.payload<Incidence::Ptr>();

    if (leftIncidencePtr->type() == Incidence::TypeEvent) {
        reporter->setLeftPropertyValueTitle(i18n("Changed Event"));
        reporter->setRightPropertyValueTitle(i18n("Conflicting Event"));
    } else if (leftIncidencePtr->type() == Incidence::TypeTodo) {
        reporter->setLeftPropertyValueTitle(i18n("Changed Todo"));
        reporter->setRightPropertyValueTitle(i18n("Conflicting Todo"));
    }

    compareIncidenceBase(reporter, leftIncidencePtr, rightIncidencePtr);
    compareIncidence(reporter, leftIncidencePtr, rightIncidencePtr);

    const Event::Ptr leftEvent = leftIncidencePtr.dynamicCast<Event>();
    const Event::Ptr rightEvent = rightIncidencePtr.dynamicCast<Event>();
    if (leftEvent && rightEvent) {
        compareEvent(reporter, leftEvent, rightEvent);
    } else {
        const Todo::Ptr leftTodo = leftIncidencePtr.dynamicCast<Todo>();
        const Todo::Ptr rightTodo = rightIncidencePtr.dynamicCast<Todo>();
        if (leftTodo && rightTodo) {
            compareTodo(reporter, leftTodo, rightTodo);
        }
    }
}

//// GidExtractorInterface

QString SerializerPluginKCalCore::extractGid(const Item &item) const
{
    if (!item.hasPayload<Incidence::Ptr>()) {
        return {};
    }
    return item.payload<Incidence::Ptr>()->instanceIdentifier();
}

#include "moc_akonadi_serializer_kcalcore.cpp"
