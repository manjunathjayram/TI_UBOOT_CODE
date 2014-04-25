/*
 * Copyright (C) 2012 - 2014 Texas Instruments Incorporated
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

#ifndef _EMAC_DEFS_H_
#define _EMAC_DEFS_H_

#include <asm/arch/hardware.h>
#include <asm/io.h>

typedef enum { FALSE = 0, TRUE = 1 } bool;

#define DEVICE_REG32_R(a)			readl(a)
#define DEVICE_REG32_W(a,v)			writel(v,a)

#define KS2_EMAC_GIG_ENABLE

#define MAC_ID_BASE_ADDR		(KS2_DEVICE_STATE_CTRL_BASE + 0x110)

/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		(clk_get_rate(pass_pll_clk))
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		1000000		/* 1.0 MHz */

/* MII Status Register */
#define MII_STATUS_REG			1
#define MII_STATUS_LINK_MASK		(0x4)

/* Marvell 88E1111 PHY ID */
#define PHY_MARVELL_88E1111		(0x01410cc0)



#define MDIO_CONTROL_IDLE		(0x80000000)
#define MDIO_CONTROL_ENABLE		(0x40000000)
#define MDIO_CONTROL_FAULT_ENABLE	(0x40000)
#define MDIO_CONTROL_FAULT		(0x80000)
#define MDIO_USERACCESS0_GO		(0x80000000)
#define MDIO_USERACCESS0_WRITE_READ	(0x0)
#define MDIO_USERACCESS0_WRITE_WRITE	(0x40000000)
#define MDIO_USERACCESS0_ACK		(0x20000000)

#define EMAC_MACCONTROL_MIIEN_ENABLE		(0x20)
#define EMAC_MACCONTROL_FULLDUPLEX_ENABLE	(0x1)
#define EMAC_MACCONTROL_GIGABIT_ENABLE		(1 << 7)
#define EMAC_MACCONTROL_GIGFORCE		(1 << 17)
#define EMAC_MACCONTROL_RMIISPEED_100		(1 << 15)

#define EMAC_MIN_ETHERNET_PKT_SIZE	60

struct mac_sl_cfg {
	u_int32_t max_rx_len;	/* Maximum receive packet length. */
	u_int32_t ctl;		/* Control bitfield */
};

/*******************************************************************************************
 * Definition: Control bitfields used in the ctl field of hwGmacSlCfg_t
 *******************************************************************************************/
#define GMACSL_RX_ENABLE_RCV_CONTROL_FRAMES       (1 << 24)
#define GMACSL_RX_ENABLE_RCV_SHORT_FRAMES         (1 << 23)
#define GMACSL_RX_ENABLE_RCV_ERROR_FRAMES         (1 << 22)
#define GMACSL_RX_ENABLE_EXT_CTL                  (1 << 18)  /* duplex and gig read from input pins */
#define GMACSL_RX_ENABLE_GIG_FORCE                (1 << 17)
#define GMACSL_RX_ENABLE_IFCTL_B                  (1 << 16)
#define GMACSL_RX_ENABLE_IFCTL_A                  (1 << 15)
#define GMACSL_RX_ENABLE_CMD_IDLE                 (1 << 11)
#define GMACSL_TX_ENABLE_SHORT_GAP                (1 << 10)
#define GMACSL_ENABLE_GIG_MODE                    (1 <<  7)
#define GMACSL_TX_ENABLE_PACE                     (1 <<  6)
#define GMACSL_ENABLE                             (1 <<  5)
#define GMACSL_TX_ENABLE_FLOW_CTL                 (1 <<  4)
#define GMACSL_RX_ENABLE_FLOW_CTL                 (1 <<  3)
#define GMACSL_ENABLE_LOOPBACK                    (1 <<  1)
#define GMACSL_ENABLE_FULL_DUPLEX                 (1 <<  0)


/********************************************************************************************
 * DEFINTITION: function return values
 ********************************************************************************************/
#define GMACSL_RET_OK                        0
#define GMACSL_RET_INVALID_PORT             -1
#define GMACSL_RET_WARN_RESET_INCOMPLETE    -2
#define GMACSL_RET_WARN_MAXLEN_TOO_BIG      -3
#define GMACSL_RET_CONFIG_FAIL_RESET_ACTIVE -4

/* Soft reset register values */
#define CPGMAC_REG_RESET_VAL_RESET_MASK      (1 << 0)
#define CPGMAC_REG_RESET_VAL_RESET           (1 << 0)



/* Maxlen register values */
#define CPGMAC_REG_MAXLEN_LEN                0x3fff

/**
 *  @brief
 *      Hardware network subsystem support, ethernet switch
 */

 /* Control bitfields */
#define CPSW_CTL_P2_PASS_PRI_TAGGED     (1 << 5)
#define CPSW_CTL_P1_PASS_PRI_TAGGED     (1 << 4)
#define CPSW_CTL_P0_PASS_PRI_TAGGED     (1 << 3)
#define CPSW_CTL_P0_ENABLE              (1 << 2)
#define CPSW_CTL_VLAN_AWARE             (1 << 1)

#define targetGetSwitchCtl()        CPSW_CTL_P0_ENABLE   /* Enable port 0 */
#define targetGetSwitchMaxPktSize() 9000

/* Register values */
#define CPSW_REG_VAL_ALE_CTL_RESET_AND_ENABLE	((u_int32_t)0xc0000000)
#define CPSW_REG_VAL_ALE_CTL_BYPASS		((u_int32_t)0x00000010)
#define CPSW_REG_VAL_PORTCTL_FORWARD_MODE        0x3

#define SGMII_REG_STATUS_LOCK		BIT(4)
#define	SGMII_REG_STATUS_LINK		BIT(0)
#define SGMII_REG_STATUS_AUTONEG	BIT(2)
#define SGMII_REG_CONTROL_AUTONEG	BIT(0)
#define SGMII_REG_CONTROL_MASTER	BIT(5)
#define	SGMII_REG_MR_ADV_ENABLE		BIT(0)
#define	SGMII_REG_MR_ADV_LINK		BIT(15)
#define	SGMII_REG_MR_ADV_FULL_DUPLEX	BIT(12)
#define SGMII_REG_MR_ADV_GIG_MODE	BIT(11)

#define SGMII_LINK_MAC_MAC_AUTONEG	0
#define SGMII_LINK_MAC_PHY		1
#define SGMII_LINK_MAC_MAC_FORCED	2
#define SGMII_LINK_MAC_FIBER		3
#define SGMII_LINK_MAC_PHY_FORCED	4

#define TARGET_SGMII_EXTERNAL_SERDES
#define TARGET_SGMII_TYPE_2             /* Use second sgmii setup sequence */

#define TARGET_SGMII_SERDES_BASE        (KS2_DEVICE_STATE_CTRL_BASE + 0x340)
#define TARGET_SGMII_SERDES_STATUS_BASE (KS2_DEVICE_STATE_CTRL_BASE + 0x158)
#define TARGET_SGMII_SOFT_RESET         0x04
#define TARGET_SGMII_CONTROL            0x10
#define TARGET_SGMII_MR_ADV_ABILITY     0x18

#define DEVICE_EMACSL_RESET_POLL_COUNT  100

/* EMAC MDIO Registers Structure */
typedef struct  {
	dv_reg		VERSION;
	dv_reg		CONTROL;
	dv_reg		ALIVE;
	dv_reg		LINK;
	dv_reg		LINKINTRAW;
	dv_reg		LINKINTMASKED;
	u_int8_t	RSVD0[8];
	dv_reg		USERINTRAW;
	dv_reg		USERINTMASKED;
	dv_reg		USERINTMASKSET;
	dv_reg		USERINTMASKCLEAR;
	u_int8_t	RSVD1[80];
	dv_reg		USERACCESS0;
	dv_reg		USERPHYSEL0;
	dv_reg		USERACCESS1;
	dv_reg		USERPHYSEL1;
} mdio_regs;

typedef struct
{
	char	name[64];
	int	(*init)(int phy_addr);
	int	(*is_phy_connected)(int phy_addr);
	int	(*get_link_speed)(int phy_addr);
	int	(*auto_negotiate)(int phy_addr);
} phy_t;

#define SGMII_ACCESS(port,reg)   *((volatile unsigned int *)(sgmiis[port] + reg))

typedef struct
{
	char	int_name[32];
	int	rx_flow;
	int	phy_addr;
	int	slave_port;
	int	sgmii_link_type;
	struct	eth_device *dev;
} eth_priv_t;

void keystone2_eth_open_close(struct eth_device *dev);

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/emac_defs_netcp.h>
#endif

#if defined(CONFIG_SOC_K2E) || defined(CONFIG_SOC_K2L)
#include <asm/arch/emac_defs_netcp2.h>
#endif

#endif  /* _EMAC_DEFS_H_ */
