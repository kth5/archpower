file(GLOB SVG_SCHOOL_ICONS "*.svg")

install( FILES 
	${SVG_SCHOOL_ICONS}
        iconinformation.txt
	DESTINATION  ${KDE_INSTALL_DATADIR}/kalzium/data/iconsets/school )
