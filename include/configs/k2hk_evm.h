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

#ifndef __CONFIG_H
#define __CONFIG_H

/* Platform type */
#define CONFIG_SOC_K2HK
#define CONFIG_K2HK_EVM

/* Board specific default environment variables */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	"addr_mon=0x0c5f0000\0"						\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs"		\
	" root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"		\
	"name_fdt=uImage-k2hk-evm.dtb\0"				\
	"name_fs=arago-console-image-k2hk-evm.cpio.gz\0"		\
	"name_mon=skern-k2hk-evm.bin\0"					\
	"name_uboot=u-boot-spi-k2hk-evm.gph\0"				\
	"name_ubi=k2hk-evm-ubifs.ubi\0"					\
	"has_mdio=0\0"

#include <configs/ks2_evm.h>

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_KS2_SETTINGS	\
					CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c200000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT		"K2HK EVM # "

#define CONFIG_MCAST_TFTP

#define CONFIG_USB_MAX_CONTROLLER_COUNT	1

#endif /* __CONFIG_H */
