/*
 * Keystone2: Architecture initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/msmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/psc_defs.h>

void chip_configuration_unlock(void)
{
	__raw_writel(KS2_KICK0_MAGIC, KS2_KICK0);
	__raw_writel(KS2_KICK1_MAGIC, KS2_KICK1);
}

#ifdef CONFIG_SOC_K2L
void osr_init(void)
{
	u32 i;
	u32 j;
	u32 val;
	u32 base = KS2_OSR_CFG_BASE;
	u32 ecc_ctrl[KS2_OSR_NUM_RAM_BANKS];

	/* Enable the OSR clock domain */
	psc_enable_module(KS2_LPSC_OSR);

	/* Disable OSR ECC check for all the ram banks */
	for (i = 0; i < KS2_OSR_NUM_RAM_BANKS; i++) {
		val = i | KS2_OSR_ECC_VEC_TRIG_RD |
			(KS2_OSR_ECC_CTRL << KS2_OSR_ECC_VEC_RD_ADDR_SH);

		writel(val , base + KS2_OSR_ECC_VEC);

		/**
		 * wait till read is done.
		 * Print should be added after earlyprintk support is added.
		 */
		for (j = 0; j < 10000; j++) {
			val = readl(base + KS2_OSR_ECC_VEC);
			if (val & KS2_OSR_ECC_VEC_RD_DONE)
				break;
		}

		ecc_ctrl[i] = readl(base + KS2_OSR_ECC_CTRL) ^
						KS2_OSR_ECC_CTRL_CHK;

		writel(ecc_ctrl[i], KS2_MSMC_DATA_BASE + i * 4);
		writel(ecc_ctrl[i], base + KS2_OSR_ECC_CTRL);
	}

	/* Reset OSR memory to all zeros */
	for (i = 0; i < KS2_OSR_SIZE; i += 4)
		writel(0, KS2_OSR_DATA_BASE + i);

	/* Enable OSR ECC check for all the ram banks */
	for (i = 0; i < KS2_OSR_NUM_RAM_BANKS; i++)
		writel(ecc_ctrl[i] |
		       KS2_OSR_ECC_CTRL_CHK, base + KS2_OSR_ECC_CTRL);
}
#endif

int arch_cpu_init(void)
{
	chip_configuration_unlock();
	icache_enable();

	msmc_share_all_segments(8);  /* TETRIS */
	msmc_share_all_segments(9);  /* NETCP */
	msmc_share_all_segments(10); /* QM PDSP */
	msmc_share_all_segments(11); /* PCIE 0 */
#ifdef CONFIG_SOC_K2E
	msmc_share_all_segments(13); /* PCIE 1 */
#endif
#ifdef CONFIG_SOC_K2L
	msmc_share_all_segments(14); /* PCIE 1 */
	osr_init();
#endif

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
	volatile u32 *rstctrl = (volatile u32 *)(KS2_RSTCTRL);
	u32 tmp;

	tmp = *rstctrl & KS2_RSTCTRL_MASK;
	*rstctrl = tmp | KS2_RSTCTRL_KEY;

	*rstctrl &= KS2_RSTCTRL_SWRST;

	for (;;)
		;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
#endif
}
