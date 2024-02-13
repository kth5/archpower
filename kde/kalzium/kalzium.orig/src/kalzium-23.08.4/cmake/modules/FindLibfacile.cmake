# - Try to find Libfacile
# Once done this will define
#
#  LIBFACILE_FOUND - system has Libfacile
#  LIBFACILE_INCLUDE_DIR - the Libfacile include directory
#  LIBFACILE_LIBRARIES - Link these to use Libfacile
#
# SPDX-FileCopyrightText: 2006 Carsten Niehaus <cniehaus@gmx.de>
# SPDX-FileCopyrightText: 2006 Montel Laurent <montel@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

find_package(OCaml)

if( OCAML_FOUND )
   find_library(LIBFACILE_LIBRARIES NAMES facile.a
       HINTS ${OCAMLC_DIR}
       PATH_SUFFIXES facile ocaml/facile
   )
   find_path(LIBFACILE_INCLUDE_DIR NAMES facile.cmi
       HINTS ${OCAMLC_DIR}
       PATH_SUFFIXES facile lib/ocaml/facile
   )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libfacile DEFAULT_MSG LIBFACILE_INCLUDE_DIR
	LIBFACILE_LIBRARIES OCAML_FOUND)

# show the LIBFACILE_INCLUDE_DIR and LIBFACILE_LIBRARIES variables only in the advanced view
mark_as_advanced(LIBFACILE_INCLUDE_DIR LIBFACILE_LIBRARIES )
 
