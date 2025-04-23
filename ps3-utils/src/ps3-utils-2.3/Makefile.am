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

SUBDIRS = lib

composed_bin_scripts =

bin_SCRIPTS = $(composed_bin_scripts)

composed_sbin_scripts = \
	ps3-boot-game-os \
	ps3-dump-bootloader \
	ps3-rtc-init

sbin_SCRIPTS = $(composed_sbin_scripts)

composed_mans = \
	ps3-boot-game-os.8 \
	ps3-dump-bootloader.8 \
	ps3-flash-util.8 \
	ps3-rtc-init.8 \
	ps3-utils.8 \
	ps3-video-mode.8

man_MANS = $(composed_mans)

AM_CPPFLAGS = -I$(srcdir)/lib

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

AM_LDFLAGS = -Llib

bin_PROGRAMS = ps3-video-mode
sbin_PROGRAMS = ps3-flash-util

ps3_flash_util_DEPENDENCIES = Makefile
ps3_flash_util_SOURCES = ps3-flash-util.c
ps3_flash_util_LDADD = -lps3-utils

ps3_video_mode_DEPENDENCIES = Makefile
ps3_video_mode_SOURCES = ps3-video-mode.c
ps3_video_mode_LDADD = -lps3-utils

edit = sed \
	-e 's|@PACKAGE_NAME\@|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE_VERSION\@|$(PACKAGE_VERSION)|g' \
	-e 's|@PACKAGE_BUGREPORT\@|$(PACKAGE_BUGREPORT)|g' \
	-e 's|@libexecdir\@|$(libexecdir)|g' \
	-e 's|@datadir\@|$(pkgdatadir)|g' \
	-e 's|@prefix\@|$(prefix)|g'

ps3-boot-game-os: Makefile $(srcdir)/ps3-boot-game-os.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

ps3-dump-bootloader: Makefile $(srcdir)/ps3-dump-bootloader.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

ps3-rtc-init: Makefile $(srcdir)/ps3-rtc-init.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

ps3-boot-game-os.8: Makefile $(srcdir)/ps3-boot-game-os.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

ps3-dump-bootloader.8: Makefile $(srcdir)/ps3-dump-bootloader.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

ps3-flash-util.8: Makefile $(srcdir)/ps3-flash-util.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

ps3-rtc-init.8: Makefile $(srcdir)/ps3-rtc-init.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

ps3-utils.8: Makefile $(srcdir)/ps3-utils.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

ps3-video-mode.8: Makefile $(srcdir)/ps3-video-mode.8.in
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	groff -Cz -man $@.tmp
	mv $@.tmp $@

composed_files = $(composed_mans) $(composed_bin_scripts) \
	$(composed_sbin_scripts)

EXTRA_DIST = bootstrap configure.ac.in $(addsuffix .in, $(composed_files))

MAINTAINERCLEANFILES = aclocal.m4 config.* configure configure.ac depcomp \
	install-sh ltmain.sh Makefile.in missing $(PACKAGE)-*.gz

CLEANFILES = $(composed_files) $(addsuffix .tmp, $(composed_files))
