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
#ifndef __ASM_ARCH_HARDWARE_TCI6638_H
#define __ASM_ARCH_HARDWARE_TCI6638_H

#define TCI6638_ASYNC_EMIF_CNTRL_BASE		(0x21000a00)
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		(TCI6638_ASYNC_EMIF_CNTRL_BASE)
#define TCI6638_ASYNC_EMIF_DATA_CE0_BASE	(0x30000000)
#define TCI6638_ASYNC_EMIF_DATA_CE1_BASE	(0x34000000)
#define TCI6638_ASYNC_EMIF_DATA_CE2_BASE	(0x38000000)
#define TCI6638_ASYNC_EMIF_DATA_CE3_BASE	(0x3c000000)

#define TCI6638_PLL_CNTRL_BASE		(0x02310000)
#define CLOCK_BASE			TCI6638_PLL_CNTRL_BASE
#define TCI6638_PSC_BASE		(0x02350000)
#define TCI66XX_DEVICE_STATE_CTRL_BASE	(0x02620000)
#define JTAG_ID_REG			(TCI66XX_DEVICE_STATE_CTRL_BASE + 0x18)
#define TCI6638_DEVSTAT			(TCI66XX_DEVICE_STATE_CTRL_BASE + 0x20)

#define TCI6638_SPI0_BASE		(0x21000400)
#define TCI6638_SPI1_BASE		(0x21000600)
#define TCI6638_SPI2_BASE		(0x21000800)
#define TCI6638_SPI_BASE		TCI6638_SPI0_BASE

/* Chip configuration unlock codes and registers */
#define KEYSTONE_KICK0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x38)
#define KEYSTONE_KICK1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x3c)
#define KEYSTONE_KICK0_MAGIC	0x83e70b13
#define KEYSTONE_KICK1_MAGIC	0x95a4f1e0

/* PA SS Registers */
#define TCI66XX_PASS_BASE			(0x02000000)

/* PLL control registers */
#define TCI6638_MAINPLLCTL0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x350)
#define TCI6638_MAINPLLCTL1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x354)
#define TCI6638_PASSPLLCTL0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x358)
#define TCI6638_PASSPLLCTL1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x35C)
#define TCI6638_DDR3APLLCTL0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x360)
#define TCI6638_DDR3APLLCTL1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x364)
#define TCI6638_DDR3BPLLCTL0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x368)
#define TCI6638_DDR3BPLLCTL1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x36C)
#define TCI6638_ARMPLLCTL0		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x370)
#define TCI6638_ARMPLLCTL1		(TCI66XX_DEVICE_STATE_CTRL_BASE+0x374)

/******************************************************************************/
/* Power and Sleep Controller (PSC) Domains */
#define TCI6638_LPSC_MOD		0
#define TCI6638_LPSC_DUMMY1		1
#define TCI6638_LPSC_USB		2
#define TCI6638_LPSC_EMIF25_SPI		3
#define TCI6638_LPSC_TSIP		4
#define TCI6638_LPSC_DEBUGSS_TRC		5
#define TCI6638_LPSC_TETB_TRC		6
#define TCI6638_LPSC_PKTPROC		7
#define TCI66XX_LPSC_PA			TCI6638_LPSC_PKTPROC
#define TCI6638_LPSC_SGMII		8
#define TCI66XX_LPSC_CPGMAC		TCI6638_LPSC_SGMII
#define TCI6638_LPSC_CRYPTO		9
#define TCI6638_LPSC_PCIE		10
#define TCI6638_LPSC_SRIO		11
#define TCI6638_LPSC_VUSR0		12
#define TCI6638_LPSC_CHIP_SRSS		13
#define TCI6638_LPSC_MSMC		14
#define TCI6638_LPSC_GEM_0		15
#define TCI6638_LPSC_GEM_1		16
#define TCI6638_LPSC_GEM_2		17
#define TCI6638_LPSC_GEM_3		18
#define TCI6638_LPSC_GEM_4		19
#define TCI6638_LPSC_GEM_5		20
#define TCI6638_LPSC_GEM_6		21
#define TCI6638_LPSC_GEM_7		22
#define TCI6638_LPSC_EMIF4F_DDR3A	23
#define TCI6638_LPSC_EMIF4F_DDR3B	24
#define TCI6638_LPSC_TAC		25
#define TCI6638_LPSC_RAC		26
#define TCI6638_LPSC_RAC_1		27
#define TCI6638_LPSC_FFTC_A		28
#define TCI6638_LPSC_FFTC_B		29
#define TCI6638_LPSC_FFTC_C		30
#define TCI6638_LPSC_FFTC_D		31
#define TCI6638_LPSC_FFTC_E		32
#define TCI6638_LPSC_FFTC_F		33
#define TCI6638_LPSC_AI2		34
#define TCI6638_LPSC_TCP3D_0		35
#define TCI6638_LPSC_TCP3D_1		36
#define TCI6638_LPSC_TCP3D_2		37
#define TCI6638_LPSC_TCP3D_3		38
#define TCI6638_LPSC_VCP2X4_A		39
#define TCI6638_LPSC_CP2X4_B		40
#define TCI6638_LPSC_VCP2X4_C		41
#define TCI6638_LPSC_VCP2X4_D		42
#define TCI6638_LPSC_VCP2X4_E		43
#define TCI6638_LPSC_VCP2X4_F		44
#define TCI6638_LPSC_VCP2X4_G		45
#define TCI6638_LPSC_VCP2X4_H		46
#define TCI6638_LPSC_BCP		47
#define TCI6638_LPSC_DXB		48
#define TCI6638_LPSC_VUSR1		49
#define TCI6638_LPSC_XGE		50
#define TCI6638_LPSC_ARM_SREFLEX	51
#define TCI6638_LPSC_TETRIS		52


#define TCI6638_UART0_BASE			(0x02530c00)
#define TCI6638_UART1_BASE			(0x02531000)

/* DDR3A definitions */
#define TCI6638_DDR3A_EMIF_CTRL_BASE		0x21010000
#define TCI6638_DDR3A_EMIF_DATA_BASE		0x80000000
#define TCI6638_DDR3A_DDRPHYC			0x02329000
/* DDR3B definitions */
#define TCI6638_DDR3B_EMIF_CTRL_BASE		0x21020000
#define TCI6638_DDR3B_EMIF_DATA_BASE		0x60000000
#define TCI6638_DDR3B_DDRPHYC			0x02328000 

#define TCI6638_DDRPHY_PIR_OFFSET		0x04
#define TCI6638_DDRPHY_PGCR0_OFFSET		0x08
#define TCI6638_DDRPHY_PGCR1_OFFSET		0x0C
#define TCI6638_DDRPHY_PGSR0_OFFSET		0x10
#define TCI6638_DDRPHY_PGSR1_OFFSET		0x14
#define TCI6638_DDRPHY_PLLCR_OFFSET		0x18
#define TCI6638_DDRPHY_PTR0_OFFSET		0x1C
#define TCI6638_DDRPHY_PTR1_OFFSET		0x20
#define TCI6638_DDRPHY_PTR2_OFFSET		0x24
#define TCI6638_DDRPHY_PTR3_OFFSET		0x28
#define TCI6638_DDRPHY_PTR4_OFFSET		0x2C
#define TCI6638_DDRPHY_DCR_OFFSET		0x44

#define TCI6638_DDRPHY_DTPR0_OFFSET		0x48
#define TCI6638_DDRPHY_DTPR1_OFFSET		0x4C
#define TCI6638_DDRPHY_DTPR2_OFFSET		0x50

#define TCI6638_DDRPHY_MR0_OFFSET		0x54
#define TCI6638_DDRPHY_MR1_OFFSET		0x58
#define TCI6638_DDRPHY_MR2_OFFSET		0x5C
#define TCI6638_DDRPHY_DTCR_OFFSET		0x68
#define TCI6638_DDRPHY_PGCR2_OFFSET		0x8C

#define TCI6638_DDRPHY_ZQ0CR1_OFFSET		0x184
#define TCI6638_DDRPHY_ZQ1CR1_OFFSET  		0x194
#define TCI6638_DDRPHY_ZQ2CR1_OFFSET		0x1A4
#define TCI6638_DDRPHY_ZQ3CR1_OFFSET		0x1B4

#define TCI6638_DDRPHY_DATX8_8_OFFSET 		0x3C0



#define IODDRM_MASK            0x00000180  
#define ZCKSEL_MASK            0x01800000
#define CL_MASK				   0x00000072
#define WR_MASK				   0x00000E00
#define BL_MASK				   0x00000003
#define RRMODE_MASK            0x00040000
#define UDIMM_MASK             0x20000000
#define BYTEMASK_MASK          0x0003FC00
#define MPRDQ_MASK             0x00000080
#define PDQ_MASK               0x00000070
#define NOSRA_MASK             0x08000000
#define ECC_MASK               0x00000001



#define TCI6638_DDR3_MIDR_OFFSET		0x00
#define TCI6638_DDR3_STATUS_OFFSET		0x04
#define TCI6638_DDR3_SDCFG_OFFSET		0x08
#define TCI6638_DDR3_SDRFC_OFFSET		0x10
#define TCI6638_DDR3_SDTIM1_OFFSET		0x18
#define TCI6638_DDR3_SDTIM2_OFFSET		0x1C
#define TCI6638_DDR3_SDTIM3_OFFSET		0x20
#define TCI6638_DDR3_SDTIM4_OFFSET		0x28
#define TCI6638_DDR3_PMCTL_OFFSET		0x38
#define TCI6638_DDR3_ZQCFG_OFFSET		0xC8

/* DDR3 ECC */
#define TCI6638_DDR3_ECC_INT_RAW_STATUS_OFFSET		0x0A4
#define TCI6638_DDR3_ECC_INT_STATUS_OFFSET		0x0AC
#define TCI6638_DDR3_ECC_INT_ENABLE_SET_SYS_OFFSET	0x0B4
#define TCI6638_DDR3_ECC_CTRL_OFFSET			0x110
#define TCI6638_DDR3_ECC_ADDR_RANGE1_OFFSET		0x114
#define TCI6638_DDR3_ECC_ADDR_RANGE2_OFFSET		0x118
#define TCI6638_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET		0x130
#define TCI6638_DDR3_ONE_BIT_ECC_ERR_THRSH_OFFSET	0x134
#define TCI6638_DDR3_ONE_BIT_ECC_ERR_DIST_1_OFFSET	0x138
#define TCI6638_DDR3_ONE_BIT_ECC_ERR_ADDR_LOG_OFFSET	0x13C
#define TCI6638_DDR3_TWO_BIT_ECC_ERR_ADDR_LOG_OFFSET	0x140
#define TCI6638_DDR3_ONE_BIT_ECC_ERR_DIST_2_OFFSET	0x144

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

/* Queue manager */
#define DEVICE_QM_MANAGER_BASE          0x02a02000
#define DEVICE_QM_DESC_SETUP_BASE       0x02a03000
#define DEVICE_QM_MANAGER_QUEUES_BASE   0x02a80000
#define DEVICE_QM_MANAGER_Q_PROXY_BASE  0x02ac0000
#define DEVICE_QM_QUEUE_STATUS_BASE	0x02a40000
#define DEVICE_QM_NUM_LINKRAMS          2
#define DEVICE_QM_NUM_MEMREGIONS        20

#define DEVICE_PA_CDMA_GLOBAL_CFG_BASE   0x02004000
#define DEVICE_PA_CDMA_TX_CHAN_CFG_BASE  0x02004400
#define DEVICE_PA_CDMA_RX_CHAN_CFG_BASE  0x02004800
#define DEVICE_PA_CDMA_RX_FLOW_CFG_BASE  0x02005000

#define DEVICE_PA_CDMA_RX_NUM_CHANNELS   24
#define DEVICE_PA_CDMA_RX_NUM_FLOWS      32
#define DEVICE_PA_CDMA_TX_NUM_CHANNELS   9

/* MSMC control */
#define TCI6638_MSMC_CTRL_BASE		0x0bc00000

/* EDMA */
#define TCI6638_EDMA0_BASE			0x02700000

/* Chip Interrupt Controller */
#define TCI6638_CIC2_BASE			0x02608000

#define DDR3_ECC_CIC2_IRQ_NUM		0x0D3   /* DDR3 ECC system irq # */
#define DDR3_ECC_CIC2_CHAN_NUM		0x01D   /* DDR3 ECC int mapped to CIC2
						   channel 29 */

#endif /* __ASM_ARCH_HARDWARE_H */