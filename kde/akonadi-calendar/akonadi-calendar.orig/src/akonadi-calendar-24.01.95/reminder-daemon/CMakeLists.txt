# SPDX-FileCopyrightText: 2021 Claudio Cambra <claudio.cambra@gmail.com>
#
# SPDX-License-Identifier: BSD-2-Clause


add_executable(kalendarac)
add_definitions(-DTRANSLATION_DOMAIN=\"kalendarac\")

qt_add_dbus_interface(kalendarac_SRCS org.kde.calendar.Calendar.xml calendarinterface)

target_sources(kalendarac PRIVATE
    kalendaralarmclient.cpp
    alarmnotification.cpp
    kalendaralarmclient.h
    alarmnotification.h
    kalendaracmain.cpp
    ${kalendarac_SRCS}
    )

target_include_directories(kalendarac PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kalendarac
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::CalendarCore
    KF6::DBusAddons
    KPim6::AkonadiCore
    KPim6::AkonadiCalendar
    KPim6::IdentityManagementCore
    KF6::KIOGui
    KF6::I18n
    KF6::Notifications
    Qt::Core
    Qt::DBus
    )

ecm_qt_declare_logging_category(kalendarac
    HEADER logging.h
    IDENTIFIER REMINDER_DAEMON_LOG
    CATEGORY_NAME org.kde.kalendarac
    DESCRIPTION "Reminder daemon"
    EXPORT REMINDER_DAEMON
)

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(kalendarac PROPERTIES UNITY_BUILD ON)
endif()

install(TARGETS
    kalendarac ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )

install(FILES
    org.kde.kalendarac.desktop
    DESTINATION ${KDE_INSTALL_AUTOSTARTDIR}
    )

install(FILES kalendarac.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

set(SERV_EXEC ${KDE_INSTALL_FULL_BINDIR}/kalendarac)
configure_file(org.kde.kalendarac.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kalendarac.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kalendarac.service DESTINATION ${KDE_INSTALL_FULL_DBUSSERVICEDIR})
ecm_qt_install_logging_categories(
    EXPORT REMINDER_DAEMON
    FILE org_kde_kalendarac.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()
