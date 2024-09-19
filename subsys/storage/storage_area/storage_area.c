/*
 * Copyright (c) 2024 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/util.h>
#include <string.h>
#include <zephyr/storage/storage_area/storage_area.h>

static bool sa_range_valid(const struct storage_area *area, size_t start,
			   size_t len)
{
	const size_t asize = area->erase_size * area->erase_blocks;
	
	if ((asize < len) || ((asize - len) < start)) {
		return false;
	}

	return true;
}

static size_t sa_chunk_size(const struct storage_area_chunk *ch, size_t cnt)
{
	size_t rv = 0U;

	for (size_t i = 0U; i < cnt; i++) {
		rv += ch[i].len;
	}

	return rv;
}

int storage_area_read(const struct storage_area *area, size_t start,
		      const struct storage_area_chunk *ch, size_t cnt)
{
	if ((area == NULL) || (area->api == NULL) || 
	    (area->api->read == NULL)) {
		return -ENOTSUP;
	}

	const size_t len = sa_chunk_size(ch, cnt);

	if (!sa_range_valid(area, start, len)) {
		return -EINVAL;
	}
	
	return area->api->read(area, start, ch, cnt);
}

int storage_area_prog(const struct storage_area *area, size_t start,
		      const struct storage_area_chunk *ch, size_t cnt)
{
	if ((area == NULL) || (area->api == NULL) || 
	    (area->api->prog == NULL)) {
		return -ENOTSUP;
	}

	const size_t len = sa_chunk_size(ch, cnt);

	if ((!sa_range_valid(area, start, len)) ||
	    (((len & (area->write_size - 1)) != 0U))) {
		return -EINVAL;
	}
	
	if (STORAGE_AREA_HAS_PROPERTY(area, SA_PROP_READONLY)) {
		return -EROFS;
	}

	return area->api->prog(area, start, ch, cnt);
}

int storage_area_erase(const struct storage_area *area, size_t start,
		       size_t bcnt)
{
	if ((area == NULL) || (area->api == NULL) || 
	    (area->api->erase == NULL)) {
		return -ENOTSUP;
	}

	const size_t ablocks = area->erase_blocks;

	if ((ablocks < bcnt) || ((ablocks - bcnt) < start)) {
		return -EINVAL;
	}

	if (STORAGE_AREA_HAS_PROPERTY(area, SA_PROP_READONLY)) {
		return -EROFS;
	}

	return area->api->erase(area, start, bcnt);
}

int storage_area_ioctl(const struct storage_area *area,
		       enum storage_area_ioctl_cmd cmd, void *data)
{
	if ((area == NULL) || (area->api == NULL) ||
	    (area->api->ioctl == NULL)) {
		return -ENOTSUP;
	}

	return area->api->ioctl(area, cmd, data);
}