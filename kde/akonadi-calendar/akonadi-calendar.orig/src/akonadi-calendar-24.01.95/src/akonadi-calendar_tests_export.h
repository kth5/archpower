/**
  SPDX-FileCopyrightText: 2017-2024 Laurent Montel <montel.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "akonadi-calendar_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef AKONADI_CALENDAR_TESTS_EXPORT
#define AKONADI_CALENDAR_TESTS_EXPORT AKONADI_CALENDAR_EXPORT
#endif
#else /* not compiling tests */
#define AKONADI_CALENDAR_TESTS_EXPORT
#endif
