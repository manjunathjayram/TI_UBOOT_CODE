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
#define CONFIG_SOC_K2L
#define CONFIG_K2L_EVM

/* Board specific default environment variables */
#define CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS				\
	"addr_mon=0x0c140000\0"						\
	"args_ubi=setenv bootargs ${bootargs} rootfstype=ubifs"		\
	" root=ubi0:rootfs rootflags=sync rw ubi.mtd=2,4096\0"		\
	"name_fdt=uImage-k2l-evm.dtb\0"					\
	"name_fs=arago-console-image-k2l-evm.cpio.gz\0"			\
	"name_mon=skern-k2l-evm.bin\0"					\
	"name_ubi=k2l-evm-ubifs.ubi\0"					\
	"name_uboot=u-boot-spi-k2l-evm.gph\0"				\
	"has_mdio=1\0"

#include <configs/ks2_evm.h>

#define CONFIG_EXTRA_ENV_SETTINGS	CONFIG_EXTRA_ENV_KS2_SETTINGS	\
					CONFIG_EXTRA_ENV_KS2_BOARD_SETTINGS

/* SPL SPI Loader Configuration */
#define CONFIG_SPL_TEXT_BASE		0x0c100000

/* NAND Configuration */
#define CONFIG_SYS_NAND_PAGE_4K

/* U-Boot general configuration */
#define CONFIG_SYS_PROMPT		"K2L EVM # "

/* SRSS configuration */
#define CONFIG_SRSS_C0
#define CONFIG_SRSS_I2C_BUS		0x2	/* SoC I2C bus used to
						   communicate with TA */
#define CONFIG_PC_I2C_SLAVE		0x9	/* External power controller
						   I2C slave address */

#endif /* __CONFIG_H */
