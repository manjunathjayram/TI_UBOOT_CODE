/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * K2E EVM : Board initialization
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
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>
#include <asm/io.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/emac_defs.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int external_clk[ext_clk_count] = {
	[sys_clk]	=	100000000,
	[alt_core_clk]	=	100000000,
	[pa_clk]	=	100000000,
	[ddr3_clk]	=	100000000,
	[mcm_clk]	=	312500000,
	[pcie_clk]	=	100000000,
	[sgmii_clk]	=	156250000,
	[xgmii_clk]	=	156250000,
	[usb_clk]	=	100000000
};

static struct async_emif_config async_emif_config[ASYNC_EMIF_NUM_CS] = {
	{			/* CS0 */
		.mode		= ASYNC_EMIF_MODE_NAND,
		.wr_setup	= 0xf,
		.wr_strobe	= 0x3f,
		.wr_hold	= 7,
		.rd_setup	= 0xf,
		.rd_strobe	= 0x3f,
		.rd_hold	= 7,
		.turn_around	= 3,
		.width		= ASYNC_EMIF_8,
	},

};

static struct pll_init_data pll_config[] = {
	CORE_PLL_1200,
	PASS_PLL_1000,
};

#ifdef CONFIG_SPL_BOARD_INIT
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_800,
};

void spl_init_keystone_plls(void)
{
	init_plls(ARRAY_SIZE(spl_pll_config), spl_pll_config);
}
#endif

int dram_init(void)
{
	init_ddr3();

	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE,
				    CONFIG_MAX_RAM_BANK_SIZE);
	init_async_emif(ARRAY_SIZE(async_emif_config), async_emif_config);
	init_ddr3_ecc(KS2_DDR3_EMIF_CTRL_BASE);
	return 0;
}

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	init_plls(ARRAY_SIZE(pll_config), pll_config);
	return 0;
}
#endif

int board_init(void)
{

	gd->bd->bi_arch_number = -1; /* MACH_TYPE_K2E_EVM; */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	return 0;
}

