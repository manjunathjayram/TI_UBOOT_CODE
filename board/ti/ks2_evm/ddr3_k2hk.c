/*
 * Copyright (C) 2013 - 2014 Texas Instruments Inc.
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

static void ddr_reset_workaround(void)
{

	unsigned int tmp, tmp_a, tmp_b;

	/*
	 * Check for PGSR0 error bits of DDR3 PHY.
	 * Check for WLERR, QSGERR, WLAERR,
	 * RDERR, WDERR, REERR, WEERR error to see if they are set or not
	 */
	tmp_a = __raw_readl(K2HK_DDR3A_DDRPHYC +
				KS2_DDRPHY_PGSR0_OFFSET);
	tmp_b = __raw_readl(K2HK_DDR3B_DDRPHYC +
				KS2_DDRPHY_PGSR0_OFFSET);

	if (((tmp_a & 0x0FE00000) != 0) || ((tmp_b & 0x0FE00000) != 0)) {

		printf("DDR Leveling Error Detected!\n");
		printf("DDR3A PGSR0 = 0x%x\n", tmp_a);
		printf("DDR3B PGSR0 = 0x%x\n", tmp_b);

		/*
		 * Write Keys to KICK registers to enable writes to registers
		 * in boot config space
		 */
		__raw_writel(KEYSTONE_KICK0_MAGIC, KEYSTONE_KICK0);
		__raw_writel(KEYSTONE_KICK1_MAGIC, KEYSTONE_KICK1);

		/*
		 * Move DDR3A Module out of reset isolation by setting
		 * MDCTL23[12] = 0
		 */
		tmp_a = __raw_readl(KS2_PSC_BASE + 0xA5C);
		tmp_a &= ~(0x1000);
		__raw_writel(tmp_a, KS2_PSC_BASE + 0xA5C);

		/*
		 * Move DDR3B Module out of reset isolation by setting
		 * MDCTL24[12] = 0
		 */
		 tmp_b = __raw_readl(KS2_PSC_BASE + 0xA60);
		 tmp_b &= ~(0x1000);
		__raw_writel(tmp_b, KS2_PSC_BASE + 0xA60);

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes
		 * to RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0xFFFF);
		tmp |= 0x5A69;
		__raw_writel(tmp, K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/*
		 * Set PLL Controller to drive hard reset on SW trigger by
		 * setting RSTCFG[13] = 0
		 */
		tmp = __raw_readl(K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCFG);
		tmp &= ~(0x2000);
		tmp |= 0x0000;
		__raw_writel(tmp, K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCFG);

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes to
		 * RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0xFFFF);
		tmp |= 0x5A69;
		__raw_writel(tmp, K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/*
		 * Write RSTCTRL[16] = 0 to initiate software reset via PLL
		 * controller
		 */
		tmp = __raw_readl(K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0x10000);
		tmp = 0x00000;
		__raw_writel(tmp, K2HK_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/* Wait for Reset */
		for (;;)
			;
	}
}


struct pll_init_data ddr3a_333 = DDR3_PLL_333(A);
struct pll_init_data ddr3b_333 = DDR3_PLL_333(B);
struct pll_init_data ddr3a_400 = DDR3_PLL_400(A);
struct pll_init_data ddr3b_400 = DDR3_PLL_400(B);

void init_ddr3(void)
{
	char dimm_name[32];

	if (__raw_readl(K2HK_PLL_CNTRL_BASE + MAIN_PLL_CTRL_RSTYPE) & 0x1) {
		if (cpu_revision() > 0)
			init_pll(&ddr3a_400);
		else
			init_pll(&ddr3a_333);
		init_pll(&ddr3b_333);
	} else
		printf("Power-on reset was not the last reset to occur !\n");

	init_ddrphy(K2HK_DDR3B_DDRPHYC, &ddr3phy_1333_2g);
	init_ddremif(K2HK_DDR3B_EMIF_CTRL_BASE, &ddr3_1333_2g);

	get_dimm_params(dimm_name);

	printf("Detected SO-DIMM [%s]\n", dimm_name);

	if (!strcmp(dimm_name, "18KSF1G72HZ-1G6E2 ")) {
		/* 8G SO-DIMM */
		if (cpu_revision() > 0) {
			if (cpu_revision() > 1) {
				/* PG 2.0 */
				/* Reset DDR3A PHY after PLL enabled */
				reset_ddrphy(KS2_DEVICE_STATE_CTRL_BASE);
				ddr3phy_1600_8g.zq0cr1 |= 0x10000;
				ddr3phy_1600_8g.zq1cr1 |= 0x10000;
				ddr3phy_1600_8g.zq2cr1 |= 0x10000;
				init_ddrphy(K2HK_DDR3A_DDRPHYC,
						&ddr3phy_1600_8g);
			} else {
				/* PG 1.1 */
				init_ddrphy(K2HK_DDR3A_DDRPHYC,
						&ddr3phy_1600_8g);
			}
			init_ddremif(K2HK_DDR3A_EMIF_CTRL_BASE,
					&ddr3_1600_8g);
			gd->ddr3_size = 8;
			printf("DRAM:  8 GiB (includes reported below)\n");
		} else {
			init_ddrphy(K2HK_DDR3A_DDRPHYC, &ddr3phy_1600_8g);
			ddr3_1600_8g.sdcfg |= 0x1000;
			init_ddremif(K2HK_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_8g);
			gd->ddr3_size = 4;
			printf("DRAM: 4 GiB (includes reported below)\n");
		}
	} else {
		/* 2G SO-DIMM */
		if (cpu_revision() > 0) {
			if (cpu_revision() > 1) {
				/* PG 2.0 */
				/* Reset DDR3A PHY after PLL enabled */
				reset_ddrphy(KS2_DEVICE_STATE_CTRL_BASE);
				ddr3phy_1333_2g.zq0cr1 |= 0x10000;
				ddr3phy_1333_2g.zq1cr1 |= 0x10000;
				ddr3phy_1333_2g.zq2cr1 |= 0x10000;
				init_ddrphy(K2HK_DDR3A_DDRPHYC,
						&ddr3phy_1333_2g);
			} else {
				/* PG 1.1 */
				init_ddrphy(K2HK_DDR3A_DDRPHYC,
						&ddr3phy_1333_2g);
			}
			init_ddremif(K2HK_DDR3A_EMIF_CTRL_BASE, &ddr3_1333_2g);
			gd->ddr3_size = 2;
			printf("DRAM: 2 GiB\n");
		} else {
			init_ddrphy(K2HK_DDR3A_DDRPHYC, &ddr3phy_1333_2g);
			ddr3_1333_2g.sdcfg |= 0x1000;
			init_ddremif(K2HK_DDR3A_EMIF_CTRL_BASE, &ddr3_1333_2g);
			gd->ddr3_size = 1;
			printf("DRAM: 2 GiB\n");
		}
	}

	/* Apply the workaround for PG 1.0 and 1.1 Silicons */
	if (cpu_revision() <= 1)
		ddr_reset_workaround();
}

u32 get_ddr_seg_num(void)
{
	/* DDR3 size in segments (4KB seg size) */
	return gd->ddr3_size << (30 - MSMC_SEG_SIZE_SHIFT);
}

