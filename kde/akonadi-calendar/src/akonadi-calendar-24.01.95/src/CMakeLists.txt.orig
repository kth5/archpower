# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"libakonadi-calendar6\")

add_library(KPim6AkonadiCalendar)
add_library(KPim6::AkonadiCalendar ALIAS KPim6AkonadiCalendar)

target_sources(KPim6AkonadiCalendar PRIVATE
    blockalarmsattribute.cpp
    blockalarmsattribute.h
    calendarbase.cpp
    calendarbase.h
    calendarbase_p.h
    calendarclipboard.cpp
    calendarclipboard.h
    calendarclipboard_p.h
    calendarmodel.cpp
    calendarmodel_p.h
    calendarutils.cpp
    calendarutils.h
    calfilterpartstatusproxymodel_p.cpp
    calfilterpartstatusproxymodel_p.h
    calfilterproxymodel_p.cpp
    calfilterproxymodel_p.h
    collectioncalendar.cpp
    collectioncalendar.h
    etmcalendar.cpp
    etmcalendar.h
    etmcalendar_p.h
    fetchjobcalendar.cpp
    fetchjobcalendar.h
    fetchjobcalendar_p.h
    freebusydownloadjob_p.cpp
    freebusydownloadjob_p.h
    freebusymanager.cpp
    freebusymanager.h
    freebusymanager_p.h
    freebusyproviderbase.cpp
    freebusyproviderbase.h
    freebusyproviderbase_p.h
    history.cpp
    history.h
    history_p.cpp
    history_p.h
    icalimporter.cpp
    icalimporter.h
    icalimporter_p.h
    incidencechanger.cpp
    incidencechanger.h
    incidencechanger_p.cpp
    incidencechanger_p.h
    incidencefetchjob_p.cpp
    incidencefetchjob_p.h
    incidencetreemodel.cpp
    incidencetreemodel.h
    incidencetreemodel_p.h
    itiphandler.cpp
    itiphandler.h
    itiphandlerhelper_p.cpp
    itiphandlerhelper_p.h
    itiphandler_p.cpp
    itiphandler_p.h
    kcolumnfilterproxymodel.cpp
    kcolumnfilterproxymodel_p.h
    mailclient_p.cpp
    mailclient_p.h
    mailscheduler_p.cpp
    mailscheduler_p.h
    publishdialog.cpp
    publishdialog.h
    publishdialog_p.cpp
    publishdialog_p.h
    scheduler_p.cpp
    scheduler_p.h
    searchcollectionhelper.cpp
    searchcollectionhelper.h
    standardcalendaractionmanager.cpp
    standardcalendaractionmanager.h
    todomodel.cpp
    todomodel.h
    todopurger.cpp
    todopurger.h
    todopurger_p.h
    utils_p.cpp
    utils_p.h
)

ecm_qt_declare_logging_category(KPim6AkonadiCalendar HEADER akonadicalendar_debug.h IDENTIFIER AKONADICALENDAR_LOG CATEGORY_NAME org.kde.pim.akonadicalendar
        DESCRIPTION "akonadicalendar (pim lib)"
        OLD_CATEGORY_NAMES log_akonadicalendar
        EXPORT AKONADI-CALENDAR
    )

kconfig_add_kcfg_files(KPim6AkonadiCalendar GENERATE_MOC calendarsettings.kcfgc)
ki18n_wrap_ui(akonadicalendar_LIB_SRC publishdialog_base.ui)

qt_add_dbus_adaptor( akonadicalendar_LIB_SRC interfaces/org.freedesktop.Akonadi.Resource.FreeBusyProvider.xml freebusyproviderbase_p.h Akonadi::FreeBusyProviderBasePrivate freebusyprovideradaptor  Akonadi__FreeBusyProviderAdaptor )
target_sources(KPim6AkonadiCalendar PRIVATE ${akonadicalendar_LIB_SRC})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6AkonadiCalendar PROPERTIES UNITY_BUILD ON)
endif()
ecm_generate_export_header(KPim6AkonadiCalendar
    VERSION ${PIM_VERSION}
    BASE_NAME akonadi_calendar
    EXPORT_FILE_NAME akonadi-calendar_export.h
)


target_include_directories(KPim6AkonadiCalendar INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendar>")
target_include_directories(KPim6AkonadiCalendar PUBLIC "$<BUILD_INTERFACE:${Akonadi-Calendar_SOURCE_DIR}/src;${Akonadi-Calendar_BINARY_DIR}/src>")

target_link_libraries(KPim6AkonadiCalendar
PUBLIC
    KPim6::AkonadiCore
    KF6::CalendarCore
    KF6::WidgetsAddons
    KF6::I18n
    KF6::ConfigWidgets
    KPim6::IdentityManagementCore
PRIVATE
    KF6::Codecs
    KF6::KIOCore
    KF6::JobWidgets
    KF6::XmlGui
    KPim6::AkonadiContactCore
    KPim6::AkonadiMime
    KPim6::CalendarUtils
    KPim6::AkonadiWidgets
    KPim6::MailTransport
    KPim6::Mime
    KPim6::MessageCore
    KPim6::MessageComposer
    KPim6::Libkleo
    KF6::Codecs
)

set_target_properties(KPim6AkonadiCalendar PROPERTIES
    VERSION ${AKONADICALENDAR_VERSION}
    SOVERSION ${AKONADICALENDAR_SOVERSION}
    EXPORT_NAME AkonadiCalendar
)

install(TARGETS
    KPim6AkonadiCalendar
    EXPORT KPim6AkonadiCalendarTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(AkonadiCalendar_CamelCase_HEADERS
  HEADER_NAMES
  BlockAlarmsAttribute
  CalendarBase
  CalendarClipboard
  CalendarUtils
  CollectionCalendar
  ETMCalendar
  FetchJobCalendar
  FreeBusyManager
  FreeBusyProviderBase
  History
  ICalImporter
  IncidenceChanger
  IncidenceTreeModel
  ITIPHandler
  PublishDialog
  StandardCalendarActionManager
  SearchCollectionHelper
  TodoModel
  TodoPurger
  REQUIRED_HEADERS AkonadiCalendar_HEADERS
  PREFIX Akonadi
)

install( FILES
  ${AkonadiCalendar_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendar/Akonadi COMPONENT Devel
)
install( FILES
  ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_export.h
  ${AkonadiCalendar_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/calendarsettings.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendar/akonadi COMPONENT Devel
)


if (BUILD_QCH)
    ecm_add_qch(
        KPim6AkonadiCalendar_QCH
        NAME AkonadiCalendar
        BASE_NAME KPim6AkonadiCalendar
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${AkonadiCalendar_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            AKONADI_CALENDAR_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
