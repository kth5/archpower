add_library(compoundviewer)

set_target_properties(compoundviewer PROPERTIES
    VERSION ${KALZIUMLIB_VERSION}
    SOVERSION ${KALZIUMLIB_SOVERSION}
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

generate_export_header(compoundviewer)

target_sources(compoundviewer PRIVATE
    iowrapper.cpp
    kalziumglwidget.cpp
    iowrapper.h
    kalziumglwidget.h
)

target_link_libraries(compoundviewer
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::WidgetsAddons
    KF${KF_MAJOR_VERSION}::CoreAddons
    Qt::OpenGL
    Qt::Gui
    Qt::Widgets
    ${OPENBABEL_LIBRARIES}
    AvogadroCore
    AvogadroIO
    AvogadroQtGui
    AvogadroQtOpenGL
    AvogadroQtPlugins
    Eigen3::Eigen
)

target_include_directories(compoundviewer PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${OPENBABEL_INCLUDE_DIR}
    ${AvogadroLibs_INCLUDE_DIRS}
)

install(TARGETS compoundviewer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

#add_subdirectory(widgets)
