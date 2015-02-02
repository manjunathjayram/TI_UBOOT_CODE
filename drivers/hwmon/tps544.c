/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone2: tps544 driver
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

#define PMBUS_ON_OFF_CONFIG		0x02
#define PMBUS_VOUT_OV_FAULT_LIMIT	0x40
#define PMBUS_VOUT_OV_WARN_LIMIT	0x42
#define PMBUS_VOUT_COMMAND		0x21
#define PMBUS_READ_VOUT			0x8b
/* Initialize Top Avatar, send initial Vout PMBus command to Top Avatar */
int srss_tps544_init(u32 vid, u32 i2c_bus, u8 i2c_addr)
{
	/* Vout in PMBus format, Vout (Volt) = vout_pmbus[vid]/512,
	 * e.g. vid = 0, Vout = 0x0166 / 512 = 0.699 (Volt) */
	uchar vout_pmbus[64][2] = {
		{0x66, 0x1}, {0x69, 0x1}, {0x6d, 0x1}, {0x70, 0x1},
		{0x73, 0x1}, {0x76, 0x1}, {0x79, 0x1}, {0x7d, 0x1},
		{0x80, 0x1}, {0x84, 0x1}, {0x87, 0x1}, {0x8a, 0x1},
		{0x8d, 0x1}, {0x90, 0x1}, {0x94, 0x1}, {0x97, 0x1},
		{0x9a, 0x1}, {0x9e, 0x1}, {0xa1, 0x1}, {0xa4, 0x1},
		{0xa7, 0x1}, {0xab, 0x1}, {0xae, 0x1}, {0xb1, 0x1},
		{0xb5, 0x1}, {0xb8, 0x1}, {0xbb, 0x1}, {0xbe, 0x1},
		{0xc2, 0x1}, {0xc5, 0x1}, {0xc8, 0x1}, {0xcb, 0x1},
		{0xcf, 0x1}, {0xd2, 0x1}, {0xd6, 0x1}, {0xd9, 0x1},
		{0xdc, 0x1}, {0xdf, 0x1}, {0xe2, 0x1}, {0xe6, 0x1},
		{0xe9, 0x1}, {0xec, 0x1}, {0xf0, 0x1}, {0xf3, 0x1},
		{0xf6, 0x1}, {0xf9, 0x1}, {0xfc, 0x1}, {0x00, 0x2},
		{0x03, 0x2}, {0x07, 0x2}, {0x0a, 0x2}, {0x0d, 0x2},
		{0x10, 0x2}, {0x13, 0x2}, {0x17, 0x2}, {0x1a, 0x2},
		{0x1d, 0x2}, {0x21, 0x2}, {0x24, 0x2}, {0x27, 0x2},
		{0x2b, 0x2}, {0x2e, 0x2}, {0x31, 0x2}, {0x34, 0x2}
	};
	uchar on_off_config[] = {0x2,};
	uchar vout_ov_fault_limit = {0x33, 0x02};	/* 1.1 V, 0x0233/512 */
	uchar vout_ov_warn_limit = {0x29, 0x02};	/* 1.08 V, 0x0229/512 */
	u32 old_bus;

	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(i2c_bus);

	if (i2c_write(i2c_addr, PMBUS_ON_OFF_CONFIG, 1,	on_off_config, 1) != 0)
		goto err;
	if (i2c_write(i2c_addr, PMBUS_VOUT_OV_FAULT_LIMIT, 1,
			vout_ov_fault_limit, 2) != 0)
		goto err;
	if (i2c_write(i2c_addr, PMBUS_VOUT_OV_WARN_LIMIT, 1,
			vout_ov_warn_limit, 2) != 0)
		goto err;
	if (i2c_write(i2c_addr, PMBUS_VOUT_COMMAND, 1, vout_pmbus[vid], 2) != 0)
		goto err;
	i2c_set_bus_num(old_bus);
	return 0;
err:
	printf("tps544_init i2c write error\n");
	i2c_set_bus_num(old_bus);
	return -1;
}

