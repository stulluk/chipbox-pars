/*
 * include/linux/superhyway.h
 *
 * SuperHyway Bus definitions
 *
 * Copyright (C) 2004, 2005  Paul Mundt <lethal@linux-sh.org>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#ifndef __LINUX_SUPERHYWAY_H
#define __LINUX_SUPERHYWAY_H

#include <linux/device.h>

/*
 * SuperHyway IDs
 */
#define SUPERHYWAY_DEVICE_ID_SH5_DMAC	0x0183

struct vcr_info {
	__u8	perr_flags;	/* P-port Error flags */
	__u8	merr_flags;	/* Module Error flags */
	__u16	mod_vers;	/* Module Version */
	__u16	mod_id;		/* Module ID */
	__u8	bot_mb;		/* Bottom Memory block */
	__u8	top_mb;		/* Top Memory block */
};

struct superhyway_device_id {
	unsigned int id;
	unsigned long driver_data;
};

struct superhyway_device;
extern struct bus_type superhyway_bus_type;

struct superhyway_driver {
	char *name;

	const struct superhyway_device_id *id_table;
	struct device_driver drv;

	int (*probe)(struct superhyway_device *dev, const struct superhyway_device_id *id);
	void (*remove)(struct superhyway_device *dev);
};

#define to_superhyway_driver(d)	container_of((d), struct superhyway_driver, drv)

struct superhyway_device {
	char name[32];

	struct device dev;

	struct superhyway_device_id id;
	struct superhyway_driver *drv;

	struct resource resource;
	struct vcr_info vcr;
};

#define to_superhyway_device(d)	container_of((d), struct superhyway_device, dev)

#define superhyway_get_drvdata(d)	dev_get_drvdata(&(d)->dev)
#define superhyway_set_drvdata(d,p)	dev_set_drvdata(&(d)->dev, (p))

extern int superhyway_scan_bus(void);

/* drivers/sh/superhyway/superhyway.c */
int superhyway_register_driver(struct superhyway_driver *);
void superhyway_unregister_driver(struct superhyway_driver *);
int superhyway_add_device(unsigned int, unsigned long, unsigned long long);

/* drivers/sh/superhyway/superhyway-sysfs.c */
extern struct device_attribute superhyway_dev_attrs[];

#endif /* __LINUX_SUPERHYWAY_H */
