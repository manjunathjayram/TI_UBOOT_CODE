/*
 * Copyright (C) 2014 Texas Instruments Inc.
 *
 * Keystone2: DDR3 test commands
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
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>
#include <common.h>
#include <command.h>

#define DDR_MIN_ADDR	CONFIG_SYS_SDRAM_BASE
#define DDR_MAX_SIZE	CONFIG_MAX_RAM_BANK_SIZE
#define DDR_MAX_ADDR	(DDR_MIN_ADDR + DDR_MAX_SIZE)

#define DDR_REMAP_ADDR	0x80000000
#define ECC_START_ADDR1	((DDR_MIN_ADDR - DDR_REMAP_ADDR) >> 17)
#define ECC_END_ADDR1	(((DDR_MAX_ADDR - DDR_REMAP_ADDR - \
			   CONFIG_MAX_UBOOT_MEM_SIZE) >> 17) - 1)

#define DDR_TEST_BURST_SIZE		1024
int ddr_memory_test(u32 start_address, u32 end_address, int quick)
{
	u32 index_start, value, index;

	index_start = start_address;

	while (1) {
		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4)
			__raw_writel(index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4) {
			value = __raw_readl(index);
			if (value != index) {
				printf("ddr_memory_test: Failed at address"
				       " index = 0x%x value = 0x%x *(index)"
				       " = 0x%x\n",
					index, value, __raw_readl(index));
				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		if (quick)
			continue;

		/* Write a pattern for complementary values */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4)
			__raw_writel((u32)~index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4) {
			value = __raw_readl(index);
			if (value != ~index) {
				printf("ddr_memory_test: Failed at address "
				       "index = 0x%x value = 0x%x *(index) "
				       "= 0x%x\n",
					index, value, __raw_readl(index));
				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if ((index_start >= end_address))
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2)
			__raw_writew((u16)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2) {
			value = __raw_readw(index);
			if (value != (u16)index) {
				printf("ddr_memory_test: Failed at address "
				       "index = 0x%x value = 0x%x *(index) ="
				       " 0x%x\n",
					index, value, __raw_readw(index));
				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if ((index_start >= end_address))
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1)
			__raw_writeb((u8)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1) {
			value = __raw_readb(index);
			if (value != (u8)index) {
				printf("ddr_memory_test: Failed at address "
				       "index = 0x%x value = 0x%x *(index) "
				       "= 0x%x\n",
					index, value, __raw_readb(index));
				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if ((index_start >= end_address))
			break;
	}

	printf("ddr_memory_test PASSED!\n");
	return 0;
}

static int ddr_memory_compare(u32 address1, u32 address2, u32 size)
{
	u32 index, value, index2, value2;

	for (index = address1, index2 = address2;
	     index < address1 + size;
	     index += 4, index2 += 4) {
		value = __raw_readl(index);
		value2 = __raw_readl(index2);

		if (value  != value2) {
			printf("ddr_memory_test: Compare failed at address = "
			       "0x%x value = 0x%x, address2 = 0x%x value2 = "
			       "0x%x\n", index, value, index2, value2);
			return -1;
		}
	}

	printf("ddr_memory_compare PASSED!\n");
	return 0;
}

int ddr_memory_ecc_err(u32 base, u32 address, u32 ecc_err)
{
	u32 value1, value2, value3;

	printf("Disabling DDR ECC ...\n");
	ddr3_disable_ecc(base);

	value1 = __raw_readl(address);
	value2 = value1 ^ ecc_err;
	__raw_writel(value2, address);

	value3 = __raw_readl(address);
	printf("ECC err test, addr 0x%x, read data 0x%x, wrote data "
		"0x%x, err pattern: 0x%x, read after write data 0x%x\n",
		address, value1, value2, ecc_err, value3);

	__raw_writel(ECC_START_ADDR1 | (ECC_END_ADDR1 << 16),
			base + K2HK_DDR3_ECC_ADDR_RANGE1_OFFSET);

	printf("Enabling DDR ECC ...\n");
	ddr3_enable_ecc(base, 1);

	value1 = __raw_readl(address);
	printf("ECC err test, addr 0x%x, read data 0x%x\n", address, value1);

	check_ddr3_ecc_int(base);
	return 0;
}

int do_ddr_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 start_addr, end_addr, size, ecc_err;
	int test_case;

	if ((argc == 4) && (strncmp(argv[1], "test", 5) == 0)) {
		test_case = 1;
		start_addr = simple_strtoul(argv[2], NULL, 16);
		end_addr = simple_strtoul(argv[3], NULL, 16);
	} else if ((argc == 5) && (strncmp(argv[1], "compare", 8) == 0)) {
		test_case = 2;
		start_addr = simple_strtoul(argv[2], NULL, 16);
		end_addr = simple_strtoul(argv[3], NULL, 16);
	} else if ((argc == 4) && (strncmp(argv[1], "ecc_err", 8) == 0)) {
		test_case = 3;
		start_addr = simple_strtoul(argv[2], NULL, 16);
		ecc_err = simple_strtoul(argv[3], NULL, 16);
	} else {
		test_case = -1;
	}

	if ((test_case == 1) || (test_case == 2)) {
		if (
			(start_addr < CONFIG_SYS_SDRAM_BASE)		||
			(start_addr > (CONFIG_SYS_SDRAM_BASE + \
				       CONFIG_MAX_RAM_BANK_SIZE - 1))	||
			(end_addr < CONFIG_SYS_SDRAM_BASE)		||
			(end_addr > (CONFIG_SYS_SDRAM_BASE + \
				     CONFIG_MAX_RAM_BANK_SIZE - 1))	||
			(start_addr >= end_addr)
		   ) {
			printf("Invalid start or end address!\n");
			goto ddr_cmd_usage;
		}
		if (test_case == 1) {
			ddr_memory_test(start_addr, end_addr, 0);
		} else if (test_case == 2) {
			size = simple_strtoul(argv[4], NULL, 16);
			ddr_memory_compare(start_addr, end_addr, size);
		}
	} else if (test_case == 3) {
		if (
			(start_addr < CONFIG_SYS_SDRAM_BASE)		||
			(start_addr > (CONFIG_SYS_SDRAM_BASE + \
				       CONFIG_MAX_RAM_BANK_SIZE - 1))
		   ) {
			printf("Invalid address!\n");
			goto ddr_cmd_usage;
		}
		ddr_memory_ecc_err(K2HK_DDR3A_EMIF_CTRL_BASE, start_addr,
				   ecc_err);
	} else {
		goto ddr_cmd_usage;
	}
	return 0;

ddr_cmd_usage:
	return cmd_usage(cmdtp);
}


U_BOOT_CMD(
	ddr,	5,	1,	do_ddr_test,
	"DDR3 test",
	"test <start_addr in hex> <end_addr in hex> - test DDR from start address to end address\n"
	"ddr compare <start_addr in hex> <end_addr in hex> <size in hex> - compare DDR data of (size) bytes from start address to end address\n"
	"ddr ecc_err <addr in hex> <bit_err in hex> - generate bit errors in DDR data at <addr>, the command will read a 32-bit data from <addr>, and write (data ^ bit_err) back to <addr>\n"
);


