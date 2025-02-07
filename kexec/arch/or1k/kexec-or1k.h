#ifndef KEXEC_OR1K_H
#define KEXEC_OR1K_H

#include <stdint.h>

#include "kexec.h"

/* Definitions needed for the fs2dt utility */
#define BOOT_BLOCK_VERSION 17
#define BOOT_BLOCK_LAST_COMP_VERSION 16

extern unsigned long initrd_base;
extern off_t initrd_size;

#define MAX_LINE	160
#define COMMAND_LINE_SIZE	2048 /* from kernel */

#define KiB(x) ((x) * 1024UL)
#define MiB(x) (KiB(x) * 1024UL)
#define GiB(x) (MiB(x) * 1024UL)

int elf_or1k_probe(const char *buf, off_t len);
int elf_or1k_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info);
void elf_or1k_usage(void);

unsigned long or1k_locate_kernel_hole(struct kexec_info *info,
	unsigned long kernel_size);
int or1k_load_other_segments(struct kexec_info *info,
	unsigned long image_base, unsigned long image_size);

#endif /* KEXEC_OR1K_H */
