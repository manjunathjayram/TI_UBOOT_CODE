/*
 * Copyright (C) 2013 Texas Instruments Inc.
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
#include <common.h>
#include <command.h>

#define EDMA_BLK_SIZE_SHIFT	10
#define EDMA_BLK_SIZE		(1 << EDMA_BLK_SIZE_SHIFT)
#define EDMA_BCNT		0x8000
#define EDMA_CCNT		1
#define EDMA_XF_SIZE		(EDMA_BLK_SIZE * EDMA_BCNT)

/************************* *****************************/
static struct ddr3_phy_config ddr3phy_1600_64A = {
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
	.dtpr0		= 0xA19DBB66ul,
	.dtpr1		= 0x32868300ul,
	.dtpr2		= 0x50035200ul,
	.mr0		= 0x00001C70ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000018ul,
	.dtcr		= 0x730035C7ul,
	.pgcr2		= 0x00F07A12ul,
	.zq0cr1		= 0x0000005Dul,
	.zq1cr1		= 0x0000005Bul,
	.zq2cr1		= 0x0000005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};

static struct ddr3_emif_config ddr3_1600_64 = {
	.sdcfg		= 0x6200CE6Aul,
	.sdtim1		= 0x16709C55ul,
	.sdtim2		= 0x00001D4Aul,
	.sdtim3		= 0x435DFF54ul,
	.sdtim4		= 0x553F0CFFul,
	.zqcfg		= 0xF0073200ul,
	.sdrfc		= 0x00001869ul,
};

static struct ddr3_phy_config ddr3phy_1600_32 = {
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
	.dtpr0		= 0xA19DBB66ul,
	.dtpr1		= 0x32868300ul,
	.dtpr2		= 0x50035200ul,
	.mr0		= 0x00001C70ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000018ul,
	.dtcr		= 0x730035C7ul,
	.pgcr2		= 0x00F07A12ul,
	.zq0cr1		= 0x0000005Dul,
	.zq1cr1		= 0x0000005Bul,
	.zq2cr1		= 0x0000005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};

static struct ddr3_emif_config ddr3_1600_32 = {
	.sdcfg		= 0x6200DE6Aul,
	.sdtim1		= 0x16709C55ul,
	.sdtim2		= 0x00001D4Aul,
	.sdtim3		= 0x435DFF54ul,
	.sdtim4		= 0x553F0CFFul,
	.zqcfg		= 0xF0073200ul,
	.sdrfc		= 0x00001869ul,
};

/************************* *****************************/
static struct ddr3_phy_config ddr3phy_1333_64A = {
	.pllcr		= 0x0005C000ul,
	.pgcr1_mask	= (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val	= ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0		= 0x42C21590ul,
	.ptr1		= 0xD05612C0ul,
	.ptr2		= 0, /* not set in gel */
	.ptr3		= 0x0B4515C2ul,
	.ptr4		= 0x0A6E08B4ul,
	.dcr_mask	= (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val	= ((1 << 10)),
	.dtpr0		= 0x8558AA55ul,
	.dtpr1		= 0x32857280ul,
	.dtpr2		= 0x5002C200ul,
	.mr0		= 0x00001A60ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000010ul,
	.dtcr		= 0x710035C7ul,
	.pgcr2		= 0x00F065B8ul,
	.zq0cr1		= 0x0000005Dul,
	.zq1cr1		= 0x0000005Bul,
	.zq2cr1		= 0x0000005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};

static struct ddr3_emif_config ddr3_1333_64 = {
	.sdcfg		= 0x62008C62ul,
	.sdtim1		= 0x125C8044ul,
	.sdtim2		= 0x00001D29ul,
	.sdtim3		= 0x32CDFF43ul,
	.sdtim4		= 0x543F0ADFul,
	.zqcfg		= 0x70073200ul,
	.sdrfc		= 0x00001457ul,
};

static struct ddr3_phy_config ddr3phy_1333_32 = {
	.pllcr		= 0x0005C000ul,
	.pgcr1_mask	= (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val	= ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0		= 0x42C21590ul,
	.ptr1		= 0xD05612C0ul,
	.ptr2		= 0, /* not set in gel */
	.ptr3		= 0x0B4515C2ul,
	.ptr4		= 0x0A6E08B4ul,
	.dcr_mask	= (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val	= ((1 << 10)),
	.dtpr0		= 0x8558AA55ul,
	.dtpr1		= 0x32857280ul,
	.dtpr2		= 0x5002C200ul,
	.mr0		= 0x00001A60ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000010ul,
	.dtcr		= 0x710035C7ul,
	.pgcr2		= 0x00F065B8ul,
	.zq0cr1		= 0x0000005Dul,
	.zq1cr1		= 0x0000005Bul,
	.zq2cr1		= 0x0000005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};

static struct ddr3_emif_config ddr3_1333_32 = {
	.sdcfg		= 0x62009C62ul,
	.sdtim1		= 0x125C8044ul,
	.sdtim2		= 0x00001D29ul,
	.sdtim3		= 0x32CDFF43ul,
	.sdtim4		= 0x543F0ADFul,
	.zqcfg		= 0x70073200ul,
	.sdrfc		= 0x00001457ul,
};

/************************* *****************************/
static struct ddr3_phy_config ddr3phy_1333_64 = {
	.pllcr		= 0x0005C000ul,
	.pgcr1_mask	= (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val	= ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0		= 0x42C21590ul,
	.ptr1		= 0xD05612C0ul,
	.ptr2		= 0, /* not set in gel */
	.ptr3		= 0x0B4515C2ul,
	.ptr4		= 0x0A6E08B4ul,
	.dcr_mask	= (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val	= ((1 << 10)),
	.dtpr0		= 0x8558AA55ul,
	.dtpr1		= 0x32857280ul,
	.dtpr2		= 0x5002C200ul,
	.mr0		= 0x00001A60ul,
	.mr1		= 0x00000006ul,
	.mr2		= 0x00000010ul,
	.dtcr		= 0x710035C7ul,
	.pgcr2		= 0x00F065B8ul,
	.zq0cr1		= 0x0000005Dul,
	.zq1cr1		= 0x0000005Bul,
	.zq2cr1		= 0x0000005Bul,
	.pir_v1		= 0x00000033ul,
	.pir_v2		= 0x0000FF81ul,
};
/************************* *****************************/

void init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg)
{
	unsigned int tmp;

	while((__raw_readl(base + TCI6638_DDRPHY_PGSR0_OFFSET)
		 & 0x00000001) != 0x00000001);

	__raw_writel(phy_cfg->pllcr, base + TCI6638_DDRPHY_PLLCR_OFFSET);

	tmp = __raw_readl(base + TCI6638_DDRPHY_PGCR1_OFFSET);
	tmp &= ~(phy_cfg->pgcr1_mask);
	tmp |= phy_cfg->pgcr1_val;
        __raw_writel(tmp, base + TCI6638_DDRPHY_PGCR1_OFFSET);

	__raw_writel(phy_cfg->ptr0,   base + TCI6638_DDRPHY_PTR0_OFFSET);
	__raw_writel(phy_cfg->ptr1,   base + TCI6638_DDRPHY_PTR1_OFFSET);
	__raw_writel(phy_cfg->ptr3,  base + TCI6638_DDRPHY_PTR3_OFFSET);
	__raw_writel(phy_cfg->ptr4,  base + TCI6638_DDRPHY_PTR4_OFFSET);

	tmp =  __raw_readl(base + TCI6638_DDRPHY_DCR_OFFSET);
	tmp &= ~(phy_cfg->dcr_mask);
	tmp |= phy_cfg->dcr_val;
	__raw_writel(tmp, base + TCI6638_DDRPHY_DCR_OFFSET);

	__raw_writel(phy_cfg->dtpr0, base + TCI6638_DDRPHY_DTPR0_OFFSET);
	__raw_writel(phy_cfg->dtpr1, base + TCI6638_DDRPHY_DTPR1_OFFSET);
	__raw_writel(phy_cfg->dtpr2, base + TCI6638_DDRPHY_DTPR2_OFFSET);
	__raw_writel(phy_cfg->mr0,   base + TCI6638_DDRPHY_MR0_OFFSET);
	__raw_writel(phy_cfg->mr1,   base + TCI6638_DDRPHY_MR1_OFFSET);
	__raw_writel(phy_cfg->mr2,   base + TCI6638_DDRPHY_MR2_OFFSET);
	__raw_writel(phy_cfg->dtcr,  base + TCI6638_DDRPHY_DTCR_OFFSET);
	__raw_writel(phy_cfg->pgcr2, base + TCI6638_DDRPHY_PGCR2_OFFSET);

	__raw_writel(phy_cfg->zq0cr1, base + TCI6638_DDRPHY_ZQ0CR1_OFFSET);
	__raw_writel(phy_cfg->zq1cr1, base + TCI6638_DDRPHY_ZQ1CR1_OFFSET);
	__raw_writel(phy_cfg->zq2cr1, base + TCI6638_DDRPHY_ZQ2CR1_OFFSET);

	__raw_writel(phy_cfg->pir_v1, base + TCI6638_DDRPHY_PIR_OFFSET);
	do {
		tmp = __raw_readl(base + TCI6638_DDRPHY_PGSR0_OFFSET);
	} while	((tmp & 0x1) != 0x1);

	__raw_writel(phy_cfg->pir_v2, base + TCI6638_DDRPHY_PIR_OFFSET);
	while ((__raw_readl(base + TCI6638_DDRPHY_PGSR0_OFFSET) & 0x1) != 0x1);
}

void init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg)
{
	__raw_writel(emif_cfg->sdcfg,  base + TCI6638_DDR3_SDCFG_OFFSET );
	__raw_writel(emif_cfg->sdtim1, base + TCI6638_DDR3_SDTIM1_OFFSET);
	__raw_writel(emif_cfg->sdtim2, base + TCI6638_DDR3_SDTIM2_OFFSET);
	__raw_writel(emif_cfg->sdtim3, base + TCI6638_DDR3_SDTIM3_OFFSET);
	__raw_writel(emif_cfg->sdtim4, base + TCI6638_DDR3_SDTIM4_OFFSET);
	__raw_writel(emif_cfg->zqcfg,  base + TCI6638_DDR3_ZQCFG_OFFSET );
	__raw_writel(emif_cfg->sdrfc,  base + TCI6638_DDR3_SDRFC_OFFSET );
}

static int ddr_ecc_support_rmw(u32 base)
{
	u32 value = __raw_readl(base + TCI6638_DDR3_MIDR_OFFSET);

	/* Check the DDR3 controller ID reg if the controllers
	   supports ECC RMW or not */
	if (value == 0x40461C02)
		return 1;

	return 0;
}

void config_ddr3_ecc(u32 base, u32 value)
{
	u32 data;

	__raw_writel(value,  base + TCI6638_DDR3_ECC_CTRL_OFFSET);
	udelay(100000); /* delay required to synchronize across clock domains */

	if (value & DDR3_ECC_EN) {
		/* Clear the 1-bit error count */
		data = __raw_readl(base + TCI6638_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);
		__raw_writel(data, base + TCI6638_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);

		/* enable the ECC interrupt */
		__raw_writel(DDR3_1B_ECC_ERR_SYS | DDR3_2B_ECC_ERR_SYS | \
			DDR3_WR_ECC_ERR_SYS,
			base + TCI6638_DDR3_ECC_INT_ENABLE_SET_SYS_OFFSET);

		/* Clear the ECC error interrupt status */
		__raw_writel(DDR3_1B_ECC_ERR_SYS | DDR3_2B_ECC_ERR_SYS | \
				DDR3_WR_ECC_ERR_SYS,
				base + TCI6638_DDR3_ECC_INT_STATUS_OFFSET);
	}
}

/* This functions uses EDMA driver to copy the data from
src address, to the dst address */
static int ddr3_scrub_memory(u32 base, u32 src, u32 dst)
{
	/* Clear the pending int bit */
	__raw_writel(1, base + EDMA_ICR);

	/* QDMA channel 0 uses parameter set 1, trigger word 7 */
	__raw_writel(0x3c, base + EDMA_QCHMAP0);

	/* Enable qdma channel 0 */
	__raw_writel(1, base + EDMA_QEESR);

	/* parameter 0, opt - tciintn, static and syncdim set */
	__raw_writel(0x0010000c, base + EDMA_PARAM_1(0));

	/* parameter 1 - source address */
	__raw_writel(src, base + EDMA_PARAM_1(1));

	/* parameter 2 - bcount, acount */
	__raw_writel((EDMA_BCNT << 16) | EDMA_BLK_SIZE,
			base + EDMA_PARAM_1(2));

	/* parameter 3, destination */
	__raw_writel(dst, base + EDMA_PARAM_1(3));

	/* parameter 4  dstbidx = 1k, srcbidx = 0 */
	__raw_writel(EDMA_BLK_SIZE << 16, base + EDMA_PARAM_1(4));

	/* parameter 5 - link */
	__raw_writel(0xffff, base + EDMA_PARAM_1(5));

	/* parameter 6 - dstcidx, srccidx */
	__raw_writel(0, base + EDMA_PARAM_1(6));

	/*  paramter 7 - ccnt */
	__raw_writel(EDMA_CCNT, base + EDMA_PARAM_1(7));

	/* Wait until complete interrupt */
	while (!(__raw_readl(base + EDMA_IPR) & 0x1))
		udelay(10);

	return 0;
}

static void reset_ddr_data(u32 base, u32 seg_num)
{
	u32 edma_src[EDMA_BLK_SIZE/4] __attribute__((aligned(16))) = {0, };
	u32 seg, blks, dst, edma_blks;
	u32 edma_base = TCI6638_EDMA0_BASE;
	u32 mpax[2];

	/* Setup an edma to copy the 1k block to the entire DDR */
	printf("\nReseting entire DDR3 memory to 0 ...\n");

	/* save the SES MPAX regs */
	get_ses_mpax(8, 0, mpax);
	for (seg = 0; seg < seg_num; seg += MSMC_MAP_SEG_NUM) {
		/* map 2GB 36-bit DDR address to 32-bit DDR address in EMIF
		   access slave interface so that edma driver can access */
		map_ses_segment(8, 0, base >> MSMC_SEG_SIZE_SHIFT,
		     MSMC_DST_SEG_BASE + seg, 0x1e);

		if ((seg_num - seg) > MSMC_MAP_SEG_NUM)
			edma_blks = MSMC_MAP_SEG_NUM << (MSMC_SEG_SIZE_SHIFT \
				    - EDMA_BLK_SIZE_SHIFT);
		else
			edma_blks = (seg_num - seg) << (MSMC_SEG_SIZE_SHIFT \
				    - EDMA_BLK_SIZE_SHIFT);

		/* Use edma driver to scrub 2GB DDR memory */
		for (dst = base, blks = 0;
		     blks < edma_blks;
		     blks += EDMA_BCNT, dst += EDMA_XF_SIZE)
			ddr3_scrub_memory(edma_base, (u32)edma_src, dst);
	}
	/* restore the SES MPAX regs */
	set_ses_mpax(8, 0, mpax);

	/* clean up the interrupt indication */
	__raw_writel(1, edma_base + EDMA_ICR);

	/* Disable qdma channel 0 */
	__raw_writel(1, edma_base + EDMA_QEECR);

	/* Clear the channel map */
	__raw_writel(0, edma_base + EDMA_QCHMAP0);
}

void check_ddr3_ecc_int(u32 base)
{
	u32 value = __raw_readl(base + TCI6638_DDR3_ECC_INT_STATUS_OFFSET);
	char *env;
	int ecc_test = 0;

	env = getenv("ecc_test");
	if (env)
		ecc_test = simple_strtol(env, NULL, 0);

	if (value & DDR3_WR_ECC_ERR_SYS)
		printf("DDR3 ECC write error interrupted\n");

	if (value & DDR3_2B_ECC_ERR_SYS) {
		printf("DDR3 ECC 2-bit error interrupted\n");
		if (!ecc_test) {
			printf("Reseting the device ...\n");
			reset_cpu(0);
		}
	}

	value = __raw_readl(base + TCI6638_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);
	if (value) {
		printf("1-bit ECC err count: 0x%x\n", value);
		value = __raw_readl(base + \
				TCI6638_DDR3_ONE_BIT_ECC_ERR_ADDR_LOG_OFFSET);
		printf("1-bit ECC err address log: 0x%x\n", value);
	}
}

static void keystone_init_cic(u32 base)
{
	/* Disable CIC global interrupts */
	__raw_writel(0, base + CIC_GLOBAL_ENABLE);

	/* Set to normal mode, no nesting, no priority hold */
	__raw_writel(0, base + CIC_CTRL);
	__raw_writel(0, base + CIC_HOST_CTRL);

	/* Enable CIC global interrupts */
	__raw_writel(1, base + CIC_GLOBAL_ENABLE);
}

static void keystone_map_cic_to_gic(u32 base, u32 chan_num, u32 irq_num)
{
	/* Map the system interrupt to a CIC channel */
	__raw_writeb(chan_num, base + CIC_CHAN_MAP(0) + irq_num);

	/* Enable CIC system interrupt */
	__raw_writel(irq_num, base + CIC_SYS_ENABLE_IDX_SET);

	/* Enable CIC Host interrupt */
	__raw_writel(chan_num, base + CIC_HOST_ENABLE_IDX_SET);
}

static void keystone_map_ddr3_ecc_cic2_irq(u32 base)
{
	if (cpu_is_tci6638()) {
		keystone_init_cic(base);
		keystone_map_cic_to_gic(base, DDR3_ECC_CIC2_CHAN_NUM,
					 DDR3_ECC_CIC2_IRQ_NUM);
	}
}

static void ddr3_init_ecc(u32 base)
{
	u32 ecc_val = DDR3_ECC_EN;
	u32 rmw = ddr_ecc_support_rmw(base);

	if (rmw)
		ecc_val |= DDR3_ECC_RMW_EN;
	__raw_writel(0, base + TCI6638_DDR3_ECC_ADDR_RANGE1_OFFSET);

	config_ddr3_ecc(base, ecc_val);
}

void ddr3_enable_ecc(u32 base, int test)
{
	u32 ecc_val = DDR3_ECC_ENABLE;
	u32 rmw = ddr_ecc_support_rmw(base);

	if (test)
		ecc_val |= DDR3_ECC_ADDR_RNG_1_EN;

	if (rmw)
		ecc_val |= DDR3_ECC_RMW_EN;
	else {
		if (!test)
			/* by default, disable ecc when rmw = 0 and no
			   ecc test */
			ecc_val = 0;
	}

	config_ddr3_ecc(base, ecc_val);
}

void ddr3_disable_ecc(u32 base)
{
	config_ddr3_ecc(base, 0);
}

int init_ddr3_ecc(u32 base)
{
	u32 seg_num;

	ddr3_init_ecc(base);

	seg_num = get_ddr_seg_num();

	reset_ddr_data(CONFIG_SYS_SDRAM_BASE, seg_num);

	/* mapping DDR3 ECC system interrupt from CIC2 to GIC */
	keystone_map_ddr3_ecc_cic2_irq(TCI6638_CIC2_BASE);

	if (ddr_ecc_support_rmw(base))
		ddr3_enable_ecc(base, 0);
	else
		ddr3_disable_ecc(base);
}

/*********** Begin DDR Reset Workaround ***********/
void ddr_reset_workaround(void)
{

	unsigned int tmp, tmp_a, tmp_b;

	/*
	 * Check for PGSR0 error bits of DDR3 PHY.
	 * Check for WLERR, QSGERR, WLAERR,
	 * RDERR, WDERR, REERR, WEERR error to see if they are set or not
	 */
	tmp_a = __raw_readl(TCI6638_DDR3A_DDRPHYC +
				TCI6638_DDRPHY_PGSR0_OFFSET);
	tmp_b = __raw_readl(TCI6638_DDR3B_DDRPHYC +
				TCI6638_DDRPHY_PGSR0_OFFSET);

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
		tmp_a = __raw_readl(TCI6638_PSC_BASE + 0xA5C);
		tmp_a &= ~(0x1000);
		__raw_writel(tmp_a, TCI6638_PSC_BASE + 0xA5C);

		/*
		 * Move DDR3B Module out of reset isolation by setting
		 * MDCTL24[12] = 0
		 */
		 tmp_b = __raw_readl(TCI6638_PSC_BASE + 0xA60);
		 tmp_b &= ~(0x1000);
		__raw_writel(tmp_b, TCI6638_PSC_BASE + 0xA60);

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes
		 * to RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0xFFFF);
		tmp |= 0x5A69;
		__raw_writel(tmp, TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/*
		 * Set PLL Controller to drive hard reset on SW trigger by
		 * setting RSTCFG[13] = 0
		 */
		tmp = __raw_readl(TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCFG);
		tmp &= ~(0x2000);
		__raw_writel(tmp, TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCFG);

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes to
		 * RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0xFFFF);
		tmp |= 0x5A69;
		__raw_writel(tmp, TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/*
		 * Write RSTCTRL[16] = 0 to initiate software reset via PLL
		 * controller
		 */
		tmp = __raw_readl(TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);
		tmp &= ~(0x10000);
		__raw_writel(tmp, TCI6638_PLL_CNTRL_BASE +
					MAIN_PLL_CTRL_RSTCTRL);

		/* Wait for Reset */
		for(;;);
	}
}

int get_dimm_params(char *dimm_name)
{
	u8 spd_params[256];
	int ret, y, x;
	int old_bus;

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	ret = i2c_read(0x53, 0, 1, spd_params, 256);

	i2c_set_bus_num(old_bus);

	dimm_name[0] = '\0';

	if (ret) {
		puts("Cannot read DIMM params\n");
		return 1;
	}

#if 0
	for (y=0; y<16; y++) {
		for (x=0; x<16; x++)
			printf("%02x ", spd_params[y*16+x]);
		printf("   ");

		for (x=0; x<16; x++)
			if ((spd_params[y*16+x] >= 0x20) &&
			    (spd_params[y*16+x] <= 0x7e))
				printf("%c", spd_params[y*16+x]);
			else
				printf(".");
		printf("\n");
	}
#endif
	/*
	 * We need to convert spd data to dimm parameters
	 * and to DDR3 EMIF and PHY regirsters values.
	 * For now we just return DIMM type string value.
	 * Caller may use this value to choose appropriate
	 * a pre-set DDR3 configuration
	 */

	strncpy(dimm_name, &spd_params[0x80], 18);
	dimm_name[18] = '\0';

	return 0;
}

struct pll_init_data ddr3a_333 = DDR3_PLL_333(A);
struct pll_init_data ddr3b_333 = DDR3_PLL_333(B);
struct pll_init_data ddr3a_400 = DDR3_PLL_400(A);
struct pll_init_data ddr3b_400 = DDR3_PLL_400(B);

void init_ddr3( void ) {

	char dimm_name[32];

	if (__raw_readl(TCI6638_PLL_CNTRL_BASE + MAIN_PLL_CTRL_RSTYPE) & 0x1) {
		if (cpu_revision() > 0)
			init_pll(&ddr3a_400);
		else
			init_pll(&ddr3a_333);
		init_pll(&ddr3b_333);
	} else
		printf("Power-on reset was not the last reset to occur !\n");

	get_dimm_params(dimm_name);

	printf("Detected SO-DIMM [%s]\n", dimm_name);

	if (!strcmp(dimm_name, "18KSF1G72HZ-1G6E2 ")) {
		/* 8G SO-DIMM */
		if (cpu_revision() > 0) {
			init_ddrphy(TCI6638_DDR3A_DDRPHYC, &ddr3phy_1600_64A);
			init_ddremif(TCI6638_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_64);
			printf("DRAM:  Capacity 8 GiB (includes reported below)\n");
		}
		else {
			init_ddrphy(TCI6638_DDR3A_DDRPHYC, &ddr3phy_1600_32);
			init_ddremif(TCI6638_DDR3A_EMIF_CTRL_BASE, &ddr3_1600_32);
			printf("DRAM:  Capacity 4 GiB (includes reported below)\n");
		}
	}
	else {
		/* 2G SO-DIMM */
		if (cpu_revision() > 0) {
			init_ddrphy(TCI6638_DDR3A_DDRPHYC, &ddr3phy_1333_64A);
			init_ddremif(TCI6638_DDR3A_EMIF_CTRL_BASE, &ddr3_1333_64);
		}
		else {
			init_ddrphy(TCI6638_DDR3A_DDRPHYC, &ddr3phy_1333_32);
			init_ddremif(TCI6638_DDR3A_EMIF_CTRL_BASE, &ddr3_1333_32);
		}
	}

	init_ddrphy(TCI6638_DDR3B_DDRPHYC, &ddr3phy_1333_64);
	init_ddremif(TCI6638_DDR3B_EMIF_CTRL_BASE, &ddr3_1333_64);

	/* Apply the workaround for PG 1.0 and 1.1 Silicons */
	if (cpu_revision() <= 1)
		ddr_reset_workaround();
}

u32 get_ddr_seg_num(void)
{
	/* tmp: DDR3 size in segments (4KB seg size), hardcoded to 8GB size */
	return 8 << (30 - MSMC_SEG_SIZE_SHIFT);
}

