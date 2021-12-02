
#ifndef __FS_IO_H__
#define __FS_IO_H__

int usb_start();
int usb_stop();

int fs_list(const char *dir, void (*proc)(const char *fsname, int sz, char attr, void *context));
int fs_read(const char *fsname, void *buffer, unsigned long maxsize);

typedef enum 
{
   EVT_CONNECT 	  = 0x01,
   EVT_DISCONNECT = 0x02,
   EVT_ERROR      = 0x10
} USBEVT_TYPE;

typedef void (* event_proc)(void *puser, int events);

int usb_register_event(event_proc func, void *puser, int evt_type);

#endif

/* EOF */

