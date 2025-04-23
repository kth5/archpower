/*
 *  PS3 flash memory utility.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ps3-flash.h"

#define PS3_UTILS_APP_NAME "ps3-flash-util"

#if defined(PACKAGE_VERSION) && defined(PACKAGE_NAME)
#define PS3_UTILS_VERSION PS3_UTILS_APP_NAME " (" PACKAGE_NAME ") " PACKAGE_VERSION "\n"
#else
#define PS3_UTILS_VERSION ""
#endif

#if defined(PACKAGE_BUGREPORT)
#define PS3_UTILS_BUGREPORT "Send bug reports to " PACKAGE_BUGREPORT "\n"
#else
#define PS3_UTILS_BUGREPORT ""
#endif

#if defined(DEBUG)
#define DBG(_args...) do {fprintf(stderr, _args);} while(0)
#else
static inline int __attribute__ ((format (printf, 1, 2))) DBG(
	__attribute__((unused)) const char *fmt, ...) {return 0;}
#endif

static const char flash_dev[] = "/dev/ps3flash";

/**
 * enum db_op_type - Types of database operations.
 */

enum db_op_type {
	op_undef = 0,
	op_print,
	op_remove,
	op_set_16,
	op_set_32,
	op_set_64
};

/**
 * struct db_op_entry - A database operations list entry.
 */

struct db_op_entry {
	enum db_op_type type;
	struct os_area_db_id id;
	int64_t value;
	struct db_op_entry *next;
};

/**
 * struct db_op_entry - Add an entry into a database operations list.
 */

static int db_opt_add(struct db_op_entry** list, const struct db_op_entry *e)
{
	struct db_op_entry *p;

	DBG("%s:%d: (%d:%d) = %llu (%llxh)\n", __func__, __LINE__,
		(unsigned int)e->id.owner, (unsigned int)e->id.key,
		(unsigned long long)e->value, (unsigned long long)e->value);

	if (!*list) {
		/* init list */
		*list = malloc(sizeof(struct db_op_entry));
		p = *list;
	} else {
		/* add to list */
		p = *list;
		while(p->next)
			p = p->next;
		p->next = malloc(sizeof(struct db_op_entry));
		p = p->next;
	}

	*p = *e;
	p->next = 0;

	return 0;
}

static void print_version(void)
{
	printf(PS3_UTILS_VERSION);
}

static void print_usage(void)
{
	fprintf(stderr, PS3_UTILS_VERSION);
	fprintf(stderr,
"SYNOPSIS\n"
"     ps3-flash-util [-d, --device flash-dev] [-s, --show-settings]\n"
"                    [-w, --write-image image-file]\n"
"                    [-g, --set-game-os | -o, --set-other-os]\n"
"                    [-r, --set-raw | -z, --set-gzip] [-t, --game-time]\n"
"                    [-T, --db-test] [-F, --db-format]\n"
"                    [-P, --db-print owner key]\n"
"                    [-D, --db-write-dword owner key dword]\n"
"                    [-W, --db-write-word owner key word]\n"
"                    [-H, --db-write-half owner key half]\n"
"                    [-R, --db-remove owner key] [-L, --db-list-known]\n"
"                    [-h, --help] [-v, --verbose] [-V, --version]\n"
"OPTIONS\n"
"     -d, --device flash-dev\n"
"             Use the flash device node flash-dev (default=/dev/ps3flash).\n"
"     -s, --show-settings\n"
"             Show the current flash settings (non-destructive).\n"
"     -w, --write-image image-file\n"
"             Write the Other OS image image-file to flash memory and update\n"
"             the OS area header with information for the new image.  This is\n"
"             the option to use to write a new bootloader image to flash mem-\n"
"             ory.  Use ’-’ for data on stdin.\n"
"     -g, --set-game-os\n"
"             Set the system boot flag to Game OS.\n"
"     -o, --set-other-os\n"
"             Set the system boot flag to Other OS.\n"
"     -r, --set-raw\n"
"             Set the Other OS image compression flag to raw (not compressed).\n"
"     -z, --set-gzip\n"
"             Set the Other OS image compression flag to gzip compressed.\n"
"     -t, --game-time\n"
"             Print the Game OS RTC diff value.\n"
"     -T, --db-test\n"
"             Test for the existence of an Other OS database in flash memory.\n"
"             Exits with a zero status if a database is found.\n"
"     -F, --db-format\n"
"             Format (write) an empty Other OS database to flash memory.  Any\n"
"             existing data in the flash memory will be lost.\n"
"     -P, --db-print owner key\n"
"             Print owner:key database entries.  A negative one (-1) value for\n"
"             owner or key can be used as a wildcard to match any owner or key.\n"
"     -D, --db-write-dword owner key dword\n"
"             Add or update a 64 bit owner:key database entry.  The dword argu-\n"
"             ment supports input matching the scanf \"%%Li\" format specifica-\n"
"             tion.\n"
"     -W, --db-write-word owner key word\n"
"             Add or update a 32 bit owner:key database entry.  The word argu-\n"
"             ment supports input matching the scanf \"%%Li\" format specifica-\n"
"             tion.\n"
"     -H, --db-write-half owner key half\n"
"             Add or update a 16 bit owner:key database entry.  The half argu-\n"
"             ment supports input matching the scanf \"%%Li\" format specifica-\n"
"             tion.\n"
"     -R, --db-remove owner key\n"
"             Remove an owner:key entry from the database.  A negative one (-1)\n"
"             value for owner or key can be used as a wildcard to match any\n"
"             owner or key.\n"
"     -L, --db-list-known\n"
"             List known database owners and keys.\n"
"     -h, --help\n"
"             Print a help message.\n"
"     -v, --verbose\n"
"             Program verbosity level.  The level is additive.\n"
"     -V, --version\n"
"             Display the program version number.\n\n"
"See the ps3-flash-util man page for more detailed information.\n"
	);
	fprintf(stderr, PS3_UTILS_BUGREPORT);
}

static const struct option long_options[] = {
	{"device",         required_argument, NULL, 'd'},
	{"show-settings",  no_argument,       NULL, 's'},
	{"write-image",    required_argument, NULL, 'w'},
	{"set-game-os",    no_argument,       NULL, 'g'},
	{"set-other-os",   no_argument,       NULL, 'o'},
	{"set-raw",        no_argument,       NULL, 'r'},
	{"set-gzip",       no_argument,       NULL, 'z'},
	{"game-time",      no_argument,       NULL, 't'},
	{"db-test",        no_argument,       NULL, 'T'},
	{"db-format",      no_argument,       NULL, 'F'},
	{"db-print",       required_argument, NULL, 'P'},
	{"db-write-dword", required_argument, NULL, 'D'},
	{"db-write-word",  required_argument, NULL, 'W'},
	{"db-write-half",  required_argument, NULL, 'H'},
	{"db-remove",      required_argument, NULL, 'R'},
	{"db-list-known",  no_argument,       NULL, 'L'},
	{"help",           no_argument,       NULL, 'h'},
	{"verbose",        no_argument,       NULL, 'v'},
	{"version",        no_argument,       NULL, 'V'},
	{ NULL,            0,                 NULL, 0},
};

static const char short_options[] = "d:sw:gorztTFP:D:W:H:R:LhvV";

/**
 * enum opt_value - Tri-state options variables.
 */

enum opt_value {opt_undef = 0, opt_yes, opt_no};

/**
 * struct opts - User specified options.
 */

struct opts {
	unsigned int verbosity;
	unsigned int show_version;
	unsigned int show_help;
	unsigned int show_settings;
	unsigned int show_game_time;
	unsigned int boot_other_os;
	unsigned int compressed_image;
	unsigned int db_test;
	unsigned int db_format;
	unsigned int db_list_owners;
	struct db_op_entry* db_op_list;
	const char *device;
	const char *image;
};

static void show_settings(const struct os_area_header *h,
	const struct os_area_params *p, const struct os_area_db *db)
{
	int64_t rtc_diff;

	printf(PS3_UTILS_VERSION);
	os_area_header_dump(h, " header ", 1);
	os_area_params_dump(p, " param  ", 2);

	if (!db)
		printf(" db     :3: not found\n");
	else {
		int result;

		os_area_db_dump_header(db,    " db     ", 3);

		result = os_area_db_get_rtc_diff(db, &rtc_diff);
		if(!result)
			printf(" rtc    :4: %lld\n", (long long int)rtc_diff);
	}
}

static int write_image(FILE *dev, struct os_area_header *h, const char *image)
{
	int result;
	FILE *img;

	if (image[0] == '-' && image[1] == '\0') {
		DBG("%s:%d: read on stdin\n", __func__, __LINE__);
		img = stdin;
	} else
		img = fopen(image, "r");

	if (!img) {
		DBG("%s:%d: fopen failed.\n", __func__, __LINE__);
		perror(image);
		return -1;
	}

	result = os_area_image_write(img, h, dev);

	return result;
}

static void db_list_owners(void)
{
	char s[2048];

	os_area_db_list_owners(s, sizeof(s));
	printf(PS3_UTILS_VERSION);
	printf("%s\n", s);
}

/**
 * pickup_db_op_entry - Pickup a database option from the command line.
 */

static int pickup_db_op_entry(char *argv[], enum db_op_type type,
	unsigned int required, struct db_op_entry** list)
{
	int result;
	struct db_op_entry e;
	unsigned long long values[3];
	unsigned int i;

	values[0] = values[1] = values[2] = 0;

	for (i = 0; i < required; i++) {
		if (!argv[i + 1]) {
			if (required > i) {
				fprintf(stderr, "missing option: -%c at '%s'\n",
					argv[0][1], argv[i + 1]);
				return -1;
			}
			break;
		}

		result = sscanf(argv[i + 1], "%Li", &values[i]);

		if (result != 1) {
			fprintf(stderr, "bad option: -%c at '%s'\n",
				argv[0][1], argv[i + 1]);
			return -1;
		}

		DBG("%s:%d: (%u) found '%s' => %llu (%llxh)\n", __func__,
			__LINE__, i, argv[i + 1], values[i], values[i]);
	}

	if((int)values[0] != OS_AREA_DB_OWNER_ANY
		&& (int)values[0] >= OS_AREA_DB_OWNER_MAX) {
		fprintf(stderr, "bad owner value: %u\n",
			(unsigned int)values[0]);
		return -1;
	}

	if((int)values[1] != OS_AREA_DB_KEY_ANY
		&& (int)values[1] >= OS_AREA_DB_KEY_MAX) {
		fprintf(stderr, "bad key value: %u\n",
			(unsigned int)values[1]);
		return -1;
	}

	memset(&e, 0, sizeof(struct db_op_entry));
	e.type = type;
	e.id.owner = values[0];
	e.id.key = values[1];
	e.value = values[2];

	db_opt_add(list, &e);
	return 0;
}

static int add_db_op_entry(enum db_op_type type, enum os_area_db_owner owner,
	enum os_area_db_key key, const char *optarg, struct db_op_entry** list)
{
	int result;
	struct db_op_entry e;

	memset(&e, 0, sizeof(struct db_op_entry));
	e.type = type;
	e.id.owner = owner;
	e.id.key = key;
	result = sscanf(optarg, "%Li", (long long int *)&e.value);

	if (result != 1) {
		fprintf(stderr, "bad option: at '%s'\n", optarg);
		return -1;
	}
	db_opt_add(list, &e);
	return 0;
}

/**
 * opts_parse - Parse the command line options.
 */

static int opts_parse(struct opts* opts, int argc, char *argv[])
{
	int result;
	int got_work = 0;

	memset(opts, 0, sizeof(*opts));

	while(1) {
		int c = getopt_long(argc, argv, short_options, long_options,
			NULL);

		if (c == EOF)
			break;

		if (0) {
			DBG("%s:%d: '%c'\n", __func__, __LINE__, (char)c);
			DBG("%s:%d: optind = %d\n", __func__, __LINE__, optind);
			DBG("%s:%d: optarg = '%s'\n", __func__, __LINE__,
				(optarg ? optarg : ""));
			DBG("%s:%d: argv[optind] = '%s'\n", __func__, __LINE__,
				argv[optind]);
		}

		switch(c) {
		case 'd':
			opts->device = optarg;
			break;
		case 's':
			opts->show_settings = opt_yes;
			got_work++;
			break;
		case 'w':
			opts->image = optarg;
			got_work++;
			break;
		case 'g':
			opts->boot_other_os = opt_no;
			got_work++;
			break;
		case 'o':
			opts->boot_other_os = opt_yes;
			got_work++;
			break;
		case 'z':
			opts->compressed_image = opt_yes;
			got_work++;
			break;
		case 'r':
			opts->compressed_image = opt_no;
			got_work++;
			break;
		case 't':
			opts->show_game_time = opt_yes;
			got_work++;
			break;
		case 'T':
			opts->db_test = opt_yes;
			got_work++;
			break;
		case 'F':
			opts->db_format = opt_yes;
			got_work++;
			break;
		case 'P':
			result = pickup_db_op_entry(&argv[optind - 2],
				op_print, 2, &opts->db_op_list);
			if (result) {
				fprintf(stderr, "parse failed: %s\n\n",
					argv[optind - 2]);
				return -1;
			}
			optind += 1;
			got_work++;
			break;
		case 'D':
			result = pickup_db_op_entry(&argv[optind - 2],
				op_set_64, 3, &opts->db_op_list);
			if (result) {
				fprintf(stderr, "parse failed: %s\n\n",
					argv[optind - 2]);
				return -1;
			}
			optind += 2;
			got_work++;
			break;
		case 'W':
			result = pickup_db_op_entry(&argv[optind - 2],
				op_set_32, 3, &opts->db_op_list);
			if (result) {
				fprintf(stderr, "parse failed: %s\n\n",
					argv[optind - 2]);
				return -1;
			}
			optind += 2;
			got_work++;
			break;
		case 'H':
			result = pickup_db_op_entry(&argv[optind - 2],
				op_set_32, 3, &opts->db_op_list);
			if (result) {
				fprintf(stderr, "parse failed: %s\n\n",
					argv[optind - 2]);
				return -1;
			}
			optind += 2;
			got_work++;
			break;
		case 'R':
			result = pickup_db_op_entry(&argv[optind - 2],
				op_remove, 2, &opts->db_op_list);
			if (result) {
				fprintf(stderr, "parse failed: %s\n\n",
					argv[optind - 2]);
				return -1;
			}
			optind += 1;
			got_work++;
			break;
		case 'L':
			opts->db_list_owners = opt_yes;
			got_work++;
			break;
		case 'h':
			opts->show_help = opt_yes;
			got_work++;
			break;
		case 'v':
			opts->verbosity++;
			break;
		case 'V':
			opts->show_version = opt_yes;
			got_work++;
			break;
		default:
			opts->show_help = opt_yes;
			return -1;
		}
	}

	if (0) {
		DBG("%s:%d: argc     %d\n", __func__, __LINE__, argc);
		DBG("%s:%d: optind   %d\n", __func__, __LINE__, optind);
		DBG("%s:%d: got_work %d\n", __func__, __LINE__, got_work);
	}

	return !got_work;
}

static int process_op_list(const struct db_op_entry* db_op_list,
	struct os_area_db *db, const struct os_area_header *h, FILE *dev,
	int verbose)
{
	int result = 0;
	const struct db_op_entry* e;
	int need_write = 0;

	for (e = db_op_list; e; e = e->next) {
		DBG("%s:%d: %p: type %d, (%d:%d) = %llu\n", __func__, __LINE__,
			e, e->type, e->id.owner, e->id.key,
			(unsigned long long)e->value);

		switch(e->type) {
		case op_print:
			os_area_db_print(db, &e->id, verbose);
			break;
		case op_remove:
			result = os_area_db_remove(db, &e->id);
			need_write = !result;
			break;
		case op_set_64:
			result = os_area_db_set_64(db, &e->id, e->value);
			need_write = !result;
			break;
		case op_set_32:
			result = os_area_db_set_32(db, &e->id, e->value);
			need_write = !result;
			break;
		case op_set_16:
			result = os_area_db_set_16(db, &e->id, e->value);
			need_write = !result;
			break;
		default:
			DBG("%s:%d: unknown type %d\n", __func__, __LINE__,
				e->type);
			result = -1;
			break;
		}

		if (result) {
			fprintf(stderr, "%s:%d: process_op_list failed\n",
				__func__, __LINE__);
			return -1;
		}
	}

	if (need_write) {
		os_area_db_write(db, h, dev);
		result = os_area_db_read(db, h, dev);

		if (result)
			fprintf(stderr, "%s:%d: process_op_list failed\n",
				__func__, __LINE__);
	}

	return result;
}

int main(int argc, char *argv[])
{
	int result;
	int found_db;
	struct opts opts;
	struct os_area_header header;
	struct os_area_params params;
	struct os_area_db db;
	FILE *dev;

	result = opts_parse(&opts, argc, argv);

	if (result) {
		print_usage();
		DBG("%s:%d: failed.\n", __func__, __LINE__);
		exit(1);
	}

	if (opts.show_help) {
		print_usage();
		exit(0);
	}

	if (opts.show_version) {
		print_version();
		exit(0);
	}

	if (opts.db_list_owners)
		db_list_owners();

	opts.device = opts.device ? opts.device : flash_dev;
	dev = fopen(opts.device, "r+");

	if (!dev) {
		DBG("%s:%d: fopen failed.\n", __func__, __LINE__);
		perror(opts.device);
		exit(1);
	}

	result = os_area_fixed_read(&header, &params, dev);

	if (result) {
		fprintf(stderr, "%s:%d: os_area_read_hp error.\n", __func__,
			__LINE__);
		exit(1);
	}

	if (opts.db_format) {
		result = os_area_db_format(&db, &header, dev);
		if (result) {
			fprintf(stderr, "%s:%d: db_format failed.\n",
				__func__, __LINE__);
			exit(1);
		}
	}

	if (opts.show_game_time)
		printf("%lld\n", (long long int)params.rtc_diff);

	found_db = !os_area_db_read(&db, &header, dev);

	if (opts.db_test) {
		printf(PS3_UTILS_VERSION);
		if (found_db)
			printf("db found\n");
		else
			printf("db not found\n");
		exit(!found_db);
	}

	if (opts.show_settings)
		show_settings(&header, &params, found_db ? &db : NULL);

	if (opts.image) {
		result = write_image(dev, &header, opts.image);
		if (result) {
			fprintf(stderr, "%s:%d: write_image failed: '%s'\n",
				__func__, __LINE__, opts.image);
			exit(1);
		}
	}

	if (opts.compressed_image) {
		result = os_area_set_ldr_format(&header, dev,
			opts.compressed_image == opt_yes);
		if (result) {
			fprintf(stderr, "%s:%d: set_compress_flag failed.\n",
				__func__, __LINE__);
			exit(1);
		}
	}

	if (opts.boot_other_os) {
		result = os_area_set_boot_flag(&params, dev,
			opts.boot_other_os == opt_yes);
		if (result) {
			fprintf(stderr, "%s:%d: set_boot_flag failed.\n",
				__func__, __LINE__);
			exit(1);
		}
	}

	if (opts.db_op_list) {
		result = process_op_list(opts.db_op_list, &db, &header, dev,
			opts.verbosity);

		if (result) {
			fprintf(stderr, "%s:%d: process_op_list failed.\n",
				__func__, __LINE__);
			exit(1);
		}
	}
	return 0;
}
