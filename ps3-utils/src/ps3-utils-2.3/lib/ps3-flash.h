/*
 *  PS3 flash memory os area.
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
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 */

#if !defined(_C5A0A0BE_F815_48A6_BBE0_F553923CEFEE_H)
#define _C5A0A0BE_F815_48A6_BBE0_F553923CEFEE_H

#include <stdint.h>
#include <stdio.h>

enum {
	OS_AREA_SEGMENT_SIZE = 0X200,
};

enum os_area_ldr_format {
	HEADER_LDR_FORMAT_RAW = 0,
	HEADER_LDR_FORMAT_GZIP = 1,
};

/**
 * struct os_area_header - os area header segment.
 * @magic_num: Always 'cell_ext_os_area'.
 * @hdr_version: Header format version number.
 * @db_area_offset: Starting segment number of other os database area.
 * @ldr_area_offset: Starting segment number of bootloader image area.
 * @ldr_format: HEADER_LDR_FORMAT flag.
 * @ldr_size: Size of bootloader image in bytes.
 *
 * Note that the docs refer to area offsets.  These are offsets in units of
 * segments from the start of the os area (top of the header).  These are
 * better thought of as segment numbers.  The os area of the os area is
 * reserved for the os image.
 */

struct os_area_header {
	uint8_t magic_num[16];
	uint32_t hdr_version;
	uint32_t db_area_offset;
	uint32_t ldr_area_offset;
	uint32_t _reserved_1;
	uint32_t ldr_format;
	uint32_t ldr_size;
	uint32_t _reserved_2[6];
};

enum os_area_boot_flag {
	PARAM_BOOT_FLAG_GAME_OS = 0,
	PARAM_BOOT_FLAG_OTHER_OS = 1,
};

enum {
	PARAM_CTRL_BUTTON_O_IS_YES = 0,
	PARAM_CTRL_BUTTON_X_IS_YES = 1,
};

/**
 * struct os_area_params - os area params segment.
 * @boot_flag: User preference of operating system, PARAM_BOOT_FLAG flag.
 * @num_params: Number of params in this (params) segment.
 * @rtc_diff: Difference in seconds between game os wall time and the ps3 rtc value.
 * @av_multi_out: User preference of AV output, PARAM_AV_MULTI_OUT flag.
 * @ctrl_button: User preference of controller button config, PARAM_CTRL_BUTTON
 *	flag.
 * @static_ip_addr: User preference of static IP address.
 * @network_mask: User preference of static network mask.
 * @default_gateway: User preference of static default gateway.
 * @dns_primary: User preference of static primary dns server.
 * @dns_secondary: User preference of static secondary dns server.
 *
 * User preference of zero for static_ip_addr means use dhcp.
 */

struct os_area_params {
	uint32_t boot_flag;
	uint32_t _reserved_1[3];
	uint32_t num_params;
	uint32_t _reserved_2[3];
	/* param 0 */
	int64_t rtc_diff;
	uint8_t av_multi_out;
	uint8_t ctrl_button;
	uint8_t _reserved_3[6];
	/* param 1 */
	uint8_t static_ip_addr[4];
	uint8_t network_mask[4];
	uint8_t default_gateway[4];
	uint8_t _reserved_4[4];
	/* param 2 */
	uint8_t dns_primary[4];
	uint8_t dns_secondary[4];
	uint8_t _reserved_5[8];
};

#if defined(__cplusplus)
extern "C" {
#endif

int os_area_header_read(struct os_area_header *h, FILE *dev);
int os_area_header_write(const struct os_area_header *h, FILE *dev);
int os_area_params_read(struct os_area_params *p, FILE *dev);
int os_area_params_write(const struct os_area_params *p, FILE *dev);
int os_area_image_write(FILE *image, struct os_area_header *h, FILE *dev);
int os_area_header_verify(const struct os_area_header *h);
static inline int os_area_fixed_read(struct os_area_header *h,
	struct os_area_params *p, FILE *dev)
{
	int result = os_area_header_read(h, dev);
	return result ? result : os_area_params_read(p, dev);
}
int os_area_set_ldr_format(struct os_area_header *h, FILE *dev,
	enum os_area_ldr_format value);
int os_area_set_ldr_size(struct os_area_header *h, FILE *dev,
	unsigned int value);
int os_area_set_boot_flag(struct os_area_params *p, FILE *dev,
	enum os_area_boot_flag value);
#define os_area_header_debug_dump(_a) os_area_header_dump(_a, __func__, \
	__LINE__)
void os_area_header_dump(const struct os_area_header *h, const char *func,
	int line);
#define os_area_params_debug_dump(_a) os_area_params_dump(_a, __func__, \
	__LINE__)
void os_area_params_dump(const struct os_area_params *p, const char *func,
	int line);

#if defined(__cplusplus)
}
#endif

/* flash db routines */

/**
 * struct os_area_db - Shared flash memory database.
 * @magic_num: Always '-db-' = 0x2d64622d.
 * @version: os_area_db format version number.
 * @index_64: byte offset of the database id index for 64 bit variables.
 * @count_64: number of usable 64 bit index entries
 * @index_32: byte offset of the database id index for 32 bit variables.
 * @count_32: number of usable 32 bit index entries
 * @index_16: byte offset of the database id index for 16 bit variables.
 * @count_16: number of usable 16 bit index entries
 *
 * Flash rom storage for exclusive use by guests running in the other os lpar.
 * The current system configuration allocates 1K (two segments) for other os
 * use.
 */

struct os_area_db {
	uint32_t magic_num;
	uint16_t version;
	uint16_t _reserved_1;
	uint16_t index_64;
	uint16_t count_64;
	uint16_t index_32;
	uint16_t count_32;
	uint16_t index_16;
	uint16_t count_16;
	uint32_t _reserved_2;
	uint8_t _reserved_3[1000];
};

/**
 * enum os_area_db_owner - Data owners.
 */

enum os_area_db_owner {
	OS_AREA_DB_OWNER_ANY = -1,
	OS_AREA_DB_OWNER_NONE = 0,
	OS_AREA_DB_OWNER_PROTOTYPE = 1,
	OS_AREA_DB_OWNER_LINUX = 2,
	OS_AREA_DB_OWNER_PETITBOOT = 3,
	OS_AREA_DB_OWNER_MAX = 32,
};

enum os_area_db_key {
	OS_AREA_DB_KEY_ANY = -1,
	OS_AREA_DB_KEY_NONE = 0,
	OS_AREA_DB_KEY_RTC_DIFF = 1,
	OS_AREA_DB_KEY_VIDEO_MODE = 2,
	OS_AREA_DB_KEY_MAX = 8,
};

struct os_area_db_id {
	int owner;
	int key;
};

static const struct os_area_db_id os_area_db_empty_id =
	{OS_AREA_DB_OWNER_NONE, OS_AREA_DB_KEY_NONE};
static const struct os_area_db_id os_area_db_any_id =
	{OS_AREA_DB_OWNER_ANY, OS_AREA_DB_KEY_ANY};

#if defined(__cplusplus)
extern "C" {
#endif

int os_area_db_read(struct os_area_db *db, const struct os_area_header *h,
		FILE *dev);
int os_area_db_write(const struct os_area_db *db, const struct os_area_header *h,
	FILE *dev);
void os_area_db_init(struct os_area_db *db);
int os_area_db_verify(const struct os_area_db *db);
int os_area_db_get(const struct os_area_db *db, const struct os_area_db_id *id,
	uint64_t *value);
int os_area_db_remove(struct os_area_db *db, const struct os_area_db_id *id);
int os_area_db_set_64(struct os_area_db *db, const struct os_area_db_id *id,
	uint64_t value);
int os_area_db_set_32(struct os_area_db *db, const struct os_area_db_id *id,
	uint32_t value);
int os_area_db_set_16(struct os_area_db *db, const struct os_area_db_id *id,
	uint16_t value);
void os_area_db_print(const struct os_area_db *db,
	const struct os_area_db_id *id, int verbose);
int os_area_db_format(struct os_area_db *db, const struct os_area_header *h,
	FILE *dev);
int os_area_db_get_rtc_diff(const struct os_area_db *db, int64_t *rtc_diff);
int os_area_db_get_video_mode(const struct os_area_db *db,
	unsigned int *video_mode);
int os_area_db_list_owners(char *buf, unsigned int size);
void os_area_db_dump_header(const struct os_area_db *db, const char *func,
	int line);

#if defined(__cplusplus)
}
#endif

#endif
