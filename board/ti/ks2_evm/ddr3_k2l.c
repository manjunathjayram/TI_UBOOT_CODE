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

/************************* *****************************/
static struct ddr3_phy_config ddr3phy_1600_64 = {
	.pllcr		= 0x0001C000ul,
	.pgcr1_mask	= (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val	= ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0		= 0x42C21590ul,
	.ptr1		= 0xD05612C0ul,
	.ptr2		= 0, /* not set in gel */
	.ptr3		= 0x0D861A80ul,
	.ptr4		= 0x0C827100ul,
	.dcr_mask	= (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val	= ((1 << 10)),
	.dtpr0		= 0x9D5CBB66ul,
	.dtpr1		= 0x12868300ul,
	.dtpr2		= 0x5002D200ul,
	.mr0		= 0x00001C70ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000018ul,
	.dtcr		= 0x710035C7ul,
	.pgcr2		= 0x00F07A12ul,
	.zq0cr1		= 0x0001005Dul,
	.zq1cr1		= 0x0001005Bul,
	.zq2cr1		= 0x0001005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};

static struct ddr3_emif_config ddr3_1600_64 = {
	.sdcfg		= 0x6200CE62ul,
	.sdtim1		= 0x166C9855ul,
	.sdtim2		= 0x00001D4Aul,
	.sdtim3		= 0x435DFF53ul,
	.sdtim4		= 0x543F0CFFul,
	.zqcfg		= 0x70073200ul,
	.sdrfc		= 0x00001869ul,
};

struct pll_init_data ddr3_400 = DDR3_PLL_400;

void init_ddr3(void)
{
	char dimm_name[32];

	if (~(readl(K2L_PLL_CNTRL_BASE + MAIN_PLL_CTRL_RSTYPE) & 0x1))
		init_pll(&ddr3_400);

	/* No SO-DIMM, 2GB discreet DDR */
	printf("DRAM: 2 GiB\n");

	init_ddrphy(K2L_DDR3_DDRPHYC, &ddr3phy_1600_64);
	init_ddremif(K2L_DDR3_EMIF_CTRL_BASE, &ddr3_1600_64);
}

/* Get the total segment number of the DDR memory, each segment is 4KB size */
u32 get_ddr_seg_num(void)
{
	/* DDR3 size in segments (4KB seg size), hardcoded to 2GB size */
	return (u32)CONFIG_MAX_RAM_BANK_SIZE >> MSMC_SEG_SIZE_SHIFT;
}

