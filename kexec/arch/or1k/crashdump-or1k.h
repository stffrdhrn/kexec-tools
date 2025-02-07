/*
 * OpenRISC crashdump.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef CRASHDUMP_OR1K_H
#define CRASHDUMP_OR1K_H

#include "kexec.h"

#define CRASH_MAX_MEMORY_RANGES	32768

/* crash dump kernel supports one range. */
#define CRASH_MAX_RESERVED_RANGES	1

extern struct memory_ranges usablemem_rgns;

#endif /* CRASHDUMP_OR1K_H */
