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

#ifndef _EMAC_DEFS_NETCP2_H_
#define _EMAC_DEFS_NETCP2_H_

#define EMAC_EMACSL_BASE_ADDR		(KS2_PASS_BASE + 0x00222000)
#define EMAC_MDIO_BASE_ADDR		(KS2_PASS_BASE + 0x00200f00)
#define EMAC_SGMII_BASE_ADDR		(KS2_PASS_BASE + 0x00200100)

/* EMAC SL Register offsets */
#define CPGMACSL_REG_RX_PRI_MAP		0x020
#define CPGMACSL_REG_MAXLEN		0x024
#define CPGMACSL_REG_SA_LO		0x308
#define CPGMACSL_REG_SA_HI		0x30c
#define CPGMACSL_REG_CTL		0x330
#define CPGMACSL_REG_STATUS		0x334
#define CPGMACSL_REG_RESET		0x338

/*
 *	Ethernet switch definitions
 */
#ifdef CONFIG_SOC_K2E
#define DEVICE_CPSW_NUM_PORTS		9	/* 9 switch ports */
#endif
#ifdef CONFIG_SOC_K2L
#define DEVICE_CPSW_NUM_PORTS		5	/* 5 switch ports */
#endif
#define DEVICE_N_GMACSL_PORTS		(DEVICE_CPSW_NUM_PORTS - 1)

#define DEVICE_CPSW_BASE		(KS2_PASS_BASE + 0x00220000)

/* CPSW Register offsets */
#define CPSW_REG_CTL			0x00004
#define CPSW_REG_STAT_PORT_EN		0x00014
#define CPSW_REG_MAXLEN			0x01024	/* CPPI P0 RX Maxlen reg */
#define CPSW_REG_ALE_CONTROL		0x1e008
#define CPSW_REG_ALE_PORTCTL(x)		(0x1e040 + (x)*4)

/* CPSW Register values */
#define CPSW_REG_VAL_STAT_ENABLE_ALL	0x1ff

/* CPSW Net RX flow */
#define CPSW_NET_RX_FLOW		0
#define CPSW_PORT_RX_FLOW(x)		(x * 8)

/* SGMII definitions */
#define TARGET_SGMII_BASE		EMAC_SGMII_BASE_ADDR
#ifdef CONFIG_SOC_K2E
#define TARGET_SGMII_BASE_ADDRESSES    {EMAC_SGMII_BASE_ADDR, \
					EMAC_SGMII_BASE_ADDR + 0x100, \
					EMAC_SGMII_BASE_ADDR + 0x200, \
					EMAC_SGMII_BASE_ADDR + 0x300, \
					EMAC_SGMII_BASE_ADDR + 0x400, \
					EMAC_SGMII_BASE_ADDR + 0x500, \
					EMAC_SGMII_BASE_ADDR + 0x600, \
					EMAC_SGMII_BASE_ADDR + 0x700}
#endif
#ifdef CONFIG_SOC_K2L
#define TARGET_SGMII_BASE_ADDRESSES    {EMAC_SGMII_BASE_ADDR, \
					EMAC_SGMII_BASE_ADDR + 0x100, \
					EMAC_SGMII_BASE_ADDR + 0x200, \
					EMAC_SGMII_BASE_ADDR + 0x300}
#endif

/*
 * SGMII registers
 */
#define DEVICE_EMACSL_BASE(x)		(EMAC_EMACSL_BASE_ADDR + (x) * 0x1000)

#define SGMII_OFFSET(x)			((x) * 0x100)	/* SGMII port offset */

#define SGMII_IDVER_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x000)
#define SGMII_SRESET_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x004)
#define SGMII_CTL_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x010)
#define SGMII_STATUS_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x014)
#define SGMII_MRADV_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x018)
#define SGMII_LPADV_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x020)
#define SGMII_TXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x030)
#define SGMII_RXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x034)
#define SGMII_AUXCFG_REG(x)	(TARGET_SGMII_BASE + SGMII_OFFSET(x) + 0x038)

/* Streaming switch */
#define DEVICE_PSTREAM_CFG_REG_ADDR		(KS2_PASS_BASE + 0x0500)
#define DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_PDSP0	0x02020202
#define DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI	0x0

#define hwConfigStreamingSwitch()	\
	DEVICE_REG32_W(DEVICE_PSTREAM_CFG_REG_ADDR, \
		       DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI); \
	DEVICE_REG32_W(DEVICE_PSTREAM_CFG_REG_ADDR + 4, \
		       DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI); \
	DEVICE_REG32_W(DEVICE_PSTREAM_CFG_REG_ADDR + 8, \
		       DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI); \
	DEVICE_REG32_W(DEVICE_PSTREAM_CFG_REG_ADDR + 12, \
		       DEVICE_PSTREAM_CFG_REG_VAL_ROUTE_CPPI);

#endif  /* _EMAC_DEFS_NETCP2_H_ */
