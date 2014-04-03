/*
 * Ethernet driver for TI KS2 EVM.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated
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
#include <command.h>

#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/keystone_nav.h>

unsigned int emac_dbg = 0;
#define debug_emac(fmt, args...) if (emac_dbg) printf(fmt, ##args)

unsigned int emac_open = 0;
static unsigned int loopback_test = 0;
static unsigned int sys_has_mdio = 1;

#ifdef KS2_EMAC_GIG_ENABLE
#define emac_gigabit_enable(x)	keystone2_eth_gigabit_enable(x)
#else
#define emac_gigabit_enable(x)	/* no gigabit to enable */
#endif

#define RX_BUFF_NUMS	24
#define RX_BUFF_LEN	1520
#define MAX_SIZE_STREAM_BUFFER RX_BUFF_LEN

static u8 rx_buffs[RX_BUFF_NUMS * RX_BUFF_LEN] __attribute__((aligned(16)));

struct rx_buff_desc net_rx_buffs = {
	.buff_ptr	= rx_buffs,
	.num_buffs	= RX_BUFF_NUMS,
	.buff_len	= RX_BUFF_LEN,
	.rx_flow	= 22,
};

extern int cpu_to_bus(u32 *ptr, u32 length);

static void keystone2_eth_mdio_enable(void);

static int gen_init_phy(int phy_addr);
static int gen_is_phy_connected(int phy_addr);
static int gen_get_link_speed(int phy_addr);
static int gen_auto_negotiate(int phy_addr);

static int marvell_88e1111_init_phy(int phy_addr);
static int marvell_88e1111_is_phy_connected(int phy_addr);
static int marvell_88e1111_get_link_speed(int phy_addr);
static int marvell_88e1111_auto_negotiate(int phy_addr);
void sgmii_serdes_setup_156p25mhz(void);
void sgmii_serdes_shutdown(void);

/* EMAC Addresses */
static volatile emac_regs	*adap_emac = (emac_regs *)EMAC_EMACSL_BASE_ADDR;
static volatile mdio_regs	*adap_mdio = (mdio_regs *)EMAC_MDIO_BASE_ADDR;

phy_t phy;

static void chip_delay(u32 del)
{
	volatile unsigned int i;

	for (i = 0; i < (del / 8); i++);
}

int keystone2_eth_read_mac_addr(struct eth_device *dev)
{
	eth_priv_t *eth_priv;
	u32 maca = 0;
	u32 macb = 0;

	eth_priv = (eth_priv_t*)dev->priv;

	/* Read the e-fuse mac address */
	if (eth_priv->slave_port == 1) {
		maca = __raw_readl(MAC_ID_BASE_ADDR);
		macb = __raw_readl(MAC_ID_BASE_ADDR + 4);
	}

	dev->enetaddr[0] = (macb >>  8) & 0xff;
	dev->enetaddr[1] = (macb >>  0) & 0xff;
	dev->enetaddr[2] = (maca >> 24) & 0xff;
	dev->enetaddr[3] = (maca >> 16) & 0xff;
	dev->enetaddr[4] = (maca >>  8) & 0xff;
	dev->enetaddr[5] = (maca >>  0) & 0xff;

	return 0;
}

static void keystone2_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	writel((clkdiv & 0xffff) |
	       MDIO_CONTROL_ENABLE |
	       MDIO_CONTROL_FAULT |
	       MDIO_CONTROL_FAULT_ENABLE,
	       &adap_mdio->CONTROL);

	while (readl(&adap_mdio->CONTROL) & MDIO_CONTROL_IDLE)
		;
}

/* Read a PHY register via MDIO inteface. Returns 1 on success, 0 otherwise */
int keystone2_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_READ |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while ((tmp = readl(&adap_mdio->USERACCESS0)) & MDIO_USERACCESS0_GO)
		;

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return(0);
	}

	*data = -1;
	return(-1);
}

/* Write to a PHY register via MDIO inteface. Blocks until operation is complete. */
int keystone2_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{

	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	writel(MDIO_USERACCESS0_GO |
	       MDIO_USERACCESS0_WRITE_WRITE |
	       ((reg_num & 0x1f) << 21) |
	       ((phy_addr & 0x1f) << 16) |
	       (data & 0xffff),
	       &adap_mdio->USERACCESS0);

	/* Wait for command to complete */
	while (readl(&adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO)
		;

	return(0);
}

/* PHY functions for a generic PHY */
static int __attribute__((unused)) gen_init_phy(int phy_addr)
{
	int	ret = -1;

	if (!gen_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = gen_get_link_speed(phy_addr);
	}

	return(ret);
}

static int __attribute__((unused)) gen_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return keystone2_eth_phy_read(phy_addr, MII_PHYSID1, &dummy);
}

static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if ((!keystone2_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp)) &&
			(tmp & 0x04)) {
		return(0);
	}

	return(-1);
}

static int __attribute__((unused)) gen_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;

	if (keystone2_eth_phy_read(phy_addr, MII_BMCR, &tmp))
		return(-1);

	/* Restart Auto_negotiation  */
	tmp |= BMCR_ANENABLE;
	keystone2_eth_phy_write(phy_addr, MII_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (keystone2_eth_phy_read(phy_addr, MII_BMSR, &tmp))
		return(-1);

	if (!(tmp & BMSR_ANEGCOMPLETE))
		return(0);

	return(gen_get_link_speed(phy_addr));
}

/* PHY functions for Marvell 88E1111 PHY */
static int marvell_88e1111_init_phy(int phy_addr)
{
	int	ret = -1;

	if (!marvell_88e1111_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = marvell_88e1111_get_link_speed(phy_addr);
	}

	return(ret);
}

static int marvell_88e1111_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return keystone2_eth_phy_read(phy_addr, MII_PHYSID1, &dummy);
}

static int marvell_88e1111_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if ((!keystone2_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp)) &&
			(tmp & MII_STATUS_LINK_MASK)) {

		return(0);
	}

	return(-1);
}

static int marvell_88e1111_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;

	if (keystone2_eth_phy_read(phy_addr, MII_BMCR, &tmp))
		return(-1);

	/* Restart Auto_negotiation  */
	tmp |= BMCR_ANENABLE;
	keystone2_eth_phy_write(phy_addr, MII_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (keystone2_eth_phy_read(phy_addr, MII_BMSR, &tmp))
		return(-1);

	if (!(tmp & BMSR_ANEGCOMPLETE))
		return(0);

	return(marvell_88e1111_get_link_speed(phy_addr));
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int keystone2_mii_phy_read(const char *devname, unsigned char addr,
				unsigned char reg, unsigned short *value)
{
	return keystone2_eth_phy_read(addr, reg, value) ? -1 : 0;
}

static int keystone2_mii_phy_write(const char *devname, unsigned char addr,
				 unsigned char reg, unsigned short value)
{
	return keystone2_eth_phy_write(addr, reg, value) ? -1 : 0;
}
#endif

static void  __attribute__((unused)) 
	keystone2_eth_gigabit_enable(struct eth_device *dev)
{
	u_int16_t data;
	eth_priv_t *eth_priv = (eth_priv_t*)dev->priv;

	if (sys_has_mdio) {
		if (keystone2_eth_phy_read(eth_priv->phy_addr, 0, &data) ||
			!(data & (1 << 6))) /* speed selection MSB */
			return;
	}

	/*
	 * Check if link detected is giga-bit
	 * If Gigabit mode detected, enable gigbit in MAC
	 */
	writel(readl(&(adap_emac[eth_priv->slave_port - 1].MACCONTROL)) |
	        EMAC_MACCONTROL_GIGFORCE | EMAC_MACCONTROL_GIGABIT_ENABLE, 
		&(adap_emac[eth_priv->slave_port - 1].MACCONTROL));
}

int keystone_sgmii_link_status(int port)
{
	u32 status = 0;

	status = __raw_readl(SGMII_STATUS_REG(port));

	return (status & SGMII_REG_STATUS_LINK);
}


int keystone_get_link_status(struct eth_device *dev)
{
	eth_priv_t *eth_priv = (eth_priv_t*)dev->priv;
	int sgmii_link;
	int link_state = 0;
#if CONFIG_GET_LINK_STATUS_ATTEMPTS > 1
	int j;

	for (j = 0; (j < CONFIG_GET_LINK_STATUS_ATTEMPTS) && (link_state == 0); j++) {
#endif
		sgmii_link = keystone_sgmii_link_status(eth_priv->slave_port - 1);
		if (sgmii_link) {
			link_state = 1;

			if (eth_priv->sgmii_link_type == SGMII_LINK_MAC_PHY)
				if (phy.get_link_speed (eth_priv->phy_addr))
					link_state = 0;
		}
#if CONFIG_GET_LINK_STATUS_ATTEMPTS > 1
	}
#endif
	return link_state;
}

int keystone_sgmii_config(int port, int interface)
{
	unsigned int i, status, mask;
	unsigned int mr_adv_ability, control;

	switch (interface) {
	case SGMII_LINK_MAC_MAC_AUTONEG:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= (SGMII_REG_CONTROL_MASTER |
				   SGMII_REG_CONTROL_AUTONEG);

		break;
	case SGMII_LINK_MAC_PHY:
	case SGMII_LINK_MAC_PHY_FORCED:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	case SGMII_LINK_MAC_MAC_FORCED:
		mr_adv_ability	= (SGMII_REG_MR_ADV_ENABLE |
				   SGMII_REG_MR_ADV_LINK |
				   SGMII_REG_MR_ADV_FULL_DUPLEX |
				   SGMII_REG_MR_ADV_GIG_MODE);
		control		= SGMII_REG_CONTROL_MASTER;

		break;
	case SGMII_LINK_MAC_FIBER:
		mr_adv_ability	= 0x20;
		control		= SGMII_REG_CONTROL_AUTONEG;

		break;
	default:
		mr_adv_ability	= SGMII_REG_MR_ADV_ENABLE;
		control		= SGMII_REG_CONTROL_AUTONEG;
	}

	__raw_writel(0, SGMII_CTL_REG(port));

	/*
	 * Wait for the SerDes pll to lock,
	 * but don't trap if lock is never read
	 */
	for (i = 0; i < 1000; i++)  {
		udelay(2000);
		status = __raw_readl(SGMII_STATUS_REG(port));
		if ((status & SGMII_REG_STATUS_LOCK) != 0)
			break;
	}

	__raw_writel(mr_adv_ability, SGMII_MRADV_REG(port));
	__raw_writel(control, SGMII_CTL_REG(port));


	mask = SGMII_REG_STATUS_LINK;

	if (control & SGMII_REG_CONTROL_AUTONEG)
		mask |= SGMII_REG_STATUS_AUTONEG;

	for (i = 0; i < 1000; i++)  {
		status = __raw_readl(SGMII_STATUS_REG(port));
		if ((status & mask) == mask)
			break;
	}

	return 0;
}

int mac_sl_reset(u32 port)
{
	u32 i, v;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return (GMACSL_RET_INVALID_PORT);

	/* Set the soft reset bit */
	DEVICE_REG32_W(DEVICE_EMACSL_BASE(port) +
		       CPGMACSL_REG_RESET, CPGMAC_REG_RESET_VAL_RESET);

	/* Wait for the bit to clear */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = DEVICE_REG32_R (DEVICE_EMACSL_BASE(port) +
				    CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			return (GMACSL_RET_OK);
	}

	/* Timeout on the reset */
	return (GMACSL_RET_WARN_RESET_INCOMPLETE);
}

int mac_sl_config(u_int16_t port, struct mac_sl_cfg *cfg)
{
	u32 v, i;
	int ret = GMACSL_RET_OK;

	if (port >= DEVICE_N_GMACSL_PORTS)
		return (GMACSL_RET_INVALID_PORT);

	if (cfg->max_rx_len > CPGMAC_REG_MAXLEN_LEN) {
		cfg->max_rx_len = CPGMAC_REG_MAXLEN_LEN;
		ret = GMACSL_RET_WARN_MAXLEN_TOO_BIG;
	}

	/* Must wait if the device is undergoing reset */
	for (i = 0; i < DEVICE_EMACSL_RESET_POLL_COUNT; i++) {
		v = DEVICE_REG32_R(DEVICE_EMACSL_BASE(port) +
				   CPGMACSL_REG_RESET);
		if ((v & CPGMAC_REG_RESET_VAL_RESET_MASK) !=
		    CPGMAC_REG_RESET_VAL_RESET)
			break;
	}

	if (i == DEVICE_EMACSL_RESET_POLL_COUNT)
		return (GMACSL_RET_CONFIG_FAIL_RESET_ACTIVE);

	DEVICE_REG32_W(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_MAXLEN,
			cfg->max_rx_len);

	DEVICE_REG32_W(DEVICE_EMACSL_BASE(port) + CPGMACSL_REG_CTL,
			cfg->ctl);

	return (ret);
}

int ethss_config(u32 ctl, u32 max_pkt_size)
{
	u32 i;

	/* Max length register */
	DEVICE_REG32_W(DEVICE_CPSW_BASE + CPSW_REG_MAXLEN, max_pkt_size);

	/* Control register */
	DEVICE_REG32_W(DEVICE_CPSW_BASE + CPSW_REG_CTL, ctl);

	/* All statistics enabled by default */
	DEVICE_REG32_W(DEVICE_CPSW_BASE + CPSW_REG_STAT_PORT_EN,
		       CPSW_REG_VAL_STAT_ENABLE_ALL);

	/* Reset and enable the ALE */
	DEVICE_REG32_W(DEVICE_CPSW_BASE + CPSW_REG_ALE_CONTROL,
		       CPSW_REG_VAL_ALE_CTL_RESET_AND_ENABLE |
		       CPSW_REG_VAL_ALE_CTL_BYPASS);

	/* All ports put into forward mode */
	for (i = 0; i < DEVICE_CPSW_NUM_PORTS; i++)
		DEVICE_REG32_W(DEVICE_CPSW_BASE + CPSW_REG_ALE_PORTCTL(i),
			       CPSW_REG_VAL_PORTCTL_FORWARD_MODE);

	return (0);
}

int ethss_start(void)
{
	int i;
	struct mac_sl_cfg cfg;

	cfg.max_rx_len	= MAX_SIZE_STREAM_BUFFER;
	cfg.ctl		= GMACSL_ENABLE | GMACSL_RX_ENABLE_EXT_CTL;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++) {
		mac_sl_reset(i);
		mac_sl_config(i, &cfg);
	}

	return (0);
}

int ethss_stop(void)
{
	int i;

	for (i = 0; i < DEVICE_N_GMACSL_PORTS; i++)
		mac_sl_reset(i);

	return (0);
}

int32_t cpmac_drv_send(u32* buffer, int num_bytes, int slave_port_num)
{
	if (num_bytes < EMAC_MIN_ETHERNET_PKT_SIZE)
		num_bytes = EMAC_MIN_ETHERNET_PKT_SIZE;

	return netcp_send(buffer, num_bytes,
			  (slave_port_num) << 16);
}

/* Eth device open */
static int keystone2_eth_open(struct eth_device *dev, bd_t *bis)
{
	u_int32_t clkdiv;
	int link;

	eth_priv_t *eth_priv = (eth_priv_t*)dev->priv;

	debug_emac("+ emac_open\n");

	net_rx_buffs.rx_flow	= eth_priv->rx_flow;

	psc_enable_module(KS2_LPSC_PA);
	psc_enable_module(KS2_LPSC_CPGMAC);

	sgmii_serdes_setup_156p25mhz();

	if (sys_has_mdio)
		keystone2_eth_mdio_enable();

	keystone_sgmii_config(eth_priv->slave_port - 1,
			      eth_priv->sgmii_link_type);

	udelay(10000);

	/* On chip switch configuration */
	ethss_config (targetGetSwitchCtl(), targetGetSwitchMaxPktSize());

	/* TODO: add error handling code */
	if (qm_init()) {
		printf("ERROR: qm_init()\n");
		return (-1);
	}
	if (netcp_init(&net_rx_buffs)) {
		qm_close();
		printf("ERROR: netcp_init()\n");
		return (-1);
	}

	/*
	 * Streaming switch configuration. If not present this
	 * statement is defined to void in target.h.
	 * If present this is usually defined to a series of register writes
	 */
	hwConfigStreamingSwitch();

	if (sys_has_mdio) {
		/* Init MDIO & get link state */
		clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
		writel((clkdiv & 0xff) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT,
		&adap_mdio->CONTROL);

		/* We need to wait for MDIO to start */
		udelay(1000);
		if (!loopback_test) {
			link = keystone_get_link_status(dev);
			if (link == 0) {
				netcp_close();
				qm_close();
				return -1;
			}
		}
	}

	if (!loopback_test) {
		emac_gigabit_enable(dev);
	}

	ethss_start();

	debug_emac("- emac_open\n");

	emac_open = 1;

	return(0);
}

/* Eth device close */
void keystone2_eth_close(struct eth_device *dev)
{
	debug_emac("+ emac_close\n");

	if (!emac_open)
		return;

	ethss_stop();

	netcp_close();
	qm_close();

#if 0
 	sgmii_serdes_shutdown();
	psc_disable_module(KS2_LPSC_CPGMAC);
	psc_disable_module(KS2_LPSC_PA);
	psc_disable_domain(2);
#endif
	emac_open = 0;

	debug_emac("- emac_close\n");
}

void k2hk_eth_open_close(struct eth_device *dev)
{
	keystone2_eth_open(dev, NULL);
	keystone2_eth_close(dev);
}

static int tx_send_loop = 0;

/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int keystone2_eth_send_packet(struct eth_device *dev,
				     volatile void *packet, int length)
{
	int ret_status = -1;
	eth_priv_t *eth_priv = (eth_priv_t*)dev->priv;

	tx_send_loop = 0;

	if (!loopback_test) {
		if (keystone_get_link_status(dev) == 0)
			return -1;

		emac_gigabit_enable(dev);
	}

	if (cpmac_drv_send ((u32*) packet, length, eth_priv->slave_port) != 0)
		return ret_status;

	if (!loopback_test) {
		if (keystone_get_link_status(dev) == 0)
			return -1;

		emac_gigabit_enable(dev);
	}

	return (length);
}

/*
 * This function handles receipt of a packet from the network
 */
static int keystone2_eth_rcv_packet(struct eth_device *dev)
{
	void *hd;
	u32  pkt_size;
	u32  *pkt;

	hd = netcp_recv(&pkt, &pkt_size);
	if (hd == NULL)
		return (0);

	NetReceive ((volatile uchar *)pkt, pkt_size);

	netcp_release_rxhd(hd);

	return (pkt_size);
}

inline void keystone2_emac_set_has_mdio(int has_mdio)
{
	sys_has_mdio = has_mdio;
}

inline int keystone2_emag_get_has_mdio(void)
{
	return sys_has_mdio;
}

inline void keystone2_emac_set_loopback_test(int val)
{
	loopback_test = val;
}
/*
 * This function initializes the EMAC hardware. It does NOT initialize
 * EMAC modules power or pin multiplexors, that is done by board_init()
 * much earlier in bootup process. Returns 1 on success, 0 otherwise.
 */
int keystone2_emac_initialize(eth_priv_t *eth_priv)
{
	struct eth_device *dev;
	static int phy_registered = 0;

	dev = malloc(sizeof *dev);
	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof *dev);

	strcpy(dev->name, eth_priv->int_name);
	dev->priv = eth_priv;

	keystone2_eth_read_mac_addr(dev);

	dev->iobase		= 0;
	dev->init		= keystone2_eth_open;
	dev->halt		= keystone2_eth_close;
	dev->send		= keystone2_eth_send_packet;
	dev->recv		= keystone2_eth_rcv_packet;
	eth_priv->dev		= dev;
	eth_register(dev);

	if (sys_has_mdio && !phy_registered) {
		phy_registered = 1;
#ifdef CONFIG_ETH_PHY_MARVEL_88E1111
		sprintf(phy.name, "88E1111");
		phy.init = marvell_88e1111_init_phy;
		phy.is_phy_connected = marvell_88e1111_is_phy_connected;
		phy.get_link_speed = marvell_88e1111_get_link_speed;
		phy.auto_negotiate = marvell_88e1111_auto_negotiate;
#else
		sprintf(phy.name, "GENERIC");
		phy.init = gen_init_phy;
		phy.is_phy_connected = gen_is_phy_connected;
		phy.get_link_speed = gen_get_link_speed;
		phy.auto_negotiate = gen_auto_negotiate;
#endif
		miiphy_register(phy.name, keystone2_mii_phy_read,
				keystone2_mii_phy_write);
	}

	return(0);
}

#define reg_rmw(addr, value, mask) \
	writel(((readl(addr) & (~(mask))) | (value) ), (addr))

void sgmii_serdes_setup_156p25mhz()
{
	unsigned int cnt;

	reg_rmw(0x0232a000, 0x00800000, 0xffff0000);
	reg_rmw(0x0232a014, 0x00008282, 0x0000ffff);
	reg_rmw(0x0232a060, 0x00142438, 0x00ffffff);
	reg_rmw(0x0232a064, 0x00c3c700, 0x00ffff00);
	reg_rmw(0x0232a078, 0x0000c000, 0x0000ff00);

	reg_rmw(0x0232a204, 0x38000080, 0xff0000ff);
	reg_rmw(0x0232a208, 0x00000000, 0x000000ff);
	reg_rmw(0x0232a20c, 0x02000000, 0xff000000);
	reg_rmw(0x0232a210, 0x1b000000, 0xff000000);
	reg_rmw(0x0232a214, 0x00006fb8, 0x0000ffff);
	reg_rmw(0x0232a218, 0x758000e4, 0xffff00ff);
	reg_rmw(0x0232a2ac, 0x00004400, 0x0000ff00);
	reg_rmw(0x0232a22c, 0x00200800, 0x00ffff00);
	reg_rmw(0x0232a280, 0x00820082, 0x00ff00ff);
	reg_rmw(0x0232a284, 0x1d0f0385, 0xffffffff);

	reg_rmw(0x0232a404, 0x38000080, 0xff0000ff);
	reg_rmw(0x0232a408, 0x00000000, 0x000000ff);
	reg_rmw(0x0232a40c, 0x02000000, 0xff000000);
	reg_rmw(0x0232a410, 0x1b000000, 0xff000000);
	reg_rmw(0x0232a414, 0x00006fb8, 0x0000ffff);
	reg_rmw(0x0232a418, 0x758000e4, 0xffff00ff);
	reg_rmw(0x0232a4ac, 0x00004400, 0x0000ff00);
	reg_rmw(0x0232a42c, 0x00200800, 0x00ffff00);
	reg_rmw(0x0232a480, 0x00820082, 0x00ff00ff);
	reg_rmw(0x0232a484, 0x1d0f0385, 0xffffffff);

	reg_rmw(0x0232a604, 0x38000080, 0xff0000ff);
	reg_rmw(0x0232a608, 0x00000000, 0x000000ff);
	reg_rmw(0x0232a60c, 0x02000000, 0xff000000);
	reg_rmw(0x0232a610, 0x1b000000, 0xff000000);
	reg_rmw(0x0232a614, 0x00006fb8, 0x0000ffff);
	reg_rmw(0x0232a618, 0x758000e4, 0xffff00ff);
	reg_rmw(0x0232a6ac, 0x00004400, 0x0000ff00);
	reg_rmw(0x0232a62c, 0x00200800, 0x00ffff00);
	reg_rmw(0x0232a680, 0x00820082, 0x00ff00ff);
	reg_rmw(0x0232a684, 0x1d0f0385, 0xffffffff);

	reg_rmw(0x0232a804, 0x38000080, 0xff0000ff);
	reg_rmw(0x0232a808, 0x00000000, 0x000000ff);
	reg_rmw(0x0232a80c, 0x02000000, 0xff000000);
	reg_rmw(0x0232a810, 0x1b000000, 0xff000000);
	reg_rmw(0x0232a814, 0x00006fb8, 0x0000ffff);
	reg_rmw(0x0232a818, 0x758000e4, 0xffff00ff);
	reg_rmw(0x0232a8ac, 0x00004400, 0x0000ff00);
	reg_rmw(0x0232a82c, 0x00200800, 0x00ffff00);
	reg_rmw(0x0232a880, 0x00820082, 0x00ff00ff);
	reg_rmw(0x0232a884, 0x1d0f0385, 0xffffffff);

	reg_rmw(0x0232aa00, 0x00000800, 0x0000ff00);
	reg_rmw(0x0232aa08, 0x38a20000, 0xffff0000);
	reg_rmw(0x0232aa30, 0x008a8a00, 0x00ffff00);
	reg_rmw(0x0232aa84, 0x00000600, 0x0000ff00);
	reg_rmw(0x0232aa94, 0x10000000, 0xff000000);
	reg_rmw(0x0232aaa0, 0x81000000, 0xff000000);
	reg_rmw(0x0232aabc, 0xff000000, 0xff000000);
	reg_rmw(0x0232aac0, 0x0000008b, 0x000000ff);
	reg_rmw(0x0232ab08, 0x583f0000, 0xffff0000);
	reg_rmw(0x0232ab0c, 0x0000004e, 0x000000ff);
	reg_rmw(0x0232a000, 0x00000003, 0x000000ff);
	reg_rmw(0x0232aa00, 0x0000005f, 0x000000ff);

	reg_rmw(0x0232aa48, 0x00fd8c00, 0x00ffff00);
	reg_rmw(0x0232aa54, 0x002fec72, 0x00ffffff);
	reg_rmw(0x0232aa58, 0x00f92100, 0xffffff00);
	reg_rmw(0x0232aa5c, 0x00040060, 0xffffffff);
	reg_rmw(0x0232aa60, 0x00008000, 0xffffffff);
	reg_rmw(0x0232aa64, 0x0c581220, 0xffffffff);
	reg_rmw(0x0232aa68, 0xe13b0602, 0xffffffff);
	reg_rmw(0x0232aa6c, 0xb8074cc1, 0xffffffff);
	reg_rmw(0x0232aa70, 0x3f02e989, 0xffffffff);
	reg_rmw(0x0232aa74, 0x00000001, 0x000000ff);
	reg_rmw(0x0232ab20, 0x00370000, 0x00ff0000);
	reg_rmw(0x0232ab1c, 0x37000000, 0xff000000);
	reg_rmw(0x0232ab20, 0x0000005d, 0x000000ff);

	/*Bring SerDes out of Reset if SerDes is Shutdown & is in Reset Mode*/
	reg_rmw(0x0232a010, 0x00000000, 1 << 28);

	/* Enable TX and RX via the LANExCTL_STS 0x0000 + x*4 */
	reg_rmw(0x0232a228, 0x00000000, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe0);
	reg_rmw(0x0232a428, 0x00000000, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe4);
	reg_rmw(0x0232a628, 0x00000000, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe8);
	reg_rmw(0x0232a828, 0x00000000, 1 << 29);
	writel(0xF800F8C0, 0x0232bfec);

	/*Enable pll via the pll_ctrl 0x0014*/
	writel(0xe0000000, 0x0232bff4);

	/*Waiting for SGMII Serdes PLL lock.*/
	for (cnt = 10000;
	     cnt > 0 && ((readl(0x02090114) & 0x10) == 0);
	     cnt--);

	for (cnt = 10000;
	     cnt > 0 && ((readl(0x02090214) & 0x10) == 0);
	     cnt--);

	for (cnt = 10000;
	     cnt > 0 && ((readl(0x02090414) & 0x10) == 0);
	     cnt--);

	for (cnt = 10000;
	     cnt > 0 && ((readl(0x02090514) & 0x10) == 0);
	     cnt--);

	chip_delay(45000);
}

void sgmii_serdes_shutdown()
{

	reg_rmw(0x0232bfe0, 0, 3 << 29 | 3 << 13);
	reg_rmw(0x02320228, 1 << 29, 1 << 29);
	reg_rmw(0x0232bfe4, 0, 3 << 29 | 3 << 13);
	reg_rmw(0x02320428, 1 << 29, 1 << 29);
	reg_rmw(0x0232bfe8, 0, 3 << 29 | 3 << 13);
	reg_rmw(0x02320628, 1 << 29, 1 << 29);
	reg_rmw(0x0232bfec, 0, 3 << 29 | 3 << 13);
	reg_rmw(0x02320828, 1 << 29, 1 << 29);

	reg_rmw(0x02320034, 0, 3 << 29);
	reg_rmw(0x02320010, 1 << 28, 1 << 28);
}

