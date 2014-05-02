/*
 * Copyright (C) 2012 - 2014 Texas Instruments Inc.
 *
 * K2HK EVM : Board initialization
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
	[sys_clk]	=	122880000,
	[alt_core_clk]	=	125000000,
	[pa_clk]	=	122880000,
	[tetris_clk]	=	125000000,
	[ddr3a_clk]	=	100000000,
	[ddr3b_clk]	=	100000000,
	[mcm_clk]	=	312500000,
	[pcie_clk]	=	100000000,
	[sgmii_srio_clk] =	156250000,
	[xgmii_clk]	=	156250000,
	[usb_clk]	=	100000000,
	[rp1_clk]	=	123456789    /* TODO: cannot find
						what is that */
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
	CORE_PLL_1228,
	PASS_PLL_983,
	TETRIS_PLL_1200,
};

#ifdef CONFIG_SPL_BOARD_INIT
static struct pll_init_data spl_pll_config[] = {
	CORE_PLL_799,
	TETRIS_PLL_500,
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
	init_ddr3_ecc(K2HK_DDR3A_EMIF_CTRL_BASE);
	return 0;
}

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
eth_priv_t eth_priv_cfg[] = {
	{
		.int_name	= "K2HK_EMAC",
		.rx_flow	= CPSW_PORT_RX_FLOW(0),
		.phy_addr	= 0,
		.slave_port	= 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC1",
		.rx_flow	= CPSW_PORT_RX_FLOW(1),
		.phy_addr	= 1,
		.slave_port	= 2,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
	},
	{
		.int_name	= "K2HK_EMAC2",
		.rx_flow	= CPSW_PORT_RX_FLOW(2),
		.phy_addr	= 2,
		.slave_port	= 3,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
	{
		.int_name	= "K2HK_EMAC3",
		.rx_flow	= CPSW_PORT_RX_FLOW(3),
		.phy_addr	= 3,
		.slave_port	= 4,
		.sgmii_link_type = SGMII_LINK_MAC_MAC_FORCED,
	},
};

inline int get_num_eth_ports(void)
{
	return sizeof(eth_priv_cfg) / sizeof(eth_priv_t);
}

eth_priv_t *get_eth_priv_ptr(int port_num)
{
	if (port_num < 0 || port_num >= get_num_eth_ports())
		return NULL;

	return &eth_priv_cfg[port_num];
}

int get_eth_env_param(char *env_name)
{
	char *env;
	int  res = -1;

	env = getenv(env_name);
	if (env)
		res = simple_strtol(env, NULL, 0);

	return res;
}

int board_eth_init(bd_t *bis)
{
	int	j;
	int	res;
	int	link_type_name[32];

	for (j = 0; j < get_num_eth_ports(); j++) {
		sprintf(link_type_name, "sgmii%d_link_type", j);
		res = get_eth_env_param(link_type_name);
		if (res >= 0)
			eth_priv_cfg[j].sgmii_link_type = res;

		keystone2_emac_initialize(&eth_priv_cfg[j]);
	}

	keystone2_eth_open_close(eth_priv_cfg[0].dev);

	return 0;
}
#endif

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	init_plls(ARRAY_SIZE(pll_config), pll_config);
	return 0;
}
#endif

int board_init(void)
{

	gd->bd->bi_arch_number = -1;
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	return 0;
}

