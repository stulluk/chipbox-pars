
#include "fs_io.h"

void list_proc(const char *fsname, int sz, char attr, void *context)
{
	printf(">>> [%c], %ld, %s \n", attr, sz, fsname); 
}

int main(int argc, char *argv[])
{
	long sz, ii;
	
	get_usb_bus();
	
	timer_init(); /* maybe you do not need to call it */
	
	for (ii=0;;ii++) {
	usb_start();
	fs_list("/", list_proc);
	sz = fs_read(argc > 1 ? argv[1] : "/vlc-0.8.6i.tar.bz2", (void*)0x100000, 0);
	printf(" \n %d bytes were read to 0x100000. (%ld)\n", sz, ii);
	usb_stop();
	}
	free_usb_bus();

	return 0;
}

