# - Try to find OCaml
# Once done this will define
#
#  OCAML_FOUND - system has OCaml
#  OCAML_OCAMLC_EXECUTABLE - the Libfacile include directory
#  OCAML_OCAMLDEP_EXECUTABLE - Link these to use Libfacile
#  OCAML_OCAMLOPT_EXECUTABLE - Compiler switches required for using Libfacile
#  OCAMLC_DIR
#
# SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@gmx.de>
# SPDX-FileCopyrightText: 2006 Montel Laurent <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if ( OCAML_OCAMLC_EXECUTABLE AND OCAML_OCAMLDEP_EXECUTABLE AND OCAML_OCAMLOPT_EXECUTABLE)

  # in cache already
  set(OCAML_FOUND TRUE)

else ()

find_program(OCAML_OCAMLC_EXECUTABLE NAMES ocamlc)
find_program(OCAML_OCAMLDEP_EXECUTABLE NAMES ocamldep)
find_program(OCAML_OCAMLOPT_EXECUTABLE NAMES ocamlopt)

if( OCAML_OCAMLC_EXECUTABLE AND OCAML_OCAMLDEP_EXECUTABLE AND OCAML_OCAMLOPT_EXECUTABLE)
   execute_process(COMMAND ${OCAML_OCAMLC_EXECUTABLE} -where OUTPUT_VARIABLE OCAMLC_DIR)
   string(REPLACE "\n" "" OCAMLC_DIR "${OCAMLC_DIR}")
   #MESSAGE(STATUS "ocamlc directory <${OCAMLC_DIR}>")

     # show the LIBFACILE_INCLUDE_DIR and LIBFACILE_LIBRARIES variables only in the advanced view
     MARK_AS_ADVANCED(LIBFACILE_INCLUDE_DIR LIBFACILE_LIBRARIES )
 
   set(OCAML_FOUND TRUE)
else()

   if(NOT OCAML_OCAMLC_EXECUTABLE)
      message(STATUS "ocamlc not found.")
   endif()
   if(NOT OCAML_OCAMLDEP_EXECUTABLE)
      message(STATUS "ocamldep not found.")
   endif()
   if(NOT OCAML_OCAMLOPT_EXECUTABLE)
      message(STATUS "ocamlopt not found.")
   endif()
   set(OCAML_FOUND FALSE)
endif()


if(OCAML_FOUND)
   if(NOT OCaml_FIND_QUIETLY)
      message(STATUS "Found OCaml: ${OCAML_OCAMLC_EXECUTABLE}")
   endif()
else()
   if(OCaml_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find OCaml")
   endif()
endif()

endif ()

