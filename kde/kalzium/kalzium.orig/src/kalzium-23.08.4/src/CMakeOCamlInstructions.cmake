link_directories (${OCAMLC_DIR})

macro(OCAML_MLI_TO_CMI _cmi _mli)
   add_custom_command(OUTPUT ${_cmi}
                      COMMAND ${OCAML_OCAMLC_EXECUTABLE} ARGS -o ${_cmi} -I ${LIBFACILE_INCLUDE_DIR} -c ${_mli}
                      DEPENDS ${_mli} ${ARGN}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${_cmi})
endmacro()

macro(OCAML_ML_TO_CMX _cmx _ml)
   add_custom_command(OUTPUT ${_cmx}
                   COMMAND ${OCAML_OCAMLOPT_EXECUTABLE} ARGS -o ${_cmx} -I ${LIBFACILE_INCLUDE_DIR} -c ${_ml}
                   DEPENDS ${_ml} ${ARGN}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${_cmx})
endmacro()

ocaml_mli_to_cmi(${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi ${CMAKE_CURRENT_SOURCE_DIR}/solver/chemset.mli
                 ${CMAKE_CURRENT_SOURCE_DIR}/solver/calc.mli  )

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/chemset.ml
                ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi  )

ocaml_mli_to_cmi(${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmi ${CMAKE_CURRENT_SOURCE_DIR}/solver/datastruct.mli
                 ${CMAKE_CURRENT_SOURCE_DIR}/solver/datastruct.mli ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi)

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/datastruct.ml
                ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmi ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx  )


ocaml_mli_to_cmi(${CMAKE_CURRENT_BINARY_DIR}/chem.cmi ${CMAKE_CURRENT_SOURCE_DIR}/solver/chem.mli
                 ${CMAKE_CURRENT_SOURCE_DIR}/solver/chem.mli ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmi)

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/chem.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/chem.ml
                ${CMAKE_CURRENT_BINARY_DIR}/chem.cmi ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx  )


ocaml_mli_to_cmi(${CMAKE_CURRENT_BINARY_DIR}/parser.cmi ${CMAKE_CURRENT_SOURCE_DIR}/solver/parser.mli
                 ${CMAKE_CURRENT_SOURCE_DIR}/solver/parser.mli ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi)

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/parser.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/parser.ml
                ${CMAKE_CURRENT_BINARY_DIR}/parser.cmi ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx )

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/lexer.ml
                ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx ${CMAKE_CURRENT_BINARY_DIR}/parser.cmx  )


ocaml_mli_to_cmi(${CMAKE_CURRENT_BINARY_DIR}/calc.cmi ${CMAKE_CURRENT_SOURCE_DIR}/solver/calc.mli
                 ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmi ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx  ${CMAKE_CURRENT_SOURCE_DIR}/solver/calc.mli ${CMAKE_CURRENT_SOURCE_DIR}/solver/datastruct.mli ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmx ${CMAKE_CURRENT_BINARY_DIR}/chem.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/chem.mli ${CMAKE_CURRENT_BINARY_DIR}/parser.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/parser.mli ${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx)

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/calc.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/calc.ml
                ${CMAKE_CURRENT_BINARY_DIR}/calc.cmi )

ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/lexer.ml
                ${CMAKE_CURRENT_SOURCE_DIR}/solver/parser.ml ${CMAKE_CURRENT_SOURCE_DIR}/solver/lexer.ml )

# object files

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/modwrap.o
                   COMMAND ${OCAML_OCAMLOPT_EXECUTABLE} -I ${LIBFACILE_INCLUDE_DIR} -c ${CMAKE_CURRENT_SOURCE_DIR}/solver/modwrap.c
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/solver/modwrap.c ${CMAKE_CURRENT_BINARY_DIR}/solver.o
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/modwrap.o)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/solver.o
                   COMMAND ${OCAML_OCAMLOPT_EXECUTABLE} -output-obj -o ${CMAKE_CURRENT_BINARY_DIR}/solver.o ${LIBFACILE_INCLUDE_DIR}/facile.cmxa  ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx ${CMAKE_CURRENT_BINARY_DIR}/parser.cmx ${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmx ${CMAKE_CURRENT_BINARY_DIR}/chem.cmx ${CMAKE_CURRENT_BINARY_DIR}/calc.cmx
                   DEPENDS ${LIBFACILE_INCLUDE_DIR}/facile.cmxa  ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx ${CMAKE_CURRENT_BINARY_DIR}/parser.cmx ${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmx ${CMAKE_CURRENT_BINARY_DIR}/chem.cmx ${CMAKE_CURRENT_BINARY_DIR}/calc.cmx
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/solver.o)



if(KDE4_BUILD_TESTS)
   ocaml_ml_to_cmx(${CMAKE_CURRENT_BINARY_DIR}/main.cmx ${CMAKE_CURRENT_SOURCE_DIR}/solver/main.ml
	           ${CMAKE_CURRENT_SOURCE_DIR}/solver/main.ml  )
	
   set(atestprog.opt_SRCS ${CMAKE_CURRENT_BINARY_DIR}/main.cmx ${CMAKE_CURRENT_BINARY_DIR}/chemset.cmx ${CMAKE_CURRENT_BINARY_DIR}/parser.cmx ${CMAKE_CURRENT_BINARY_DIR}/lexer.cmx ${CMAKE_CURRENT_BINARY_DIR}/datastruct.cmx ${CMAKE_CURRENT_BINARY_DIR}/chem.cmx ${CMAKE_CURRENT_BINARY_DIR}/calc.cmx  )

   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/solver/atestprog.opt
                   COMMAND ${OCAML_OCAMLOPT_EXECUTABLE}  ARGS -o ${atestprog.opt_SRCS}
                   DEPENDS ${atestprog.opt_SRCS}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/solver/atestprog.opt)

	   #install( PROGRAMS atestprog.opt DESTINATION  ${BIN_INSTALL_DIR} )
endif()
