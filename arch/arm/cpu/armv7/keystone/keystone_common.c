/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone EVM : Board initialization
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
#include <asm/io.h>

/* Byte swap the 32-bit data if the device is BE */
int cpu_to_bus(u32 *ptr, u32 length)
{
	u32 i;

	if (!(readl(KS2_DEVSTAT) & 0x1))
		for (i = 0; i < length; i++, ptr++)
			*ptr = __swab32(*ptr);
	return 0;
}

int turn_off_all_dsps(int num_dsps)
{
	int j;
	int ret = 0;

	for (j = 0; j < num_dsps; j++) {
		if (psc_disable_module(j + KS2_LPSC_GEM_0))
			ret = 1;
		if (psc_disable_domain(j + 8))
			ret = 1;
	}

	return ret;
}

int turn_off_myself(void)
{
	printf("Turning off ourselves\r\n");
	mon_power_off(0);

	psc_disable_module(KS2_LPSC_TETRIS);
	psc_disable_domain(KS2_TETRIS_PWR_DOMAIN);

	asm volatile (
		      "isb\n"
		      "dsb\n"
		      "wfi\n"
		     );

	printf("What! Should not see that\n");
	return 0;
}

int do_killme_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return turn_off_myself();
}

U_BOOT_CMD(
	killme,	1,	0,	do_killme_cmd,
	"turn off main ARM core",
	"turn off main ARM core. Should not live after that :(\n"
);

int misc_init_r(void)
{
	char *env;

	debug_options = 0;
	env = getenv("debug_options");

	if (env)
		debug_options = simple_strtol(env, NULL, 0);

	if ((debug_options & DBG_LEAVE_DSPS_ON) == 0)
		turn_off_all_dsps(KS2_NUM_DSPS);

	return 0;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
#endif
}
