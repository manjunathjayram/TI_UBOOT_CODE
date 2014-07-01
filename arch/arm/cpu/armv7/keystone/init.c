/*
 * Copyright (C) 2012-2014 Texas Instruments
 *
 * Keystone: Architecture initialization
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
#include <ns16550.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/psc_defs.h>

/* this register is used for K2HK revision 1.x */
#define K2HK_REV1_DEVSPEED	(KS2_DEVICE_STATE_CTRL_BASE + 0xc98)
/* all revisiona above and not K2HK SoCs use EFUSE_BOOTROM */
#define KS2_EFUSE_BOOTROM	(KS2_DEVICE_STATE_CTRL_BASE + 0xc90)


u32 debug_options;

void chip_configuration_unlock(void)
{
	__raw_writel(KEYSTONE_KICK0_MAGIC, KEYSTONE_KICK0);
	__raw_writel(KEYSTONE_KICK1_MAGIC, KEYSTONE_KICK1);
}

#ifdef CONFIG_SOC_K2L
int osr_init(u32 base, u32 num_ram_banks)
{
	u32 i, ecc_ctrl[OSR_NUM_RAM_BANKS];

	/* Enable the OSR clock domain */
	psc_enable_module(K2L_LPSC_OSR);

	/* Disable OSR ECC check for all the ram banks */
	for (i = 0; i < num_ram_banks; i++) {
		writel(i | OSR_ECC_VEC_TRIG_RD | \
			(KS2_OSR_ECC_CTRL << OSR_ECC_VEC_RD_ADDR_SH),
			base + KS2_OSR_ECC_VEC);
		while (!(readl(base + KS2_OSR_ECC_VEC) & OSR_ECC_VEC_RD_DONE))
			;
		ecc_ctrl[i] = readl(base + KS2_OSR_ECC_CTRL) ^ OSR_ECC_CTRL_CHK;
		writel(ecc_ctrl[i], 0xc000000 + i * 4);
		writel(ecc_ctrl[i], base + KS2_OSR_ECC_CTRL);
	}

	/* Reset OSR memory to all zeros */
	for (i = 0; i < OSR_SIZE; i += 4)
		writel(0, KS2_OSR_DATA_BASE + i);

	/* Enable OSR ECC check for all the ram banks */
	for (i = 0; i < num_ram_banks; i++)
		writel(ecc_ctrl[i] | OSR_ECC_CTRL_CHK, base + KS2_OSR_ECC_CTRL);
}
#endif

int arch_cpu_init(void)
{
	chip_configuration_unlock();
	icache_enable();

	share_all_segments(8);  /* Tetris */
	share_all_segments(9);  /* Netcp */
	share_all_segments(10); /* QM PDSP */
	share_all_segments(11); /* PCIE 0 */
	if (cpu_is_k2e())
		share_all_segments(13); /* PCIE 1 */
	if (cpu_is_k2l()) {
		share_all_segments(14); /* PCIE 1 */
#ifdef CONFIG_SOC_K2L
		osr_init(KS2_OSR_CFG_BASE, OSR_NUM_RAM_BANKS);
#endif
	}
	/*
	 * just initialise the COM2 port so that TI specific
	 * UART register PWREMU_MGMT is initialized. Linux UART
	 * driver doesn't handle this.
	 */
	NS16550_init((NS16550_t)(CONFIG_SYS_NS16550_COM2),
		CONFIG_SYS_NS16550_CLK / 16 / CONFIG_BAUDRATE);
	return 0;
}

void reset_cpu(ulong addr)
{
	volatile u32* rstctrl = (volatile u32*)	(CLOCK_BASE + 0xe8);
	u32 tmp;

	tmp = *rstctrl & 0xffff0000;
	*rstctrl = tmp | 0x5a69;

	*rstctrl &= 0xfffe0000;

	for (;;);
}

#ifdef CONFIG_SOC_K2E
#define MAX_SPEEDS 13
static int dev_speeds[MAX_SPEEDS] = {
	SPD800,
	SPD850,
	SPD1000,
	SPD1250,
	SPD1350,
	SPD1400,
	SPD1500,
	SPD1400,
	SPD1350,
	SPD1250,
	SPD1000,
	SPD850,
	SPD800
};

#else
#define MAX_SPEEDS 11
static int dev_speeds[MAX_SPEEDS] = {
	SPD800,
	SPD1000,
	SPD1200,
	SPD_RSV,
	SPD_RSV,
	SPD_RSV,
	SPD_RSV,
	SPD_RSV,
	SPD1200,
	SPD1350,
	SPD1400
};

static int arm_speeds[MAX_SPEEDS] = {
	SPD800,
	SPD1000,
	SPD1200,
	SPD1350,
	SPD1400,
	SPD_RSV,
	SPD1400,
	SPD1350,
	SPD1200,
	SPD1000,
	SPD800
};
#endif

int get_max_speed(u32 val, int *speeds)
{
	int	j;

	if (!val)
		return speeds[0];

	for (j = 1; j < MAX_SPEEDS; j++) {
		if (val == 0x0001)
			return speeds[j];
		val >>= 1;
	}

	return SPD_RSV;
}

#ifdef CONFIG_SOC_K2HK
unsigned int read_efuse_bootrom(void)
{
	return (cpu_revision() > 1) ?  __raw_readl(KS2_EFUSE_BOOTROM) :
		 __raw_readl(K2HK_REV1_DEVSPEED);
}
#else
#define read_efuse_bootrom() __raw_readl(KS2_EFUSE_BOOTROM)
#endif

int inline get_max_dev_speed(void)
{
	return get_max_speed(read_efuse_bootrom() & 0xffff, dev_speeds);
}

#ifndef CONFIG_SOC_K2E
int inline get_max_arm_speed(void)
{
	return get_max_speed((read_efuse_bootrom() >> 16) & 0xffff, arm_speeds);
}
#endif
