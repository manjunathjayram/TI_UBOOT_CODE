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
#ifndef __ASM_ARCH_HARDWARE_K2E_H
#define __ASM_ARCH_HARDWARE_K2E_H

#define K2E_ASYNC_EMIF_CNTRL_BASE	0x21000a00
#define KS2_ASYNC_EMIF_CNTRL_BASE	K2E_ASYNC_EMIF_CNTRL_BASE
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE	K2E_ASYNC_EMIF_CNTRL_BASE
#define K2E_ASYNC_EMIF_DATA_CE0_BASE	0x30000000
#define K2E_ASYNC_EMIF_DATA_CE1_BASE	0x34000000
#define K2E_ASYNC_EMIF_DATA_CE2_BASE	0x38000000
#define K2E_ASYNC_EMIF_DATA_CE3_BASE	0x3c000000

#define K2E_PLL_CNTRL_BASE		0x02310000
#define CLOCK_BASE			K2E_PLL_CNTRL_BASE
#define KS2_DEVICE_STATE_CTRL_BASE	0x02620000
#define JTAG_ID_REG			(KS2_DEVICE_STATE_CTRL_BASE + 0x18)
#define K2E_DEVSTAT			(KS2_DEVICE_STATE_CTRL_BASE + 0x20)

#define K2E_SPI0_BASE			0x21000400
#define K2E_SPI1_BASE			0x21000600
#define K2E_SPI2_BASE			0x21000800
#define K2E_SPI_BASE			K2E_SPI0_BASE
#define KS2_SPI_BASE			K2E_SPI_BASE

/* Chip configuration unlock codes and registers */
#define KEYSTONE_KICK0			(KS2_DEVICE_STATE_CTRL_BASE + 0x38)
#define KEYSTONE_KICK1			(KS2_DEVICE_STATE_CTRL_BASE + 0x3c)
#define KEYSTONE_KICK0_MAGIC		0x83e70b13
#define KEYSTONE_KICK1_MAGIC		0x95a4f1e0

/* PA SS Registers */
#define KS2_PASS_BASE			0x24000000

/* PLL control registers */
#define K2E_MAINPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x350)
#define K2E_MAINPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x354)
#define K2E_PASSPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x358)
#define K2E_PASSPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x35C)
#define K2E_DDR3PLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x360)
#define K2E_DDR3PLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x364)

/******************************************************************************/
/* Power and Sleep Controller (PSC) Domains */
#define K2E_LPSC_MOD_RST		0
#define K2E_LPSC_USB_1			1
#define K2E_LPSC_USB			2
#define K2E_LPSC_EMIF25_SPI		3
#define KS2_LPSC_EMIF25_SPI		K2E_LPSC_EMIF25_SPI
#define K2E_LPSC_TSIP			4
#define K2E_LPSC_DEBUGSS_TRC		5
#define K2E_LPSC_TETB_TRC		6
#define K2E_LPSC_PKTPROC		7
#define KS2_LPSC_PA			K2E_LPSC_PKTPROC
#define K2E_LPSC_SGMII			8
#define KS2_LPSC_CPGMAC			K2E_LPSC_SGMII
#define K2E_LPSC_CRYPTO			9
#define K2E_LPSC_PCIE			10
#define K2E_LPSC_VUSR0			12
#define K2E_LPSC_CHIP_SRSS		13
#define K2E_LPSC_MSMC			14
#define K2E_LPSC_GEM_0			15
#define K2E_LPSC_EMIF4F_DDR3		23
#define K2E_LPSC_PCIE_1			27
#define K2E_LPSC_XGE			50
#define K2E_LPSC_TETRIS			52


#define K2E_UART0_BASE			0x02530c00
#define KS2_UART0_BASE			K2E_UART0_BASE

/* DDR3 definitions */
#define K2E_DDR3_EMIF_CTRL_BASE		0x21010000
#define K2E_DDR3_EMIF_DATA_BASE		0x80000000
#define K2E_DDR3_DDRPHYC		0x02329000

/* Queue manager */
#define DEVICE_QM_MANAGER_BASE		0x02a02000
#define DEVICE_QM_DESC_SETUP_BASE	0x02a03000
#define DEVICE_QM_MANAGER_QUEUES_BASE	0x02a80000
#define DEVICE_QM_MANAGER_Q_PROXY_BASE	0x02ac0000
#define DEVICE_QM_QUEUE_STATUS_BASE	0x02a40000
#define DEVICE_QM_NUM_LINKRAMS		2
#define DEVICE_QM_NUM_MEMREGIONS	20

#define DEVICE_PA_CDMA_GLOBAL_CFG_BASE	0x24186000
#define DEVICE_PA_CDMA_TX_CHAN_CFG_BASE	0x24187000
#define DEVICE_PA_CDMA_RX_CHAN_CFG_BASE	0x24188000
#define DEVICE_PA_CDMA_RX_FLOW_CFG_BASE	0x24189000

#define DEVICE_PA_CDMA_RX_NUM_CHANNELS	24
#define DEVICE_PA_CDMA_RX_NUM_FLOWS	32
#define DEVICE_PA_CDMA_TX_NUM_CHANNELS	9

/* MSMC control */
#define K2E_MSMC_CTRL_BASE		0x0bc00000

#endif /* __ASM_ARCH_HARDWARE_H */
