/*
 * kexec-elf-or1k.c - kexec Elf loader for the OpenRISC
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <linux/elf.h>

#include "arch/options.h"
#include "kexec-or1k.h"
#include "kexec-elf.h"
#include "kexec-syscall.h"

int elf_or1k_probe(const char *buf, off_t len)
{

	struct mem_ehdr ehdr;
	int result;

	result = build_elf_exec_info(buf, len, &ehdr, 0);
	if (result < 0) {
		dbgprintf("%s: Not an ELF executable.\n", __func__);
		goto out;
	}

	if (ehdr.e_machine != EM_OPENRISC) {
		dbgprintf("%s: Not an OR1K ELF executable.\n", __func__);
		result = -1;
		goto out;
	}
	result = 0;
 out:
	free_elf_info(&ehdr);
	return result;
}

int elf_or1k_load(int argc, char **argv, const char *kernel_buf,
	off_t kernel_size, struct kexec_info *info)
{
	struct mem_ehdr ehdr;
	unsigned long kernel_segment;
	int result;
	int i;

	result = build_elf_exec_info(kernel_buf, kernel_size, &ehdr, 0);

	if (result < 0) {
		dbgprintf("%s: build_elf_exec_info failed\n", __func__);
		goto exit;
	}

	/* Find and process the openrisc image header. */

	for (i = 0; i < ehdr.e_phnum; i++) {
		struct mem_phdr *phdr = &ehdr.e_phdr[i];

		if (phdr->p_type != PT_LOAD)
			continue;

		kernel_size = phdr->p_memsz;

		dbgprintf("%s: e_entry:        %08llx\n", __func__,
			  ehdr.e_entry);
		dbgprintf("%s: p_vaddr:        %08llx\n", __func__,
			  phdr->p_vaddr);

		break;
	}

	if (i == ehdr.e_phnum) {
		dbgprintf("%s: Valid or1k header not found\n", __func__);
		result = EFAILED;
		goto exit;
	}

	kernel_segment = or1k_locate_kernel_hole(info, kernel_size);

	if (kernel_segment == ULONG_MAX) {
		dbgprintf("%s: Kernel segment is not allocated\n", __func__);
		result = EFAILED;
		goto exit;
	}

	dbgprintf("%s: kernel_segment: %08lx\n", __func__, kernel_segment);
	dbgprintf("%s: kernel_size:    %08llx\n", __func__, kernel_size);

	result = elf_exec_load(&ehdr, info);

	if (result) {
		dbgprintf("%s: elf_exec_load failed\n", __func__);
		goto exit;
	}

	/* load additional data */
	result = or1k_load_other_segments(info, kernel_segment, kernel_size);

exit:
	free_elf_info(&ehdr);
	if (result)
		fprintf(stderr, "kexec: Bad elf image file, load failed.\n");
	return result;
}

void elf_or1k_usage(void)
{
	printf(
"    An OpenRISC ELF image, typically vmlinux or a stripped\n"
"    version of vmlinux.\n\n");
}
