/*
 * MCNAV driver for TI Keystone 2 devices.
 *
 * Copyright (C) 2013 - 2014 Texas Instruments Incorporated
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
#include <asm/io.h>
#include <asm/arch/keystone_nav.h>

extern int cpu_to_bus(u32 *ptr, u32 length);

struct qm_config ks2_qm_memmap = {
	.stat_cfg	= 0x02a40000,
	.queue		= (struct qm_reg_queue *)0x02a80000,
	.mngr_vbusm	= 0x23a80000,
	.i_lram		= 0x00100000,
	.proxy		= (struct qm_reg_queue *)0x02ac0000,
	.status_ram	= 0x02a06000,
	.mngr_cfg	= (struct qm_cfg_reg *)0x02a02000,
	.intd_cfg	= 0x02a0c000,
	.desc_mem	= (struct descr_mem_setup_reg *)0x02a03000,
	.region_num	= 64,
	.pdsp_cmd	= 0x02a20000,
	.pdsp_ctl	= 0x02a0f000,
	.pdsp_iram	= 0x02a10000,

	.i_lram_size	= 0x7fff,
	.start_queue	= 0,
	.num_queues	= 8192,
	.qpool_num	= 4000,
};

struct qm_config ks2_qm2_memmap = {
	.stat_cfg	= 0x02a60000,				/* peek */
	.queue		= (struct qm_reg_queue *)0x02aa0000,	/* push pop */
	.mngr_vbusm	= 0x23aa0000,
	.i_lram		= 0x00100000,
	.proxy		= (struct qm_reg_queue *)0x02ad0000,
	.status_ram	= 0x02a06400,				/* status */
	.mngr_cfg	= (struct qm_cfg_reg *)0x02a04000,	/* config */
	.intd_cfg	= 0x02a0d000,
	.desc_mem	= (struct descr_mem_setup_reg *)0x02a05000, /* region */
	.region_num	= 64,
	.pdsp_cmd	= 0x02a24000,
	.pdsp_ctl	= 0x02a0f100,
	.pdsp_iram	= 0x02a11000,

	.i_lram_size	= 0x7fff,
	.start_queue	= 8192,
	.num_queues	= 8192,
	.qpool_num	= 9028,
};

/*
 * We are going to use only one type of descriptors - host packet
 * descriptors. We staticaly allocate memory for them here
 */
struct qm_host_desc desc_pool[HDESC_NUM]
	__attribute__((aligned(sizeof(struct qm_host_desc))));

static struct qm_config *qm_cfg = NULL;
static struct qm_config *qm2_cfg;

int inline num_of_desc_to_reg(int num_descr)
{
	int j, num;

	for (j=0, num=32; j<15; j++, num *= 2) {
		if (num_descr <= num)
			return j;
	}

	return 15;
}

static inline u32 qnum_to_logical_qmgr(u32 qnum)
{
	return qnum / KS2_QUEUES_PER_QUEUE_MGR;
}

static inline int qmgr_manage_qnum(struct qm_config *qm, u32 qnum)
{
	u32 startq, endq;
	int ret = 0;

	startq = qm->start_queue;
	endq = qm->start_queue + qm->num_queues - 1;

	if ((qnum >= startq) && (qnum <= endq))
		ret = 1;

	return ret;
}

static inline struct qm_config *qnum_to_qmgr(u32 qnum)
{
	if (qm_cfg && qmgr_manage_qnum(qm_cfg, qnum))
		return qm_cfg;

	if (qm2_cfg && qmgr_manage_qnum(qm2_cfg, qnum))
		return qm2_cfg;

	return NULL;
}

static int _qm_init(struct qm_config * cfg)
{
	u32	j;
	struct qm_host_desc *desc;

	if (cfg == NULL)
		return QM_ERR;

	cfg->mngr_cfg->link_ram_base0	= cfg->i_lram;
	cfg->mngr_cfg->link_ram_size0	= cfg->i_lram_size;
	cfg->mngr_cfg->link_ram_base1	= 0;
	cfg->mngr_cfg->link_ram_size1	= 0;
	cfg->mngr_cfg->link_ram_base2	= 0;

	cfg->desc_mem[0].base_addr = cfg->desc_pool_base;
	cfg->desc_mem[0].start_idx = 0;
	cfg->desc_mem[0].desc_reg_size =
		(((sizeof(struct qm_host_desc) >> 4) - 1) << 16) |
		num_of_desc_to_reg(HDESC_NUM);

	memset((void *)cfg->desc_pool_base, 0, cfg->desc_pool_size);

	desc = (struct qm_host_desc *)cfg->desc_pool_base;
	for (j = 0; j < cfg->num_desc; j++)
		qm_push(&desc[j], cfg->qpool_num);

	return QM_OK;
}

int qm_init(void)
{
	int ret;

	if (qm_cfg)
		return QM_OK;

	if (qm2_cfg)
		return QM_ERR;

	memset(desc_pool, 0, sizeof(desc_pool));
	ks2_qm_memmap.desc_pool_base = (u32)desc_pool;
	ks2_qm_memmap.desc_pool_size = sizeof(desc_pool);
	ks2_qm_memmap.num_desc = HDESC_NUM;

	qm_cfg = &ks2_qm_memmap;

	ret = _qm_init(qm_cfg);
	if (ret != QM_OK)
		qm_cfg = NULL;

	return ret;
}

int qm2_init(void)
{
	int ret;

	if (qm2_cfg)
		return QM_OK;

	if (qm_cfg)
		return QM_ERR;

	memset(desc_pool, 0, sizeof(desc_pool));
	ks2_qm2_memmap.desc_pool_base = (u32)desc_pool;
	ks2_qm2_memmap.desc_pool_size = sizeof(desc_pool);
	ks2_qm2_memmap.num_desc = HDESC_NUM;

	qm2_cfg = &ks2_qm2_memmap;

	ret = _qm_init(qm2_cfg);
	if (ret != QM_OK)
		qm2_cfg = NULL;

	return ret;
}

static void _qm_close(struct qm_config *qm)
{
	u32	j;

	if (qm == NULL)
		return;

	queue_close(qm->qpool_num);

	qm->mngr_cfg->link_ram_base0	= 0;
	qm->mngr_cfg->link_ram_size0	= 0;
	qm->mngr_cfg->link_ram_base1	= 0;
	qm->mngr_cfg->link_ram_size1	= 0;
	qm->mngr_cfg->link_ram_base2	= 0;

	for (j = 0; j < qm->region_num; j++) {
		qm->desc_mem[j].base_addr = 0;
		qm->desc_mem[j].start_idx = 0;
		qm->desc_mem[j].desc_reg_size = 0;
	}
}

void qm_close(void)
{
	if (!qm_cfg)
		return;

	_qm_close(qm_cfg);
	qm_cfg = NULL;
}

void qm2_close(void)
{
	if (!qm2_cfg)
		return;

	_qm_close(qm2_cfg);
	qm2_cfg = NULL;
}

void qm_push(struct qm_host_desc *hd, u32 qnum)
{
	u32 regd;
	struct qm_config *qm;

	qm = qnum_to_qmgr(qnum);
	if (!qm)
		return;

	qnum -= qm->start_queue;

	cpu_to_bus((u32*)hd, sizeof(struct qm_host_desc)/4);
	regd = (u32)hd | ((sizeof(struct qm_host_desc) >> 4) - 1);
	writel(regd, &qm->queue[qnum].ptr_size_thresh);
}

void qm_buff_push(struct qm_host_desc *hd, u32 qnum,
		    void *buff_ptr, u32 buff_len)
{
	hd->orig_buff_len = hd->buff_len = buff_len;
	hd->orig_buff_ptr = hd->buff_ptr = (u32)buff_ptr;
	qm_push(hd, qnum);
}

struct qm_host_desc * qm_pop(u32 qnum)
{
	u32 uhd;
	struct qm_config *qm;

	qm = qnum_to_qmgr(qnum);
	if (!qm)
		return NULL;

	qnum -= qm->start_queue;

	uhd = readl(&qm->queue[qnum].ptr_size_thresh) & ~0xf;
	if (uhd)
		cpu_to_bus((u32 *)uhd, sizeof(struct qm_host_desc)/4);

	return (struct qm_host_desc *)uhd;
}

struct qm_host_desc * qm_pop_from_free_pool(void)
{
	if (!qm_cfg)
		return NULL;

	return qm_pop(qm_cfg->qpool_num);
}

void queue_close(u32 qnum)
{
	struct qm_host_desc *hd;

	while ((hd = qm_pop(qnum)))
		;
}

/************************************************
 * DMA API
 ***********************************************/
struct pktdma_cfg k2hk_netcp_pktdma = {
	.global		= (struct global_ctl_regs*)0x02004000,
	.tx_ch		= (struct tx_chan_regs*)0x02004400,
	.tx_ch_num	= 9,
	.rx_ch		= (struct rx_chan_regs*)0x02004800,
	.rx_ch_num	= 26,
	.tx_sched	= (u32*)0x02004c00,
	.rx_flows	= (struct rx_flow_regs*)0x02005000,
	.rx_flow_num	= 32,

	.rx_free_q	= 4001,
	.rx_rcv_q	= 4002,
	.tx_snd_q       = 648,
	.dest_port_info	= PKT_INFO,
};

struct pktdma_cfg k2e_netcp_pktdma = {
	.global		= (struct global_ctl_regs *)0x24186000,
	.tx_ch		= (struct tx_chan_regs *)0x24187000,
	.tx_ch_num	= 21,
	.rx_ch		= (struct rx_chan_regs *)0x24188000,
	.rx_ch_num	= 91,
	.tx_sched	= (u32 *)0x24186100,
	.rx_flows	= (struct rx_flow_regs *)0x24189000,
	.rx_flow_num	= 96,

	.rx_free_q	= 4001,
	.rx_rcv_q	= 4002,
	.tx_snd_q	= 896,
	.dest_port_info	= TAG_INFO,
};

struct pktdma_cfg k2l_netcp_pktdma = {
	.global		= (struct global_ctl_regs *)0x26186000,
	.tx_ch		= (struct tx_chan_regs *)0x26187000,
	.tx_ch_num	= 21,
	.rx_ch		= (struct rx_chan_regs *)0x26188000,
	.rx_ch_num	= 91,
	.tx_sched	= (u32 *)0x26186100,
	.rx_flows	= (struct rx_flow_regs *)0x26189000,
	.rx_flow_num	= 96,

	.rx_free_q	= 4001,
	.rx_rcv_q	= 4002,
	.tx_snd_q	= 896,
	.dest_port_info	= TAG_INFO,
};

struct pktdma_cfg k2hk_netcpx_pktdma = {
	.global		= (struct global_ctl_regs *)0x2fa1000,
	.tx_ch		= (struct tx_chan_regs *)0x2fa1400,
	.tx_ch_num	= 8,
	.rx_ch		= (struct rx_chan_regs *)0x02fa1800,
	.rx_ch_num	= 16,
	.tx_sched	= (u32 *)0x02fa1c00,
	.rx_flows	= (struct rx_flow_regs *)0x02fa2000,
	.rx_flow_num	= 16,

	.rx_free_q	= 9029,
	.rx_rcv_q	= 9030,
	.tx_snd_q       = 8752,
	.dest_port_info	= TAG_INFO,
};

struct pktdma_cfg k2e_netcpx_pktdma = {
	.global		= (struct global_ctl_regs *)0x2fa1000,
	.tx_ch		= (struct tx_chan_regs *)0x2fa1400,
	.tx_ch_num	= 8,
	.rx_ch		= (struct rx_chan_regs *)0x02fa1800,
	.rx_ch_num	= 16,
	.tx_sched	= (u32 *)0x02fa1c00,
	.rx_flows	= (struct rx_flow_regs *)0x02fa2000,
	.rx_flow_num	= 16,

	.rx_free_q	= 4001,
	.rx_rcv_q	= 4002,
	.tx_snd_q       = 692,
	.dest_port_info	= TAG_INFO,
};

struct pktdma_cfg *netcp;
struct pktdma_cfg *netcpx;

static int netcp_rx_disable(struct pktdma_cfg *netcp_cfg)
{
	u32 j, v, k;

	for (j = 0; j < netcp_cfg->rx_ch_num; j++) {
		v = readl(&netcp_cfg->rx_ch[j].cfg_a);
		if (!(v & CPDMA_CHAN_A_ENABLE))
			continue;

		writel(v | CPDMA_CHAN_A_TDOWN, &netcp_cfg->rx_ch[j].cfg_a);
		for (k = 0; k < TDOWN_TIMEOUT_COUNT; k++) {
			udelay(100);
			v = readl(&netcp_cfg->rx_ch[j].cfg_a);
			if (!(v & CPDMA_CHAN_A_ENABLE))
				continue;

		}
		if (k == TDOWN_TIMEOUT_COUNT) {
			/* TODO: teardown error */
		}
	}

	/* Clear all of the flow registers */
	for (j = 0; j < netcp_cfg->rx_flow_num; j++)  {
		writel(0, &netcp_cfg->rx_flows[j].control);
		writel(0, &netcp_cfg->rx_flows[j].tags);
		writel(0, &netcp_cfg->rx_flows[j].tag_sel);
		writel(0, &netcp_cfg->rx_flows[j].fdq_sel[0]);
		writel(0, &netcp_cfg->rx_flows[j].fdq_sel[1]);
		writel(0, &netcp_cfg->rx_flows[j].thresh[0]);
		writel(0, &netcp_cfg->rx_flows[j].thresh[1]);
		writel(0, &netcp_cfg->rx_flows[j].thresh[2]);
	}

	return QM_OK;
}

static int netcp_tx_disable(struct pktdma_cfg *netcp_cfg)
{
	u32 j, v, k;

	for (j = 0; j < netcp_cfg->tx_ch_num; j++) {
		v = readl(&netcp_cfg->tx_ch[j].cfg_a);
		if (!(v & CPDMA_CHAN_A_ENABLE))
			continue;

		writel(v | CPDMA_CHAN_A_TDOWN, &netcp_cfg->rx_ch[j].cfg_a);
		for (k = 0; k < TDOWN_TIMEOUT_COUNT; k++) {
			udelay(100);
			v = readl(&netcp_cfg->rx_ch[j].cfg_a);
			if (!(v & CPDMA_CHAN_A_ENABLE))
				continue;

		}
		if (k == TDOWN_TIMEOUT_COUNT) {
			/* TODO: teardown error */
		}
	}

	return QM_OK;
}

static int _netcp_init(struct pktdma_cfg *ncpcfg,
	struct rx_buff_desc *rx_buffers, u32 qpool_num)
{
	u32 j, v, qm, qm_offset, num_lqms = 4;
	struct qm_host_desc *hd;
	u8* rx_ptr;

	if (ncpcfg == NULL || rx_buffers == NULL ||
	    rx_buffers->buff_ptr == NULL)
		return QM_ERR;

	ncpcfg->rx_flow = rx_buffers->rx_flow;
	ncpcfg->qpool_num = qpool_num;

	/* init rx queue */
	rx_ptr = rx_buffers->buff_ptr;

	for (j=0; j < rx_buffers->num_buffs; j++) {
		hd = qm_pop(qpool_num);
		if (hd == NULL)
			return QM_ERR;

		qm_buff_push(hd, ncpcfg->rx_free_q,
			     rx_ptr, rx_buffers->buff_len);

		rx_ptr += rx_buffers->buff_len;
	}

	netcp_rx_disable(ncpcfg);

	/* configure rx channels */
	qm = qnum_to_logical_qmgr(ncpcfg->rx_rcv_q);
	v = CPDMA_REG_VAL_MAKE_RX_FLOW_A(1, 1, 0, 0, 0, 0,
				qm, ncpcfg->rx_rcv_q);
	writel(v, &ncpcfg->rx_flows[ncpcfg->rx_flow].control);
	writel(0, &ncpcfg->rx_flows[ncpcfg->rx_flow].tags);
	writel(0, &ncpcfg->rx_flows[ncpcfg->rx_flow].tag_sel);

	v = CPDMA_REG_VAL_MAKE_RX_FLOW_D(qm, ncpcfg->rx_free_q,
				 qm, ncpcfg->rx_free_q);

	writel(v, &ncpcfg->rx_flows[ncpcfg->rx_flow].fdq_sel[0]);
	writel(v, &ncpcfg->rx_flows[ncpcfg->rx_flow].fdq_sel[1]);
	writel(0, &ncpcfg->rx_flows[ncpcfg->rx_flow].thresh[0]);
	writel(0, &ncpcfg->rx_flows[ncpcfg->rx_flow].thresh[1]);
	writel(0, &ncpcfg->rx_flows[ncpcfg->rx_flow].thresh[2]);

	for (j = 0; j < ncpcfg->rx_ch_num; j++)
		writel(CPDMA_CHAN_A_ENABLE, &ncpcfg->rx_ch[j].cfg_a);

	/* configure tx channels */
	/* Disable loopback in the tx direction */
	writel(0, &ncpcfg->global->emulation_control);

	/* Set QM base address, only for K2x devices */
	qm_offset = KS2_QUEUES_PER_QUEUE_MGR * sizeof(struct qm_reg_queue);
	if (cpu_is_k2e())
		num_lqms = 2;
	for (j = 0; j < 4; j++)
		writel(KS2_QM_BASE_ADDRESS + (j % num_lqms) * qm_offset,
			&ncpcfg->global->qm_base_addr[j]);

	/* Enable all channels. The current state isn't important */
	for (j = 0; j < ncpcfg->tx_ch_num; j++)  {
		writel(0, &ncpcfg->tx_ch[j].cfg_b);
		writel(CPDMA_CHAN_A_ENABLE, &ncpcfg->tx_ch[j].cfg_a);
	}

	return QM_OK;
}

int netcp_init(struct rx_buff_desc *rx_buffers)
{
	if (!qm_cfg)
		return QM_ERR;

	if (cpu_is_k2hk()) {
		netcp = &k2hk_netcp_pktdma;
		_netcp_init(netcp, rx_buffers, qm_cfg->qpool_num);
		return QM_OK;
	} else if (cpu_is_k2e()) {
		netcp = &k2e_netcp_pktdma;
		_netcp_init(netcp, rx_buffers, qm_cfg->qpool_num);
		return QM_OK;
	} else if (cpu_is_k2l()) {
		netcp = &k2l_netcp_pktdma;
		_netcp_init(netcp, rx_buffers, qm_cfg->qpool_num);
		return QM_OK;
	} else
		return QM_ERR;
}

int netcpx_init(struct rx_buff_desc *rx_buffers)
{
	if (cpu_is_k2hk()) {
		if (!qm2_cfg)
			return QM_ERR;

		netcpx = &k2hk_netcpx_pktdma;
		_netcp_init(netcpx, rx_buffers, qm2_cfg->qpool_num);
		return QM_OK;
	}

	if (cpu_is_k2e()) {
		if (!qm_cfg)
			return QM_ERR;

		netcpx = &k2e_netcpx_pktdma;
		_netcp_init(netcpx, rx_buffers, qm_cfg->qpool_num);
		return QM_OK;
	}

	return QM_ERR;
}

static int _netcp_close(struct pktdma_cfg *ncp)
{
	if (!ncp)
		return QM_ERR;

	netcp_tx_disable(ncp);
	netcp_rx_disable(ncp);

	queue_close(ncp->rx_free_q);
	queue_close(ncp->rx_rcv_q);
	queue_close(ncp->tx_snd_q);

	return QM_OK;
}

int netcp_close(void)
{
	return _netcp_close(netcp);
}

int netcpx_close(void)
{
	return _netcp_close(netcpx);
}

static int _netcp_send(struct pktdma_cfg *ncp, u32 *pkt,
			int num_bytes, u32 dest_port, u32 qpool_num)
{
	struct qm_host_desc *hd;

	hd = qm_pop(qpool_num);
	if (hd == NULL) {
		printf("_netcp_send: no desc qpool_num %u\n", qpool_num);
		return QM_ERR;
	}

	dest_port &= 0xf;
	hd->desc_info = num_bytes;
	if (netcp->dest_port_info == PKT_INFO)
		hd->packet_info = qpool_num | (dest_port << 16);
	else {
		hd->packet_info = qpool_num;
		hd->tag_info = dest_port;
	}

	qm_buff_push(hd, ncp->tx_snd_q, pkt, num_bytes);

	return QM_OK;
}

int netcp_send(u32 *pkt, int num_bytes, u32 dest_port)
{
	return _netcp_send(netcp, pkt, num_bytes, dest_port, netcp->qpool_num);
}

int netcpx_send(u32 *pkt, int num_bytes, u32 dest_port)
{
	return _netcp_send(netcpx, pkt, num_bytes,
			dest_port, netcpx->qpool_num);
}

static void *_netcp_recv(struct pktdma_cfg *ncp, u32 **pkt, int *num_bytes)
{
	struct qm_host_desc *hd;

	hd = qm_pop(ncp->rx_rcv_q);
	if (!hd)
		return NULL;

	*pkt = (u32*)hd->buff_ptr;
	*num_bytes = hd->desc_info & 0x3fffff;

	return hd;
}

void *netcp_recv(u32 **pkt, int *num_bytes)
{
	return _netcp_recv(netcp, pkt, num_bytes);
}

void *netcpx_recv(u32 **pkt, int *num_bytes)
{
	return _netcp_recv(netcpx, pkt, num_bytes);
}

static void _netcp_release_rxhd(struct pktdma_cfg *ncp, void *hd)
{
	struct qm_host_desc *_hd = (struct qm_host_desc *)hd;

	_hd->buff_len = _hd->orig_buff_len;
	_hd->buff_ptr = _hd->orig_buff_ptr;

	qm_push(_hd, ncp->rx_free_q);
}

void netcp_release_rxhd(void *hd)
{
	return _netcp_release_rxhd(netcp, hd);
}

void netcpx_release_rxhd(void *hd)
{
	return _netcp_release_rxhd(netcpx, hd);
}
/* use the stucture from init.c */


