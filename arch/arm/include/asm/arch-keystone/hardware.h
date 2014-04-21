/*
 * Copyright (C) 2012 Texas Instruments
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <config.h>

#ifndef __ASSEMBLY__

#include <asm/sizes.h>
#include <asm/io.h>

#define	REG(addr)	(*(volatile unsigned int *)(addr))
#define REG_P(addr)	((volatile unsigned int *)(addr))

typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int *	dv_reg_p;

#define ASYNC_EMIF_NUM_CS		4
#define ASYNC_EMIF_MODE_NOR		0
#define ASYNC_EMIF_MODE_NAND		1
#define ASYNC_EMIF_MODE_ONENAND		2
#define ASYNC_EMIF_PRESERVE		-1

#define KS2_ASYNC_EMIF_CNTRL_BASE	0x21000a00
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE	KS2_ASYNC_EMIF_CNTRL_BASE

struct async_emif_config {
	unsigned mode;
	unsigned select_strobe;
	unsigned extend_wait;
	unsigned wr_setup;
	unsigned wr_strobe;
	unsigned wr_hold;
	unsigned rd_setup;
	unsigned rd_strobe;
	unsigned rd_hold;
	unsigned turn_around;
	enum {
		ASYNC_EMIF_8	= 0,
		ASYNC_EMIF_16	= 1,
		ASYNC_EMIF_32	= 2,
	} width;
};

void init_async_emif(int num_cs, struct async_emif_config *config);

struct ddr3_phy_config {
	unsigned int pllcr;
	unsigned int pgcr1_mask;
	unsigned int pgcr1_val;
	unsigned int ptr0;
	unsigned int ptr1;
	unsigned int ptr2;
	unsigned int ptr3;
	unsigned int ptr4;
	unsigned int dcr_mask;
	unsigned int dcr_val;
	unsigned int dtpr0;
	unsigned int dtpr1;
	unsigned int dtpr2;
	unsigned int mr0;
	unsigned int mr1;
	unsigned int mr2;
	unsigned int dtcr;
	unsigned int pgcr2;
	unsigned int zq0cr1;
	unsigned int zq1cr1;
	unsigned int zq2cr1;
	unsigned int pir_v1;
	unsigned int pir_v2;
};

struct ddr3_emif_config {
	unsigned int sdcfg;
	unsigned int sdtim1;
	unsigned int sdtim2;
	unsigned int sdtim3;
	unsigned int sdtim4;
	unsigned int zqcfg;
	unsigned int sdrfc;
};

#endif

#define		BIT(x)	(1 << (x))

/* DDR3 definitions */
#define KS2_DDRPHY_PIR_OFFSET		0x04
#define KS2_DDRPHY_PGCR0_OFFSET		0x08
#define KS2_DDRPHY_PGCR1_OFFSET		0x0C
#define KS2_DDRPHY_PGSR0_OFFSET		0x10
#define KS2_DDRPHY_PGSR1_OFFSET		0x14
#define KS2_DDRPHY_PLLCR_OFFSET		0x18
#define KS2_DDRPHY_PTR0_OFFSET		0x1C
#define KS2_DDRPHY_PTR1_OFFSET		0x20
#define KS2_DDRPHY_PTR2_OFFSET		0x24
#define KS2_DDRPHY_PTR3_OFFSET		0x28
#define KS2_DDRPHY_PTR4_OFFSET		0x2C
#define KS2_DDRPHY_DCR_OFFSET		0x44

#define KS2_DDRPHY_DTPR0_OFFSET		0x48
#define KS2_DDRPHY_DTPR1_OFFSET		0x4C
#define KS2_DDRPHY_DTPR2_OFFSET		0x50

#define KS2_DDRPHY_MR0_OFFSET		0x54
#define KS2_DDRPHY_MR1_OFFSET		0x58
#define KS2_DDRPHY_MR2_OFFSET		0x5C
#define KS2_DDRPHY_DTCR_OFFSET		0x68
#define KS2_DDRPHY_PGCR2_OFFSET		0x8C

#define KS2_DDRPHY_ZQ0CR1_OFFSET	0x184
#define KS2_DDRPHY_ZQ1CR1_OFFSET	0x194
#define KS2_DDRPHY_ZQ2CR1_OFFSET	0x1A4
#define KS2_DDRPHY_ZQ3CR1_OFFSET	0x1B4

#define KS2_DDRPHY_DATX8_8_OFFSET	0x3C0

#define IODDRM_MASK		0x00000180
#define ZCKSEL_MASK		0x01800000
#define CL_MASK			0x00000072
#define WR_MASK			0x00000E00
#define BL_MASK			0x00000003
#define RRMODE_MASK		0x00040000
#define UDIMM_MASK		0x20000000
#define BYTEMASK_MASK		0x0003FC00
#define MPRDQ_MASK		0x00000080
#define PDQ_MASK		0x00000070
#define NOSRA_MASK		0x08000000
#define ECC_MASK		0x00000001

#define KS2_DDR3_MIDR_OFFSET	0x00
#define KS2_DDR3_STATUS_OFFSET	0x04
#define KS2_DDR3_SDCFG_OFFSET	0x08
#define KS2_DDR3_SDRFC_OFFSET	0x10
#define KS2_DDR3_SDTIM1_OFFSET	0x18
#define KS2_DDR3_SDTIM2_OFFSET	0x1C
#define KS2_DDR3_SDTIM3_OFFSET	0x20
#define KS2_DDR3_SDTIM4_OFFSET	0x28
#define KS2_DDR3_PMCTL_OFFSET	0x38
#define KS2_DDR3_ZQCFG_OFFSET	0xC8

/* DDR3 ECC */
#define KS2_DDR3_ECC_INT_RAW_STATUS_OFFSET		0x0A4
#define KS2_DDR3_ECC_INT_STATUS_OFFSET		0x0AC
#define KS2_DDR3_ECC_INT_ENABLE_SET_SYS_OFFSET	0x0B4
#define KS2_DDR3_ECC_CTRL_OFFSET			0x110
#define KS2_DDR3_ECC_ADDR_RANGE1_OFFSET		0x114
#define KS2_DDR3_ECC_ADDR_RANGE2_OFFSET		0x118
#define KS2_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET		0x130
#define KS2_DDR3_ONE_BIT_ECC_ERR_THRSH_OFFSET	0x134
#define KS2_DDR3_ONE_BIT_ECC_ERR_DIST_1_OFFSET	0x138
#define KS2_DDR3_ONE_BIT_ECC_ERR_ADDR_LOG_OFFSET	0x13C
#define KS2_DDR3_TWO_BIT_ECC_ERR_ADDR_LOG_OFFSET	0x140
#define KS2_DDR3_ONE_BIT_ECC_ERR_DIST_2_OFFSET	0x144

/* DDR3 ECC Interrupt Status register definitions */
#define DDR3_1B_ECC_ERR_SYS	BIT(5)
#define DDR3_2B_ECC_ERR_SYS	BIT(4)
#define DDR3_WR_ECC_ERR_SYS	BIT(3)

/* DDR3 ECC Control register definiations */
#define DDR3_ECC_EN		BIT(31)
#define DDR3_ECC_ADDR_RNG_PROT	BIT(30)
#define DDR3_ECC_VERIFY_EN	BIT(29)
#define DDR3_ECC_RMW_EN		BIT(28)
#define DDR3_ECC_ADDR_RNG_2_EN	BIT(1)
#define DDR3_ECC_ADDR_RNG_1_EN	BIT(0)

#define DDR3_ECC_ENABLE		(DDR3_ECC_EN | DDR3_ECC_ADDR_RNG_PROT | \
				DDR3_ECC_VERIFY_EN)

/* EDMA3 register offsets */
#define EDMA_QCHMAP0		0x0200
#define EDMA_IPR		0x1068
#define EDMA_ICR		0x1070
#define EDMA_QEECR		0x1088
#define EDMA_QEESR		0x108c
#define EDMA_PARAM_1(x)		(0x4020 + (4 * x))

/* Chip Interrupt Controller register offsets */
#define CIC_REV				0x00
#define CIC_CTRL			0x04
#define CIC_HOST_CTRL			0x0C
#define CIC_GLOBAL_ENABLE		0x10
#define CIC_GLOBAL_NESTING_LEVEL	0x1C
#define CIC_SYS_STAT_IDX_SET		0x20
#define CIC_SYS_STAT_IDX_CLR		0x24
#define CIC_SYS_ENABLE_IDX_SET		0x28
#define CIC_SYS_ENABLE_IDX_CLR		0x2C
#define CIC_GLOBAL_WAKEUP_ENABLE	0x30
#define CIC_HOST_ENABLE_IDX_SET		0x34
#define CIC_HOST_ENABLE_IDX_CLR		0x38
#define CIC_PACING_PRESCALE		0x40
#define CIC_VECTOR_BASE			0x50
#define CIC_VECTOR_SIZE			0x54
#define CIC_VECTOR_NULL			0x58
#define CIC_PRIO_IDX			0x80
#define CIC_PRIO_VECTOR			0x84
#define CIC_SECURE_ENABLE		0x90
#define CIC_SECURE_PRIO_IDX		0x94
#define CIC_PACING_PARAM(n)		(0x0100 + (n << 4))
#define CIC_PACING_DEC(n)		(0x0104 + (n << 4))
#define CIC_PACING_MAP(n)		(0x0108 + (n << 4))
#define CIC_SYS_RAW_STAT(n)		(0x0200 + (n << 2))
#define CIC_SYS_STAT_CLR(n)		(0x0280 + (n << 2))
#define CIC_SYS_ENABLE_SET(n)		(0x0300 + (n << 2))
#define CIC_SYS_ENABLE_CLR(n)		(0x0380 + (n << 2))
#define CIC_CHAN_MAP(n)			(0x0400 + (n << 2))
#define CIC_HOST_MAP(n)			(0x0800 + (n << 2))
#define CIC_HOST_PRIO_IDX(n)		(0x0900 + (n << 2))
#define CIC_SYS_POLARITY(n)		(0x0D00 + (n << 2))
#define CIC_SYS_TYPE(n)			(0x0D80 + (n << 2))
#define CIC_WAKEUP_ENABLE(n)		(0x0E00 + (n << 2))
#define CIC_DEBUG_SELECT(n)		(0x0F00 + (n << 2))
#define CIC_SYS_SECURE_ENABLE(n)	(0x1000 + (n << 2))
#define CIC_HOST_NESTING_LEVEL(n)	(0x1100 + (n << 2))
#define CIC_HOST_ENABLE(n)		(0x1500 + (n << 2))
#define CIC_HOST_PRIO_VECTOR(n)		(0x1600 + (n << 2))
#define CIC_VECTOR_ADDR(n)		(0x2000 + (n << 2))

/* MSMC segment size shift bits */
#define MSMC_SEG_SIZE_SHIFT	12
#define MSMC_MAP_SEG_NUM	(2 << (30 - MSMC_SEG_SIZE_SHIFT))
#define MSMC_DST_SEG_BASE	(CONFIG_SYS_LPAE_SDRAM_BASE >> \
				 MSMC_SEG_SIZE_SHIFT)

/* PSC */
#define KS2_PSC_BASE		0x02350000

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/hardware-k2hk.h>
#endif

#ifdef CONFIG_SOC_K2E
#include <asm/arch/hardware-k2e.h>
#endif

#ifndef __ASSEMBLY__
static inline int cpu_is_k2hk(void)
{
	unsigned int jtag_id	= __raw_readl(JTAG_ID_REG);
	unsigned int part_no	= (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb981) ? 1 : 0);
}

static inline int cpu_is_k2e(void)
{
	unsigned int jtag_id	= __raw_readl(JTAG_ID_REG);
	unsigned int part_no	= (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb9a6) ? 1 : 0);
}

static inline int cpu_revision(void)
{
	unsigned int jtag_id	= __raw_readl(JTAG_ID_REG);
	unsigned int rev	= (jtag_id >> 28) & 0xf;

	return rev;
}

typedef enum {
	MPAX_SEG_4K = 0x0b,
	MPAX_SEG_8K,
	MPAX_SEG_16K,
	MPAX_SEG_32K,
	MPAX_SEG_64K,
	MPAX_SEG_128K,
	MPAX_SEG_256K,
	MPAX_SEG_512K,
	MPAX_SEG_1M,
	MPAX_SEG_2M,
	MPAX_SEG_4M,
	MPAX_SEG_8M,
	MPAX_SEG_16M,
	MPAX_SEG_32M,
	MPAX_SEG_64M,
	MPAX_SEG_128M,
	MPAX_SEG_256M,
	MPAX_SEG_512M,
	MPAX_SEG_1G,
	MPAX_SEG_2GM,
	MPAX_SEG_4G
} mpax_seg_size;

void map_ses_segment(int priv_id, int ses_pair,
		     u32 src_pfn, u32 dst_pfn, mpax_seg_size size);
void share_all_segments(int priv_id);
void get_ses_mpax(int priv_id, int ses_pair, u32 *mpax);
void set_ses_mpax(int priv_id, int ses_pair, u32 *mpax);
void check_ddr3_ecc_int(u32 base);
int init_ddr3_ecc(u32 base);
int ddr_memory_test(u32 start_address, u32 end_address, int quick);
void ddr3_enable_ecc(u32 base, int test);
void ddr3_disable_ecc(u32 base);
u32 get_ddr_seg_num(void);
void init_ddr3(void);
void init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg);
void init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg);
void ddr_reset_workaround(void);
void share_all_segments(int priv_id);

extern u32 debug_options;
#endif

#define DBG_LEAVE_DSPS_ON	0x1 

#endif /* __ASM_ARCH_HARDWARE_H */
