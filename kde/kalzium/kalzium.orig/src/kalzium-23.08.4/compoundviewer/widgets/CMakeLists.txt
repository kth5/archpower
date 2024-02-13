include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(kalziumui_PART_SRCS
# nothing left here!
   )

add_widget_files(kalziumui_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kalziumui.widgets)

add_library(kalziumuiwidgets MODULE ${kalziumui_PART_SRCS})

target_link_libraries(kalziumuiwidgets ${OPENBABEL_LIBRARIES} Qt5::Designer KF5::Core)

install(TARGETS kalziumuiwidgets DESTINATION ${KDE_INSTALL_PLUGINDIR}/plugins/designer)
