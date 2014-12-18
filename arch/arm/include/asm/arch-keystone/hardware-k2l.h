/*
 * Copyright (C) 2014 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef __ASM_ARCH_HARDWARE_K2L_H
#define __ASM_ARCH_HARDWARE_K2L_H

#define K2L_ASYNC_EMIF_DATA_CE0_BASE	0x30000000
#define K2L_ASYNC_EMIF_DATA_CE1_BASE	0x34000000
#define K2L_ASYNC_EMIF_DATA_CE2_BASE	0x38000000
#define K2L_ASYNC_EMIF_DATA_CE3_BASE	0x3c000000

#define K2L_PLL_CNTRL_BASE		0x02310000
#define CLOCK_BASE			K2L_PLL_CNTRL_BASE

/* PA SS Registers */
#define KS2_PASS_BASE			0x26000000

/* PLL control registers */
#define K2L_MAINPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x350)
#define K2L_MAINPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x354)
#define K2L_PASSPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x358)
#define K2L_PASSPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x35C)
#define K2L_DDR3PLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x360)
#define K2L_DDR3PLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x364)
#define K2L_ARMPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x370)
#define K2L_ARMPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x374)

/******************************************************************************/
/* Power and Sleep Controller (PSC) Domains */
#define K2L_LPSC_MOD			0
#define K2L_LPSC_DFE_IQN_SYS		1
#define KS2_LPSC_USB1			~0UL
#define K2L_LPSC_USB			2
#define KS2_LPSC_USB			K2L_LPSC_USB
#define K2L_LPSC_EMIF25_SPI		3
#define KS2_LPSC_EMIF25_SPI		K2L_LPSC_EMIF25_SPI
#define K2L_LPSC_DEBUGSS_TRC		5
#define K2L_LPSC_TETB_TRC		6
#define K2L_LPSC_PKTPROC		7
#define KS2_LPSC_PA			K2L_LPSC_PKTPROC
#define K2L_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			K2L_LPSC_SGMII
#define K2L_LPSC_CRYPTO			9
#define KS2_LPSC_CRYPTO			K2L_LPSC_CRYPTO
#define K2L_LPSC_PCIE0			10
#define K2L_LPSC_PCIE1			11
#define K2L_LPSC_JESD_MISC		12
#define K2L_LPSC_CHIP_SRSS		13
#define K2L_LPSC_MSMC			14
#define K2L_LPSC_GEM_0			KS2_LPSC_GEM_0
#define K2L_LPSC_GEM_1			16
#define K2L_LPSC_GEM_2			17
#define K2L_LPSC_GEM_3			18
#define K2L_LPSC_EMIF4F_DDR3		23
#define K2L_LPSC_TAC			25
#define K2L_LPSC_RAC			26
#define K2L_LPSC_DDUC4X_CFR2X_BB	27
#define K2L_LPSC_FFTC_A			28
#define K2L_LPSC_OSR			34
#define K2L_LPSC_TCP3D_0		35
#define K2L_LPSC_TCP3D_1		37
#define K2L_LPSC_VCP2X4_A		39
#define K2L_LPSC_VCP2X4_B		40
#define K2L_LPSC_VCP2X4_C		41
#define K2L_LPSC_VCP2X4_D		42
#define K2L_LPSC_BCP			47
#define K2L_LPSC_DPD4X			48
#define K2L_LPSC_FFTC_B			49
#define K2L_LPSC_IQN_AIL		50
#define KS2_LPSC_XGE			~0UL
#define K2L_LPSC_TETRIS			KS2_LPSC_TETRIS

/* DDR3 definitions */
#define K2L_DDR3_EMIF_CTRL_BASE		0x21010000
#define K2L_DDR3_EMIF_DATA_BASE		0x80000000
#define K2L_DDR3_DDRPHYC		0x02329000

/* Queue manager */
#define DEVICE_QM_MANAGER_BASE		0x02a02000
#define DEVICE_QM_DESC_SETUP_BASE	0x02a03000
#define DEVICE_QM_MANAGER_QUEUES_BASE	0x02a80000
#define DEVICE_QM_MANAGER_Q_PROXY_BASE	0x02ac0000
#define DEVICE_QM_QUEUE_STATUS_BASE	0x02a40000
#define DEVICE_QM_NUM_LINKRAMS		2
#define DEVICE_QM_NUM_MEMREGIONS	20

#define DEVICE_PA_CDMA_GLOBAL_CFG_BASE	0x02004000
#define DEVICE_PA_CDMA_TX_CHAN_CFG_BASE	0x02004400
#define DEVICE_PA_CDMA_RX_CHAN_CFG_BASE	0x02004800
#define DEVICE_PA_CDMA_RX_FLOW_CFG_BASE	0x02005000

#define DEVICE_PA_CDMA_RX_NUM_CHANNELS	24
#define DEVICE_PA_CDMA_RX_NUM_FLOWS	32
#define DEVICE_PA_CDMA_TX_NUM_CHANNELS	9

/* Chip Interrupt Controller */
#define DDR3_ECC_CIC2_IRQ_NUM		0x0D3   /* DDR3 ECC system irq # */
#define DDR3_ECC_CIC2_CHAN_NUM		0x01D   /* DDR3 ECC int mapped to CIC2
						   channel 29 */

/* SGMII SerDes */
#define KS2_SGMII_SERDES2_BASE		0x02320000
#define KS2_LANES_PER_SGMII_SERDES	2

/* OSR */
#define KS2_OSR_DATA_BASE		0x70000000	/* OSR data memory
							   base */
#define KS2_OSR_CFG_BASE		0x02348c00	/* OSR config reg
							   base */
#define KS2_OSR_ECC_VEC			0x08		/* ECC Vector reg */
#define KS2_OSR_ECC_CTRL		0x14		/* ECC control reg */

/* OSR ECC Vector register */
#define OSR_ECC_VEC_TRIG_RD		BIT(15)		/* trigger a read op */
#define OSR_ECC_VEC_RD_DONE		BIT(24)		/* read complete */

#define OSR_ECC_VEC_RAM_ID_SH		0		/* RAM ID shift */
#define OSR_ECC_VEC_RD_ADDR_SH		16		/* read address shift */

/* OSR ECC control register */
#define OSR_ECC_CTRL_EN			BIT(0)		/* ECC enable bit */
#define OSR_ECC_CTRL_CHK		BIT(1)		/* ECC check bit */
#define OSR_ECC_CTRL_RMW		BIT(2)		/* ECC check bit */

/* Number of OSR RAM banks */
#define OSR_NUM_RAM_BANKS		4

/* OSR memory size */
#define OSR_SIZE			0x100000

/* Number of DSP cores */
#define KS2_NUM_DSPS			4

#ifndef __ASSEMBLY__
enum {
	SPD800,
	SPD1000,
	SPD1200,
	SPD1350,
	SPD1400,
	SPD_RSV
};
#endif
#endif /* __ASM_ARCH_HARDWARE_H */
