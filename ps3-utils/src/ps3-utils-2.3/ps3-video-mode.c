/*
 *  PS3 video mode utility.
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this rrogram; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ps3-av.h"

#define PS3_UTILS_APP_NAME "ps3-video-mode"

#if defined(PACKAGE_VERSION) && defined(PACKAGE_NAME)
#define PS3_UTILS_VERSION PS3_UTILS_APP_NAME " (" PACKAGE_NAME ") " PACKAGE_VERSION "\n"
#else
#define PS3_UTILS_VERSION ""
#endif

#if defined(PACKAGE_BUGREPORT)
#define PS3_UTILS_BUGREPORT "Send bug reports to " PACKAGE_BUGREPORT ".\n"
#else
#define PS3_UTILS_BUGREPORT ""
#endif

#if defined(DEBUG)
#define DBG(_args...) do {fprintf(stderr, _args);} while(0)
#else
static inline int __attribute__ ((format (printf, 1, 2))) DBG(
	__attribute__((unused)) const char *fmt, ...) {return 0;}
#endif

static const char fb_dev[] = "/dev/fb0";

#define PS3FB_IOCTL_SETMODE          _IOW('r',  1, int)
#define PS3FB_IOCTL_GETMODE          _IOR('r',  2, int)

static void print_version(void)
{
	printf(PS3_UTILS_VERSION);
}

static void print_usage(void)
{
	fprintf(stderr, PS3_UTILS_VERSION);
	fprintf(stderr,
"SYNOPSIS\n"
"     ps3-video-mode [-m, --mode mode-id] [-r, --rgb] [-f, --full-screen]\n"
"                    [-g, --full-range] [-d, --dither] [-h, --help]\n"
"                    [-V, --version]\n"
"OPTIONS\n"
"     -m, --mode mode-id\n"
"             Set the system video mode to mode-id.\n"
"             AUTO Detect Mode:\n"
"                0: auto (480i/576i if not HDMI)\n"
"             60 Hz Broadcast Modes:\n"
"                1: 480i    (576 x 384)\n"
"                2: 480p    (576 x 384)\n"
"                3: 720p   (1124 x 644)\n"
"                4: 1080i  (1688 x 964)\n"
"                5: 1080p  (1688 x 964)\n"
"             50 Hz Broadcast Modes:\n"
"                6: 576i    (576 x 460)\n"
"                7: 576p    (576 x 460)\n"
"                8: 720p   (1124 x 644)\n"
"                9: 1080i  (1688 x 964)\n"
"               10: 1080p  (1688 x 964)\n"
"             VESA Modes:\n"
"               11: wxga   (1280 x 768)\n"
"               12: sxga   (1280 x 1024)\n"
"               13: wuxga  (1920 x 1200)\n"
"             60 Hz Full Screen Broadcast Modes:\n"
"              129: 480if   (720 x 480)\n"
"              130: 480pf   (720 x 480)\n"
"              131: 720pf  (1280 x 720)\n"
"              132: 1080if (1920 x 1080)\n"
"              133: 1080pf (1920 x 1080)\n"
"             50 Hz Full Screen Broadcast Modes:\n"
"              134: 576if   (720 x 576)\n"
"              135: 576pf   (720 x 576)\n"
"              136: 720pf  (1280 x 720)\n"
"              137: 1080if (1920 x 1080)\n"
"              138: 1080pf (1920 x 1080)\n"
"     -r, --rgb\n"
"             Use RGB color space mode.\n"
"     -f, --full-screen\n"
"             Use full screen mode.\n"
"     -g, --full-range\n"
"             Use full range mode.\n"
"     -d, --dither\n"
"             Use dither mode.\n"
"     -h, --help\n"
"             Print a help message.\n"
"     -V, --version\n"
"             Display the program version number.\n"
"See the ps3-video-mode man page for more detailed information.\n"
	);
	fprintf(stderr, PS3_UTILS_BUGREPORT);
}

static const struct option long_options[] = {
	{"mode",        required_argument, NULL, 'm'},
	{"rgb",         no_argument,       NULL, 'r'},
	{"full-screen", no_argument,       NULL, 'f'},
	{"full-range",  no_argument,       NULL, 'g'},
	{"dither",      no_argument,       NULL, 'd'},
	{"hdcp" ,       no_argument,       NULL, 'H'}, /* HDCP OFF, no support on retail PS3 */
	{"video" ,      required_argument, NULL, 'v'}, /* legacy option */
	{"help",        no_argument,       NULL, 'h'},
	{"version",     no_argument,       NULL, 'V'},
	{ NULL,         0,                 NULL, 0},
};

static const char short_options[] = "m:rfgdHv:hV";

int main(int argc, char *argv[])
{
	int result;
	int mode = -1;
	int flags = 0;
	int fd;

	while(1) {
		int c = getopt_long(argc, argv, short_options, long_options,
			NULL);

		if (c == EOF)
			break;

		switch(c) {
		case 'v':
			fprintf(stderr, "WARNING: Option -v obsolete, use -m.\n");
			/* fall through to 'm' */
		case 'm':
			mode = atoi(optarg);
			break;
		case 'r':
			flags |= PS3AV_MODE_RGB;
			break;
		case 'f':
			flags |= PS3AV_MODE_FULL;
			break;
		case 'g':
			flags |= PS3AV_MODE_COLOR;
			flags |= PS3AV_MODE_WHITE;
			break;
		case 'd':
			flags |= PS3AV_MODE_DITHER;
			break;
		case 'H':
			flags |= PS3AV_MODE_HDCP_OFF;
			break;
		case 'V':
			print_version();
			exit(0);
		case 'h':
		default:
			print_usage();
			exit(0);
		}
	}

	fd = open(fb_dev, O_RDWR);

	if (fd < 0) {
		DBG("%s:%d: open failed.\n", __func__, __LINE__);
		perror(fb_dev);
		exit(1);
	}

	if (mode < 0 && flags) {
		DBG("%s:%d: flags without mode.\n", __func__, __LINE__);
		print_usage();
		exit(1);
	}

	if (mode < 0) {
		result = ioctl(fd, PS3FB_IOCTL_GETMODE, (unsigned long)&mode);
		if (result < 0) {
			close(fd);
			perror("PS3FB_IOCTL_GETMODE failed");
			exit(1);
		}
		printf("%d\n", mode);
	} else {
		mode |= flags;
		result = ioctl(fd, PS3FB_IOCTL_SETMODE, (unsigned long)&mode);
		if (result < 0) {
			close(fd);
			perror("PS3FB_IOCTL_SETMODE failed");
			exit(1);
		}
		printf("video mode:%d\n", mode);
	}

	close(fd);

	return 0;
}

