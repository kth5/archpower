/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <Akonadi/DifferencesAlgorithmInterface>
#include <Akonadi/GidExtractorInterface>
#include <Akonadi/ItemSerializerPlugin>

#include <KCalendarCore/ICalFormat>

namespace Akonadi
{
class SerializerPluginKCalCore : public QObject, public ItemSerializerPlugin, public DifferencesAlgorithmInterface, public GidExtractorInterface
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::ItemSerializerPlugin)
    Q_INTERFACES(Akonadi::DifferencesAlgorithmInterface)
    Q_INTERFACES(Akonadi::GidExtractorInterface)
    Q_PLUGIN_METADATA(IID "org.kde.akonadi.SerializerPluginKCalCore")
public:
    SerializerPluginKCalCore();
    bool deserialize(Item &item, const QByteArray &label, QIODevice &data, int version) override;
    void serialize(const Item &item, const QByteArray &label, QIODevice &data, int &version) override;

    void compare(Akonadi::AbstractDifferencesReporter *reporter, const Akonadi::Item &leftItem, const Akonadi::Item &rightItem) override;

    QString extractGid(const Item &item) const override;

private:
    KCalendarCore::ICalFormat mFormat;
};
}
