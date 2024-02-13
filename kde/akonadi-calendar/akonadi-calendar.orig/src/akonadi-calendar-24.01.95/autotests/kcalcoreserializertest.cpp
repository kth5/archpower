/*
    SPDX-FileCopyrightText: 2009 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <Akonadi/Item>
#include <KCalendarCore/Event>
#include <QObject>
#include <QTest>

using namespace Akonadi;
using namespace KCalendarCore;

class KCalCoreSerializerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEventSerialize_data()
    {
        QTest::addColumn<QString>("mimeType");
        QTest::newRow("specific") << "application/x-vnd.akonadi.calendar.event";
        QTest::newRow("generic") << "text/calendar";
    }

    void testCharsets_data()
    {
        testEventSerialize_data();
    }

    void testEventSerialize()
    {
        QFETCH(QString, mimeType);

        QByteArray serialized =
            "BEGIN:VCALENDAR\n"
            "PRODID:-//K Desktop Environment//NONSGML libkcal 3.5//EN\n"
            "VERSION:2.0\n"
            "BEGIN:VEVENT\n"
            "DTSTAMP:20070109T100625Z\n"
            "ORGANIZER;CN=\"Volker Krause\":MAILTO:vkrause@kde.org\n"
            "CREATED:20070109T100553Z\n"
            "UID:libkcal-1135684253.945\n"
            "SEQUENCE:1\n"
            "LAST-MODIFIED:20070109T100625Z\n"
            "SUMMARY:Test event\n"
            "LOCATION:here\n"
            "CLASS:PUBLIC\n"
            "PRIORITY:5\n"
            "CATEGORIES:KDE\n"
            "DTSTART:20070109T183000Z\n"
            "DTEND:20070109T225900Z\n"
            "TRANSP:OPAQUE\n"
            "BEGIN:VALARM\n"
            "DESCRIPTION:\n"
            "ACTION:DISPLAY\n"
            "TRIGGER;VALUE=DURATION:-PT45M\n"
            "END:VALARM\n"
            "END:VEVENT\n"
            "END:VCALENDAR\n";

        // deserializing
        Item item;
        item.setMimeType(mimeType);
        item.setPayloadFromData(serialized);

        QVERIFY(item.hasPayload<Event::Ptr>());
        const auto event = item.payload<Event::Ptr>();
        QVERIFY(event != nullptr);

        QCOMPARE(event->summary(), QStringLiteral("Test event"));
        QCOMPARE(event->location(), QStringLiteral("here"));

        // serializing
        const QByteArray data = item.payloadData();
        QVERIFY(!data.isEmpty());
    }

    void testCharsets()
    {
        QFETCH(QString, mimeType);

        // 0 defaults to latin1.
        // QT5 QVERIFY( QTextCodec::codecForCStrings() == 0 );

        const QDate currentDate = QDate::currentDate();

        Event::Ptr event = Event::Ptr(new Event());
        event->setUid(QStringLiteral("12345"));
        event->setDtStart(QDateTime(currentDate, {}));
        event->setDtEnd(QDateTime(currentDate.addDays(1), {}));
        event->setAllDay(true);

        // ü
        const char latin1_umlaut[] = {static_cast<char>(0xFC), '\0'};
        event->setSummary(QLatin1String(latin1_umlaut));

        Item item;
        item.setMimeType(mimeType);
        item.setPayload(event);

        // Serializer the item, the serialization should be in UTF-8:
        const char utf_umlaut[] = {static_cast<char>(0xC3), static_cast<char>(0XBC), '\0'};
        const QByteArray bytes = item.payloadData();
        QVERIFY(bytes.contains(utf_umlaut));
        QVERIFY(!bytes.contains(latin1_umlaut));

        // Deserialize the data:
        Item item2;
        item2.setMimeType(mimeType);
        item2.setPayloadFromData(bytes);

        auto event2 = item2.payload<Event::Ptr>();
        QVERIFY(event2 != nullptr);
        QVERIFY(event2->summary().toUtf8() == QByteArray(utf_umlaut));
        QVERIFY(event2->summary().toLatin1() == QByteArray(latin1_umlaut));
    }
};

QTEST_MAIN(KCalCoreSerializerTest)

#include "kcalcoreserializertest.moc"
