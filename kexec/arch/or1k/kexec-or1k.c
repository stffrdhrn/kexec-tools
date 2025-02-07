/*
 * kexec-or1k.c - kexec for OpenRISC
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libfdt.h>

#include "dt-ops.h"
#include "kexec.h"
#include "kexec-syscall.h"
#include "kexec-or1k.h"
#include "fs2dt.h"
#include <arch/options.h>

#define MAX_MEMORY_RANGES 64
static struct memory_range memory_range[MAX_MEMORY_RANGES];

unsigned long initrd_base;
off_t initrd_size;

struct or1k_opts or1k_opts;

static int kexec_or1k_memory_range_callback(void *UNUSED(data), int nr,
					    char *UNUSED(str),
					    unsigned long long base,
					    unsigned long long length)
{
	if (nr < MAX_MEMORY_RANGES) {
		memory_range[nr].start = base;
		memory_range[nr].end = base + length - 1;
		memory_range[nr].type = RANGE_RAM;
		return 0;
	}

	return 1;
}

/* Return a sorted list of available memory ranges.  We will setup
 * the images to be copied to these memory ranges when the kexec syscall
 * starts the final relocate step. */
int get_memory_ranges(struct memory_range **range, int *ranges,
		unsigned long UNUSED(kexec_flags))
{
	int nr = kexec_iomem_for_each_line("System RAM\n",
				       kexec_or1k_memory_range_callback, NULL);
	*range = memory_range;
	*ranges = nr;

	dbgprint_mem_range("MEMORY RANGES", *range, *ranges);

	return 0;
}

struct file_type file_type[] = {
	{"elf-or1k", elf_or1k_probe, elf_or1k_load, elf_or1k_usage},
};
int file_types = sizeof(file_type) / sizeof(file_type[0]);

void arch_usage(void)
{
	printf(or1k_opts_usage);
}

int arch_process_options(int argc, char **argv)
{
	static const char short_options[] = KEXEC_OPT_STR "";
	static const struct option options[] = {
		KEXEC_ARCH_OPTIONS
		{ 0 }
	};
	int opt;
	char *cmdline = NULL;
	const char *append = NULL;

	for (opt = 0; opt != -1; ) {
		opt = getopt_long(argc, argv, short_options, options, 0);

		switch (opt) {
		case OPT_APPEND:
			append = optarg;
			break;
		case OPT_REUSE_CMDLINE:
			cmdline = get_command_line();
			break;
		case OPT_DTB:
			or1k_opts.dtb = optarg;
			break;
		case OPT_INITRD:
			or1k_opts.initrd = optarg;
			break;
		default:
			break; /* Ignore core and unknown options. */
		}
	}

	or1k_opts.command_line = concat_cmdline(cmdline, append);

	dbgprintf("%s:%d: command_line: %s\n", __func__, __LINE__,
		or1k_opts.command_line);
	dbgprintf("%s:%d: initrd: %s\n", __func__, __LINE__,
		or1k_opts.initrd);
	dbgprintf("%s:%d: dtb: %s\n", __func__, __LINE__,
		or1k_opts.dtb);

	return 0;
return 0;
}

/**
 * struct dtb - Info about a binary device tree.
 *
 * @buf: Device tree data.
 * @size: Device tree data size.
 * @name: Shorthand name of this dtb for messages.
 * @path: Filesystem path.
 */

struct dtb {
	char *buf;
	off_t size;
	const char *name;
	const char *path;
};

/**
 * set_bootargs - Set the dtb's bootargs.
 */

static int set_bootargs(struct dtb *dtb, const char *command_line)
{
	int result;

	if (!command_line || !command_line[0])
		return 0;

	result = dtb_set_bootargs(&dtb->buf, &dtb->size, command_line);

	if (result) {
		fprintf(stderr,
			"kexec: Set device tree bootargs failed.\n");
		return EFAILED;
	}

	return 0;
}

/**
 * read_proc_dtb - Read /proc/device-tree.
 */

static int read_proc_dtb(struct dtb *dtb)
{
	int result;
	struct stat s;
	static const char path[] = "/proc/device-tree";

	result = stat(path, &s);

	if (result) {
		dbgprintf("%s: %s\n", __func__, strerror(errno));
		return EFAILED;
	}

	dtb->path = path;
	create_flatten_tree((char **)&dtb->buf, &dtb->size, NULL);

	return 0;
}

/**
 * read_sys_dtb - Read /sys/firmware/fdt.
 */

static int read_sys_dtb(struct dtb *dtb)
{
	int result;
	struct stat s;
	static const char path[] = "/sys/firmware/fdt";

	result = stat(path, &s);

	if (result) {
		dbgprintf("%s: %s\n", __func__, strerror(errno));
		return EFAILED;
	}

	dtb->path = path;
	dtb->buf = slurp_file(path, &dtb->size);

	return 0;
}

/**
 * read_current_dtb - Read the current kernel's dtb.
 */

static int read_current_dtb(struct dtb *dtb)
{
	int result;

	dtb->name = "dtb_sys";
	result = read_sys_dtb(dtb);

	if (!result)
		goto on_success;

	dtb->name = "dtb_proc";
	result = read_proc_dtb(dtb);

	if (!result)
		goto on_success;

	dbgprintf("%s: not found\n", __func__);
	return EFAILED;

on_success:
	dbgprintf("%s: found %s\n", __func__, dtb->path);
	return 0;
}

unsigned long or1k_locate_kernel_hole(struct kexec_info *info,
	unsigned long kernel_size)
{
	unsigned long hole;
	int page_size = getpagesize();

	hole = locate_hole(info, kernel_size, page_size, 0, ULONG_MAX, 1);

	if (hole == ULONG_MAX)
		dbgprintf("%s: locate_hole failed\n", __func__);

	return hole;
}

/* Prepare the dtb and initrd segments. */
int or1k_load_other_segments(struct kexec_info *info,
	unsigned long image_base, unsigned long image_size)
{
	int result;
	unsigned long dtb_base;
	unsigned long hole_min;
	unsigned long hole_max;
	int page_size = getpagesize();
	char *initrd_buf = NULL;
	struct dtb dtb;
	char command_line[COMMAND_LINE_SIZE] = "";

	if (or1k_opts.dtb) {
		dtb.name = "dtb_user";
		dtb.buf = slurp_file(or1k_opts.dtb, &dtb.size);
	} else {
		result = read_current_dtb(&dtb);

		if (result) {
			fprintf(stderr,
				"kexec: Error: No device tree available.\n");
			return EFAILED;
		}
	}

	if (or1k_opts.command_line) {
		if (strlen(or1k_opts.command_line) >
		    sizeof(command_line) - 1) {
			fprintf(stderr,
				"Kernel command line too long for kernel!\n");
			return EFAILED;
		}

		strncpy(command_line, or1k_opts.command_line,
			sizeof(command_line) - 1);
		command_line[sizeof(command_line) - 1] = 0;

		result = set_bootargs(&dtb, command_line);
		if (result)
			return EFAILED;
	}

	/* Put the other segments after the image. */

	hole_min = image_base + _ALIGN_UP(image_size, page_size);
	hole_max = ULONG_MAX;

	if (or1k_opts.initrd) {
		initrd_buf = slurp_file(or1k_opts.initrd, &initrd_size);

		if (!initrd_buf)
			fprintf(stderr, "kexec: Empty ramdisk file.\n");
		else {
			/* Put the initrd after the kernel. */
			initrd_base = add_buffer_phys_virt(info, initrd_buf,
				initrd_size, initrd_size, 0,
				hole_min, hole_max, 1, 0);

			dbgprintf("initrd: base %lx, size %llxh (%lld)\n",
				initrd_base, initrd_size, initrd_size);

			result = dtb_set_initrd((char **)&dtb.buf,
				&dtb.size, initrd_base,
				initrd_base + initrd_size);

			if (result)
				return EFAILED;
		}
	}

	if (!initrd_buf) {
		/* Don't reuse the initrd addresses from the DTB */
		dtb_clear_initrd((char **)&dtb.buf, &dtb.size);
	}

	/* Check size limit as specified in booting.txt. */

	if (dtb.size > MiB(2)) {
		fprintf(stderr, "kexec: Error: dtb too big.\n");
		return EFAILED;
	}

	dtb_base = add_buffer_phys_virt(info, dtb.buf, dtb.size, dtb.size,
		0, hole_min, hole_max, 1, 0);

	/* dtb_base is valid if we got here. */

	dbgprintf("dtb:    base %lx, size %llxh (%lld)\n", dtb_base, dtb.size,
		dtb.size);

	return 0;
}

const struct arch_map_entry arches[] = {
	/* For compatibility with older patches
	 * use KEXEC_ARCH_DEFAULT instead of KEXEC_ARCH_PPC here.
	 */
	{ "openrisc", KEXEC_ARCH_DEFAULT },
	{ NULL, 0 },
};

int arch_compat_trampoline(struct kexec_info *UNUSED(info))
{
	return 0;
}

void arch_update_purgatory(struct kexec_info *UNUSED(info))
{
}

int is_crashkernel_mem_reserved(void)
{
	return 0;
}

int get_crash_kernel_load_range(uint64_t *start, uint64_t *end)
{
	/* Crash kernel region size is not exposed by the system */
	return -1;
}

/*
 * Mask virtual addresses in the form 0xC000000 to 0x00000000.
 * The kernel defines PAGE_OFFSET as 0xC0000000. */
unsigned long virt_to_phys(unsigned long addr)
{
	return addr & 0x3fffffff;
}

int arch_do_exclude_segment(struct kexec_info *UNUSED(info), struct kexec_segment *UNUSED(segment))
{
	return 0;
}
