/*
 * Copyright (C) 2013 Texas Instruments
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <linux/mtd/mtd.h>
#include <malloc.h>
#include <nand.h>

static unsigned int hatoi(char *p, char **errp)
{
	unsigned int res = 0;

	if ((p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
		p += 2;

	while (1) {
		if ( *p >= 'a' && *p <= 'f' )
			res |= (*p - 'a' + 10);
		else if ( *p >= 'A' && *p <= 'F' ) 
			res |= (*p - 'A' + 10);
		else if ( *p >= '0' && *p <= '9' ) 
			res |= (*p - 'A' + 10);
		else {
			if (errp)
				*errp = p;
			return res;
		}
		
		if (*p == 0) {
			break;
		}
		res <<= 4;
	}

	if (errp)
		*errp = NULL;

	return res;
}

#define DATA_ADDR					0x80000000
#define OOB_RD_ADDR					0x81000000
#define OOB_WR_ADDR					0x82000000
#define NAND_NUM_PAGES_PER_BLOCK	64
int do_oob_format(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong off, start_addr, fmt_size, fmt_pages, size;
	unsigned int page;
	int ret;
	char *err;
	u_char *data_buf, *oobbuf_rd, *oobbuf_wr;
	nand_info_t *nand;
	mtd_oob_ops_t oob_ops;
	nand_erase_options_t erase_ops;
	struct nand_chip *chip;

	if (argc == 4) {
		if (strncmp(argv[1], "fmt", 4) == 0) {
			start_addr = hatoi(argv[2], &err);
			fmt_size = hatoi(argv[3], &err);
			nand = &nand_info[nand_curr_device];
			start_addr &= ~(nand->writesize - 1);
			fmt_pages = fmt_size/nand->writesize;

			/* Initialize the read/write buffer */
			data_buf = (u_char *)DATA_ADDR;
			oobbuf_rd = (u_char *)OOB_RD_ADDR;
			oobbuf_wr = (u_char *)OOB_WR_ADDR;
			memset (oobbuf_wr, 0xff, fmt_pages*nand->oobsize);

			memset(&oob_ops, 0, sizeof(oob_ops));
			oob_ops.len = nand->writesize;
			oob_ops.ooblen = nand->oobsize;
			oob_ops.mode = MTD_OOB_RAW;

			/* Read the data and oob and copy to the DDR */
			off = start_addr;
			size = 0;
			while (size < fmt_size) {
				/* skip the  bad blocks */
				while(nand->block_isbad(nand, (loff_t)off))
				{
					off += (nand->writesize * NAND_NUM_PAGES_PER_BLOCK);
				}

				/* Read the data and oob of the page */
				oob_ops.datbuf = data_buf;
				oob_ops.oobbuf = oobbuf_rd; /* must exist, but oob data will be appended to oob_ops.datbuf */
				ret = nand->read_oob(nand, (loff_t)off, &oob_ops);
				if (ret < 0) {
					printf("Error (%d) reading page %08lx\n", ret, off);
					return -1;
				}

				/* re-format the oob layout for the RBL readable format, Linux uses
				   the last 40 bytes of ECC in the 64-byte oob, while RBL uses last 10 bytes
				   of each 16 bytes oob (total 4x16 bytes) */
				memcpy(&oobbuf_wr[16*0+6], &data_buf[nand->writesize+24+10*0], 10);
				memcpy(&oobbuf_wr[16*1+6], &data_buf[nand->writesize+24+10*1], 10);
				memcpy(&oobbuf_wr[16*2+6], &data_buf[nand->writesize+24+10*2], 10);
				memcpy(&oobbuf_wr[16*3+6], &data_buf[nand->writesize+24+10*3], 10);

				/* Increment one page */
				data_buf += (nand->writesize + nand->oobsize);
				oobbuf_rd += nand->oobsize;
				oobbuf_wr += nand->oobsize;
				size += nand->writesize;
				off += nand->writesize;
			}

			/* Erase the blocks */
			memset(&erase_ops, 0, sizeof(erase_ops));
			erase_ops.offset = start_addr;
			erase_ops.length = fmt_size;
			erase_ops.spread = 1;
			erase_ops.quiet = 1;
			ret = nand_erase_opts(nand, &erase_ops);
			if (ret < 0) {
				printf("Error (%d) erase off %08lx\n", ret, start_addr);
				return -1;
			}

			/* Write raw data and raw oob to NAND */
			off = start_addr;
			size = 0;
			chip = nand->priv;

			/* Initialize the write buffer */
			data_buf = (u_char *)DATA_ADDR;
			oobbuf_wr = (u_char *)OOB_WR_ADDR;

			while (size < fmt_size) {
				/* skip the  bad blocks */
				while(nand->block_isbad(nand, (loff_t)off))
				{
					off += (nand->writesize * NAND_NUM_PAGES_PER_BLOCK);
				}

				/* Write a raw page and oob */
				chip->state = FL_WRITING; /* Get the device */
				chip->ops.len = nand->writesize;
				chip->ops.datbuf = data_buf;
				memcpy(chip->oob_poi, oobbuf_wr, nand->oobsize);
				page = (int)(off >> chip->page_shift) & chip->pagemask;
				ret = chip->write_page(nand, chip, chip->ops.datbuf, page, 0, 1);
				if (ret < 0) {
					printf("Error (%d) write page off %08lx\n", ret, off);
					return -1;
				}
				chip->select_chip(nand, -1);	/* De-select the NAND device */

				/* Increment one page */
				data_buf += (nand->writesize + nand->oobsize);
				oobbuf_wr += nand->oobsize;
				size += nand->writesize;
				off += nand->writesize;
			}
		}
		else {
			goto oob_cmd_usage;
		}
	} else {
		goto oob_cmd_usage;
	}
	return 0;

oob_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	oob,	4,	1,	do_oob_format,
	"Re-format the oob data from the U-boot layout to the RBL readable layout",
	"fmt <start_addr in hex> <size in hex> - Re-format the OOB data \n"
	"    from start offset byte addr with size bytes\n"
);

int do_factory_image(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong off, off_addr, img_size, img_pages, size, data_addr, oob_addr;
	unsigned int page;
	int ret;
	char *err;
	u_char *data_buf, *oob_buf_rd, *oob_buf_wr;
	nand_info_t *nand;
	mtd_oob_ops_t oob_ops;
	nand_erase_options_t erase_ops;
	struct nand_chip *chip;

	if (argc == 5) {
		off_addr = hatoi(argv[1], &err);
		img_size = hatoi(argv[2], &err);
		data_addr = hatoi(argv[3], &err);
		oob_addr = hatoi(argv[4], &err);

		nand = &nand_info[nand_curr_device];
		off_addr &= ~(nand->writesize - 1);
		img_pages = img_size/nand->writesize;

		/* Initialize the read/write buffer */
		data_buf = (u_char *)data_addr;
		oob_buf_rd = (u_char *)oob_addr;

		memset(&oob_ops, 0, sizeof(oob_ops));
		oob_ops.len = nand->writesize;
		oob_ops.ooblen = nand->oobsize;
		oob_ops.mode = MTD_OOB_RAW;

		/* Read the data and oob */
		off = off_addr;
		size = 0;
		while (size < img_size) {
			/* skip the  bad blocks */
			while(nand->block_isbad(nand, (loff_t)off))
					off += (nand->writesize * NAND_NUM_PAGES_PER_BLOCK);

			/* Read the data and oob of the page */
			oob_ops.datbuf = data_buf;
			oob_ops.oobbuf = oob_buf_rd; /* must exist, but oob data will be appended to oob_ops.datbuf */
			ret = nand->read_oob(nand, (loff_t)off, &oob_ops);
			if (ret < 0) {
				printf("Error (%d) reading page %08lx\n", ret, off);
				return -1;
			}

			/* Copy the oob data to oob_buf_rd */
			memcpy(oob_buf_rd, &data_buf[nand->writesize], nand->oobsize);

			/* Increment one page */
			data_buf += nand->writesize;
			oob_buf_rd += nand->oobsize;
			size += nand->writesize;
			off += nand->writesize;
		}

#if 1
		/* Erase the blocks */
		memset(&erase_ops, 0, sizeof(erase_ops));
		erase_ops.offset = off_addr;
		erase_ops.length = img_size;
		erase_ops.spread = 1;
		erase_ops.quiet = 1;
		ret = nand_erase_opts(nand, &erase_ops);
		if (ret < 0) {
			printf("Error (%d) erase off %08lx\n", ret, off_addr);
			return -1;
		}

		/* Write raw data and raw oob to NAND */
		off = off_addr;
		size = 0;
		chip = nand->priv;

		/* Initialize the write buffer */
		data_buf = (u_char *)data_addr;
		oob_buf_wr = (u_char *)oob_addr;

		while (size < img_size) {
			/* skip the  bad blocks */
			while(nand->block_isbad(nand, (loff_t)off))
				off += (nand->writesize * NAND_NUM_PAGES_PER_BLOCK);

			/* Write a raw page and oob */
			chip->state = FL_WRITING; /* Get the device */
			chip->ops.len = nand->writesize;
			chip->ops.datbuf = data_buf;
			memcpy(chip->oob_poi, oob_buf_wr, nand->oobsize);
			page = (int)(off >> chip->page_shift) & chip->pagemask;
			ret = chip->write_page(nand, chip, chip->ops.datbuf, page, 0, 1);
			if (ret < 0) {
				printf("Error (%d) write page off %08lx\n", ret, off);
				return -1;
			}
			chip->select_chip(nand, -1);	/* De-select the NAND device */

			/* Increment one page */
			data_buf += nand->writesize;
			oob_buf_wr += nand->oobsize;
			size += nand->writesize;
			off += nand->writesize;
		}
#endif
	} else {
		goto factory_image_cmd_usage;
	}
	return 0;

factory_image_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	facimg,	5,	1,	do_factory_image,
	"Read nand page data and oob data in raw format to memory",
	"<off in hex> <size in hex> <data_addr in hex> <oob_addr in hex>\n"
	"    read 'size' bytes starting at offset 'off'\n"
	"    to memory address 'data_addr' and 'oob_addr', skipping bad blocks.\n"
);
