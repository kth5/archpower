# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(akonadicalendarplugin)
target_sources(akonadicalendarplugin PRIVATE
    akonadicalendarplugin.cpp
    singlecollectioncalendar.cpp
    akonadicalendarplugin.h
    singlecollectioncalendar.h
)

ecm_qt_declare_logging_category(akonadicalendarplugin
    HEADER akonadicalendarplugin_debug.h
    IDENTIFIER AKONADICALENDARPLUGIN_LOG
    CATEGORY_NAME org.kde.kcalendarcore.akonadiplugin
    DESCRIPTION "KCalendarCore (Akonadi plugin)"
    EXPORT AKONADI-CALENDAR
)

target_link_libraries(akonadicalendarplugin PRIVATE
    KF6::CalendarCore
    KPim6::AkonadiCalendar
)

install(TARGETS akonadicalendarplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/org.kde.kcalendarcore.calendars/)
