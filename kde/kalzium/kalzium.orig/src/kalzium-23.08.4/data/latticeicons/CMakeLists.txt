file(GLOB SVG_CRYSTAL "*.svgz")

install( FILES 
	${SVG_CRYSTAL}
	DESTINATION  ${KDE_INSTALL_DATADIR}/kalzium/data/latticeicons
)
