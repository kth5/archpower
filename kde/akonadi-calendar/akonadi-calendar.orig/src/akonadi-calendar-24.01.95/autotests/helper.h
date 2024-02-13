/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <Akonadi/Item>

namespace Helper
{
bool confirmExists(const Akonadi::Item &item);
bool confirmDoesntExist(const Akonadi::Item &item);
Akonadi::Collection fetchCollection();
}
