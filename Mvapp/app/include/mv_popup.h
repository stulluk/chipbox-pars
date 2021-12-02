#ifndef _POPUP_MOUNT_H_
#define _POPUP_MOUNT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	POPUP_STATUS_UNKNOWN      		= 0,
	POPUP_STATUS_WARNING_OPEN  		= 1,
	POPUP_STATUS_WARNING_CLOSE 		= 2,
	POPUP_STATUS_REPORT_OPEN   		= 3,
	POPUP_STATUS_REPORT_CLOSE 		= 4,
	POPUP_STATUS_ATTENTION_OPEN		= 5,
	POPUP_STATUS_ATTENTION_CLOSE 	= 6
} PopupStatus_t;

int PopUp_Init( void (*pfNotifier)( PopupStatus_t status ) );
PopupStatus_t PopUp_GetStatus( void );
#ifdef __cplusplus
}
#endif
#endif
