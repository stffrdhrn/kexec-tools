/*
 * OpenRISC crashdump. placeholder.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <linux/elf.h>

#include "kexec.h"
#include "crashdump.h"
#include "crashdump-or1k.h"
#include "mem_regions.h"

/* memory range reserved for crashkernel */
struct memory_range crash_reserved_mem[CRASH_MAX_RESERVED_RANGES];
struct memory_ranges usablemem_rgns = {
	.size = 0,
	.max_size = CRASH_MAX_RESERVED_RANGES,
	.ranges = crash_reserved_mem,
};
