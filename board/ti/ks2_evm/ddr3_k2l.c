/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone2: DDR3 initialization
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

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>
#include "ddr3_cfg.h"

DECLARE_GLOBAL_DATA_PTR;

struct pll_init_data ddr3_400 = DDR3_PLL_400;

void init_ddr3(void)
{
	char dimm_name[32];
	u32 tmp;

	if (~(readl(K2L_PLL_CNTRL_BASE + MAIN_PLL_CTRL_RSTYPE) & 0x1))
		init_pll(&ddr3_400);

	/* No SO-DIMM, 2GB discreet DDR */
	printf("DRAM: 2 GiB\n");

	/* Reset DDR3 PHY after PLL enabled */
	reset_ddrphy(KS2_DEVICE_STATE_CTRL_BASE);

	init_ddrphy(K2L_DDR3_DDRPHYC, &ddr3phy_1600_2g);
	init_ddremif(K2L_DDR3_EMIF_CTRL_BASE, &ddr3_1600_2g);
}

/* Get the total segment number of the DDR memory, each segment is 4KB size */
u32 get_ddr_seg_num(void)
{
	/* DDR3 size in segments (4KB seg size), hardcoded to 2GB size */
	return (u32)CONFIG_MAX_RAM_BANK_SIZE >> MSMC_SEG_SIZE_SHIFT;
}

