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
#include <common.h>

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>
#include "ddr3_cfg.h"

DECLARE_GLOBAL_DATA_PTR;

struct pll_init_data ddr3_400 = DDR3_PLL_400;
struct pll_init_data ddr3_333 = DDR3_PLL_333;

void init_ddr3(void)
{
	struct ddr3_spd_cb spd_cb;

	if (get_dimm_params_from_spd(&spd_cb)) {
		printf("Sorry, I don't know how to configure DDR3A.\n"
		       "Buy :(\n");
		for (;;)
			;
	}

	printf("Detected SO-DIMM [%s]\n", spd_cb.dimm_name);

	if (__raw_readl(K2E_PLL_CNTRL_BASE + MAIN_PLL_CTRL_RSTYPE) & 0x1) {
		printf("DDR3 speed %d\n", spd_cb.ddrspdclock);
		if (spd_cb.ddrspdclock == 1600)
			init_pll(&ddr3_400);
		else
			init_pll(&ddr3_333);
	} else
		printf("Power-on reset was not the last reset to occur !\n");

	/* Reset DDR3 PHY after PLL enabled */
	reset_ddrphy(KS2_DEVICE_STATE_CTRL_BASE);

	spd_cb.phy_cfg.zq0cr1 |= 0x10000;
	spd_cb.phy_cfg.zq1cr1 |= 0x10000;
	spd_cb.phy_cfg.zq2cr1 |= 0x10000;
	init_ddrphy(K2E_DDR3_DDRPHYC, &(spd_cb.phy_cfg));
	init_ddremif(K2E_DDR3_EMIF_CTRL_BASE, &(spd_cb.emif_cfg));

	gd->ddr3_size = spd_cb.ddr_size_gbyte;
	printf("DRAM: %d GiB\n", gd->ddr3_size);
}

/* Get the total segment number of the DDR memory, each segment is 4KB size */
u32 get_ddr_seg_num(void)
{
	/* DDR3 size in segments (4KB seg size) */
	return gd->ddr3_size << (30 - MSMC_SEG_SIZE_SHIFT);
}

