# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar6-serializer\")
add_library(akonadi_serializer_kcalcore MODULE)

target_sources(akonadi_serializer_kcalcore PRIVATE
    akonadi_serializer_kcalcore.cpp
    akonadi_serializer_kcalcore.h
)
ecm_qt_declare_logging_category(akonadi_serializer_kcalcore HEADER serializer_debug.h IDENTIFIER AKONADI_SERIALIZER_CALENDAR_LOG CATEGORY_NAME org.kde.pim.calendar_serializer
        DESCRIPTION "calendar serializer (plugin)"
        EXPORT AKONADI-CALENDAR
    )


target_link_libraries(akonadi_serializer_kcalcore
    KPim6::CalendarUtils
    KF6::CalendarCore
    KPim6::AkonadiCore
    KF6::I18n
)
install(TARGETS akonadi_serializer_kcalcore
    DESTINATION ${KDE_INSTALL_PLUGINDIR}
)
install(FILES akonadi_serializer_kcalcore.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/akonadi/plugins/serializer
)
