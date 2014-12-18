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

/* Board specific default environment variables */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	"addr_mon=0x0c140000\0"						\
	"args_hdd=setenv bootargs ${bootargs} rw root=/dev/sda1\0"	\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs"		\
	" root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,2048\0"		\
	"get_fdt_hdd=dhcp ${addr_fdt} ${tftp_root}/${name_fdt}\0"	\
	"get_kern_hdd=dhcp ${addr_kern} ${tftp_root}/${name_kern}\0"	\
	"get_mon_hdd=dhcp ${addr_mon} ${tftp_root}/${name_mon}\0"	\
	"has_mdio=1\0"							\
	"init_hdd=run set_fs_none args_all args_hdd\0"			\
	"name_fdt=uImage-k2e-evm.dtb\0"					\
	"name_fs=arago-console-image-k2e-evm.cpio.gz\0"			\
	"name_mon=skern-k2e-evm.bin\0"					\
	"name_ubi=k2e-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2e-evm.gph\0"

#include <configs/ks2_evm.h>

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_KS2_SETTINGS	\
					CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c100000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_2K

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT		"K2E EVM # "

#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#endif /* __CONFIG_H */
