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
 */

#include <string.h>
#include "ps3-flash.h"

#if defined(DEBUG)
#define DBG(_args...) do {fprintf(stderr, _args);} while(0)
#else
static inline int __attribute__ ((format (printf, 1, 2))) DBG(
	__attribute__((unused)) const char *fmt, ...) {return 0;}
#endif

struct index {
       uint8_t owner:5;
       uint8_t key:3;
};

struct iterator {
	const struct os_area_db *db;
	struct os_area_db_id match_id;
	struct index *idx;
	struct index *last_idx;
	union {
		uint64_t *value_64;
		uint32_t *value_32;
		uint16_t *value_16;
	};
};

static unsigned int align_up(unsigned int val, unsigned int size)
{
	return (val + (size - 1)) & (~(size - 1));
}

/* 64 bit */

/**
 * index_array_64 - Helper to get the 64 bit index array.
 */

static struct index *index_array_64(const struct os_area_db *db)
{
	return (void *)db + db->index_64;
}

/**
 * value_array_64 - Helper to get the 64 bit values array.
 */

static uint64_t *value_array_64(const struct os_area_db *db)
{
	return (void *)db + db->index_64 + align_up(db->count_64, 8);
}

/**
 * for_each_64 - Iterator for 64 bit entries.
 *
 * A NULL value for id can be used to match all entries.
 * OS_AREA_DB_OWNER_ANY and OS_AREA_DB_KEY_ANY can be used to match all.
 */

static int for_each_64(const struct os_area_db *db,
	const struct os_area_db_id *match_id, struct iterator *i)
{
next:
	if (!i->db) {
		i->db = db;
		i->match_id = match_id ? *match_id : os_area_db_any_id;
		i->idx = index_array_64(db);
		i->last_idx = i->idx + db->count_64;
		i->value_64 = value_array_64(db);
	} else {
		i->idx++;
		i->value_64++;
	}

	if (i->idx >= i->last_idx) {
		DBG("%s:%d: reached end\n", __func__, __LINE__);
		return 0;
	}

	if (i->match_id.owner != OS_AREA_DB_OWNER_ANY
		&& i->match_id.owner != (int)i->idx->owner)
		goto next;
	if (i->match_id.key != OS_AREA_DB_KEY_ANY
		&& i->match_id.key != (int)i->idx->key)
		goto next;

	return 1;
}

/**
 * enumerate_64 - Helper to find and operate on 64 bit entries.
 */

static int enumerate_64(struct os_area_db *db,
	const struct os_area_db_id *id,
	int (*fn)(void *inst, struct index *idx, uint64_t *value),
	void *inst)
{
	struct iterator i;

	for (i.db = NULL; for_each_64(db, id, &i); )
		if (fn(inst, i.idx, i.value_64))
			break;
	return 0;
}

static int delete_64(struct os_area_db *db, const struct os_area_db_id *id)
{
	//DBG("%s:%d: (%d:%d)\n", __func__, __LINE__, id->owner, id->key);
	struct iterator i;

	for (i.db = NULL; for_each_64(db, id, &i); ) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_64);

		i.idx->owner = 0;
		i.idx->key = 0;
		*i.value_64 = 0;
	}
	return 0;
}

int os_area_db_set_64(struct os_area_db *db, const struct os_area_db_id *id,
	uint64_t value)
{
	struct iterator i;

	DBG("%s:%d: (%d:%d) <= %llxh\n", __func__, __LINE__,
		id->owner, id->key, (unsigned long long)value);

	if (!id->owner || id->owner == OS_AREA_DB_OWNER_ANY
		|| id->key == OS_AREA_DB_KEY_ANY) {
		fprintf(stderr, "%s:%d: bad id: (%d:%d)\n", __func__,
			__LINE__, id->owner, id->key);
		return -1;
	}

	os_area_db_remove(db, id);

	i.db = NULL;
	if (for_each_64(db, &os_area_db_empty_id, &i)) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_64);

		i.idx->owner = id->owner;
		i.idx->key = id->key;
		*i.value_64 = value;

		DBG("%s:%d: set (%d:%d) <= %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_64);
		return 0;
	}
	fprintf(stderr, "%s:%d: database full.\n",
		__func__, __LINE__);
	return -1;
}

static int get_64(const struct os_area_db *db,
	const struct os_area_db_id *id, uint64_t *value)
{
	struct iterator i;

	i.db = NULL;
	if (for_each_64(db, id, &i)) {
		*value = *i.value_64;
		DBG("%s:%d: found %lld\n", __func__, __LINE__,
				(long long int)*i.value_64);
		return 0;
	}
	DBG("%s:%d: not found\n", __func__, __LINE__);
	return -1;
}

/* 32 bit */

/**
 * index_array_32 - Helper to get the 32 bit index array.
 */

static struct index *index_array_32(const struct os_area_db *db)
{
	return (void *)db + db->index_32;
}

/**
 * value_array_32 - Helper to get the 32 bit values array.
 */

static uint32_t *value_array_32(const struct os_area_db *db)
{
	return (void *)db + db->index_32 + align_up(db->count_32, 8);
}

/**
 * for_each_32 - Iterator for 32 bit entries.
 *
 * A NULL value for id can be used to match all entries.
 * OS_AREA_DB_OWNER_ANY and OS_AREA_DB_KEY_ANY can be used to match all.
 */

static int for_each_32(const struct os_area_db *db,
	const struct os_area_db_id *match_id, struct iterator *i)
{
next:
	if (!i->db) {
		i->db = db;
		i->match_id = match_id ? *match_id : os_area_db_any_id;
		i->idx = index_array_32(db);
		i->last_idx = i->idx + db->count_32;
		i->value_32 = value_array_32(db);
	} else {
		i->idx++;
		i->value_32++;
	}

	if (i->idx >= i->last_idx) {
		DBG("%s:%d: reached end\n", __func__, __LINE__);
		return 0;
	}

	if (i->match_id.owner != OS_AREA_DB_OWNER_ANY
		&& i->match_id.owner != (int)i->idx->owner)
		goto next;
	if (i->match_id.key != OS_AREA_DB_KEY_ANY
		&& i->match_id.key != (int)i->idx->key)
		goto next;

	return 1;
}

static int delete_32(struct os_area_db *db, const struct os_area_db_id *id)
{
	//DBG("%s:%d: (%d:%d)\n", __func__, __LINE__, id->owner, id->key);
	struct iterator i;

	for (i.db = NULL; for_each_32(db, id, &i); ) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_32);

		i.idx->owner = 0;
		i.idx->key = 0;
		*i.value_32 = 0;
	}
	return 0;
}

int os_area_db_set_32(struct os_area_db *db, const struct os_area_db_id *id,
	uint32_t value)
{
	struct iterator i;

	DBG("%s:%d: (%d:%d) <= %llxh\n", __func__, __LINE__,
		id->owner, id->key, (unsigned long long)value);

	if (!id->owner || id->owner == OS_AREA_DB_OWNER_ANY
		|| id->key == OS_AREA_DB_KEY_ANY) {
		fprintf(stderr, "%s:%d: bad id: (%d:%d)\n", __func__,
			__LINE__, id->owner, id->key);
		return -1;
	}

	os_area_db_remove(db, id);

	i.db = NULL;
	if (for_each_32(db, &os_area_db_empty_id, &i)) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_32);

		i.idx->owner = id->owner;
		i.idx->key = id->key;
		*i.value_32 = value;

		DBG("%s:%d: set (%d:%d) <= %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_32);
		return 0;
	}
	fprintf(stderr, "%s:%d: database full.\n",
		__func__, __LINE__);
	return -1;
}

static int get_32(const struct os_area_db *db,
	const struct os_area_db_id *id, uint32_t *value)
{
	struct iterator i;

	i.db = NULL;
	if (for_each_32(db, id, &i)) {
		*value = *i.value_32;
		DBG("%s:%d: found %lld\n", __func__, __LINE__,
				(long long int)*i.value_32);
		return 0;
	}
	DBG("%s:%d: not found\n", __func__, __LINE__);
	return -1;
}

/* 16 bit */

/**
 * index_array_16 - Helper to get the 16 bit index array.
 */

static struct index *index_array_16(const struct os_area_db *db)
{
	return (void *)db + db->index_16;
}

/**
 * value_array_16 - Helper to get the 16 bit values array.
 */

static uint16_t *value_array_16(const struct os_area_db *db)
{
	return (void *)db + db->index_16 + align_up(db->count_16, 8);
}

/**
 * for_each_16 - Iterator for 16 bit entries.
 *
 * A NULL value for id can be used to match all entries.
 * OS_AREA_DB_OWNER_ANY and OS_AREA_DB_KEY_ANY can be used to match all.
 */

static int for_each_16(const struct os_area_db *db,
	const struct os_area_db_id *match_id, struct iterator *i)
{
next:
	if (!i->db) {
		i->db = db;
		i->match_id = match_id ? *match_id : os_area_db_any_id;
		i->idx = index_array_16(db);
		i->last_idx = i->idx + db->count_16;
		i->value_16 = value_array_16(db);
	} else {
		i->idx++;
		i->value_16++;
	}

	if (i->idx >= i->last_idx) {
		DBG("%s:%d: reached end\n", __func__, __LINE__);
		return 0;
	}

	if (i->match_id.owner != OS_AREA_DB_OWNER_ANY
		&& i->match_id.owner != (int)i->idx->owner)
		goto next;
	if (i->match_id.key != OS_AREA_DB_KEY_ANY
		&& i->match_id.key != (int)i->idx->key)
		goto next;

	return 1;
}

static int delete_16(struct os_area_db *db, const struct os_area_db_id *id)
{
	//DBG("%s:%d: (%d:%d)\n", __func__, __LINE__, id->owner, id->key);
	struct iterator i;

	for (i.db = NULL; for_each_16(db, id, &i); ) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_16);

		i.idx->owner = 0;
		i.idx->key = 0;
		*i.value_16 = 0;
	}
	return 0;
}

int os_area_db_set_16(struct os_area_db *db, const struct os_area_db_id *id,
	uint16_t value)
{
	struct iterator i;

	DBG("%s:%d: (%d:%d) <= %llxh\n", __func__, __LINE__,
		id->owner, id->key, (unsigned long long)value);

	if (!id->owner || id->owner == OS_AREA_DB_OWNER_ANY
		|| id->key == OS_AREA_DB_KEY_ANY) {
		fprintf(stderr, "%s:%d: bad id: (%d:%d)\n", __func__,
			__LINE__, id->owner, id->key);
		return -1;
	}

	os_area_db_remove(db, id);

	i.db = NULL;
	if (for_each_16(db, &os_area_db_empty_id, &i)) {

		DBG("%s:%d: got (%d:%d) %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_16);

		i.idx->owner = id->owner;
		i.idx->key = id->key;
		*i.value_16 = value;

		DBG("%s:%d: set (%d:%d) <= %llxh\n", __func__, __LINE__,
			i.idx->owner, i.idx->key,
			(unsigned long long)*i.value_16);
		return 0;
	}
	fprintf(stderr, "%s:%d: database full.\n",
		__func__, __LINE__);
	return -1;
}

static int get_16(const struct os_area_db *db,
	const struct os_area_db_id *id, uint16_t *value)
{
	struct iterator i;

	i.db = NULL;
	if (for_each_16(db, id, &i)) {
		*value = *i.value_16;
		DBG("%s:%d: found %lld\n", __func__, __LINE__,
				(long long int)*i.value_16);
		return 0;
	}
	DBG("%s:%d: not found\n", __func__, __LINE__);
	return -1;
}

/**
 * os_area_db_remove - Remove an entry from the database.
 *
 * Does an exhaustive search to clean stale entries.
 */

int os_area_db_remove(struct os_area_db *db,
	const struct os_area_db_id *id)
{
	delete_64(db, id);
	delete_32(db, id);
	delete_16(db, id);
	return 0;
}

/**
 * os_area_db_read - Read the os area database.
 */

int os_area_db_read(struct os_area_db *db, const struct os_area_header *h,
	FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = os_area_header_verify(h);

	if (result) {
		DBG("%s:%d: invalid os_area_header\n", __func__, __LINE__);
		return -1;
	}

	/* other_os */

	result = fseek(dev, h->db_area_offset
		* OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_other_os.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fread(db, 1, sizeof(struct os_area_db), dev);

	if (bytes < sizeof(struct os_area_db)) {
		fprintf(stderr, "%s:%d: read error: os_area_other_os.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	result = os_area_db_verify(db);

	if (result) {
		fprintf(stderr, "%s:%d: invalid db header\n",
			__func__, __LINE__);
		return -1;
	}

	return 0;
}

int os_area_db_write(const struct os_area_db *db, const struct os_area_header *h,
	FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = os_area_header_verify(h);

	if (result) {
		fprintf(stderr, "%s:%d: invalid os_area_header\n",
			__func__, __LINE__);
		return -1;
	}

	if (!h->db_area_offset) {
		DBG("%s:%d: bad db_area_offset\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, h->db_area_offset * OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_other_os.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fwrite(db, 1, sizeof(struct os_area_db), dev);

	if (bytes < sizeof(struct os_area_db)) {
		fprintf(stderr, "%s:%d: fwrite error: os_area_other_os.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	return 0;
}

int os_area_db_get(const struct os_area_db *db,
	const struct os_area_db_id *id, uint64_t *value)
{
	int result;
	uint32_t tmp_32;
	uint16_t tmp_16;

	result = get_64(db, id, value);
	if (!result)
		return result;

	result = get_32(db, id, &tmp_32);
	*value = tmp_32;
	if (!result)
		return result;

	result = get_16(db, id, &tmp_16);
	*value = tmp_16;
	return result;
}

int os_area_db_get_rtc_diff(const struct os_area_db *db, int64_t *rtc_diff)
{
	static const struct os_area_db_id id = {
		.owner = OS_AREA_DB_OWNER_LINUX,
		.key = OS_AREA_DB_KEY_RTC_DIFF
	};

	return get_64(db, &id, (uint64_t*)rtc_diff);
}

int os_area_db_get_video_mode(const struct os_area_db *db,
	unsigned int *video_mode)
{
	static const struct os_area_db_id id = {
		.owner = OS_AREA_DB_OWNER_LINUX,
		.key = OS_AREA_DB_KEY_VIDEO_MODE
	};

	return get_64(db, &id, (uint64_t*)video_mode);
}

static void print_64(const struct os_area_db *db,
	const struct os_area_db_id *const id, int verbose)
{
	struct iterator i;
	unsigned int c;

	for (c = 0, i.db = NULL; for_each_64(db, id, &i); c++) {
		const char *p = (const char *)i.value_64;

		if (!verbose)
			printf("%llu\n", (unsigned long long)*i.value_64);
		else {
			char s[9];

			s[0] = (p[0] < 127 && p[0] > 31) ? p[0] : '.';
			s[1] = (p[1] < 127 && p[1] > 31) ? p[1] : '.';
			s[2] = (p[2] < 127 && p[2] > 31) ? p[2] : '.';
			s[3] = (p[3] < 127 && p[3] > 31) ? p[3] : '.';
			s[4] = (p[4] < 127 && p[4] > 31) ? p[4] : '.';
			s[5] = (p[5] < 127 && p[5] > 31) ? p[5] : '.';
			s[6] = (p[6] < 127 && p[6] > 31) ? p[6] : '.';
			s[7] = (p[7] < 127 && p[7] > 31) ? p[7] : '.';
			s[8] = 0;

			printf("  64[%2u] (%d:%d): %llxh(%llu) - %s\n", c,
				i.idx->owner, i.idx->key,
				(unsigned long long)*i.value_64,
				(unsigned long long)*i.value_64, s);
		}
	}
}

static void print_32(const struct os_area_db *db,
	const struct os_area_db_id *const id, int verbose)
{
	struct iterator i;
	unsigned int c;

	for (c = 0, i.db = NULL; for_each_32(db, id, &i); c++) {
		const char *p = (const char *)i.value_32;

		if (!verbose)
			printf("%lu\n", (unsigned long)*i.value_32);
		else {
			char s[9];

			s[0] = (p[0] < 127 && p[0] > 31) ? p[0] : '.';
			s[1] = (p[1] < 127 && p[1] > 31) ? p[1] : '.';
			s[2] = (p[2] < 127 && p[2] > 31) ? p[2] : '.';
			s[3] = (p[3] < 127 && p[3] > 31) ? p[3] : '.';
			s[4] = 0;

			printf("  32[%2u] (%d:%d): %lxh(%lu) - %s\n", c,
				i.idx->owner, i.idx->key,
				(unsigned long)*i.value_32,
				(unsigned long)*i.value_32, s);
		}
	}
}

static void print_16(const struct os_area_db *db,
	const struct os_area_db_id *const id, int verbose)
{
	struct iterator i;
	unsigned int c;

	for (c = 0, i.db = NULL; for_each_16(db, id, &i); c++) {
		const char *p = (const char *)i.value_16;

		if (!verbose)
			printf("%u\n", (unsigned int)*i.value_16);
		else {
			char s[9];


			s[0] = (p[0] < 127 && p[0] > 31) ? p[0] : '.';
			s[1] = (p[1] < 127 && p[1] > 31) ? p[1] : '.';
			s[2] = 0;

			printf("  16[%2u] (%d:%d): %xh(%u) - %s\n", c,
				i.idx->owner, i.idx->key,
				(unsigned int)*i.value_16,
				(unsigned int)*i.value_16, s);
		}
	}
}

void os_area_db_print(const struct os_area_db *db,
	const struct os_area_db_id *const id, int verbose)
{
	DBG("%s:%d: (%d:%d)\n", __func__, __LINE__, id->owner, id->key);

	print_64(db, id, verbose);
	print_32(db, id, verbose);
	print_16(db, id, verbose);
}

void os_area_db_init(struct os_area_db *db)
{
	/*
	 * item      | start | size
	 * ----------+-------+-------
	 * header    | 0     | 24
	 * index_64  | 24    | 64
	 * values_64 | 88    | 57*8 = 456
	 * index_32  | 544   | 64
	 * values_32 | 609   | 57*4 = 228
	 * index_16  | 836   | 64
	 * values_16 | 900   | 57*2 = 114
	 * end       | 1014  | -
	 */

	memset(db, 0, sizeof(struct os_area_db));

	db->magic_num = 0x2d64622dU;
	db->version = 1;
	db->index_64 = 24;
	db->count_64 = 57;
	db->index_32 = 544;
	db->count_32 = 57;
	db->index_16 = 836;
	db->count_16 = 57;
}

int os_area_db_verify(const struct os_area_db *db)
{
	if (db->magic_num != 0x2d64622dU) {
		fprintf(stderr, "%s:%d magic_num failed\n", __func__, __LINE__);
		return -1;
	}

	if (db->version != 1) {
		fprintf(stderr, "%s:%d version failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

int os_area_db_format(struct os_area_db *db, const struct os_area_header *h,
	FILE *dev)
{
	int result;

	os_area_db_init(db);
	os_area_db_write(db, h, dev);
	result = os_area_db_read(db, h, dev);

	if (result)
		fprintf(stderr, "%s:%d: db_format failed.\n", __func__,
		__LINE__);

	return result;
}

int os_area_db_list_owners(char *buf, unsigned int size)
{
	return snprintf(buf, size,
	"Known database owners:\n"
	"  owner: ANY:        %d\n"
	"  owner: PROTOTYPE:   %d\n"
	"  owner: LINUX:       %d\n"
	"  owner: PETITBOOT:   %d\n"
	"Known database keys:\n"
	"  key: ANY:          %d\n"
	"  key: RTC_DIFF:      %d\n"
	"  key: VIDEO_MODE:    %d\n",

	OS_AREA_DB_OWNER_ANY,
	OS_AREA_DB_OWNER_PROTOTYPE,
	OS_AREA_DB_OWNER_LINUX,
	OS_AREA_DB_OWNER_PETITBOOT,

	OS_AREA_DB_KEY_ANY,
	OS_AREA_DB_KEY_RTC_DIFF,
	OS_AREA_DB_KEY_VIDEO_MODE);
}

void os_area_db_dump_header(const struct os_area_db *db, const char *func,
	int line)
{
	printf("%s:%d: db.magic_num:      '%s'\n", func, line,
		(const char*)&db->magic_num);
	printf("%s:%d: db.version:         %u\n", func, line,
		db->version);
	printf("%s:%d: db.index_64:        %u\n", func, line,
		db->index_64);
	printf("%s:%d: db.count_64:        %u\n", func, line,
		db->count_64);
	printf("%s:%d: db.index_32:        %u\n", func, line,
		db->index_32);
	printf("%s:%d: db.count_32:        %u\n", func, line,
		db->count_32);
	printf("%s:%d: db.index_16:        %u\n", func, line,
		db->index_16);
	printf("%s:%d: db.count_16:        %u\n", func, line,
		db->count_16);
}
