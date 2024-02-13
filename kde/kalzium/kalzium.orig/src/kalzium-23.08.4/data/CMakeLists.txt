add_subdirectory(hazardsymbols)
add_subdirectory(htmlview)
add_subdirectory(icons)
add_subdirectory(iconsets)
add_subdirectory(toolpics)
add_subdirectory(latticeicons)
add_subdirectory(maps)
add_subdirectory(molecules)

########### install files ###############

install( FILES bg.jpg knowledge.xml tools.xml  DESTINATION  ${KDE_INSTALL_DATADIR}/kalzium/data )

