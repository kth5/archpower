add_executable(kalzium)

target_include_directories(kalzium PRIVATE
   tools
   calculator
   psetable
   isotopetable
   ${CMAKE_CURRENT_BINARY_DIR}/..
)

ecm_setup_version(${RELEASE_SERVICE_VERSION}
   VARIABLE_PREFIX KALZIUM
   VERSION_HEADER kalzium_version.h
)

ecm_qt_declare_logging_category(kalzium
   HEADER kalzium_debug.h
   IDENTIFIER KALZIUM_LOG
   CATEGORY_NAME org.kde.kalzium
   DESCRIPTION "Kalzium"
   EXPORT KALZIUM
)

# The tools can only be built when OpenBabel is found.
# The moleculeviewer needs not only OpenBabel but also Eigen and Avogadro,
# so there is a nested if-check.
if (HAVE_OPENBABEL)
   target_sources(kalzium PRIVATE
      tools/obconverter.cpp
   )

   ki18n_wrap_ui(kalzium
      tools/obconverterwidget.ui
   )
   target_include_directories(kalzium SYSTEM PRIVATE ${OPENBABEL_INCLUDE_DIR})

   if (EIGEN3_FOUND AND AvogadroLibs_FOUND)
      #include(${Avogadro_USE_FILE})

      target_sources(kalzium PRIVATE
         tools/moleculeview.cpp
         ../compoundviewer/kalziumglwidget.cpp
      )

      ki18n_wrap_ui(kalzium
         tools/moleculeviewerwidget.ui
      )
   endif ()
endif ()

## Kalzium calculator files
target_sources(kalzium PRIVATE
   calculator/calculator.cpp
   calculator/calculator.h
   calculator/concCalculator.cpp
   calculator/concCalculator.h
   calculator/gasCalculator.cpp
   calculator/gasCalculator.h
   calculator/nuclearCalculator.cpp
   calculator/nuclearCalculator.h
   calculator/titrationCalculator.cpp
   calculator/titrationCalculator.h
#   calculator/massCalculator.cpp
)
ki18n_wrap_ui(kalzium
   calculator/calculator.ui
   calculator/nuclearCalculator.ui
   calculator/gasCalculator.ui
   calculator/concCalculator.ui
   calculator/settings_calc.ui
   calculator/titrationCalculator.ui
#   calculator/massCalculator.ui
)

target_sources(kalzium PRIVATE
   detailedgraphicaloverview.cpp
   detailedgraphicaloverview.h
   detailinfodlg.cpp
   detailinfodlg.h
   elementdataviewer.cpp
   elementdataviewer.h
   exportdialog.cpp
   exportdialog.h
   gradientwidget_impl.cpp
   gradientwidget_impl.h
   isotopetable/informationitem.cpp
   isotopetable/informationitem.h
   isotopetable/isotopeguideview.cpp
   isotopetable/isotopeguideview.h
   isotopetable/isotopeitem.cpp
   isotopetable/isotopeitem.h
   isotopetable/isotopescene.cpp
   isotopetable/isotopescene.h
   isotopetable/isotopetabledialog.cpp
   isotopetable/isotopetabledialog.h
   isotopetable/isotopeview.cpp
   isotopetable/isotopeview.h
   kalzium.cpp
   kalziumdataobject.cpp
   kalziumdataobject.h
   kalziumelementproperty.cpp
   kalziumelementproperty.h
   kalziumgradienttype.cpp
   kalziumgradienttype.h
   kalzium.h
   kalziumnumerationtype.cpp
   kalziumnumerationtype.h
   kalziumschemetype.cpp
   kalziumschemetype.h
   kalziumunitcombobox.cpp
   kalziumunitcombobox.h
   kalziumutils.cpp
   kalziumutils.h
   kdeeduglossary.cpp
   kdeeduglossary.h
   legendwidget.cpp
   legendwidget.h
   main.cpp
   molcalcwidget.cpp
   molcalcwidget.h
   orbitswidget.cpp
   orbitswidget.h
   psetable/elementitem.cpp
   psetable/elementitem.h
   psetable/numerationitem.cpp
   psetable/numerationitem.h
   psetable/periodictablescene.cpp
   psetable/periodictablescene.h
   psetable/periodictablestates.cpp
   psetable/periodictablestates.h
   psetable/periodictableview.cpp
   psetable/periodictableview.h
   psetable/statemachine.cpp
   psetable/statemachine.h
   rsdialog.cpp
   rsdialog.h
   search.cpp
   search.h
   searchwidget.cpp
   searchwidget.h
   spectrumviewimpl.cpp
   spectrumviewimpl.h
   spectrumwidget.cpp
   spectrumwidget.h
   tableinfowidget.cpp
   tableinfowidget.h
   tablesdialog.cpp
   tablesdialog.h
   settings/kalziumconfigdialog.cpp
   settings/kalziumconfigdialog.h
   settings/unitsettingsdialog.cpp
   settings/unitsettingsdialog.h
   settings/isotopetablesettingsdialog.cpp
   settings/isotopetablesettingsdialog.h
   settings/isotopetablesettingscard.cpp
   settings/isotopetablesettingscard.h

   kalzium.qrc
)

if (LIBFACILE_FOUND)
   include(CMakeOCamlInstructions.cmake)

   target_sources(kalzium PRIVATE
      eqchemview.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/solver.o
      ${CMAKE_CURRENT_BINARY_DIR}/modwrap.o
   )
   ki18n_wrap_ui(kalzium
      equationview.ui
   )
endif ()

ki18n_wrap_ui(kalzium
   isotopetable/isotopedialog.ui
   plotsetupwidget.ui
   molcalcwidgetbase.ui
   spectrumview.ui
   rswidget.ui
   gradientwidget.ui
   exportdialog.ui
   settings/settings_colors.ui
   settings/settings_gradients.ui
   )

kconfig_add_kcfg_files(kalzium GENERATE_MOC prefs.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-kalzium.png")
ecm_add_app_icon(kalzium ICONS ${ICONS_SRCS})

target_link_libraries(kalzium
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::ItemViews
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::Plotting
    KF${KF_MAJOR_VERSION}::TextWidgets
    KF${KF_MAJOR_VERSION}::UnitConversion
    KF${KF_MAJOR_VERSION}::XmlGui
    Qt::Svg
    science
)
if (QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(kalzium Qt::Script)
else()
    target_link_libraries(kalzium Qt::StateMachine)
endif()
if (HAVE_OPENBABEL)
   target_link_libraries(kalzium ${OPENBABEL_LIBRARIES})
   if (EIGEN3_FOUND AND AvogadroLibs_FOUND)
      target_link_libraries(kalzium
         KF${KF_MAJOR_VERSION}::NewStuff
         Qt::OpenGL
         compoundviewer
         AvogadroQtGui
         AvogadroQtOpenGL
         AvogadroQtPlugins
         Eigen3::Eigen
       )
      install(FILES kalzium.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
   endif ()
endif ()

if (LIBFACILE_FOUND)
   link_directories(${OCAMLC_DIR})
   set(CMAKE_LIBRARY_PATH ${OCAMLC_DIR})

   find_library(OCAML_ASMRUN_LIBRARY NAMES asmrun)
   if (OCAML_ASMRUN_LIBRARY)
      target_link_libraries(kalzium ${OCAML_ASMRUN_LIBRARY})
   endif ()

   find_library(OCAML_STR_LIBRARY NAMES str)
   if (OCAML_STR_LIBRARY)
      target_link_libraries(kalzium ${OCAML_STR_LIBRARY})
   endif ()

   find_library(OCAML_NUMS_LIBRARY NAMES nums)
   if (OCAML_NUMS_LIBRARY)
      target_link_libraries(kalzium ${OCAML_NUMS_LIBRARY})
   endif ()

   target_link_libraries(kalzium m ${CMAKE_DL_LIBS})
endif ()

install(TARGETS kalzium ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kalzium.desktop org.kde.kalzium_cml.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES settings/kalzium.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_install_icons(ICONS
    16-apps-kalzium.png
    22-apps-kalzium.png
    32-apps-kalzium.png
    48-apps-kalzium.png
    64-apps-kalzium.png
    128-apps-kalzium.png
    sc-apps-kalzium.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
