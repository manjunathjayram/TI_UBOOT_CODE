/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone EVM : Board initialization
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <fdt_support.h>
#include <libfdt.h>

#include <asm/arch/hardware.h>
#include <asm/io.h>

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	u64 start[2];
	u64 size[2];
	char name[32], *env, *endp;
	int lpae, nodeoffset, nbanks, unitrd_fixup = 0;
	u32 ddr3a_size;

	env = getenv("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);
	env = getenv("uinitrd_fixup");
	unitrd_fixup = env && simple_strtol(env, NULL, 0);

	ddr3a_size = 0;
	if (lpae) {
		env = getenv("ddr3a_size");
		if (env)
			ddr3a_size = simple_strtol(env, NULL, 10);
		if ((ddr3a_size != 8) && (ddr3a_size != 4))
			ddr3a_size = 0;
	}

	nbanks = 1;
	start[0] = bd->bi_dram[0].start;
	size[0]  = bd->bi_dram[0].size;

	/* adjust memory start address for LPAE */
	if (lpae) {
		start[0] -= CONFIG_SYS_SDRAM_BASE;
		start[0] += CONFIG_SYS_LPAE_SDRAM_BASE;
	}

	if ((size[0] == 0x80000000) && (ddr3a_size != 0)) {
		size[1] = ((u64)ddr3a_size - 2) << 30;
		start[1] = 0x880000000;
		nbanks++;
	}

	/* reserve memory at start of bank */
	sprintf(name, "mem_reserve_head");
	env = getenv(name);
	if (env) {
		start[0] += ustrtoul(env, &endp, 0);
		size[0] -= ustrtoul(env, &endp, 0);
	}

	sprintf(name, "mem_reserve");
	env = getenv(name);
	if (env)
		size[0] -= ustrtoul(env, &endp, 0);

	fdt_fixup_memory_banks(blob, start, size, nbanks);

	/* Fix up the initrd */
	if (lpae && unitrd_fixup) {
		u64 initrd_start, initrd_end, *reserve_start, size;
		u32 *prop1, *prop2;
		int err;

		nodeoffset = fdt_path_offset(blob, "/chosen");
		if (nodeoffset >= 0) {
			prop1 = fdt_getprop(blob, nodeoffset,
					    "linux,initrd-start", NULL);
			prop2 = fdt_getprop(blob, nodeoffset,
					    "linux,initrd-end", NULL);
			if (prop1 && prop2) {
				initrd_start = __be32_to_cpu(*prop1);
				initrd_start -= CONFIG_SYS_SDRAM_BASE;
				initrd_start += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_start = __cpu_to_be64(initrd_start);
				initrd_end = __be32_to_cpu(*prop2);
				initrd_end -= CONFIG_SYS_SDRAM_BASE;
				initrd_end += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_end = __cpu_to_be64(initrd_end);

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-start");
				if (err < 0)
					printf("error deleting initrd-start\n");

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-end");
				if (err < 0)
					printf("error deleting initrd-end\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-start",
						  &initrd_start,
						  sizeof(initrd_start));
				if (err < 0)
					printf("error adding initrd-start\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-end",
						  &initrd_end,
						  sizeof(initrd_end));
				if (err < 0)
					printf("error adding initrd-end\n");
			}

			/*
			 * the initrd and other reserved memory areas are
			 * embedded in in the DTB itslef. fix up these addresses
			 * to 36 bit format
			 */
			reserve_start = (char *)blob + fdt_off_mem_rsvmap(blob);
			while (1) {
				*reserve_start = __cpu_to_be64(*reserve_start);
				size = __cpu_to_be64(*(reserve_start + 1));
				if (size) {
					*reserve_start -= CONFIG_SYS_SDRAM_BASE;
					*reserve_start +=
						CONFIG_SYS_LPAE_SDRAM_BASE;
					*reserve_start =
						__cpu_to_be64(*reserve_start);
				} else
					break;
				reserve_start += 2;
			}
		}
	}
	check_ddr3_ecc_int(KS2_DDR3_EMIF_CTRL_BASE);
}
#endif

