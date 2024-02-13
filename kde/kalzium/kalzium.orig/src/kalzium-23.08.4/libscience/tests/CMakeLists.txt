set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

add_executable(elementparsertest xmlreadingtest.cpp)

target_link_libraries(elementparsertest science Qt::Core Qt::Xml)
ecm_mark_nongui_executable(elementparsertest)

########### next target ###############

add_executable(isotopeparsertest isotopereadingtest.cpp)
ecm_mark_nongui_executable(isotopeparsertest)

target_link_libraries(isotopeparsertest science Qt::Core Qt::Xml)

########### next target ###############

add_executable(spectrumtest spectrumreadingtests.cpp)
ecm_mark_nongui_executable(spectrumtest)

target_link_libraries(spectrumtest science Qt::Core Qt::Xml)


