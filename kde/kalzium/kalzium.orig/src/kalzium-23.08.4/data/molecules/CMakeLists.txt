file(GLOB CML_EXAMPLE_FILES "*.cml")

install( FILES 
   ${CML_EXAMPLE_FILES}
	DESTINATION  ${KDE_INSTALL_DATADIR}/kalzium/data/molecules )
