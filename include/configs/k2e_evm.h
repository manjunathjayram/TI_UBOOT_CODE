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

#ifndef __CONFIG_H
#define __CONFIG_H

/* Platform type */
#define CONFIG_SOC_K2E
#define CONFIG_K2E_EVM

/* Defualt environment variables */
#define ADDR_MON	"addr_mon=0x0c140000\0"
#define NAME_FDT	"name_fdt=uImage-k2e-evm.dtb\0"
#define NAME_MON	"name_mon=skern-k2e.bin\0"
#define NAME_UBOOT	"name_uboot=u-boot-k2e-spi-keystone-evm.gph\0"
#define HAS_MDIO	"has_mdio=1\0"
#define ARGS_UBI	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs"\
			" root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"

#include <configs/ks2_evm.h>

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c100000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT		"K2E EVM # "

#endif /* __CONFIG_H */
