/*
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QTemporaryFile>

#include <Akonadi/Collection>
#include <Akonadi/Item>

class CollectionCalendarTest : public QObject
{
    Q_OBJECT
public:
    explicit CollectionCalendarTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testPopulateFromReadyETM();
    void testPopulateWhenETMLoads();
    void testItemAdded();
    void testItemRemoved();
    void testItemChanged();
    void testUnrelatedItemIsNotSeen();

private:
    Akonadi::Collection testCollection;
    Akonadi::Collection otherCollection;
    QTemporaryFile otherResourceConfig;
};
