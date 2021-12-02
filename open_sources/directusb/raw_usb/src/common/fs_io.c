
#include <types.h>
#include <fat.h>

int fs_cd(const char *path)
{
	/* not implemented. */

	return 0;
}

int fs_list(const char *dir, void (*proc)(const char *fsname, int sz, char attr, void *context))
{
	block_dev_desc_t *dev_desc = NULL;
	
	dev_desc = get_dev("usb", 0);
	if (NULL == dev_desc)
		return 1;

	if (fat_register_device(dev_desc, 1) != 0) {
		printf("\n** Unable to use %s %d:%d for fatls **\n", "usb", 0, 0);
		return 1;
	}

	if (file_fat_ls(dir, proc) != 0)
		return 1;

	return 0;
}

int fs_read(const char *fsname, void *buffer, unsigned long maxsize)
{
	long size;
	block_dev_desc_t *dev_desc = NULL;

	dev_desc = get_dev("usb", 0);
	if (NULL == dev_desc)
		return 1;

	if (fat_register_device(dev_desc, 1) != 0) {
		printf("\n** Unable to use %s %d:%d for fatls **\n", "usb", 0, 0);
		return 1;
	}

	size = file_fat_read(fsname, (unsigned char *) buffer, maxsize);
	
	if (-1 == size)
		return 1;

	return size;
}

