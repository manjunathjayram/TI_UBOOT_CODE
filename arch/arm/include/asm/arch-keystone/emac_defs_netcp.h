/*
 * Copyright (C) 2014 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _EMAC_DEFS_NETCP_H_
#define _EMAC_DEFS_NETCP_H_

#define EMAC_EMACSL_BASE_ADDR		(KS2_PASS_BASE + 0x00090900)
#define EMAC_MDIO_BASE_ADDR		(KS2_PASS_BASE + 0x00090300)
#define EMAC_SGMII_BASE_ADDR		(KS2_PASS_BASE + 0x00090100)

/* EMAC SL Register offsets */
#define CPGMACSL_REG_ID			0x00
#define CPGMACSL_REG_CTL		0x04
#define CPGMACSL_REG_STATUS		0x08
#define CPGMACSL_REG_RESET		0x0c
#define CPGMACSL_REG_MAXLEN		0x10
#define CPGMACSL_REG_BOFF		0x14
#define CPGMACSL_REG_RX_PAUSE		0x18
#define CPGMACSL_REG_TX_PAURSE		0x1c
#define CPGMACSL_REG_EM_CTL		0x20
#define CPGMACSL_REG_PRI		0x24

/*
 *	Ethernet switch definitions
 */

/* CPSW control reg bitfields */
#define CPSW_CTL_FIFO_LOOPBACK		BIT(0)

#define DEVICE_CPSW_NUM_PORTS		5	/* 5 switch ports */
#define DEVICE_CPSW_BASE		(KS2_PASS_BASE + 0x00090800)

/* Register offsets */
#define CPSW_REG_CTL			0x004
#define CPSW_REG_STAT_PORT_EN		0x00c
#define CPSW_REG_MAXLEN			0x040
#define CPSW_REG_ALE_CONTROL		0x608
#define CPSW_REG_ALE_PORTCTL(x)		(0x640 + (x) * 4)

/* Register values */
#define CPSW_REG_VAL_STAT_ENABLE_ALL	0xf

/* CPSW Net RX flow */
#define CPSW_NET_RX_FLOW		22

/* SGMII definitions */
#define TARGET_SGMII_BASE		KS2_PASS_BASE + 0x00090100
#define TARGET_SGMII_BASE_ADDRESSES    {KS2_PASS_BASE+0x00090100, \
					KS2_PASS_BASE+0x00090200, \
					KS2_PASS_BASE+0x00090400, \
					KS2_PASS_BASE+0x00090500  }

#define SGMII_OFFSET(x)	((x <= 1) ? (x * 0x100) : ((x * 0x100) + 0x100))

/*
 * SGMII registers
 */
#define SGMII_IDVER_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x000)
#define SGMII_SRESET_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x004)
#define SGMII_CTL_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x010)
#define SGMII_STATUS_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x014)
#define SGMII_MRADV_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x018)
#define SGMII_LPADV_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x020)
#define SGMII_TXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x030)
#define SGMII_RXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x034)
#define SGMII_AUXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x038)

#define DEVICE_EMACSL_BASE(x)           (KS2_PASS_BASE + 0x00090900 + (x)*0x040)
#define DEVICE_N_GMACSL_PORTS		(DEVICE_CPSW_NUM_PORTS - 1)
#define DEVICE_EMACSL_RESET_POLL_COUNT	100

#define DEVICE_PSTREAM_CFG_REG_ADDR		(KS2_PASS_BASE + 0x604)
#define DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_PDSP0	0x0
#define DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI	0x06060606

#define hwConfigStreamingSwitch()	\
	DEVICE_REG32_W(DEVICE_PSTREAM_CFG_REG_ADDR, \
		       DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI);

#endif  /* _EMAC_DEFS_NETCP_H_ */
