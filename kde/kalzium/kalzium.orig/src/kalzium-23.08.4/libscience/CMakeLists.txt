add_subdirectory(data)
add_subdirectory(tests)

add_library(science SHARED)

set_target_properties(science PROPERTIES
   VERSION ${KALZIUMLIB_VERSION}
   SOVERSION ${KALZIUMLIB_SOVERSION}
)

generate_export_header(science)

ecm_qt_declare_logging_category(science
   HEADER kalzium_libscience_debug.h
   IDENTIFIER KALZIUM_LIBSCIENCE_LOG
   CATEGORY_NAME org.kde.kalzium.libscience
   DESCRIPTION "Kalzium (libscience)"
   EXPORT KALZIUM
)

target_sources(science PRIVATE
   element.cpp
   spectrum.cpp
   isotope.cpp
   spectrumparser.cpp
   elementparser.cpp
   isotopeparser.cpp
   chemicaldataobject.cpp
   moleculeparser.cpp
   parser.cpp
   psetables.cpp
   element.h
   spectrum.h
   isotope.h
   spectrumparser.h
   elementparser.h
   isotopeparser.h
   chemicaldataobject.h
   moleculeparser.h
   parser.h
   psetables.h
)

target_link_libraries(science
   PUBLIC
      KF${KF_MAJOR_VERSION}::UnitConversion
      KF${KF_MAJOR_VERSION}::WidgetsAddons
      KF${KF_MAJOR_VERSION}::I18n
      Qt::Xml
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(science PUBLIC Qt6::Core5Compat)
endif()

install(TARGETS science  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
   chemicaldataobject.h
   element.h
   elementparser.h
   isotope.h
   isotopeparser.h
   ${CMAKE_CURRENT_BINARY_DIR}/science_export.h
   moleculeparser.h
   parser.h
   spectrum.h
   spectrumparser.h
   psetables.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR}/libkdeedu  COMPONENT Devel
)
