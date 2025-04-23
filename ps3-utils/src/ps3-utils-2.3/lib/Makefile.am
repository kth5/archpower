#
#  Copyright 2007 Sony Corp.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

if COND_DEBUG
 extra_cppflags = -DDEBUG
else
 extra_cppflags = -DNDEBUG
endif

include_HEADERS = ps3-flash.h ps3-av.h

AM_CFLAGS = $(extra_cppflags) \
	-g \
	-Wall \
	-W \
	-Wunused \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wredundant-decls \
	-Winline

lib_LTLIBRARIES = libps3-utils.la

# version-info rules (current:revision:age):
# * If the library source code has changed since the last release, then
#   increment revision.
# * If any interfaces have been added, removed, or changed since the
#   last release, then increment current and set revision to 0.
# * If any interfaces have been added since the last release, then
#   increment age.
# * If any interfaces have been removed since the last release, then
#   set age to 0.

libps3_utils_la_DEPENDENCIES = Makefile
libps3_utils_la_SOURCES = flash.c flash-db.c
libps3_utils_la_LDFLAGS = -version-info 2:0:0 ${EXTRA_LDFLAGS}

EXTRA_DIST = bootstrap configure.ac.in

MAINTAINERCLEANFILES = aclocal.m4 config.* configure configure.ac Makefile.in
