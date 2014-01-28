/*
 * Copyright (C) 2010, 2011, 2012, 2013, 2014 Texas Instruments Incorporated
 * Authors: Aurelien Jacquiot <a-jacquiot@ti.com>
 * - Main driver implementation.
 * - Updated for support on TI KeyStone 2 platform.
 *
 * Copyright (C) 2012 Texas Instruments Incorporated
 * WingMan Kwok <w-kwok2@ti.com>
 * - Updated for support on TI KeyStone 1 platform.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/dma-mapping.h>
#include <asm/io.h>
#include <asm/arch/psc_defs.h>
#include "keystone_rio.h"

#define DRIVER_VER	    "v1.0"

#ifdef CONFIG_SOC_TCI6614
/* K1 devices */
#define KEYSTONE_RIO_LPSC TCI6614_LPSC_SRIO
#define KEYSTONE_RIO_IS_K2  0
#else /* CONFIG_SOC_TCI6614 */
/* K2 devices*/
#define KEYSTONE_RIO_LPSC TCI6638_LPSC_SRIO
#define KEYSTONE_RIO_IS_K2  1
#endif  /* CONFIG_SOC_TCI6614 */

#define K2_SERDES(p)        ((p)->board_rio_cfg.keystone2_serdes)

static unsigned int rio_dbg = 0;
#define debug_rio(fmt, args...)	if (rio_dbg) printf(fmt, ##args)

/*
 * Main KeyStone RapidIO driver data
 */
struct keystone_rio_data {
	int			riohdid;
	u32			rio_pe_feat;

	u32			ports_registering;
	u32			port_chk_cnt;

	volatile u32		*jtagid_reg;
	volatile u32		*serdes_sts_reg;
	volatile struct keystone_srio_serdes_regs     *serdes_regs;
	volatile struct keystone_rio_regs	      *regs;

	volatile struct keystone_rio_car_csr_regs     *car_csr_regs;
	volatile struct keystone_rio_serial_port_regs *serial_port_regs;
	volatile struct keystone_rio_err_mgmt_regs    *err_mgmt_regs;
	volatile struct keystone_rio_phy_layer_regs   *phy_regs;
	volatile struct keystone_rio_transport_layer_regs *transport_regs;
	volatile struct keystone_rio_pkt_buf_regs     *pkt_buf_regs;
	volatile struct keystone_rio_evt_mgmt_regs    *evt_mgmt_regs;
	volatile struct keystone_rio_port_write_regs  *port_write_regs;
	volatile struct keystone_rio_link_layer_regs  *link_regs;
	volatile struct keystone_rio_fabric_regs      *fabric_regs;
	u32					      car_csr_regs_base;

	struct keystone_rio_board_controller_info     board_rio_cfg;
} __krio_priv;

static u32 __ffs(u32 mask)
{
	u32 i = 0;

	while (i < 32) {
		if (mask & (0x1 << i))
			return i;
		else
			++i;
	}
	return 32;
}

#define reg_rmw(addr, value, mask) \
	writel(((readl(addr) & (~(mask))) | (value)), (addr))

/*--------------------- Maintenance Request Management  ---------------------*/

static u32 keystone_rio_dio_get_lsu_cc(u32 lsu_id, u8 ltid, u8 *lcb,
				       struct keystone_rio_data *krio_priv)
{
	u32 idx;
	u32 shift;
	u32 value;
	u32 cc;
	/* lSU shadow register status mapping */
	u32 lsu_index[8] = { 0, 9, 15, 20, 24, 33, 39, 44 };

	/* Compute LSU stat index from LSU id and LTID */
	idx   = (lsu_index[lsu_id] + ltid) >> 3;
	shift = ((lsu_index[lsu_id] + ltid) & 0x7) << 2;

	/* Get completion code and context */
	value  = readl(&(krio_priv->regs->lsu_stat_reg[idx]));
	cc     = (value >> (shift + 1)) & 0x7;
	*lcb   = (value >> shift) & 0x1;

	return cc;
}

/**
 * maint_request - Perform a maintenance request
 * @port_id: output port ID of transaction
 * @dest_id: destination ID of target device
 * @hopcount: hopcount for this request
 * @offset: offset in the RapidIO configuration space
 * @buff: dma address of the data on the host
 * @buff_len: length of the data
 * @size: 1 for 16bit, 0 for 8bit ID size
 * @type: packet type
 *
 * Returns %0 on success or %-1 on failure.
 */
static inline int keystone_rio_maint_request(int port_id, 
					     u32 dest_id, 
					     u8  hopcount,
					     u32 offset, 
					     dma_addr_t buff,
					     int buff_len,
					     u16 size,
					     u16 type, 
					     struct keystone_rio_data *krio_priv)
{
	unsigned int count;
	unsigned int status = 0;
	unsigned int res    = 0;
	u8           context;
	u8           ltid;

	/* Check is there is space in the LSU shadow reg and that it is free */
	count = 0;
	while (1) {
		status = readl(&(krio_priv->regs->lsu_reg[0].busy_full));
		if (((status & KEYSTONE_RIO_LSU_FULL_MASK) == 0x0)
		    && ((status & KEYSTONE_RIO_LSU_BUSY_MASK) == 0x0))
			break;
		count++;

		if (count >= KEYSTONE_RIO_TIMEOUT_CNT) {
			debug_rio("RIO: no LSU available, status = 0x%x\n", status);
			res = -1;
			goto out;
		}
		udelay(1);
	}

	/* Get LCB and LTID, LSU reg 6 is already read */
	context = (status >> 4) & 0x1;
	ltid    = status & 0xf;

	/* LSU Reg 0 - MSB of RapidIO address */
	writel(0, &(krio_priv->regs->lsu_reg[0].addr_msb));

	/* LSU Reg 1 - LSB of destination */
	writel(offset, &(krio_priv->regs->lsu_reg[0].addr_lsb_cfg_ofs));

	/* LSU Reg 2 - source address */
	writel(buff, &(krio_priv->regs->lsu_reg[0].dsp_addr));

	/* LSU Reg 3 - byte count */
	writel(buff_len,
	       &(krio_priv->regs->lsu_reg[0].dbell_val_byte_cnt));

	/* LSU Reg 4 - */
	writel(((port_id << 8)
		| (KEYSTONE_RIO_LSU_PRIO << 4)
		| (size ? (1 << 10) : 0)
		| ((u32) dest_id << 16)),
	       &(krio_priv->regs->lsu_reg[0].destid));
	
	/* LSU Reg 5 */
	writel(((hopcount & 0xff) << 8) | (type & 0xff),
	       &(krio_priv->regs->lsu_reg[0].dbell_info_fttype));
	
	/* Retrieve our completion code */
	count = 0;
	res   = 0;
	while (1) {
		u8 lcb;
		status = keystone_rio_dio_get_lsu_cc(0, ltid, &lcb, krio_priv);
		if (lcb == context)
			break;
		count++;
		if (count >= KEYSTONE_RIO_TIMEOUT_CNT) {
			debug_rio("RIO: timeout %d, ltid = %d, context = %d, "
				  "lcb = %d, cc = %d\n",
				  count, ltid, context, lcb, status);
			res = -2;
			break;
		}
		udelay(1);
	}
out:
	if (res)
		return res;

	if (status)
		debug_rio("RIO: transfer error = 0x%x\n", status);

	switch (status) {
	case KEYSTONE_RIO_LSU_CC_TIMEOUT:
	case KEYSTONE_RIO_LSU_CC_XOFF:
	case KEYSTONE_RIO_LSU_CC_ERROR:
	case KEYSTONE_RIO_LSU_CC_INVALID:
	case KEYSTONE_RIO_LSU_CC_DMA:
		return -3;
		break;
	case KEYSTONE_RIO_LSU_CC_RETRY:
		return -4;
		break;
	case KEYSTONE_RIO_LSU_CC_CANCELED:
		return -5;
		break;
	default:
		break;
	}
	return 0;
}

/**
 * keystone_rio_config_read - Generate a KeyStone read maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Location to be read into
 *
 * Generates a KeyStone read maintenance transaction. Returns %0 on
 * success or %-1 on failure.
 */
int keystone_rio_config_read(int  portid, 
			     u16  destid,
			     u8   hopcount,
			     u32  offset,
			     int  len,
			     u32 *val)
{
	struct keystone_rio_data *krio_priv = &__krio_priv;
	u32 *tbuf;
	int res;
	dma_addr_t dma;

	tbuf = malloc(len);
	if (!tbuf)
		return -1;

	memset(tbuf, 0, len);

	dma = dma_map_single(tbuf, len, DMA_FROM_DEVICE);

	res = keystone_rio_maint_request(portid, destid, hopcount, offset, dma, len,
					 krio_priv->board_rio_cfg.size,
					 KEYSTONE_RIO_PACKET_TYPE_MAINT_R,
					 krio_priv);

	dma_unmap_single((volatile void *)tbuf, len, dma);

	/* Taking care of byteswap */
	switch (len) {
	case 1:
		*val = *((u8 *) tbuf);
		break;
	case 2:
		*val = ntohs(*((u16 *) tbuf));
		break;
	default:
		*val = ntohl(*((u32 *) tbuf));
		break;
	}

	free(tbuf);

	debug_rio("RIO: %s portid %d destid %d hopcount %d offset 0x%x "
		  "len %d val 0x%x res %d\n",
		  __FUNCTION__, portid, destid, hopcount, offset, len, *val, res);

	return res;
}

/**
 * keystone_rio_config_write - Generate a KeyStone write maintenance transaction
 * @portid: Output port ID of transaction
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Value to be written
 *
 * Generates an KeyStone write maintenance transaction. Returns %0 on
 * success or %-1 on failure.
 */
int keystone_rio_config_write(int portid,
			      u16 destid,
			      u8  hopcount,
			      u32 offset,
			      int len,
			      u32 val)
{
	struct keystone_rio_data *krio_priv = &__krio_priv;
	u32 *tbuf;
	int res;
	dma_addr_t dma;

	tbuf = malloc(len);
	if (!tbuf)
		return -1;

	memset(tbuf, 0, len);

	/* Taking care of byteswap */
	switch (len) {
	case 1:
		*tbuf = ((u8) val);
		break;
	case 2:
		*tbuf = htons((u16) val);
		break;
	default:
		*tbuf = htonl((u32) val);
		break;
	}

	dma = dma_map_single(tbuf, len, DMA_TO_DEVICE);

	res = keystone_rio_maint_request(portid, destid, hopcount, offset, dma, len,
					 krio_priv->board_rio_cfg.size,
					 KEYSTONE_RIO_PACKET_TYPE_MAINT_W,
					 krio_priv);

	dma_unmap_single((volatile void *)tbuf, len, dma);

	debug_rio("RIO: %s portid %d destid %d hopcount %d offset 0x%x "
		  "len %d val 0x%x res %d\n",
		  __FUNCTION__, portid, destid, hopcount, offset, len, val, res);

	free(tbuf);

	return res;
}

/**
 * keystone_local_config_read - Generate a KeyStone local config space read
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be read into
 *
 * Generates a KeyStone local configuration space read. Returns %0 on
 * success or %-1 on failure.
 */
int keystone_local_config_read(u32 offset, int len, u32 *data)
{
	struct keystone_rio_data *krio_priv = &__krio_priv;

	*data = readl((void *)(krio_priv->car_csr_regs_base + offset));

	debug_rio("RIO: %s offset 0x%x data 0x%x\n",
		  __FUNCTION__, offset, *data);

	return 0;
}

/**
 * keystone_local_config_write - Generate a KeyStone local config space write
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be written
 *
 * Generates a KeyStone local configuration space write. Returns %0 on
 * success or %-1 on failure.
 */
int keystone_local_config_write(u32 offset, int len, u32 data)
{
	struct keystone_rio_data *krio_priv = &__krio_priv;

	debug_rio("RIO: %s offset 0x%x data 0x%x\n",
		  __FUNCTION__, offset, data);

	writel(data, (void *)(krio_priv->car_csr_regs_base + offset));

	return 0;
}

/*------------------------- RapidIO hw controller setup ---------------------*/

struct keystone_lane_config {
	int start; /* lane start number of the port */
	int end;   /* lane end number of the port */
};

static struct keystone_lane_config keystone_lane_configs[5][KEYSTONE_RIO_MAX_PORT] = {
	{ {0, 1}, {1, 2}, {2, 3}, {3, 4} },       /* path mode 0: 4 ports in 1x    */
	{ {0, 1}, {1, 2}, {2, 4}, {-1, -1} },     /* path mode 1: 3 ports in 1x/2x */
	{ {0, 2}, {2, 3}, {3, 4}, {-1, -1} },     /* path mode 2: 3 ports in 2x/1x */
	{ {0, 2}, {2, 4}, {-1, -1}, {-1, -1} },   /* path mode 3: 2 ports in 2x    */
	{ {0, 4}, {-1, -1}, {-1, -1}, {-1, -1} }, /* path mode 4: 1 ports in 4x    */
};

/* Retrieve the corresponding lanes bitmask from ports bitmask and path_mode */
static int keystone_rio_get_lane_config(u32 ports, u32 path_mode)
{
	u32 lanes = 0;

	while (ports) {
		u32 lane;
		u32 port = __ffs(ports);
		ports &= ~(1 << port);

		if (keystone_lane_configs[path_mode][port].start == -1)
			return -1;

		for (lane = keystone_lane_configs[path_mode][port].start;
		     lane < keystone_lane_configs[path_mode][port].end;
		     lane ++) {
			lanes |= (1 << lane);
		}
	}
	return (int) lanes;
}

static void k2_rio_serdes_init_3g(u32 lanes, struct keystone_rio_data *krio_priv)
{
	void __iomem *regs = (void __iomem *) krio_priv->serdes_regs;

	/* Uses Half Rate configuration */
	reg_rmw(regs + 0x000, 0x00000000, 0xff000000);
	reg_rmw(regs + 0x014, 0x00008282, 0x0000ffff);
	reg_rmw(regs + 0x060, 0x00132c48, 0x00ffffff);
	reg_rmw(regs + 0x064, 0x00c3c700, 0x00ffff00);
	reg_rmw(regs + 0x078, 0x0000c000, 0x0000ff00);

	if (IS_SERDES_LANE_USED(lanes, 0)) {
		debug_rio("setting lane 0 SerDes to 3GHz\n");
		reg_rmw(regs + 0x204, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x208, 0x00000024, 0x000000ff);
		reg_rmw(regs + 0x20c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x210, 0x1b000000, 0xff000000);
		reg_rmw(regs + 0x214, 0x00006e7c, 0x0000ffff);
		reg_rmw(regs + 0x218, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x22c, 0x00100800, 0x00ffff00);
		reg_rmw(regs + 0x280, 0x00700070, 0x00ff00ff);
		reg_rmw(regs + 0x284, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x28c, 0x00003b00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 1)) {
		debug_rio("setting lane 1 SerDes to 3GHz\n");
		reg_rmw(regs + 0x404, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x408, 0x00000024, 0x000000ff);
		reg_rmw(regs + 0x40c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x410, 0x1b000000, 0xff000000);
		reg_rmw(regs + 0x414, 0x00006e7c, 0x0000ffff);
		reg_rmw(regs + 0x418, 0x758000e4, 0xffff0000);
		reg_rmw(regs + 0x42c, 0x00100800, 0x00ffff00);
		reg_rmw(regs + 0x480, 0x00700070, 0x00ff00ff);
		reg_rmw(regs + 0x484, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x48c, 0x00003b00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 2)) {
		debug_rio("setting lane 2 SerDes to 3GHz\n");
		reg_rmw(regs + 0x604, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x608, 0x00000024, 0x000000ff);
		reg_rmw(regs + 0x60c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x610, 0x1b000000, 0xff000000);
		reg_rmw(regs + 0x614, 0x00006e7c, 0x0000ffff);
		reg_rmw(regs + 0x618, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x62c, 0x00100800, 0x00ffff00);
		reg_rmw(regs + 0x680, 0x00700070, 0x00ff00ff);
		reg_rmw(regs + 0x684, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x68c, 0x00003b00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 3)) {
		debug_rio("setting lane 3 SerDes to 3GHz\n");
		reg_rmw(regs + 0x804, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x808, 0x00000024, 0x000000ff);
		reg_rmw(regs + 0x80c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x810, 0x1b000000, 0xff000000);
		reg_rmw(regs + 0x814, 0x00006e7c, 0x0000ffff);
		reg_rmw(regs + 0x818, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x82c, 0x00100800, 0x00ffff00);
		reg_rmw(regs + 0x880, 0x00700070, 0x00ff00ff);
		reg_rmw(regs + 0x884, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x88c, 0x00003b00, 0x0000ff00);
	}

	reg_rmw(regs + 0xa00, 0x00000800, 0x0000ff00);
	reg_rmw(regs + 0xa08, 0x37720000, 0xffff0000);
	reg_rmw(regs + 0xa30, 0x00777700, 0x00ffff00);
	reg_rmw(regs + 0xa84, 0x00000600, 0x0000ff00);
	reg_rmw(regs + 0xa94, 0x10000000, 0xff000000);
	reg_rmw(regs + 0xaa0, 0x81000000, 0xff000000);
	reg_rmw(regs + 0xabc, 0xff000000, 0xff000000);
	reg_rmw(regs + 0xac0, 0x0000008b, 0x000000ff);
	reg_rmw(regs + 0x000, 0x00000003, 0x000000ff);
	reg_rmw(regs + 0xa00, 0x0000005f, 0x000000ff);
}

static void k2_rio_serdes_init_5g(u32 lanes, struct keystone_rio_data *krio_priv)
{
	void __iomem *regs = (void __iomem *) krio_priv->serdes_regs;

	/* Uses Full Rate configuration by default */
	reg_rmw(regs + 0x000, 0x00000000, 0xff000000);
	reg_rmw(regs + 0x014, 0x00008282, 0x0000ffff);
	reg_rmw(regs + 0x060, 0x00142438, 0x00ffffff);
	reg_rmw(regs + 0x064, 0x00c3c700, 0x00ffff00);
	reg_rmw(regs + 0x078, 0x0000c000, 0x0000ff00);

	if (IS_SERDES_LANE_USED(lanes, 0)) {
		debug_rio("setting lane 0 SerDes to 5GHz\n");
		reg_rmw(regs + 0x204, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x208, 0x00000026, 0x000000ff);
		reg_rmw(regs + 0x20c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x214, 0x00006f38, 0x0000ffff);
		reg_rmw(regs + 0x218, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x22c, 0x00200800, 0x00ffff00);
		reg_rmw(regs + 0x280, 0x00860086, 0x00ff00ff);
		reg_rmw(regs + 0x284, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x28c, 0x00002c00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 1)) {
		debug_rio("setting lane 1 SerDes to 5GHz\n");
		reg_rmw(regs + 0x404, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x408, 0x00000026, 0x000000ff);
		reg_rmw(regs + 0x40c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x414, 0x00006f38, 0x0000ffff);
		reg_rmw(regs + 0x418, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x42c, 0x00200800, 0x00ffff00);
		reg_rmw(regs + 0x480, 0x00860086, 0x00ff00ff);
		reg_rmw(regs + 0x484, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x48c, 0x00002c00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 2)) {
		debug_rio("setting lane 2 SerDes to 5GHz\n");
		reg_rmw(regs + 0x604, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x608, 0x00000026, 0x000000ff);
		reg_rmw(regs + 0x60c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x614, 0x00006f38, 0x0000ffff);
		reg_rmw(regs + 0x618, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x62c, 0x00200800, 0x00ffff00);
		reg_rmw(regs + 0x680, 0x00860086, 0x00ff00ff);
		reg_rmw(regs + 0x684, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x68c, 0x00002c00, 0x0000ff00);
	}

	if (IS_SERDES_LANE_USED(lanes, 3)) {
		debug_rio("setting lane 3 SerDes to 5GHz\n");
		reg_rmw(regs + 0x804, 0x78000080, 0xff0000ff);
		reg_rmw(regs + 0x808, 0x00000026, 0x000000ff);
		reg_rmw(regs + 0x80c, 0x02000000, 0xff000000);
		reg_rmw(regs + 0x814, 0x00006f38, 0x0000ffff);
		reg_rmw(regs + 0x818, 0x758000e4, 0xffff00ff);
		reg_rmw(regs + 0x82c, 0x00200800, 0x00ffff00);
		reg_rmw(regs + 0x880, 0x00860086, 0x00ff00ff);
		reg_rmw(regs + 0x884, 0x1d0f0085, 0xffff00ff);
		reg_rmw(regs + 0x88c, 0x00002c00, 0x0000ff00);
	}

	reg_rmw(regs + 0xa00, 0x00008000, 0x0000ff00);
	reg_rmw(regs + 0xa08, 0x38d20000, 0xffff0000);
	reg_rmw(regs + 0xa30, 0x008d8d00, 0x00ffff00);
	reg_rmw(regs + 0xa84, 0x00000600, 0x0000ff00);
	reg_rmw(regs + 0xa94, 0x10000000, 0xff000000);
	reg_rmw(regs + 0xaa0, 0x81000000, 0xff000000);
	reg_rmw(regs + 0xabc, 0xff000000, 0xff000000);
	reg_rmw(regs + 0xac0, 0x0000008b, 0x000000ff);
	reg_rmw(regs + 0x000, 0x00000003, 0x000000ff);
	reg_rmw(regs + 0xa00, 0x0000005f, 0x000000ff);

	reg_rmw(regs + 0xa48, 0x00fd8c00, 0x00ffff00);
	reg_rmw(regs + 0xa54, 0x002fec72, 0x00ffffff);
	reg_rmw(regs + 0xa58, 0x00f92100, 0xffffff00);
	reg_rmw(regs + 0xa5c, 0x00040060, 0xffffffff);
	reg_rmw(regs + 0xa60, 0x00008000, 0xffffffff);
	reg_rmw(regs + 0xa64, 0x0c581220, 0xffffffff);
	reg_rmw(regs + 0xa68, 0xe13b0602, 0xffffffff);
	reg_rmw(regs + 0xa6c, 0xb8074cc1, 0xffffffff);
	reg_rmw(regs + 0xa70, 0x3f02e989, 0xffffffff);
	reg_rmw(regs + 0xa74, 0x00000001, 0x000000ff);
	reg_rmw(regs + 0xb20, 0x00370000, 0x00ff0000);
	reg_rmw(regs + 0xb1c, 0x37000000, 0xff000000);
	reg_rmw(regs + 0xb20, 0x0000005d, 0x000000ff);
}

static void k2_rio_serdes_lane_enable(u32 lane, u32 rate, 
				      struct keystone_rio_data *krio_priv)
{
	volatile void *regs = (volatile void *) krio_priv->serdes_regs;
	u32 val;

	/* Bit 28 Toggled. Bring it out of Reset TX PLL for all lanes */
	val = readl(regs + 0x200 * (lane + 1) + 0x28);
	val &= ~BIT(29);
	writel(val, regs + 0x200 * (lane + 1) + 0x28);

	/* Set Lane Control Rate */
	switch (rate) {
	case KEYSTONE_RIO_FULL_RATE:
		writel(0xF0C0F0F0, regs + 0x1fe0 + 4 * lane);
		break;
	case KEYSTONE_RIO_HALF_RATE:
		writel(0xF4C0F4F0, regs + 0x1fe0 + 4 * lane);
		break;
	case KEYSTONE_RIO_QUARTER_RATE:
		writel(0xF8C0F8F0, regs + 0x1fe0 + 4 * lane);
		break;
	default:
		return;
	}
}

static int k2_rio_serdes_config(u32 lanes, u32 baud, 
				struct keystone_rio_data *krio_priv)
{
	volatile void *regs = (volatile void *) krio_priv->serdes_regs;
	u32 rate;
	u32 val;

	/* Disable pll before configuring the SerDes registers */
	writel(0x00000000, regs + 0x1ff4);

	switch (baud) {
	case KEYSTONE_RIO_BAUD_1_250:
		rate = KEYSTONE_RIO_QUARTER_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_2_500:
		rate = KEYSTONE_RIO_HALF_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_5_000:
		rate = KEYSTONE_RIO_FULL_RATE;
		k2_rio_serdes_init_5g(lanes, krio_priv);
		break;
	case KEYSTONE_RIO_BAUD_3_125:
		rate = KEYSTONE_RIO_HALF_RATE;
		k2_rio_serdes_init_3g(lanes, krio_priv);
		break;
	default:
		printf("RIO: unsupported baud rate %d\n", baud);
		return -1;
	}

	/* Enable serdes for requested lanes */
	while(lanes) {
		u32 lane = __ffs(lanes);
		lanes &= ~(1 << lane);

		if (lane >= KEYSTONE_RIO_MAX_PORT)
			return -1;

		k2_rio_serdes_lane_enable(lane, rate, krio_priv);
	}

	/* Enable pll via the pll_ctrl 0x0014 */
	writel(0xe0000000, regs + 0x1ff4);

	/* Wait until CMU_OK bit is set */
	do {
		val = readl(regs + 0xbf8);
	} while (!(val & BIT(16)));

	return 0;
}

static int k2_rio_serdes_wait_lock(struct keystone_rio_data *krio_priv, u32 lanes)
{
	u32 loop;
	u32 val;
	u32 val_mask;
	volatile void *regs = (volatile void*) krio_priv->serdes_regs;

	val_mask = lanes | (lanes << 8);

	/* Wait for the SerDes PLL lock */
	for (loop = 0; loop < 100000; loop++) {
		/* read PLL_CTRL */
		val = readl(regs + 0x1ff4);
		if ((val & val_mask) == val_mask)
			break;
		udelay(10);
	}

	if (loop == 100000) {
		return -1;
	}

	return 0;
}

/**
 * keystone_rio_hw_init - Configure a RapidIO controller
 * @mode: serdes configuration
 * @hostid: device id of the host
 */
static void keystone_rio_hw_init(u32 mode, u32 baud, struct keystone_rio_data *krio_priv)
{
	u32 val;
	u32 block;
	struct keystone_serdes_config *serdes_config
		= &(krio_priv->board_rio_cfg.serdes_config[mode]);

	/* Set sRIO out of reset */
	writel(0x00000011, &krio_priv->regs->pcr);

	/* Clear BOOT_COMPLETE bit (allowing write) */
	writel(0x00000000, &krio_priv->regs->per_set_cntl);

	/* Enable blocks */
	writel(1, &krio_priv->regs->gbl_en);
	for (block = 0; block <= KEYSTONE_RIO_BLK_NUM; block++)
		writel(1, &(krio_priv->regs->blk[block].enable));

	/* Set control register 1 configuration */
	writel(0x00000000, &krio_priv->regs->per_set_cntl1);

	/* Set Control register */
	writel(serdes_config->cfg_cntl, &krio_priv->regs->per_set_cntl);

	if (K2_SERDES(krio_priv)) {
		u32 path_mode = krio_priv->board_rio_cfg.path_mode;
		u32 ports     = krio_priv->board_rio_cfg.ports;
		int res;

		/* K2 SerDes main configuration */
		res = keystone_rio_get_lane_config(ports, path_mode);
		if (res > 0) {
			u32 lanes = (u32) res;
			res = k2_rio_serdes_config(lanes, baud, krio_priv);
		}
	} else {
		u32 port;

		/* K1 SerDes main configuration */
		writel(serdes_config->serdes_cfg_pll,
		       &krio_priv->serdes_regs->pll);
		
		/* Per-port SerDes configuration */
		for (port = 0; port < KEYSTONE_RIO_MAX_PORT; port++) {
			writel(serdes_config->rx_chan_config[port],
			       &krio_priv->serdes_regs->channel[port].rx);
			writel(serdes_config->tx_chan_config[port],
			       &krio_priv->serdes_regs->channel[port].tx);
		}
		
		/* Check for RIO SerDes PLL lock */
		do {
			val = readl(krio_priv->serdes_sts_reg);
		} while ((val & 0x1) != 0x1);
	}

	/* Set prescalar for ip_clk */
	writel(serdes_config->prescalar_srv_clk,
	       &krio_priv->link_regs->prescalar_srv_clk);

	/* Peripheral-specific configuration and capabilities */
	writel(KEYSTONE_RIO_DEV_ID_VAL,
	       &krio_priv->car_csr_regs->dev_id);
	writel(KEYSTONE_RIO_DEV_INFO_VAL,
	       &krio_priv->car_csr_regs->dev_info);
	writel(KEYSTONE_RIO_ID_TI,
	       &krio_priv->car_csr_regs->assembly_id);
	writel(KEYSTONE_RIO_EXT_FEAT_PTR,
	       &krio_priv->car_csr_regs->assembly_info);

	/* Set host device id */
	writel((krio_priv->riohdid & 0xffff)
	       | ((krio_priv->riohdid & 0xff) << 16),
	       &krio_priv->car_csr_regs->base_dev_id);

	krio_priv->rio_pe_feat = RIO_PEF_PROCESSOR
		| RIO_PEF_CTLS
		| KEYSTONE_RIO_PEF_FLOW_CONTROL
		| RIO_PEF_EXT_FEATURES
		| RIO_PEF_ADDR_34
		| RIO_PEF_STD_RT
		| RIO_PEF_INB_DOORBELL
		| RIO_PEF_INB_MBOX;

	writel(krio_priv->rio_pe_feat,
	       &krio_priv->car_csr_regs->pe_feature);

	writel(KEYSTONE_RIO_MAX_PORT << 8,
	       &krio_priv->car_csr_regs->sw_port);

	writel((RIO_SRC_OPS_READ
		| RIO_SRC_OPS_WRITE
		| RIO_SRC_OPS_STREAM_WRITE
		| RIO_SRC_OPS_WRITE_RESPONSE
		| RIO_SRC_OPS_DATA_MSG
		| RIO_SRC_OPS_DOORBELL
		| RIO_SRC_OPS_ATOMIC_TST_SWP
		| RIO_SRC_OPS_ATOMIC_INC
		| RIO_SRC_OPS_ATOMIC_DEC
		| RIO_SRC_OPS_ATOMIC_SET
		| RIO_SRC_OPS_ATOMIC_CLR
		| RIO_SRC_OPS_PORT_WRITE),
	       &krio_priv->car_csr_regs->src_op);

	writel((RIO_DST_OPS_READ
		| RIO_DST_OPS_WRITE
		| RIO_DST_OPS_STREAM_WRITE
		| RIO_DST_OPS_WRITE_RESPONSE
		| RIO_DST_OPS_DATA_MSG
		| RIO_DST_OPS_DOORBELL
		| RIO_DST_OPS_PORT_WRITE),
	       &krio_priv->car_csr_regs->dest_op);

	writel(RIO_PELL_ADDR_34,
	       &krio_priv->car_csr_regs->pe_logical_ctl);

	val = (((KEYSTONE_RIO_SP_HDR_NEXT_BLK_PTR & 0xffff) << 16) |
	       KEYSTONE_RIO_SP_HDR_EP_REC_ID);
	writel(val, &krio_priv->serial_port_regs->sp_maint_blk_hdr);

	/* clear high bits of local config space base addr */
	writel(0x00000000, &krio_priv->car_csr_regs->local_cfg_hbar);

	/* set local config space base addr */
	writel(0x00520000, &krio_priv->car_csr_regs->local_cfg_bar);

	/* Enable HOST & MASTER_ENABLE bits */
	writel(0xe0000000, &krio_priv->serial_port_regs->sp_gen_ctl);

	/* set link timeout value */
	writel(0x000FFF00,
	       &krio_priv->serial_port_regs->sp_link_timeout_ctl);

	/* set response timeout value */
	writel(0x000FFF00,
	       &krio_priv->serial_port_regs->sp_rsp_timeout_ctl);

	/* allows SELF_RESET and PWDN_PORT resets to clear stcky reg bits */
	writel(0x00000001, &krio_priv->link_regs->reg_rst_ctl);

	/* Set error detection mode */
	/* clear all errors */
	writel(0x00000000, &krio_priv->err_mgmt_regs->err_det);

	/* enable all error detection */
	writel(0x00000000, &krio_priv->err_mgmt_regs->err_en);

	/* set err det block header */
	val = (((KEYSTONE_RIO_ERR_HDR_NEXT_BLK_PTR & 0xffff) << 16) |
	       KEYSTONE_RIO_ERR_EXT_FEAT_ID);
	writel(val, &krio_priv->err_mgmt_regs->err_report_blk_hdr);

	/* clear msb of err catptured addr reg */
	writel(0x00000000, &krio_priv->err_mgmt_regs->h_addr_capt);

	/* clear lsb of err catptured addr reg */
	writel(0x00000000, &krio_priv->err_mgmt_regs->addr_capt);

	/* clear err catptured source and dest devID reg */
	writel(0x00000000, &krio_priv->err_mgmt_regs->id_capt);

	/* clear err catptured packet info */
	writel(0x00000000, &krio_priv->err_mgmt_regs->ctrl_capt);

	/* Force all writes to finish */
	val = readl(&krio_priv->err_mgmt_regs->ctrl_capt);
}

/**
 * keystone_rio_start - Start RapidIO controller
 */
static void keystone_rio_start(struct keystone_rio_data *krio_priv)
{
	u32 val;

	/* set PEREN bit to enable logical layer data flow */
	val = (KEYSTONE_RIO_PER_EN | KEYSTONE_RIO_PER_FREE);
	writel(val, &krio_priv->regs->pcr);

	/* set BOOT_COMPLETE bit */
	val = readl(&krio_priv->regs->per_set_cntl);
	writel(val | KEYSTONE_RIO_BOOT_COMPLETE,
	       &krio_priv->regs->per_set_cntl);
}

static int
keystone_rio_test_link(u8 port, struct keystone_rio_data *krio_priv)
{
	int res;
	u32 val;

	/* perform a maintenance read */
	res = keystone_rio_config_read(port, 0xff, 0,0, 4, &val);

	return res;
}

/**
 * keystone_rio_port_status - Return if the port is OK or not
 * @port: index of the port
 *
 * Return %0 if the port is ready or %-EIO on failure.
 */
static int keystone_rio_port_status(int port,
				    struct keystone_rio_data *krio_priv)
{
	unsigned int count, value;
	int res = 0;

	count  = 0;

	if (port >= KEYSTONE_RIO_MAX_PORT)
		return -1;

	/* Check port status */
	while (1) {
		value = readl(&(krio_priv->serial_port_regs->sp[port].err_stat));
		if ((value & RIO_PORT_N_ERR_STS_PORT_OK) != 0) {
			res = keystone_rio_test_link(port, krio_priv);
			if (0 == res)
				return 0; /* port must be solid OK */
		} else {
			count++;
			if (count >= 10000)
				return -2;
		}
		udelay(10000);
	}
	return 0;
}

/**
 * keystone_rio_port_disable - Disable a RapidIO port
 * @port: index of the port to configure
 */
static void keystone_rio_port_disable(u32 port, struct keystone_rio_data *krio_priv)
{
	/* Disable port */
	writel(0x800000, &(krio_priv->serial_port_regs->sp[port].ctl));
}

/**
 * keystone_rio_port_init - Configure a RapidIO port
 * @port: index of the port to configure
 * @mode: serdes configuration
 */
static int keystone_rio_port_init(u32 port, u32 path_mode,
				  struct keystone_rio_data *krio_priv)
{
	if (port >= KEYSTONE_RIO_MAX_PORT)
		return -1;

	/* Disable packet forwarding */
	writel(0xffffffff, &(krio_priv->regs->pkt_fwd_cntl[port].pf_16b));
	writel(0x0003ffff, &(krio_priv->regs->pkt_fwd_cntl[port].pf_8b));

	/* Silence and discovery timers */
	writel(0x20000000,
	       &(krio_priv->phy_regs->phy_sp[port].silence_timer));
	writel(0x20000000,
	       &(krio_priv->phy_regs->phy_sp[port].discovery_timer));

	/* Enable port in input and output */
	writel(0x600000, &(krio_priv->serial_port_regs->sp[port].ctl));

	/* Program channel allocation to ports (1x, 2x or 4x) */
	writel(path_mode, &(krio_priv->phy_regs->phy_sp[port].path_ctl));

	return 0;
}

/**
 * keystone_rio_port_activate - Start using a RapidIO port
 * @port: index of the port to configure
 */
static int keystone_rio_port_activate(u32 port,
				      struct keystone_rio_data *krio_priv)
{
	u32 val;

	/* Cleanup port error status */
	writel(KEYSTONE_RIO_PORT_ERROR_MASK,
	       &(krio_priv->serial_port_regs->sp[port].err_stat));
	writel(0, &(krio_priv->err_mgmt_regs->sp_err[port].det));

	/* Enable promiscuous */
	writel(0x00309000,
	       &(krio_priv->transport_regs->transport_sp[port].control));

	return 0;
}

/*------------------------ Main driver functions -----------------------*/

static void keystone_rio_get_controller_defaults(struct keystone_rio_data *krio_priv,
						 int riosize,
						 u32 rioports,
						 int riopmode,
						 int riobaudrate)
{
	struct keystone_rio_board_controller_info *c = &krio_priv->board_rio_cfg;
	int i;
	
	c->keystone2_serdes = KEYSTONE_RIO_IS_K2;
	
	if (K2_SERDES(krio_priv)) {
		/* K2 configuration */
		c->rio_regs_base        = 0x2900000;
		c->rio_regs_size        = 0x40000;
		c->boot_cfg_regs_base   = 0x2620000;
		c->boot_cfg_regs_size   = 0x1000;
		c->serdes_cfg_regs_base = 0x232c000;
		c->serdes_cfg_regs_size = 0x1000;
	} else {
		/* K1 configuration */
		c->rio_regs_base        = 0x2900000;
		c->rio_regs_size        = 0x21000;
		c->boot_cfg_regs_base   = 0x2620000;
		c->boot_cfg_regs_size   = 0x3b0;
		c->serdes_cfg_regs_base = 0x2900360;
		c->serdes_cfg_regs_size = 0x1000;
	}
	
	/* dev-id-size */
	c->size = riosize;

	/* ports to use */
	c->ports = rioports;

	/* SerDes config */
	c->serdes_config_num = 1; /* total number of serdes_config[] entries */
	c->mode              = 0; /* default serdes_config[] entry to be used */
	c->path_mode         = riopmode;

	if (K2_SERDES(krio_priv)) {
		/*
		 * K2 sRIO config 0
		 */
		c->serdes_config[0].prescalar_srv_clk = 0x001f;
		c->serdes_baudrate = riobaudrate;
	} else {
		/* 
		 * K1 sRIO config 0: MPY = 5x, div rate = half,
		 * link rate = 3.125 Gbps, mode 1x
		 */

		/* setting control register config */
		c->serdes_config[0].cfg_cntl = 0x0c053860;

		/* SerDes PLL configuration */
		c->serdes_config[0].serdes_cfg_pll = 0x0229;

		/* prescalar_srv_clk */
		c->serdes_config[0].prescalar_srv_clk = 0x001e;

		/* serdes rx_chan_config */
		for (i = 0; i < KEYSTONE_RIO_MAX_PORT; i++)
			c->serdes_config[0].rx_chan_config[i] = 0x00440495;

		/* serdes tx_chan_config */
		for (i = 0; i < KEYSTONE_RIO_MAX_PORT; i++)
			c->serdes_config[0].tx_chan_config[i] = 0x00180795;
	}
}

/*
 * Platform configuration setup
 */
static int keystone_rio_setup_controller(struct keystone_rio_data *krio_priv)
{
	u32 ports;
	u32 p;
	u32 mode;
	u32 baud;
	u32 path_mode;
	u32 size = 0;
	int res = 0;
	char str[8];

	size      = krio_priv->board_rio_cfg.size;
	ports     = krio_priv->board_rio_cfg.ports;
	mode      = krio_priv->board_rio_cfg.mode;
	baud      = krio_priv->board_rio_cfg.serdes_baudrate;
	path_mode = krio_priv->board_rio_cfg.path_mode;

	debug_rio("RIO: size = %d, ports = 0x%x, mode = %d, baud = %d, path_mode = %d\n",
		  size, ports, mode, baud, path_mode);

	if (mode >= krio_priv->board_rio_cfg.serdes_config_num) {
		mode = 0;
		printf("RIO: invalid port mode, forcing it to %d\n", mode);
	}

	if (baud > KEYSTONE_RIO_BAUD_5_000) {
		baud = KEYSTONE_RIO_BAUD_5_000;
		printf("RIO: invalid baud rate, forcing it to 5Gbps\n");
	}
	
	switch (baud) {
	case KEYSTONE_RIO_BAUD_1_250:
		snprintf(str, sizeof(str), "1.25");
		break;
	case KEYSTONE_RIO_BAUD_2_500:
		snprintf(str, sizeof(str), "2.50");
		break;
	case KEYSTONE_RIO_BAUD_3_125:
		snprintf(str, sizeof(str), "3.125");
		break;
	case KEYSTONE_RIO_BAUD_5_000:
		snprintf(str, sizeof(str), "5.00");
		break;
	default:
		return -1;
	}

	printf("RIO: initializing %s Gbps interface with port configuration %d\n",
	       str, path_mode);

	/* Hardware set up of the controller */
	keystone_rio_hw_init(mode, baud, krio_priv);

	/* Disable all ports */
	for (p = 0; p < KEYSTONE_RIO_MAX_PORT; p++)
		keystone_rio_port_disable(p, krio_priv);

	/* Start the controller */
	keystone_rio_start(krio_priv);

	/* Try to lock K2 SerDes*/
	if (K2_SERDES(krio_priv)) {
		int lanes = keystone_rio_get_lane_config(ports, path_mode);
		if (lanes > 0) {
			res = k2_rio_serdes_wait_lock(krio_priv ,(u32) lanes);
			if (res < 0)
				debug_rio("SerDes for lane mask 0x%x on %s Gbps not locked\n",
					  lanes, str);
		}
	}
	
	/* Use and check ports status (but only the requested ones) */
	krio_priv->ports_registering = ports;
	while (ports) {
		int status;
		u32 port = __ffs(ports);
		if (port > 32)
			return 0;
		ports &= ~(1 << port);

		res = keystone_rio_port_init(port, path_mode, krio_priv);
		if (res < 0) {
			printf("RIO: initialization of port %d failed\n", p);
			return res;
		}

		/* Start the port */
		keystone_rio_port_activate(port, krio_priv);

		/*
		 * Check the port status here before calling the generic RapidIO
		 * layer. Port status check is done in rio_mport_is_active() as
		 * well but we need to do it our way first due to some delays in
		 * hw initialization.
		 */
		status = keystone_rio_port_status(port, krio_priv);
		if (status == 0) {
			krio_priv->ports_registering &= ~(1 << port);
			printf("RIO: port RIO%d ready\n", port);
		} else {
			printf("RIO: port %d not ready (status %d)\n", port, status);
		}
	}

	if (krio_priv->ports_registering != 0)
		return -1;

	return res;
}

void *keystone_rio_init(int riohdid,
			int riosize,
			u32 rioports,
			int riopmode,
			int riobaudrate)
{
	struct keystone_rio_data *krio_priv = &__krio_priv;
	int res = 0;
	volatile void *regs;

	keystone_rio_get_controller_defaults(krio_priv,
					     riosize,
					     rioports,
					     riopmode,
					     riobaudrate);

	regs = (volatile void *)krio_priv->board_rio_cfg.boot_cfg_regs_base;
	krio_priv->jtagid_reg     = regs + 0x0018;
	krio_priv->serdes_sts_reg = regs + 0x0154;

	regs = (volatile void *)krio_priv->board_rio_cfg.serdes_cfg_regs_base;
	krio_priv->serdes_regs = regs;

	regs = (volatile void *)krio_priv->board_rio_cfg.rio_regs_base;
	krio_priv->regs		     = regs;
	krio_priv->car_csr_regs	     = regs + 0x0b000;
	krio_priv->serial_port_regs  = regs + 0x0b100;
	krio_priv->err_mgmt_regs     = regs + 0x0c000;
	krio_priv->phy_regs	     = regs + 0x1b000;
	krio_priv->transport_regs    = regs + 0x1b300;
	krio_priv->pkt_buf_regs	     = regs + 0x1b600;
	krio_priv->evt_mgmt_regs     = regs + 0x1b900;
	krio_priv->port_write_regs   = regs + 0x1ba00;
	krio_priv->link_regs	     = regs + 0x1bd00;
	krio_priv->fabric_regs	     = regs + 0x1be00;
	krio_priv->car_csr_regs_base = (u32)regs + 0xb000;

	krio_priv->riohdid = riohdid;

	/* Enable srio clock */
	psc_enable_module(KEYSTONE_RIO_LPSC);

	printf("KeyStone RapidIO driver %s, hdid=%d\n", DRIVER_VER, riohdid);

	/* Setup the sRIO controller */
	res = keystone_rio_setup_controller(krio_priv);
	if (res < 0)
		return NULL;

	return (void *)krio_priv;
}

int keystone_rio_shutdown(void *handle)
{
	if (handle != &__krio_priv)
		return -1;

	/* power off */
	psc_disable_module(KEYSTONE_RIO_LPSC);

	return 0;
}
