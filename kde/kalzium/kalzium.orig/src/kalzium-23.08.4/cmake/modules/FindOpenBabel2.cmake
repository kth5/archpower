# - Try to find OpenBabel2
# Once done this will define
#
#  OPENBABEL2_FOUND - system has OpenBabel2
#  OPENBABEL2_INCLUDE_DIR - the OpenBabel2 include directory
#  OPENBABEL2_LIBRARIES - Link these to use OpenBabel2
# SPDX-FileCopyrightText: 2006, 2009 Pino Toscano <pino@kde.org>
# SPDX-FileCopyrightText: 2006, 2007 Carsten Niehaus <cniehaus@gmx.de>
# SPDX-FileCopyrightText: 2008 Marcus D. Hanwell <marcus@cryos.org>
# SPDX-License-Identifier: BSD-3-Clause

if (OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES AND OPENBABEL2_VERSION_MET)
  # in cache already
  set(OPENBABEL2_FOUND TRUE)

else ()
  if(NOT WIN32)

    # Use the newer PkgConfig stuff
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PC_OPENBABEL2 openbabel-2.0>=2.2.0)

    if(PC_OPENBABEL2_FOUND)
      set(OPENBABEL2_VERSION_MET TRUE)
    endif()

  else()
    set(OPENBABEL2_VERSION_MET TRUE)
  endif()

  if(OPENBABEL2_VERSION_MET)

    find_path(OPENBABEL2_INCLUDE_DIR openbabel/obconversion.h
      PATHS
      ${PC_OPENBABEL2_INCLUDEDIR}
      ${PC_OPENBABEL2_INCLUDE_DIRS}
      ${GNUWIN32_DIR}/include
      $ENV{OPENBABEL2_INCLUDE_DIR}
      PATH_SUFFIXES openbabel-2.0
    )

    find_library(OPENBABEL2_LIBRARIES NAMES openbabel openbabel-2
      PATHS
      ${PC_OPENBABEL2_LIBDIR}
      ${PC_OPENBABEL2_LIBRARY_DIRS}
      ${GNUWIN32_DIR}/lib
      $ENV{OPENBABEL2_LIBRARIES}
    )
  endif()

  if(OPENBABEL2_INCLUDE_DIR AND OPENBABEL2_LIBRARIES AND OPENBABEL2_VERSION_MET)
    set(OPENBABEL2_FOUND TRUE)
  endif()

  if (OPENBABEL2_FOUND)
    if (NOT OpenBabel2_FIND_QUIETLY)
      message(STATUS "Found OpenBabel 2.2 or later: ${OPENBABEL2_LIBRARIES}")
    endif ()
  else ()
    if (OpenBabel2_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find OpenBabel 2.2 or later ")
    endif ()
  endif ()

  mark_as_advanced(OPENBABEL2_INCLUDE_DIR OPENBABEL2_LIBRARIES)

endif ()

# Search for Open Babel2 executable
if(OPENBABEL2_EXECUTABLE)

  # in cache already
  set(OPENBABEL2_EXECUTABLE_FOUND TRUE)

else()
  find_program(OPENBABEL2_EXECUTABLE NAMES babel
    PATHS
    [HKEY_CURRENT_USER\\SOFTWARE\\OpenBabel\ 2.2.0]
    $ENV{OPENBABEL2_EXECUTABLE}
  )

  if(OPENBABEL2_EXECUTABLE)
    set(OPENBABEL2_EXECUTABLE_FOUND TRUE)
  endif()

  if(OPENBABEL2_EXECUTABLE_FOUND)
    message(STATUS "Found OpenBabel2 executable: ${OPENBABEL2_EXECUTABLE}")
  endif()

endif()

