# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

set(PIM_VERSION "5.240.95")
project(Akonadi-Calendar  VERSION ${PIM_VERSION})

# ECM setup
set(KF_MIN_VERSION "5.246.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(QT_REQUIRED_VERSION "6.6.0")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)

include(ECMSetupVersion)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(KPIM_MIME_VERSION "5.240.95")
find_package(KPim6Mime ${KPIM_MIME_VERSION} CONFIG REQUIRED)
set(AKONADICALENDAR_LIB_VERSION ${PIM_VERSION})
set(AKONADICONTACT_LIB_VERSION "5.240.95")
set(AKONADIMIME_LIB_VERSION "5.240.95")
set(AKONADI_LIB_VERSION "5.240.95")
set(MAILTRANSPORT_LIB_VERSION "5.240.95")
set(CALENDARUTILS_LIB_VERSION "5.240.95")
set(IDENTITYMANAGEMENT_LIB_VERSION "5.240.95")
set(MESSAGECORE_LIB_VERSION "5.240.95")
set(MESSAGECOMPOSER_LIB_VERSION "5.240.45")
set(LIBKLEO_LIB_VERSION "5.240.95")

ecm_setup_version(PROJECT VARIABLE_PREFIX AKONADICALENDAR
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiCalendarConfigVersion.cmake"
                        SOVERSION 6
)

configure_file(akonadi-calendar-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/akonadi-calendar-version.h @ONLY)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif()

########### Find packages ###########
find_package(KF6I18n ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6WidgetsAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6XmlGui ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6KIO ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Codecs ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6DBusAddons ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6Notifications ${KF_MIN_VERSION} CONFIG REQUIRED)
find_package(KF6CalendarCore ${KF_MIN_VERSION} CONFIG REQUIRED)

find_package(KPim6MailTransport ${MAILTRANSPORT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6IdentityManagementCore ${IDENTITYMANAGEMENT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarUtils ${CALENDARUTILS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6MessageCore ${MESSAGECORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6MessageComposer ${MESSAGECOMPOSER_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6Libkleo ${LIBKLEO_LIB_VERSION} CONFIG REQUIRED)

find_package(KPim6Akonadi ${AKONADI_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiContactCore ${AKONADICONTACT_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6AkonadiMime ${AKONADI_MIME_VERSION} CONFIG REQUIRED)

########### Targets ###########


add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.6.0 KF 5.248.0)



option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
    add_definitions(-DCOMPILE_WITH_UNITY_CMAKE_SUPPORT)
endif()

add_subdirectory(src)
add_subdirectory(serializers)
add_subdirectory(reminder-daemon)
add_subdirectory(calendar-plugin)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6AkonadiCalendar")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6AkonadiCalendar_QCH
        FILE KPim6AkonadiCalendarQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6AkonadiCalendarQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPimAkonadiCalendarConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiCalendarConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiCalendarConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPim6AkonadiCalendarConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPim6AkonadiCalendarTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPim6AkonadiCalendarTargets.cmake NAMESPACE KPim6::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/akonadi-calendar_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiCalendar COMPONENT Devel
)
ecm_qt_install_logging_categories(
        EXPORT AKONADI-CALENDAR
        FILE akonadi-calendar.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ki18n_install(po)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

