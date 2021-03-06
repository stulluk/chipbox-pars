/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <types.h>
#include <ide.h>
#include <part.h>

#undef	PART_DEBUG

#ifdef	PART_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* ------------------------------------------------------------------------- */
/*
 * reports device info to the user
 */
void dev_print(block_dev_desc_t * dev_desc)
{
#ifdef CONFIG_LBA48
	uint64_t lba512;	/* number of blocks if 512bytes block size */
#else
	lbaint_t lba512;
#endif

	if (dev_desc->type == DEV_TYPE_UNKNOWN) {
		puts("not available\n");
		return;
	}
	if (dev_desc->if_type == IF_TYPE_SCSI) {
		printf("(%d:%d) ", dev_desc->target, dev_desc->lun);
	}
	if (dev_desc->if_type == IF_TYPE_IDE) {
		printf("Model: %s Firm: %s Ser#: %s\n", dev_desc->vendor, dev_desc->revision, dev_desc->product);
	}
	else {
		printf("Vendor: %s Prod.: %s Rev: %s\n", dev_desc->vendor, dev_desc->product, dev_desc->revision);
	}
	puts("            Type: ");
	if (dev_desc->removable)
		puts("Removable ");
	switch (dev_desc->type & 0x1F) {
	case DEV_TYPE_HARDDISK:
		puts("Hard Disk");
		break;
	case DEV_TYPE_CDROM:
		puts("CD ROM");
		break;
	case DEV_TYPE_OPDISK:
		puts("Optical Device");
		break;
	case DEV_TYPE_TAPE:
		puts("Tape");
		break;
	default:
		printf("# %02X #", dev_desc->type & 0x1F);
		break;
	}
	puts("\n");
	if ((dev_desc->lba * dev_desc->blksz) > 0L) {
		ulong mb, mb_quot, mb_rem, gb, gb_quot, gb_rem;
		lbaint_t lba;

		lba = dev_desc->lba;

		lba512 = (lba * (dev_desc->blksz / 512));
		mb = (10 * lba512) / 2048;	/* 2048 = (1024 * 1024) / 512 MB */
		/* round to 1 digit */
		mb_quot = mb / 10;
		mb_rem = mb - (10 * mb_quot);

		gb = mb / 1024;
		gb_quot = gb / 10;
		gb_rem = gb - (10 * gb_quot);
#ifdef CONFIG_LBA48
		if (dev_desc->lba48)
			printf("            Supports 48-bit addressing\n");
#endif
#if defined(CFG_64BIT_LBA) && defined(CFG_64BIT_VSPRINTF)
		printf("            Capacity: %ld.%ld MB = %ld.%ld GB (%qd x %ld)\n",
		       mb_quot, mb_rem, gb_quot, gb_rem, lba, dev_desc->blksz);
#else
		printf("            Capacity: %ld.%ld MB = %ld.%ld GB (%ld x %ld)\n",
		       mb_quot, mb_rem, gb_quot, gb_rem, (ulong) lba, dev_desc->blksz);
#endif
	}
	else {
		puts("            Capacity: not available\n");
	}
}

void init_part(block_dev_desc_t * dev_desc)
{
	if (test_part_dos(dev_desc) == 0) {
		dev_desc->part_type = PART_TYPE_DOS;
		return;
	}
}

int get_partition_info(block_dev_desc_t * dev_desc, int part, disk_partition_t * info)
{
	switch (dev_desc->part_type) {
	case PART_TYPE_DOS:
		if (get_partition_info_dos(dev_desc, part, info) == 0) {
			PRINTF("## Valid DOS partition found ##\n");
			return (0);
		}
		break;
	default:
		break;
	}
	return (-1);
}

static void print_part_header(const char *type, block_dev_desc_t * dev_desc)
{
	puts("\nPartition Map for ");
	switch (dev_desc->if_type) {
	case IF_TYPE_IDE:
		puts("IDE");
		break;
	case IF_TYPE_SCSI:
		puts("SCSI");
		break;
	case IF_TYPE_ATAPI:
		puts("ATAPI");
		break;
	case IF_TYPE_USB:
		puts("USB");
		break;
	case IF_TYPE_DOC:
		puts("DOC");
		break;
	default:
		puts("UNKNOWN");
		break;
	}
	printf(" device %d  --   Partition Type: %s\n\n", dev_desc->dev, type);
}

void print_part(block_dev_desc_t * dev_desc)
{

	switch (dev_desc->part_type) {
	case PART_TYPE_DOS:
		PRINTF("## Testing for valid DOS partition ##\n");
		print_part_header("DOS", dev_desc);
		print_part_dos(dev_desc);
		return;
	}
	puts("## Unknown partition table\n");
}
