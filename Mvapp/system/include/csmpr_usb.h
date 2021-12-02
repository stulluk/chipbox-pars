#ifndef _USB_MOUNT_H_
#define _USB_MOUNT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#define			PARTITION_NUM 	10

typedef enum 
{
	USB_STATUS_UNKNOWN      	= 0,
	USB_STATUS_DISCONNECT   	= 1,
	USB_STATUS_CONNECTED    	= 2,
	USB_STATUS_MOUNTED      	= 3,
	USB_STATUS_MOUNT_FAILED 	= 4,
	USB_STATUS_UNMOUNTED		= 5,
	USB_STATUS_UNMOUNT_FAILED 	= 6
} UsbStatus_t;

typedef struct
{
	char		acPartition_Type[16];
	char		acPartition_Data[32];
	char		acPartition_Dir[256];
} stPartition;

struct f_size
{
    long blocks;
    long avail; 
};

typedef struct _mountinfo 
{
    FILE *fp;                // 파일 스트림 포인터    
    char devname[80];        // 장치 이름
    char mountdir[80];        // 마운트 디렉토리 이름
    char fstype[12];        // 파일 시스템 타입
    struct f_size size;        // 파일 시스템의 총크기/사용율 
} MOUNTP;

stPartition		stPartition_data[PARTITION_NUM];
int				acUsbDevCount;

int UsbCon_Init( void (*pfNotifier)( UsbStatus_t status ) );
UsbStatus_t UsbCon_GetStatus( void );
void *UsbCon_Task( void *param );
int  UsbCon_DetectConnection( void );
int  UsbCon_Mount( void );
int  UsbCon_Umount( void );
int  UsbCon_GetUsbDevID( void );
char *UsbGetDevFileName( void );
MOUNTP *dfopen(void);
MOUNTP *dfget(MOUNTP *MP);
int dfclose(MOUNTP *MP);
int MV_Get_USB_Format_Type(int i);

#ifdef __cplusplus
}
#endif
#endif
