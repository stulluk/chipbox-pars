#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include "mv_popup.h"
#include "cs_app_common.h"
#include "csmpr_usb.h"
#include "userdefine.h"
#include "casdrv_def.h"

//=============================================================================
static PopupStatus_t 	lv_PopupConStatus = POPUP_STATUS_UNKNOWN;
static ePopupIndex		lv_PopupKind = MV_WINDOW_UNKNOWN;
static PopupStatus_t 	lv_CAS_PopupConStatus = POPUP_STATUS_UNKNOWN;
static ePopupIndex		lv_CAS_PopupKind = MV_WINDOW_UNKNOWN;
static pthread_t  		hPopupTaskHandle;
static pthread_t  		hCAS_PopupTaskHandle;

static void (*lv_pfNotifier)( PopupStatus_t status ) = NULL;
static void (*lv_pfCAS_Notifier)( PopupStatus_t status ) = NULL;

extern U8	CasDrvGetCardStatus(U8 slot);
//=============================================================================

void *PopUp_Task( void *param );
void *CAS_PopUp_Task( void *param );

int PopUp_Init( void (*pfNotifier)( PopupStatus_t status ) )
{
	lv_pfNotifier = pfNotifier;
	pthread_create( &hPopupTaskHandle, NULL, PopUp_Task, NULL );
	return( 0 );	
}

void *PopUp_Task( void *param )
{
	static U8	BaforeUsbStatus = USB_STATUS_UNKNOWN;
	static U8	CurrentUsbStatus = USB_STATUS_UNKNOWN;
	
	HWND 		hwnd;
	
	while(1)
	{
		switch( lv_PopupConStatus )
		{
			case POPUP_STATUS_UNKNOWN:
				CurrentUsbStatus = UsbCon_GetStatus();
				if ( BaforeUsbStatus != CurrentUsbStatus )
				{
					switch(CurrentUsbStatus)
					{
						case USB_STATUS_DISCONNECT:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_DISCONNECT;
							break;
						case USB_STATUS_CONNECTED:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_CONNECT;
							break;
						case USB_STATUS_MOUNTED:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_MOUNT;
							break;
						case USB_STATUS_MOUNT_FAILED:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_MOUNT_FAIL;
							break;
						case USB_STATUS_UNMOUNTED:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_UNMOUNT;
							break;
						case USB_STATUS_UNMOUNT_FAILED:
							lv_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_PopupKind = MV_WINDOW_USB_UNMOUNT_FAIL;
							break;
						default:
							break;
					}
				}
				BaforeUsbStatus = CurrentUsbStatus;
				break;
			case POPUP_STATUS_WARNING_OPEN:
				break;
			case POPUP_STATUS_WARNING_CLOSE:
				break;
			case POPUP_STATUS_REPORT_OPEN:
				{
					//const char* spCaption;
					
					hwnd = GetActiveWindow();
					//spCaption = GetWindowCaption (hwnd);
					//printf("======= %s ==========\n", spCaption);
					if ( !CFG_Factory_Mode )
						MV_Warning_Report_Window_Open( hwnd, lv_PopupKind );

					//usleep( 1000*1000 );
					lv_PopupConStatus = POPUP_STATUS_REPORT_CLOSE;
					lv_PopupKind = POPUP_STATUS_UNKNOWN;					
				}
				break;
			case POPUP_STATUS_REPORT_CLOSE:
				{
					hwnd = GetActiveWindow();

					if ( !CFG_Factory_Mode )
						MV_Warning_Report_Window_Close( hwnd );

					lv_PopupConStatus = POPUP_STATUS_UNKNOWN;
					lv_PopupKind = POPUP_STATUS_UNKNOWN;
					
					BroadcastMessage (MSG_USB_MSG, CurrentUsbStatus, 0);
				}
				break;
			case POPUP_STATUS_ATTENTION_OPEN:
				break;
			case POPUP_STATUS_ATTENTION_CLOSE:
				break;	
		}
		usleep( 1000*1000 );
	};

	return( param );
}

int CAS_PopUp_Init( void (*pfNotifier)( PopupStatus_t status ) )
{
	lv_pfCAS_Notifier = pfNotifier;
	pthread_create( &hCAS_PopupTaskHandle, NULL, CAS_PopUp_Task, NULL );
	return( 0 );	
}

void *CAS_PopUp_Task( void *param )
{
	static U8		Bafore_CAS_Status = SMART_EVENTNONE;
	static U8		Current_CAS_Status = SMART_EVENTNONE;	
	HWND 			hwnd;
	
	while(1)
	{
		switch( lv_CAS_PopupConStatus )
		{
			case POPUP_STATUS_UNKNOWN:
				Current_CAS_Status = CasDrvGetCardStatus(0);
				if ( Bafore_CAS_Status != Current_CAS_Status )
				{
					switch(Current_CAS_Status)
					{
						case SMART_CARDINS:
							lv_CAS_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_CAS_PopupKind = MV_WINDOW_SMART_CARD_INSERT;
							break;
						case SMART_CARDREM:
							lv_CAS_PopupConStatus = POPUP_STATUS_REPORT_OPEN;
							lv_CAS_PopupKind = MV_WINDOW_SMART_CARD_REMOVE;
							break;
						default:
							break;
					}
				}
				Bafore_CAS_Status = Current_CAS_Status;
				break;
			case POPUP_STATUS_WARNING_OPEN:
				break;
			case POPUP_STATUS_WARNING_CLOSE:
				break;
			case POPUP_STATUS_REPORT_OPEN:
				{
					//const char* spCaption;
					
					hwnd = GetActiveWindow();
					//spCaption = GetWindowCaption (hwnd);
					// printf("======= %d ==========\n", lv_CAS_PopupConStatus);
					if ( !CFG_Factory_Mode )
						MV_Warning_Report_Window_Open( hwnd, lv_CAS_PopupKind );

					lv_CAS_PopupConStatus = POPUP_STATUS_REPORT_CLOSE;
					lv_CAS_PopupKind = POPUP_STATUS_UNKNOWN;					
				}
				break;
			case POPUP_STATUS_REPORT_CLOSE:
				{
					hwnd = GetActiveWindow();

					if ( !CFG_Factory_Mode )
						MV_Warning_Report_Window_Close( hwnd );

					// printf("======= %d ==========\n", lv_CAS_PopupConStatus);
					lv_CAS_PopupConStatus = POPUP_STATUS_UNKNOWN;
					lv_CAS_PopupKind = POPUP_STATUS_UNKNOWN;
				}
				break;
			case POPUP_STATUS_ATTENTION_OPEN:
				break;
			case POPUP_STATUS_ATTENTION_CLOSE:
				break;	
		}
		usleep( 1000*1000 );
	};

	return( param );
}

