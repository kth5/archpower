cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "4")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kalzium VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.92.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${kalzium_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMMarkNonGuiExecutable)
include(CheckIncludeFiles)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)
include(KDEGitCommitHooks)
include(KDEClangFormat)
include(ECMDeprecationSettings)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})


find_package(Gettext REQUIRED)
find_package(PythonInterp REQUIRED)
find_package(PkgConfig)
find_package(OpenBabel3 3.1)
if(NOT ${OpenBabel3_FOUND})
    find_package(OpenBabel2)
endif()
if (QT_MAJOR_VERSION STREQUAL "5")
    find_package(AvogadroLibs)
endif()
find_package(Eigen3 CONFIG)
find_package(OCaml)
find_package(Libfacile)

pkg_check_modules(CHEMICAL_MIME_DATA chemical-mime-data)
check_include_files(ieeefp.h HAVE_IEEEFP_H)

# create configuration file
set(HAVE_FACILE ${LIBFACILE_FOUND})
set(HAVE_OPENBABEL3 ${OpenBabel3_FOUND})
set(HAVE_OPENBABEL2 ${OPENBABEL2_FOUND})
if(HAVE_OPENBABEL2 OR HAVE_OPENBABEL3)
    set(HAVE_OPENBABEL 1)
endif()
set(HAVE_EIGEN ${EIGEN3_FOUND})
if (QT_MAJOR_VERSION STREQUAL "6")
    set(HAVE_AVOGADRO FALSE)
else()
    set(HAVE_AVOGADRO ${AvogadroLibs_FOUND})
endif()

if(HAVE_OPENBABEL3)
    set(OPENBABEL_INCLUDE_DIR ${OpenBabel3_INCLUDE_DIRS})
    set(OPENBABEL_LIBRARIES ${OpenBabel3_LIBRARIES})
elseif(HAVE_OPENBABEL2)
    set(OPENBABEL_INCLUDE_DIR ${OPENBABEL2_INCLUDE_DIR})
    set(OPENBABEL_LIBRARIES ${OPENBABEL2_LIBRARIES})
endif()

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/config-kalzium.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-kalzium.h
)

include_directories(${AvogadroLibs_INCLUDE_DIRS})

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_REQUIRED_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    Svg
    Widgets
    Xml
)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core5Compat StateMachine)
else()
    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Script)
endif()

find_package(KF${KF_MAJOR_VERSION} ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    Config
    CoreAddons
    DocTools
    I18n
    Parts
    Plotting
    UnitConversion
    WidgetsAddons
)

ecm_setup_version("5.0.0"
    VARIABLE_PREFIX KALZIUMLIB
)

add_definitions(
    -DQT_NO_URL_CAST_FROM_STRING
)
if(MSVC)
    # for correctly linking openbabel:
    # see https://www.mail-archive.com/openbabel-devel@lists.sourceforge.net/msg01975.html
    add_definitions(-DUSING_DYNAMIC_LIBS)
endif()

ecm_set_disabled_deprecation_versions(QT 5.14.0
    KF 5.101
)


if (HAVE_OPENBABEL AND AvogadroLibs_FOUND AND EIGEN3_FOUND)
  find_package(Qt${QT_MAJOR_VERSION}OpenGL ${QT_MIN_VERSION} REQUIRED)
  find_package(KF${KF_MAJOR_VERSION}NewStuff ${KF5_MIN_VERSION} REQUIRED)
  # avoid compilerwarnings about redefinitions
  # todo: use check_function_exits() ?
  message(STATUS "Kalzium molecular editor enabled")
  if (WIN32)
    add_definitions(-DHAVE_SNPRINTF -DHAVE_STRCASECMP -DHAVE_STRNCASECMP)
  endif ()
  add_subdirectory(compoundviewer)
else ()
  message(STATUS "Kalzium molecular editor disabled")
endif ()

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(libscience)

ki18n_install(po)
if (KF5DocTools_FOUND)
    kdoctools_install(po)
endif()
install(FILES org.kde.kalzium.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_qt_install_logging_categories(
        EXPORT KALZIUM
        FILE kalzium.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
