/*
 * Copyright (C) 2015 Texas Instruments Inc.
 *
 * Keystone2: DDR3 configuration
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef DUMP_DDR_CONFIG
void dump_phy_config(struct ddr3_phy_config *ptr)
{
	printf("\npllcr		0x%08X\n", ptr->pllcr);
	printf("pgcr1_mask	0x%08X\n", ptr->pgcr1_mask);
	printf("pgcr1_val	0x%08X\n", ptr->pgcr1_val);
	printf("ptr0		0x%08X\n", ptr->ptr0);
	printf("ptr1		0x%08X\n", ptr->ptr1);
	printf("ptr2		0x%08X\n", ptr->ptr2);
	printf("ptr3		0x%08X\n", ptr->ptr3);
	printf("ptr4		0x%08X\n", ptr->ptr4);
	printf("dcr_mask	0x%08X\n", ptr->dcr_mask);
	printf("dcr_val		0x%08X\n", ptr->dcr_val);
	printf("dtpr0		0x%08X\n", ptr->dtpr0);
	printf("dtpr1		0x%08X\n", ptr->dtpr1);
	printf("dtpr2		0x%08X\n", ptr->dtpr2);
	printf("mr0		0x%08X\n", ptr->mr0);
	printf("mr1		0x%08X\n", ptr->mr1);
	printf("mr2		0x%08X\n", ptr->mr2);
	printf("dtcr		0x%08X\n", ptr->dtcr);
	printf("pgcr2		0x%08X\n", ptr->pgcr2);
	printf("zq0cr1		0x%08X\n", ptr->zq0cr1);
	printf("zq1cr1		0x%08X\n", ptr->zq1cr1);
	printf("zq2cr1		0x%08X\n", ptr->zq2cr1);
	printf("pir_v1		0x%08X\n", ptr->pir_v1);
	printf("pir_v2		0x%08X\n\n", ptr->pir_v2);
};

void dump_emif_config(struct ddr3_emif_config *ptr)
{
	printf("\nsdcfg		0x%08X\n", ptr->sdcfg);
	printf("sdtim1		0x%08X\n", ptr->sdtim1);
	printf("sdtim2		0x%08X\n", ptr->sdtim2);
	printf("sdtim3		0x%08X\n", ptr->sdtim3);
	printf("sdtim4		0x%08X\n", ptr->sdtim4);
	printf("zqcfg		0x%08X\n", ptr->zqcfg);
	printf("sdrfc		0x%08X\n\n", ptr->sdrfc);
};
#endif

#define TEMP NORMAL_TEMP
#define VBUS_CLKPERIOD 1.875 /* Corresponds to vbus=533MHz, */
#define PLLGS_VAL	(4000.0 / VBUS_CLKPERIOD) /* 4 us */
#define PLLPD_VAL	(1000.0 / VBUS_CLKPERIOD) /* 1 us */
#define PLLLOCK_VAL	(100000.0 / VBUS_CLKPERIOD) /* 100 us */
#define PLLRST_VAL	(9000.0 / VBUS_CLKPERIOD) /* 9 us */
#define PHYRST_VAL	0x10
#define DDR_TERM RZQ_4_TERM
#define SDRAM_DRIVE RZQ_7_IMP
#define DYN_ODT ODT_DISABLE

struct ddr3_phy_config phy_spd_cfg;
struct ddr3_emif_config emif_spd_cfg;
static unsigned int ddrspdclock;
static int    ddr_size_gbyte;

enum ebank {
	DCE0,
	DCE0_DEC1
};

enum nm {
	DDR3busWidth_64,
	DDR3busWidth_32,
	DDR3busWidth_16
};

enum srt {
	NORMAL_TEMP,
	EXTENDED_TEMP
};

enum out_impedance {
	RZQ_6_IMP = 0,
	RZQ_7_IMP
};

enum die_term {
	ODT_DISABLE = 0,
	RZQ_4_TERM,
	RZQ_2_TERM,
	RZQ_6_TERM,
	RZQ_12_TERM,
	RZQ_8_TERM
};

struct ddr3_sodimm {
	float t_ck;
	uint32_t freqsel;
	uint32_t t_xp;
	uint32_t t_cke;
	uint32_t t_pllpd;
	uint32_t t_pllgs;
	uint32_t t_phyrst;
	uint32_t t_plllock;
	uint32_t t_pllrst;
	uint32_t t_rfc;
	uint32_t t_xs;
	uint32_t t_dinit0;
	uint32_t t_dinit1;
	uint32_t t_dinit2;
	uint32_t t_dinit3;
	uint32_t t_rtp;
	uint32_t t_wtr;
	uint32_t t_rp;
	uint32_t t_rcd;
	uint32_t t_ras;
	uint32_t t_rrd;
	uint32_t t_rc;
	uint32_t t_faw;
	uint32_t t_mrd;
	uint32_t t_mod;
	uint32_t t_wlo;
	uint32_t t_wlmrd;
	uint32_t t_xsdll;
	uint32_t t_xpdll;
	uint32_t t_ckesr;
	uint32_t t_dllk;
	uint32_t t_wr;
	uint32_t t_wr_bin;
	uint32_t cas;
	uint32_t cwl;
	uint32_t asr;
	uint32_t pasr;
	float t_refprd;
	uint8_t sdram_type;
	uint8_t ibank;
	uint8_t pagesize;
	uint8_t t_rrd2;
	uint8_t t_ras_max;
	uint8_t t_zqcs;
	uint32_t refresh_rate;
	uint8_t t_csta;

	uint8_t rank;
	uint8_t mirrored;
	uint8_t buswidth;
} spd;

uint8_t cas_latancy(uint16_t temp)
{

	int loop;
	uint8_t cas_bin;

	for (loop = 0; loop < 32; loop += 2, temp >>= 1) {
		if (temp & 0x0001)
			cas_bin = (loop > 15) ? loop - 15 : loop;
	}

	return cas_bin;
}

int ddrtimingcalculation(uint8_t buf[])
{
	float mtb, clk_freq;

	/* Determine Time Base */
	mtb = (float)buf[10] / buf[11];

	/* SDRAM Cycle Time */
	spd.t_ck = buf[12] * mtb;
	ddrspdclock = ((1 / spd.t_ck) * 1000) * 2;
	clk_freq = ddrspdclock / 2;

	if (buf[2] != 0x0b)
		return 1;

	spd.sdram_type = 0x03;

	if ((buf[4] & 0x70) != 0x00)
		return 1;

	spd.ibank = 0x03;

	/* Check number of ranks, stop if greater than two ranks */
	switch ((buf[7] & 0x38) >> 3) {
	case 0:
		spd.rank = 1;
		break;
	case 1:
		spd.rank = 2;
		break;
	default:
		return 1;
	}

	/* Identify # of column address bits */
	switch (buf[5] & 0x07) {
	case 0:
		spd.pagesize = 0x01;
		break;
	case 1:
		spd.pagesize = 0x02;
		break;
	case 2:
		spd.pagesize = 0x03;
		break;
	default:
		return 1;
	}

	/* Identify Bus Width */
	switch (buf[8] & 0x7) {
	case 1:
		spd.buswidth = DDR3busWidth_16;
		break;
	case 2:
		spd.buswidth = DDR3busWidth_32;
		break;
	case 3:
		spd.buswidth = DDR3busWidth_64;
		break;
	default:
		return 1;
	}

	spd.mirrored = (buf[63] & 1) ? 1 : 0;

	if (ddrspdclock == 1333) {
		puts("DDR3A Speed will be configured for 1333 Operation.\n");

		spd.t_xp = ((3 * spd.t_ck) > 6.0) ?
			3 : (6.0 / spd.t_ck) + 0.5;
		spd.t_cke = ((3 * spd.t_ck) > 5.625) ?
			3 : (5.625 / spd.t_ck) + 0.5;
	} else if (ddrspdclock == 1600) {
		puts("DDR3A Speed will be configured for 1600 Operation.\n");

		spd.t_xp = ((3 * spd.t_ck) > 6.0) ?
			3 : (6.0 / spd.t_ck) + 0.5;
		spd.t_cke = ((3 * spd.t_ck) > 5.0) ?
			3 : (5.0 / spd.t_ck) + 0.5;
	} else {
		printf("Unsupported DDR3 speed %d\n", ddrspdclock);
		return 1;
	}

	spd.t_xpdll = ((10 * spd.t_ck) > 24.0) ? 10 : 24.0 / spd.t_ck;
	spd.t_ckesr = spd.t_cke + 1;

	/* SPD Calculated Values */
	spd.cas = cas_latancy(((buf[15] << 8) | buf[14]));

	spd.t_wr = (buf[17] * mtb) / spd.t_ck;
	spd.t_wr_bin = (spd.t_wr / 2) & 0x07;

	spd.t_rcd = (((float)buf[18] * mtb) / spd.t_ck) + 0.5;
	spd.t_rrd = (((float)buf[19] * mtb) / spd.t_ck) + 0.5;
	spd.t_rp  = (((float)buf[20] * mtb) / spd.t_ck) + 0.5;

	spd.t_ras = (buf[21] & 0x0f) << 8;
	spd.t_ras |= buf[22];
	spd.t_ras = (((float)spd.t_ras * mtb) / spd.t_ck) + 0.5;


	spd.t_rc = (buf[21] & 0xf0) << 4;
	spd.t_rc |= buf[23];
	spd.t_rc = (((float)spd.t_rc * mtb) / spd.t_ck) + 0.5;

	spd.t_rfc = (buf[24] | (buf[25] << 8)) * mtb;
	spd.t_wtr = ((float)buf[26] * mtb) / spd.t_ck;
	spd.t_rtp = ((float)buf[27] * mtb) / spd.t_ck;

	spd.t_xs  = ((spd.t_rfc + 10) / spd.t_ck) + 0.5;
	spd.t_rfc = (spd.t_rfc / spd.t_ck) + 0.5;

	spd.t_faw = (((buf[28] << 8) | buf[29]) * mtb) / spd.t_ck;
	spd.t_rrd2 = ((((buf[28] << 8) | buf[29]) * mtb) / (4 * spd.t_ck)) - 1;

	/* Hard-coded values */
	spd.t_mrd = 0x00;
	spd.t_mod = 0x00;
	spd.t_wlo = 0x0C;
	spd.t_wlmrd = 0x28;
	spd.t_xsdll = 0x200;
	spd.t_ras_max = 0x0F;
	spd.t_csta = 0x05;
	spd.t_dllk = 0x200;

	/* CAS Write Latency */
	if (spd.t_ck >= 2.5)
		spd.cwl = 0;
	else if (spd.t_ck >= 1.875)
		spd.cwl = 1;
	else if (spd.t_ck >= 1.5)
		spd.cwl = 2;
	else if (spd.t_ck >= 1.25)
		spd.cwl = 3;
	else if (spd.t_ck >= 1.071)
		spd.cwl = 4;
	else
		spd.cwl = 5;

	/* SDRAM Thermal and Refresh Options */
	spd.asr = (buf[31] & 0x04) >> 2;
	spd.pasr = (buf[31] & 0x80) >> 7;
	spd.t_zqcs = 64;

	spd.t_refprd = ((TEMP == NORMAL_TEMP) ? 64000.00 : 32000.0) / 8192;
	spd.t_refprd = (spd.t_refprd * 1000) / spd.t_ck;

	spd.refresh_rate = spd.t_refprd;
	spd.t_refprd = spd.t_refprd * 5;

	/* Set MISC PHY space registers fields */
	if (((int)clk_freq / 2) >= 166 && ((int)clk_freq / 2 < 275))
		spd.freqsel = 0x03;
	else if (((int)clk_freq / 2) > 225 && ((int)clk_freq / 2 < 385))
		spd.freqsel = 0x01;
	else if (((int)clk_freq / 2) > 335 && ((int)clk_freq / 2 < 534))
		spd.freqsel = 0x00;

	spd.t_dinit0 = 500000.0 / spd.t_ck; /* CKE low time 500 us */
	spd.t_dinit1 = spd.t_xs;
	spd.t_dinit2 = 200000.0 / spd.t_ck; /* Reset low time 200 us */
	/* Time from ZQ initialization command to first command (1 us) */
	spd.t_dinit3 =  1000.0 / spd.t_ck;

	spd.t_pllgs = PLLGS_VAL + 1;
	spd.t_pllpd = PLLPD_VAL + 1;
	spd.t_plllock = PLLLOCK_VAL + 1;
	spd.t_pllrst = PLLRST_VAL;
	spd.t_phyrst = PHYRST_VAL;

	ddr_size_gbyte = (spd.rank * (256 << (buf[4] & 0xf))) / 1024;

	return 0;
}

static int crc16(uint8_t *ptr, int count)
{
	int crc, i;

	crc = 0;
	while (--count >= 0) {
		crc = crc ^ (int)*ptr++ << 8;
		for (i = 0; i < 8; ++i)
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
	}
	return crc & 0xffff;
}

unsigned int ddr3_spd_check(uint8_t spd[])
{

	int csum16;
	int len;
	unsigned char crc_lsb;   /* byte 126 */
	unsigned char crc_msb;   /* byte 127 */

	/*
	 * SPD byte0[7] - CRC coverage
	 * 0 = CRC covers bytes 0~125
	 * 1 = CRC covers bytes 0~116
	 */
	len = !(spd[0] & 0x80) ? 126 : 117;
	csum16 = crc16(spd, len);

	crc_lsb = (unsigned char)(csum16 & 0xff);
	crc_msb = (unsigned char)(csum16 >> 8);
	if ((spd[126] == crc_lsb) && (spd[127] == crc_msb)) {
		printf("SPD csum OK; in SPD: %02X %02X; computed %08X %02X\n",
		       spd[127], spd[126], crc_msb, crc_lsb);
		return 0;
	}

	printf("SPD csum ERROR; in SPD: %02X %02X; computed %08X %02X\n",
	       spd[127], spd[126], crc_msb, crc_lsb);

	return 1;
}

void init_ddr3param(void)
{
	phy_spd_cfg.pllcr = (spd.freqsel & 3) << 18 | 0xE << 13;
	phy_spd_cfg.pgcr1_mask = (IODDRM_MASK | ZCKSEL_MASK);
	phy_spd_cfg.pgcr1_val = ((1 << 2) | (1 << 7) | (1 << 23));
	phy_spd_cfg.ptr0 = ((spd.t_pllpd & 0x7ff) << 21) |
		((spd.t_pllgs & 0x7fff) << 6) | (spd.t_phyrst & 0x3f);
	phy_spd_cfg.ptr1 = ((spd.t_plllock & 0xffff) << 16) |
		(spd.t_pllrst & 0x1fff);
	phy_spd_cfg.ptr2 = 0;
	phy_spd_cfg.ptr3 = ((spd.t_dinit1 & 0x1ff) << 20) |
		(spd.t_dinit0 & 0xfffff);
	phy_spd_cfg.ptr4 = ((spd.t_dinit3 & 0x3ff) << 18) |
		(spd.t_dinit2 & 0x3ffff);

	phy_spd_cfg.dcr_mask = PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK;
	phy_spd_cfg.dcr_val = 1 << 10;

	if (spd.mirrored) {
		phy_spd_cfg.dcr_mask |= NOSRA_MASK | UDIMM_MASK;
		phy_spd_cfg.dcr_val |= (1 << 27) | (1 << 29);
	}

	phy_spd_cfg.dtpr0 = (spd.t_rc & 0x3f) << 26 | (spd.t_rrd & 0xf) << 22 |
		(spd.t_ras & 0x3f) << 16 | (spd.t_rcd & 0xf) << 12 |
		(spd.t_rp & 0xf) << 8 | (spd.t_wtr & 0xf) << 4 |
		(spd.t_rtp & 0xf);
	phy_spd_cfg.dtpr1 = (spd.t_wlo & 0xf) << 26 |
		(spd.t_wlmrd & 0x3f) << 20 | (spd.t_rfc & 0x1ff) << 11 |
		(spd.t_faw & 0x3f) << 5 | (spd.t_mod & 0x7) << 2|
		(spd.t_mrd & 0x3);

	phy_spd_cfg.dtpr2 = 0 << 31 | 1 << 30 | 0 << 29 |
		(spd.t_dllk & 0x3ff) << 19 | (spd.t_ckesr & 0xf) << 15;

	phy_spd_cfg.dtpr2 |= (((spd.t_xp > spd.t_xpdll) ?
			      spd.t_xp : spd.t_xpdll) &
			     0x1f) << 10;

	phy_spd_cfg.dtpr2 |= (((spd.t_xs > spd.t_xsdll) ?
			      spd.t_xs : spd.t_xsdll) &
			     0x3ff);

	phy_spd_cfg.mr0 = 1 << 12 | (spd.t_wr_bin & 0x7) << 9 | 0 << 8 |
		0 << 7 | ((spd.cas & 0x0E) >> 1) << 4 | 0 << 3 |
		(spd.cas & 0x01) << 2;

	phy_spd_cfg.mr1 = 0 << 12 | 0 << 11 | 0 << 7 | 0 << 3 |
		((DDR_TERM >> 2) & 1) << 9 | ((DDR_TERM >> 1) & 1) << 6 |
		(DDR_TERM & 0x1) << 2 | ((SDRAM_DRIVE >> 1) & 1) << 5 |
		(SDRAM_DRIVE & 1) << 1 | 0 << 0;

	phy_spd_cfg.mr2 = DYN_ODT << 9 | TEMP << 7 | (spd.asr & 1) << 6 |
		(spd.cwl & 7) << 3 | (spd.pasr & 7);

	phy_spd_cfg.dtcr = (spd.rank == 2) ? 0x730035C7 : 0x710035C7;
	phy_spd_cfg.pgcr2 = (0xF << 20) | ((int)spd.t_refprd & 0x3ffff);

	phy_spd_cfg.zq0cr1 = 0x0000005D;
	phy_spd_cfg.zq1cr1 = 0x0000005B;
	phy_spd_cfg.zq2cr1 = 0x0000005B;

	phy_spd_cfg.pir_v1 = 0x00000033;
	phy_spd_cfg.pir_v2 = 0x0000FF81;


	/* EMIF Registers */
	emif_spd_cfg.sdcfg = spd.sdram_type << 29 | (DDR_TERM & 7) << 25 |
		(DYN_ODT & 3) << 22 | (spd.cwl & 0x7) << 14 |
		(spd.cas & 0xf) << 8 | (spd.ibank & 3) << 5 |
		(spd.buswidth & 3) << 12 | (spd.pagesize & 3);

	if (spd.rank == 2)
		emif_spd_cfg.sdcfg |= DCE0_DEC1 << 3;

	emif_spd_cfg.sdtim1 = ((spd.t_wr - 1) & 0x1f) << 25 |
		((spd.t_ras - 1) & 0x7f) << 18 |
		((spd.t_rc - 1) & 0xff) << 10 |
		(spd.t_rrd2 & 0x3f) << 4  |
		((spd.t_wtr - 1) & 0xf);

	emif_spd_cfg.sdtim2 = 0x07 << 10 | ((spd.t_rp - 1) & 0x1f) << 5 |
		((spd.t_rcd-1) & 0x1f);

	emif_spd_cfg.sdtim3 = ((spd.t_xp - 2) & 0xf) << 28 |
		((spd.t_xs - 1) & 0x3ff) << 18 |
		((spd.t_xsdll - 1) & 0x3ff) << 8 |
		((spd.t_rtp - 1) & 0xf) << 4 | ((spd.t_cke) & 0xf);

	emif_spd_cfg.sdtim4 = (spd.t_csta & 0xf) << 28 |
		((spd.t_ckesr - 1) & 0xf) << 24 |
		((spd.t_zqcs - 1) & 0xff) << 16 |
		((spd.t_rfc - 1) & 0x3ff) << 4 |
		(spd.t_ras_max & 0xf);

	emif_spd_cfg.sdrfc = (spd.refresh_rate-1) & 0xffff;

	/* TODO zqcfg value fixed ,May be required correction for K2E evm. */
	emif_spd_cfg.zqcfg = (spd.rank == 2) ? 0xF0073200 : 0x70073200;
}

int get_dimm_params_from_spd(u8 *spd_params, unsigned int *ddr_clk,
			     int *ddr_size)
{
	int ret;
	int old_bus;

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	ret = i2c_read(0x53, 0, 1, spd_params, 256);

	i2c_set_bus_num(old_bus);

	if (ret) {
		puts("Cannot read DIMM params\n");
		return 1;
	}

	if (ddr3_spd_check(spd_params))
		return 1;

	if (ddrtimingcalculation(spd_params)) {
		puts("Timing caclulation error\n");
		return 1;
	}

	init_ddr3param();

#ifdef DUMP_DDR_CONFIG
	dump_emif_config(&emif_spd_cfg);
	dump_phy_config(&phy_spd_cfg);
#endif

	*ddr_clk = ddrspdclock;
	*ddr_size = ddr_size_gbyte;

	return 0;
}

