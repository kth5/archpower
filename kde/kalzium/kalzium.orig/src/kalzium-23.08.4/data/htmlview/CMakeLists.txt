file(GLOB HTMLVIEW_ICONS "*.png")

install( FILES
   ${HTMLVIEW_ICONS}
   icons.svg
   DESTINATION  ${KDE_INSTALL_DATADIR}/kalzium/data/htmlview )


