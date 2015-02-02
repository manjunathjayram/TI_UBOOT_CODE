/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone2: Smart Reflex Sub-system Initialization
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

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/psc_defs.h>
#include <common.h>
#include <i2c.h>

/* Smart Reflex Sub-System register offset */
#define SRSS_PID			0x0
#define SRSS_INTR_EN_SET		0x4
#define SRSS_EVENT_FLAG			0x8
#define SRSS_EVENT_CLR			0xc
#define SRSS_TEMP_CTL0			0x1c
#define SRSS_TEMP_CTL1			0x20
#define SRSS_TEMP_CTL2			0x24
#define SRSS_TEMP_STAT0			0x28
#define SRSS_TEMP_STAT1			0x2c
#define SRSS_VPRM_CTL0			0x80
#define SRSS_VPRM_CTL1			0x84
#define SRSS_VPRM_STAT0			0x88
#define SRSS_VPRM_STAT1			0x8c

/* SRSS_TEMP_CTL0 fields definitions */
#define TM_ENABLE		0x3
#define C0TEMP_ENABLE		(0x1 << 2)	/* enables C0 TC */
#define I2C_REQ_ENABLE		(0x1 << 3)	/* SRSS I2C */

#define THPT1_SHIFT		24
#define THPT2_SHIFT		0
#define THPT3_SHIFT		8
#define THPT4_SHIFT		16
#define THPT5_SHIFT		24
#define THPT_MASK		0xff

/* SRSS_TEMP_STAT1 fields definitions */
#define VID_SHIFT		24
#define VID_MASK		0x3f

/* SRSS_VPRM_CTL0 definitions */
#define VPRM_ENABLE		(0x1 << 0)	/* enables SR C0 */
#define VPRM_DISABLE		(0x0 << 0)	/* disable VPRM FSM */
#define SMPS_MODE_VID		(0x2 << 2)	/* VID 6-bit */
#define SMPS_MODE_I2C		(0x3 << 2)	/* I2C 6-bit */

#define VID_DEFAULT		0x2e	/* 0.994 volt */

/* Interrupt events definitions */
#define TEMP_THRPT_INT_SHIFT	17

/* SoC SRSS registers */
#define SR_C0_EN_SHIFT		26
#define SR_C0_EN_MASK		1
#define OPP_SEL_SHIFT		24
#define OPP_SEL_MASK		0x3
#define VID_OPP1_SHIFT		6
#define VID_OPP_MASK		0x3f

static void srss_vprm_c0_enable(u32 base, u32 enable)
{
	if (enable) {
		/* wait for VID_FSM to be in S0 or S1 */
		while ((__raw_readl(base + SRSS_VPRM_STAT0) & 0x3) > 1)
			;

		/* Enable Smart Reflex Class 0 */
		__raw_writel(VPRM_ENABLE | SMPS_MODE_VID,
				base + SRSS_VPRM_CTL0);
	} else {
		__raw_writel(0, base + SRSS_VPRM_CTL0);
		__raw_writel(0, base + SRSS_VPRM_CTL1);
	}
}

static void srss_c0_tc(u32 base, u32 enable)
{
	char *env = getenv("no_src0_tc");
	u32 no_src0_tc = 0;
	u32 temp_ctl0 = __raw_readl(base + SRSS_TEMP_CTL0);

	if (env)
		no_src0_tc = simple_strtol(env, NULL, 0);

	if (enable) {
		if (!no_src0_tc) {
			temp_ctl0 |= (C0TEMP_ENABLE | TM_ENABLE);
			printf("Smart Reflex Class 0 temperature "
				"compensation enabled\n");
		}
	} else {
		temp_ctl0 &= ~(C0TEMP_ENABLE | TM_ENABLE);
		printf("Smart Reflex Class 0 temperature "
			"compensation disabled\n");
	}

	/* Clear the threshold point interrupt pending flag */
	__raw_writel((1 << TEMP_THRPT_INT_SHIFT), base + SRSS_EVENT_CLR);

	__raw_writel(temp_ctl0, base + SRSS_TEMP_CTL0);
}

int srss_c0_init(u32 base, int (*pc_init)(u32, u32, u8))
{
	u32 kt_sr_c0, opp_sel, vdd_min, vid, efuse = 1;
	u32 temp_ctl0, temp_ctl1, thpt1, thpt2, thpt3, thpt4, thpt5;
	char *env = getenv("srss_vcntl");
	u32 srss_vcntl = 0;

	if (env)
		srss_vcntl = simple_strtol(env, NULL, 0);

	/* Enable SRSS clock */
	if (psc_enable_module(KS2_LPSC_CHIP_SRSS)) {
		printf("SRSS clock domain enable failed!\n");
		return -1;
	}

	srss_vprm_c0_enable(base, 0);

	kt_sr_c0 = __raw_readl(KS2_KT_VDD_SR_C0);

	/* Check if SR Class 0 is supported in this device */
	if (!((kt_sr_c0 >> SR_C0_EN_SHIFT) & SR_C0_EN_MASK)) {
		printf("SRSS Class0 not supported in the SoC!\n");
		return -1;
	}

	srss_vprm_c0_enable(base, 1);
	/* Check if kt_opp1/kt_opp2 is programmed in efuse or not */
	opp_sel = (kt_sr_c0 >> OPP_SEL_SHIFT) & OPP_SEL_MASK;
	if ((opp_sel != 1) && (opp_sel != 2)) {
		srss_vprm_c0_enable(base, 0);	/* disable SRSS C0 */
		return -1;
	}

	/* Get the initial VID value (6-bit) */
	vdd_min = (kt_sr_c0 >> ((opp_sel << 2) + (opp_sel << 1))) \
		  & VID_OPP_MASK;
	/* cannot set initial voltage to 0.7, otherwise will cause power
	   failure on the EVM */

	if (vdd_min == 0) {
		vdd_min = VID_DEFAULT;
		kt_sr_c0 &= ~(VID_OPP_MASK << \
				((opp_sel << 2) + (opp_sel << 1)));
		kt_sr_c0 |= vdd_min << ((opp_sel << 2) + (opp_sel << 1));
	}
	/* write back the default value if efuse is not programmed */
	__raw_writel(kt_sr_c0, KS2_KT_VDD_SR_C0);

	/* Check if temperature threshold points are programmed in efuse or
	 * not */
	temp_ctl0 = __raw_readl(base + SRSS_TEMP_CTL0);
	thpt1 = (temp_ctl0 >> THPT1_SHIFT) & THPT_MASK;

	temp_ctl1 = __raw_readl(base + SRSS_TEMP_CTL1);
	thpt2 = (temp_ctl1 >> THPT2_SHIFT) & THPT_MASK;
	thpt3 = (temp_ctl1 >> THPT3_SHIFT) & THPT_MASK;
	thpt4 = (temp_ctl1 >> THPT4_SHIFT) & THPT_MASK;
	thpt5 = (temp_ctl1 >> THPT5_SHIFT) & THPT_MASK;

	if ((thpt1 == 0) &&
	    (thpt2 == 0) &&
	    (thpt3 == 0) &&
	    (thpt4 == 0) &&
	    (thpt5 == 0))
		efuse = 0;

	if (efuse &&
	    ((thpt1 == thpt2) ||
	     (thpt2 == thpt3) ||
	     (thpt3 == thpt4) ||
	     (thpt4 == thpt5) ||
	     (thpt5 == thpt1)))
		efuse = 0;

	if (!efuse) {
		srss_vprm_c0_enable(base, 0);	/* disable SRSS C0 */
		return -1;
	}

	/* thpts decrement enabled by default if not efused */
	if (!(temp_ctl0 & 0x0000ffc0))
		temp_ctl0 |= 0x00005540;
	__raw_writel(temp_ctl1, base + SRSS_TEMP_CTL1);

	/* SRSS I2C interface disabled, diable TC */
	temp_ctl0 &= ~(I2C_REQ_ENABLE | C0TEMP_ENABLE);
	__raw_writel(temp_ctl0, base + SRSS_TEMP_CTL0);

	srss_c0_tc(base, 1);	/* enable temperature compensation */

	if (!srss_vcntl) {
		vid = (__raw_readl(base + SRSS_TEMP_STAT1) >> VID_SHIFT) & \
			VID_MASK;
		/* Send the VID to external power controller */
		if ((*pc_init)(vid, CONFIG_SRSS_I2C_BUS, CONFIG_PC_I2C_SLAVE) \
				!= 0) {
			srss_c0_tc(base, 0);	/* disable TC */
			srss_vprm_c0_enable(base, 0);	/* disable SRSS C0 */
		}
	}
	return 0;
}

