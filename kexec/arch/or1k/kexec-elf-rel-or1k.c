/*
 * ELF relocations for OpenRISC
 *
 * Based on the OpenRISC module loader (arch/openrisc/kernel/module.c) in the
 * Linux kernel.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */
#include <stdio.h>
#include <elf.h>
#include "kexec.h"
#include "kexec-elf.h"

int machine_verify_elf_rel(struct mem_ehdr *ehdr)
{

	if (ehdr->ei_data != ELFDATA2MSB)
		return 0;
	if (ehdr->ei_class != ELFCLASS32)
		return 0;
	if (ehdr->e_machine != EM_OPENRISC)
		return 0;

	return 1;
}

void machine_apply_elf_rel(struct mem_ehdr *UNUSED(ehdr),
	struct mem_sym *UNUSED(sym), unsigned long r_type, void *orig_loc,
	unsigned long UNUSED(address), unsigned long value)
{
	uint32_t *location = orig_loc;

	switch (r_type) {
	case R_OR1K_32:
		*location = value;
		break;
	case R_OR1K_LO_16_IN_INSN:
		*((uint16_t *)location + 1) = value;
		break;
	case R_OR1K_HI_16_IN_INSN:
		*((uint16_t *)location + 1) = value >> 16;
		break;
	case R_OR1K_INSN_REL_26:
		value -= (uint32_t)location;
		value >>= 2;
		value &= 0x03ffffff;
		value |= *location & 0xfc000000;
		*location = value;
		break;
	case R_OR1K_AHI16:
		/* Adjust the operand to match with a signed LO16.  */
		value += 0x8000;
		*((uint16_t *)location + 1) = value >> 16;
		break;
	case R_OR1K_SLO16:
		/* Split value lower 16-bits.  */
		value = ((value & 0xf800) << 10) | (value & 0x7ff);
		*location = (*location & ~0x3e007ff) | value;
		break;
	default:
	        die("Unknown rela relocation: %lu\n", r_type);
		break;
	}
}
