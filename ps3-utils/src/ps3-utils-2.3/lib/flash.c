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

#include <stdlib.h>
#include <string.h>
#include "ps3-flash.h"

#if defined(DEBUG)
#define DBG(_args...) do {fprintf(stderr, _args);} while(0)
#else
static inline int __attribute__ ((format (printf, 1, 2))) DBG(
	__attribute__((unused)) const char *fmt, ...) {return 0;}
#endif

/**
 * os_area_header_read - Read the firmware defined header.
 */

int os_area_header_read(struct os_area_header *h, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, 0, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_header.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fread(h, 1, sizeof(struct os_area_header), dev);

	if (bytes < sizeof(struct os_area_header)) {
		fprintf(stderr, "%s:%d: read error: os_area_header.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	result = os_area_header_verify(h);

	if (result) {
		fprintf(stderr, "%s:%d: invalid os_area_header\n",
			__func__, __LINE__);
		return -1;
	}

	return 0;
}

/**
 * os_area_header_write - Write the firmware defined header.
 */

int os_area_header_write(const struct os_area_header *h, FILE *dev)
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

	result = fseek(dev, 0, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_header.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fwrite(h, 1, sizeof(struct os_area_header), dev);

	if (bytes < sizeof(struct os_area_header)) {
		fprintf(stderr, "%s:%d: fwrite error: os_area_header.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	return 0;
}

/**
 * os_area_params_read - Read the firmware defined params.
 */

int os_area_params_read(struct os_area_params *p, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_params.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fread(p, 1, sizeof(struct os_area_params), dev);

	if (bytes < sizeof(struct os_area_params)) {
		fprintf(stderr, "%s:%d: read error: os_area_params.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	return 0;
}

/**
 * os_area_params_write - Write the firmware defined params.
 */

int os_area_params_write(const struct os_area_params *p, FILE *dev)
{
	int result;
	size_t bytes;

	if (!dev) {
		DBG("%s:%d: bad stream\n", __func__, __LINE__);
		return -1;
	}

	result = fseek(dev, OS_AREA_SEGMENT_SIZE, SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: os_area_params.\n",
			__func__, __LINE__);
		perror(0);
		return result;
	}

	bytes = fwrite(p, 1, sizeof(struct os_area_params), dev);

	if (bytes < sizeof(struct os_area_params)) {
		fprintf(stderr, "%s:%d: fwrite error: os_area_params.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	return 0;
}

/**
 * os_area_image_write - Write the other os boot image.
 */

int os_area_image_write(FILE *image, struct os_area_header *h, FILE *dev)
{
	int result;
	long flash_size;
	size_t image_size;
	size_t bytes;
	void* buf;

	result = fseek(dev, 0, SEEK_END);

	if (result) {
		fprintf(stderr, "%s:%d: fseek failed\n", __func__, __LINE__);
		perror(0);
		return -1;
	}

	flash_size = ftell(dev) - h->ldr_area_offset * OS_AREA_SEGMENT_SIZE;

	DBG("%s:%d: flash_size %lxh\n", __func__, __LINE__, flash_size);

	buf = malloc(flash_size);

	if (!buf) {
		fprintf(stderr, "%s:%d: malloc failed\n", __func__, __LINE__);
		perror(0);
		return -1;
	}

	image_size = fread(buf, 1, flash_size, image);

	DBG("%s:%d: image_size %xh\n", __func__, __LINE__,
		(unsigned int)image_size);

	if ((long)image_size > flash_size) {
		fprintf(stderr, "%s:%d: image-file too big.\n", __func__,
			__LINE__);
		perror(0);
		return -1;
	}

	result = fseek(dev, h->ldr_area_offset * OS_AREA_SEGMENT_SIZE,
		SEEK_SET);

	if (result) {
		fprintf(stderr, "%s:%d: seek error: ldr_area_offset.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	bytes = fwrite(buf, 1, image_size, dev);

	if (bytes < image_size) {
		fprintf(stderr, "%s:%d: fwrite error: image-file.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	result = os_area_set_ldr_size(h, dev, image_size);

	if (result) {
		fprintf(stderr, "%s:%d: os_area_set_ldr_size failed.\n",
			__func__, __LINE__);
		perror(0);
		return -1;
	}

	fprintf(stderr, "wrote image to flash (%xh bytes).\n",
		(unsigned int)image_size);

free(buf);
	return result;
}

/**
 * os_area_header_verify - Verify the firmware defined header.
 */

int os_area_header_verify(const struct os_area_header *h)
{
	if (memcmp(h->magic_num, "cell_ext_os_area", 16)) {
		fprintf(stderr, "%s:%d: magic_num failed\n", __func__,
			__LINE__);
		return -1;
	}

	if (h->hdr_version < 1) {
		fprintf(stderr, "%s:%d: hdr_version failed\n", __func__,
			__LINE__);
		return -1;
	}

	if (h->db_area_offset > h->ldr_area_offset) {
		fprintf(stderr, "%s:%d: offsets failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

/**
 * os_area_set_ldr_format - Set the data format of the other os boot image.
 * @value: Specifies the format of the os boot image, one of enum ldr_format.
 */

int os_area_set_ldr_format(struct os_area_header *h, FILE *dev,
	enum os_area_ldr_format value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)h->ldr_format, (unsigned int)h->ldr_format,
		value, value);

	if (h->ldr_format == (uint32_t)value)
		return 0;

	h->ldr_format = (uint32_t)value ? HEADER_LDR_FORMAT_GZIP
		: HEADER_LDR_FORMAT_RAW; /* for now just flatten to gzip */

	os_area_header_write(h, dev);
	result = os_area_header_read(h, dev);

	if (result)
		fprintf(stderr, "%s:%d: os_area_set_ldr_format failed.\n",
			__func__, __LINE__);
	return result;
}

/**
 * os_area_set_ldr_size - Set the size in bytes of the other os boot image.
 * @value: Size in bytes.
 */

int os_area_set_ldr_size(struct os_area_header *h, FILE *dev,
	unsigned int value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)h->ldr_size, (unsigned int)h->ldr_size,
		value, value);

	if (h->ldr_size == (uint32_t)value)
		return 0;

	h->ldr_size = (uint32_t)value;

	os_area_header_write(h, dev);
	result = os_area_header_read(h, dev);

	if (result)
		fprintf(stderr, "%s:%d: os_area_set_ldr_format failed.\n",
			__func__, __LINE__);
	return result;
}

/**
 * os_area_set_boot_flag - Set the system boot flag.
 * @value: Boot flag value, one of enum os_area_boot_flag.
 */

int os_area_set_boot_flag(struct os_area_params *p, FILE *dev,
	enum os_area_boot_flag value)
{
	int result;

	DBG("%s:%d: %u(%xh) -> %u(%xh)\n", __func__, __LINE__,
		(unsigned int)p->boot_flag, (unsigned int)p->boot_flag,
		value, value);

	if (p->boot_flag == (uint32_t)value)
		return 0;

	p->boot_flag = (uint32_t)value ? PARAM_BOOT_FLAG_OTHER_OS
		: PARAM_BOOT_FLAG_GAME_OS; /* for now just flatten to otheros */

	os_area_params_write(p, dev);
	result = os_area_params_read(p, dev);

	if (result)
		fprintf(stderr, "%s:%d: os_area_set_boot_flag failed.\n",
			__func__, __LINE__);
	return result;
}

void os_area_header_dump(const struct os_area_header *h, const char *func,
	int line)
{
	printf("%s:%d: h.magic_num:      '%s'\n", func, line,
		h->magic_num);
	printf("%s:%d: h.hdr_version:     %u\n", func, line,
		h->hdr_version);
	printf("%s:%d: h.db_area_offset:  %u\n", func, line,
		h->db_area_offset);
	printf("%s:%d: h.ldr_area_offset: %u\n", func, line,
		h->ldr_area_offset);
	printf("%s:%d: h.ldr_format:      %u (%s)\n", func, line,
		h->ldr_format, (h->ldr_format ? "gzip" : "raw"));
	printf("%s:%d: h.ldr_size:        %u (%xh)\n", func, line,
		h->ldr_size, h->ldr_size);
}

void os_area_params_dump(const struct os_area_params *p, const char *func,
	int line)
{
	printf("%s:%d: p.boot_flag:       %u (%s)\n", func, line, p->boot_flag,
		(p->boot_flag ? "other-os" : "game-os"));
	printf("%s:%d: p.num_params:      %u\n", func, line, p->num_params);
	printf("%s:%d: p.rtc_diff         %lld\n", func, line,
		(long long)p->rtc_diff);
	printf("%s:%d: p.av_multi_out     %u\n", func, line, p->av_multi_out);
	printf("%s:%d: p.ctrl_button:     %u\n", func, line, p->ctrl_button);
	printf("%s:%d: p.static_ip_addr:  %u.%u.%u.%u\n", func, line,
		p->static_ip_addr[0], p->static_ip_addr[1],
		p->static_ip_addr[2], p->static_ip_addr[3]);
	printf("%s:%d: p.network_mask:    %u.%u.%u.%u\n", func, line,
		p->network_mask[0], p->network_mask[1],
		p->network_mask[2], p->network_mask[3]);
	printf("%s:%d: p.default_gateway: %u.%u.%u.%u\n", func, line,
		p->default_gateway[0], p->default_gateway[1],
		p->default_gateway[2], p->default_gateway[3]);
	printf("%s:%d: p.dns_primary:     %u.%u.%u.%u\n", func, line,
		p->dns_primary[0], p->dns_primary[1],
		p->dns_primary[2], p->dns_primary[3]);
	printf("%s:%d: p.dns_secondary:   %u.%u.%u.%u\n", func, line,
		p->dns_secondary[0], p->dns_secondary[1],
		p->dns_secondary[2], p->dns_secondary[3]);
}
