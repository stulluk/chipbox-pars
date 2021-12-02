#include "linuxos.h"
#include "database.h"
#include "eit_engine.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mv_menu_ctr.h"
#include "mv_gui_interface.h"
#include "fe_mngr.h"
#include "csmpr_usb.h"
#include "csinstall.h"
#include "ui_common.h"
#include "casapi.h"
#include "ci_api.h"

enum
{	
	CSAPP_FAC_LIVE = 0,
	CSAPP_FAC_VIDEO,
	CSAPP_FAC_AUDIO,
	CSAPP_FAC_SYSTEM,
	CSAPP_FAC_POWER,
	CSAPP_FAC_LEFT,
	CSAPP_FAC_UP,
	CSAPP_FAC_RIGHT,
	CSAPP_FAC_DOWN,
	CSAPP_FAC_OK,
	CSAPP_FAC_MENU,
	CSAPP_FAC_EXIT,
	CSAPP_FAC_MAX
};

enum
{
	CSAPP_FS_RAM = 0,	
	CSAPP_FS_FLASH,
	CSAPP_FS_TUNER,
	CSAPP_FS_232,
	CSAPP_FS_MAX
};

enum
{
	CSAPP_VT_RGB_OFF = 0,	
	CSAPP_VT_RGB_ON,
	CSAPP_VT_SCART_4_3,
	CSAPP_VT_SCART_16_9,
	CSAPP_VT_SCART_OFF,
	CSAPP_VT_SCART_ON,
	CSAPP_VT_MAX
};

enum
{
	CSAPP_AT_VOL_MAX = 0,	
	CSAPP_AT_VOL_MIN,
	CSAPP_AT_MAX
};

char FAC_Setting_Str[CSAPP_FAC_MAX][100] = {							
							"Live Signal Check",
							"Video Function Check",
							"Audio Function Check",
							"System Check",
							"Power",
							"Left",
							"Up",
							"Right",
							"Down",
							"Ok",
							"Menu",
							"Exit"
						};

static CSAPP_Applet_t	CSApp_Factory_Applets;
static U8 				Factory_Focus_Item = CSAPP_FAC_LIVE;
static U16				Current_Item = 0;
static U32				ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static BITMAP			tmp_bmp;
static U8				u8Video_Test_state = 0;
static U8				u8Audio_Test_state = 0;
static U8				u8Channel_Kind = 0;
static U16				u16CAS_ID = 0xFFFF;

static BOOL				Power_State = UNFOCUS;
static BOOL				Left_State = UNFOCUS;
static BOOL				Up_State = UNFOCUS;
static BOOL				Right_State = UNFOCUS;
static BOOL				Down_State = UNFOCUS;
static BOOL				Ok_State = UNFOCUS;
static BOOL				Menu_State = UNFOCUS;
static BOOL				Exit_State = UNFOCUS;

#define MV_SUB_MENU_X			( MV_INSTALL_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) ) + 20
#define MV_SUB_MENU_Y			( MV_INSTALL_MENU_Y )
#define MV_SUB_MENU_DX			( (ScreenWidth - MV_INSTALL_MENU_X*2) / 2 )
#define MV_SUB_MENU_DY			( MV_INSTALL_MENU_BAR_H * 15 )
#define MV_KEY_POWER_MENU_X		MV_INSTALL_MENU_X
#define MV_KEY_POWER_MENU_Y		( MV_INSTALL_MENU_Y + CSAPP_FAC_POWER * MV_INSTALL_MENU_BAR_H * 2 )

static int Factory_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
static void MV_USB_Test(HWND hwnd);
static void MV_Network_Test(HDC hdc, BOOL s_Status);
static void MV_CAS_Test(HWND hwnd, U16 tmpCasId);
#ifdef SUPPORT_CI
static void MV_CI_Test(HWND hwnd, U16 tmpCasId);
//extern U8 MW_CI_Get_Cam_Information(U8 *infoData);
#endif

static void MV_System_Test_Menu(HDC hdc)
{
	//U32				y_position = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP );
	U32				y_position = MV_INSTALL_MENU_Y + ( ( ( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 3 );
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "RS-232 Test");
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );

	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X + MV_SUB_MENU_DX - 100), ScalerHeigthPixel( y_position ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );		
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + MV_SUB_MENU_DX - 100), ScalerHeigthPixel( y_position + 4 ), "Test ...");

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );

	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X + MV_SUB_MENU_DX - 100), ScalerHeigthPixel( y_position ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );	
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + MV_SUB_MENU_DX - 100), ScalerHeigthPixel( y_position + 4 ), "OK");	
}

static void MV_Live_Test(HDC hdc)
{
	U32					y_position = MV_INSTALL_MENU_BAR_H;
	MV_stSatInfo 		Temp_SatInfo;
	MV_stTPInfo 		Temp_TPInfo;
	MV_stServiceInfo 	ServiceData;
	char				Temp_str[100];

	MV_DB_GetServiceDataByIndex( &ServiceData, CS_DB_GetCurrentServiceIndex() );
	MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, ServiceData.u16ChIndex);
	MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, ServiceData.u16ChIndex);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );

/*--------------------------- Satellite Name & 22Khz ---------------------------------*/
	memset( Temp_str, 0x00, 100 );
	
	if ( Temp_SatInfo.u16Tone22K )
		sprintf( Temp_str, "%s, LNB Freq(%d), 22Khz( %s )", Temp_SatInfo.acSatelliteName, Temp_SatInfo.u16LocalFrequency, CS_MW_LoadStringByIdx(CSAPP_STR_ON));
	else
		sprintf( Temp_str, "%s, LNB Freq(%d), 22Khz( %s )", Temp_SatInfo.acSatelliteName, Temp_SatInfo.u16LocalFrequency, CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 0), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 0 + 4 ), Temp_str);

/*--------------------------- LNB Frequancy & TP Data ---------------------------------*/
	memset( Temp_str, 0x00, 100 );

	sprintf( Temp_str, "Freq(%d) , Symbol(%d)", Temp_TPInfo.u16TPFrequency, Temp_TPInfo.u16SymbolRate);
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 1), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 1 + 4 ), Temp_str);

/*--------------------------- LNB Frequancy & TP Data ---------------------------------*/
	memset( Temp_str, 0x00, 100 );

	if ( Temp_TPInfo.u8Polar_H )
	{
		if ( Temp_SatInfo.u16Control12V )
			sprintf( Temp_str, "Hor/Ver (%s) , 0/12 (%s)", "H", "On");
		else
			sprintf( Temp_str, "Hor/Ver (%s) , 0/12 (%s)", "H", "Off");
	} else {
		if ( Temp_SatInfo.u16Control12V )
			sprintf( Temp_str, "Hor/Ver (%s) , 0/12 (%s)", "V", "On");
		else
			sprintf( Temp_str, "Hor/Ver (%s) , 0/12 (%s)", "V", "Off");
	}
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 2), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 2 + 4 ), Temp_str);

/*--------------------------- Channel PID ---------------------------------*/
	memset( Temp_str, 0x00, 100 );

	sprintf( Temp_str, "VPID(%d) , APID(%d) , PPID(%d)", ServiceData.u16VideoPid, ServiceData.u16AudioPid, ServiceData.u16PCRPid);
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 3), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 3 + 4 ), Temp_str);

/*--------------------------- Channel name ---------------------------------*/
	memset( Temp_str, 0x00, 100 );

	sprintf( Temp_str, "%s", ServiceData.acServiceName);
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 4), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( MV_INSTALL_MENU_Y + y_position * 4 + 4 ), Temp_str);
	
	switch(u8Channel_Kind)
	{
		case 1:
			//MV_USB_Test(hwnd);
			break;
		case 2:
			MV_Network_Test(hdc, TRUE);
			break;
		case 3:
			//MV_CI_Test(hwnd);
			break;
		default:
			break;
	}
}

static void MV_Video_Test(HDC hdc)
{
	U32					y_position = MV_INSTALL_MENU_Y + ( ( ( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 1 );
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );

	switch( u8Video_Test_state )
	{
		case CSAPP_VT_RGB_OFF:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "RGB Off");
			break;
		case CSAPP_VT_RGB_ON:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "RGB On");
			break;
		case CSAPP_VT_SCART_4_3:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Scart 4:3");
			break;
		case CSAPP_VT_SCART_16_9:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Scart 16:9");
			break;
		case CSAPP_VT_SCART_OFF:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Scart Off - TV Mode");
			break;
		case CSAPP_VT_SCART_ON:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Scart On - SAT Mode");
			break;
		default:
			break;
	}
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_SUB_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( y_position ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
}

static void MV_Audio_Test(HDC hdc)
{
	U32					y_position = MV_INSTALL_MENU_Y + ( ( ( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 2 );
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );

	switch( u8Audio_Test_state )
	{
		case CSAPP_AT_VOL_MAX:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Maximum Volume");
			break;
		case CSAPP_AT_VOL_MIN:
			MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Minimum Volume");
			break;
		default:
			break;
	}
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_SUB_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( y_position ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
}

static void MV_USB_Test(HWND hwnd)
{
	HDC				hdc;
	U32				y_position = MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 6 );
	
	hdc=BeginPaint(hwnd);
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	if ( UsbCon_GetStatus() == USB_STATUS_UNKNOWN || UsbCon_GetStatus() == USB_STATUS_UNMOUNT_FAILED )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Insert USB Device");
	}
	else if ( UsbCon_GetStatus() == USB_STATUS_CONNECTED )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "USB Device Connecting");
	}
	else if ( UsbCon_GetStatus() == USB_STATUS_MOUNTED )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "USB Mount OK");
	}
	else if ( UsbCon_GetStatus() == USB_STATUS_MOUNT_FAILED )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "USB Mount FAIL !!!!");
	}
	else if ( UsbCon_GetStatus() == USB_STATUS_UNMOUNTED )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "USB UnMount OK");
	}
	else if ( UsbCon_GetStatus() == USB_STATUS_DISCONNECT )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "USB Disconnecting");
	}
	
	EndPaint(hwnd,hdc);
}

static void MV_Network_Test(HDC hdc, BOOL s_Status)
{
	U32				y_position = MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 7 );
	char 			ShellCommand[64];
	int  			Result;

	if ( s_Status == TRUE )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Get DHCP .. Waiting ..");
		
		sprintf(ShellCommand,"udhcpc -n -t 5 -T 2");
		Result = system( ShellCommand );
		
		SetBkMode(hdc,BM_TRANSPARENT);
		if ( Result != 0 )
		{
			SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
			MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		}
		else
		{
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		}
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );

		if ( Result != 0 )
		{
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "DHCP Fail");
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( y_position ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
		} else 
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "DHCP OK !!");
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Test Ready ..");
	}
}

static void MV_CAS_Test(HWND hwnd, U16 tmpCasId)
{
	HDC				hdc;
	U32				y_position = MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 8 );
//	U8				u8Card_Slot = 0;
//	U8				u8atr[33];
	U8  			cardName[50];
	U32				u32Length = 0;
	U8				u8Temp_str[100];

	//printf("\n========== MV_CI_TEST ============\n");
	hdc=BeginPaint(hwnd);

	//CasDrvSmartGetAtr(u8Card_Slot, u8atr, &u32Length);
	if ( tmpCasId )
	{
		CasGetSystemInfo (tmpCasId, cardName);
		u32Length = tmpCasId;
	} else {
		u32Length = 0;
	}
	
	memset(u8Temp_str, 0x00, 100);
	
	if ( u32Length == 0 )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Smart Card Insert Please");
	}
	else
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
		//sprintf(u8Temp_str, "Card Test OK !!!");
		sprintf(u8Temp_str, "Card Num : 0x%04X , Name : %s", tmpCasId, cardName);
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), u8Temp_str);
	}
	
	EndPaint(hwnd,hdc);
}

#ifdef SUPPORT_CI
static void MV_CI_Test(HWND hwnd, U16 tmpCasId)
{
	HDC				hdc;
	U32				y_position = MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 9 );
	unsigned char	cardName[80];
	U32				u32Length = 0;

	hdc=BeginPaint(hwnd);

	u32Length = tmpCasId;
	
	if ( u32Length == 0 )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "CAM Insert Please");
	}
	else
	{
		memset(cardName, 0x00, 80);
		MW_CI_Get_Cam_Information(cardName);
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );

		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		if ( strlen(cardName) == 0 )
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "CAM Initial");
		else
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), cardName);
	}
	
	EndPaint(hwnd,hdc);
}
#endif

#if 0
static void MV_Fac_End(HWND hwnd)
{
	HDC				hdc;
	U32				y_position = MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 7 );

	hdc=BeginPaint(hwnd);
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
	
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( y_position ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( y_position + 4 ), "Are You Test Finished ?");
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_SUB_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( y_position ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
	
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_SUB_MENU_X), ScalerHeigthPixel( ( y_position + (( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP )) ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_SUB_MENU_X + 10), ScalerHeigthPixel( ( y_position + (( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP )) + 4 ), "OK : Ok , EXIT : cancel");
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_SUB_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( ( y_position + (( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP )) ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
	
	EndPaint(hwnd,hdc);
}
#endif

MV_CFG_RETURN MV_Fac_End_cfg(BOOL b8Set_Kind)
{
	FILE* 	fp;
	FILE*	fp_temp;
    char 	tempSection [CFG_MAX_COL + 2];
	char	Temp[100];
	char 	ShellCommand[64];

	if (!(fp = fopen(CFG_FILE, "r")))
	{
		printf("\n File open Error\n");
        return CFG_NOFILE;
	}
	if (!(fp_temp = fopen(TEMP_FILE, "w+a")))
	{
		printf("\n Temp File create Error\n");
		fclose (fp);
        return CFG_NOFILE;
	}

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);
		
        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			printf("\n File read Error\n");
			fclose (fp);			
			fclose (fp_temp);
			return CFG_READ_FAIL;
        }

		MV_Parser_CFG_Value(Temp, tempSection);

		if ( MV_CFG_Namecheck(tempSection) == MVCFG_FAC_TEST )
		{
			//printf("\n==== Write factory_mode --> off \n\n");
			if ( b8Set_Kind == KIND_OFF ) /* b8Set_Kind == KIND_OFF ==> on(Test)->off(Normal) */
				fputs("factory_mode=off\n", fp_temp);
			else /* b8Set_Kind == KIND_ON ==> off(Normal)->on(Test) */
				fputs("factory_mode=on\n", fp_temp);
			break;
		} else {
			//printf("%s Write to %s\n", tempSection, TEMP_FILE);
			fputs(tempSection, fp_temp);
		}
    }
	
	fclose (fp);
	fclose (fp_temp);

	sprintf(ShellCommand, "rm %s", CFG_FILE);
	//printf("\n ====== %s =======>\n", ShellCommand);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;

	sprintf(ShellCommand, "cp %s %s", TEMP_FILE, CFG_FILE);
	//printf("\n ====== %s =======>\n", ShellCommand);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;

	sprintf(ShellCommand, "rm %s", TEMP_FILE);
	//printf("\n ====== %s =======>\n", ShellCommand);
	if ( system( ShellCommand ) )
		return CFG_READ_FAIL;
	
	return CFG_OK;
}


void MV_Draw_FACMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP ) * esItem;
	int		y_gap2 = 0;
	int 	y_gap3 = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP ) * CSAPP_FAC_POWER;
	int		x_gap = MV_KEY_POWER_MENU_X;

	if ( esItem < CSAPP_FAC_POWER || esItem > CSAPP_FAC_EXIT )
	{
		if ( esItem > CSAPP_FAC_EXIT )
			y_gap = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP ) * ( CSAPP_FAC_POWER + 1 );
		
		if ( u8Focuskind == MV_FOCUS )
		{
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( y_gap ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + ((ScreenWidth - MV_INSTALL_MENU_X*2) / 2) - ScalerWidthPixel(tmp_bmp.bmWidth) - 10 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(tmp_bmp.bmWidth),ScalerHeigthPixel(tmp_bmp.bmHeight),&tmp_bmp);
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);		
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		}		

		if ( esItem > CSAPP_FAC_EXIT )
		{
			y_gap2 = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP ) * ( CSAPP_FAC_POWER + 1 );
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap2+4), FAC_Setting_Str[esItem]);
		}
		else
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), FAC_Setting_Str[esItem]);
	} else {
		x_gap = x_gap + ( 110 * ( esItem - CSAPP_FAC_POWER ));
		
		switch(esItem)
		{
			case CSAPP_FAC_POWER:
				if ( Power_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_LEFT:
				if ( Left_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_UP:
				if ( Up_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_RIGHT:
				if ( Right_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_DOWN:
				if ( Down_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_OK:
				if ( Ok_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_MENU:
				if ( Menu_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			case CSAPP_FAC_EXIT:
				if ( Exit_State == FOCUS )
				{
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				} else {
					if ( u8Focuskind == MV_FOCUS )
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					} 
					else
					{
						SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
						SetBkMode(hdc,BM_TRANSPARENT);		
						MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(x_gap), ScalerHeigthPixel( y_gap3 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					}
				}			
	
				MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_gap + 10),ScalerHeigthPixel(y_gap3+4), FAC_Setting_Str[esItem]);
				break;
			default:
				break;
		}
	}

	switch(esItem)
	{
		case CSAPP_FAC_LIVE:
			Factory_Focus_Item = CSAPP_FAC_LIVE;
			if ( u8Focuskind == FOCUS )
			{
				tCS_DBU_Service ServiceTriplet;

				CS_MW_PlayServiceByIdx(0, NOT_TUNNING);

				ServiceTriplet.sCS_DBU_ServiceIndex = 0;
				CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));

				CS_DBU_SaveCurrentService(ServiceTriplet);
				
				MV_Live_Test(hdc);

				printf("===LIVE TEST===>>>>>>>>>>\n");
			}
			break;
		case CSAPP_FAC_VIDEO:
			Factory_Focus_Item = CSAPP_FAC_VIDEO;
			if ( u8Focuskind == FOCUS )
				MV_Video_Test(hdc);
			break;
		case CSAPP_FAC_AUDIO:
			Factory_Focus_Item = CSAPP_FAC_AUDIO;
			if ( u8Focuskind == FOCUS )
				MV_Audio_Test(hdc);
			break;
		case CSAPP_FAC_SYSTEM:
			Factory_Focus_Item = CSAPP_FAC_SYSTEM;
			if ( u8Focuskind == FOCUS )
				MV_System_Test_Menu(hdc);
			break;
		default:
			Factory_Focus_Item = esItem;
			break;
	}
}

void MV_Draw_Fac_Unfocus(HWND hwnd, U8 esItem)
{
	U16		PositionY = MV_INSTALL_MENU_Y + ( (MV_INSTALL_MENU_HEIGHT * 2) + MV_INSTALL_MENU_YGAP ) * esItem;
	U16		PositionDY = 0;
	HDC		hdc;

	//printf("\n======================== %d : %d ================================\n", esItem, PositionY);
	if ( esItem == CSAPP_FAC_LIVE )
		PositionDY = MV_INSTALL_MENU_BAR_H * 5;
	else 
		PositionDY = MV_INSTALL_MENU_BAR_H;
	
	hdc=BeginPaint(hwnd);
	MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel( MV_SUB_MENU_X ),ScalerHeigthPixel( PositionY ), ScalerWidthPixel( (ScreenWidth - MV_INSTALL_MENU_X*2) / 2 ),ScalerHeigthPixel( PositionDY ) );

#if 0
	if ( esItem == CSAPP_FAC_LIVE )
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X), ScalerHeigthPixel( MV_INSTALL_MENU_Y + ((( MV_INSTALL_MENU_HEIGHT * 2 ) + MV_INSTALL_MENU_YGAP ) * 6 ) ), ScalerWidthPixel((ScreenWidth - MV_INSTALL_MENU_X*2) / 2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
#endif

	EndPaint(hwnd,hdc);
}

void MV_Draw_FAC_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_FAC_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_FACMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}


CSAPP_Applet_t  CSApp_Factory_Test(void)
{    
	MSG                  			Msg;
	HWND                    		hMainWnd;
	MAINWINCREATE    				CreateInfo;
	int                    			BASE_X, BASE_Y, WIDTH, HEIGHT;
	
	CSApp_Factory_Applets = CSApp_Applet_Error;
	
#ifdef  Screen_1080
	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = 1920;
	HEIGHT = 1080;
#else
	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif
		
	CreateInfo.dwStyle   	= WS_VISIBLE;
	CreateInfo.dwExStyle 	= WS_EX_NONE;
	CreateInfo.spCaption 	= "Factory Test window";
	CreateInfo.hMenu     	= 0;
	CreateInfo.hCursor   	= 0;
	CreateInfo.hIcon     	= 0;
	CreateInfo.MainWindowProc = Factory_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;
	hMainWnd = CreateMainWindow (&CreateInfo);
	
	if (hMainWnd == HWND_INVALID)	return CSApp_Applet_Error;
	
	ShowWindow(hMainWnd, SW_SHOWNORMAL);
	
	while (GetMessage(&Msg, hMainWnd)) 
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	MainWindowThreadCleanup (hMainWnd);
	
	return CSApp_Factory_Applets;
}

static void Check_Front_KeyEnd(HWND hwnd)
{
	HDC		hdc;
	int		i;
	
	if ( 	Power_State == FOCUS 
		&&	Left_State == FOCUS
		&&	Up_State == FOCUS
		&&	Right_State == FOCUS
		&&	Down_State == FOCUS
		&&	Ok_State == FOCUS
		&&	Menu_State == FOCUS
		&&	Exit_State == FOCUS )
	{
		for ( i = 0 ; i < 4 ; i++ )
		{
			FbSendFndDisplay("8888");
			usleep( 300 * 1000 );
			FbSendFndDisplay("    ");
			usleep( 300 * 1000 );
		}
		usleep( 300 * 1000 );
		FbSendFndDisplay("Test");
		
		Current_Item = CSAPP_FAC_LIVE;
		Factory_Focus_Item = CSAPP_FAC_LIVE;
		
		u8Channel_Kind = 0;
		u8Video_Test_state = 0;
		u8Audio_Test_state = 0;
		Power_State = UNFOCUS;
		Left_State = UNFOCUS;
		Up_State = UNFOCUS;
		Right_State = UNFOCUS;
		Down_State = UNFOCUS;
		Ok_State = UNFOCUS;
		Menu_State = UNFOCUS;
		Exit_State = UNFOCUS;
		
		hdc=BeginPaint(hwnd);
		MV_Draw_FAC_MenuBar(hdc);
		EndPaint(hwnd,hdc);
		
		Current_Item = CSAPP_FAC_LIVE;
		Factory_Focus_Item = CSAPP_FAC_LIVE;
		
		MV_USB_Test(hwnd);

		hdc=BeginPaint(hwnd);
		MV_Network_Test(hdc, FALSE);
		EndPaint(hwnd,hdc);
	}
}


//static int msg_event_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam, int* result)
static int Factory_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	int 				temp_with,temp_heigh;
	static RECT  		prc = {0, 0, 0, 0};	   
	HDC					hdc;
	char				Y_Enter[100];
	char				strTemp[100];
	
#ifdef Screen_1080
	temp_with = ScalerWidthPixel(1920);
	temp_heigh= ScalerHeigthPixel(1080);
#else
	temp_with = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	temp_heigh= ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif

	prc.right = temp_with;
	prc.bottom = temp_heigh;

	sprintf(Y_Enter, "%sy_enter.png", CFG_Resource);
	
	switch (message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			//SetTimer(hwnd, CHECK_EPG_UPDATE_ID, SIGNAL_TIMER);
			if (MV_BitmapFromFile(HDC_SCREEN, &tmp_bmp, Y_Enter))
			{
				printf("=============>>>> Enter Image is no Image\n");
			}
			
			FbSendFndDisplay("Test");
			CS_MW_SetNormalWindow();

			{
				tCS_DBU_Service ServiceTriplet;

				CS_MW_PlayServiceByIdx(0, NOT_TUNNING);

				ServiceTriplet.sCS_DBU_ServiceIndex = 0;
				CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));

				CS_DBU_SaveCurrentService(ServiceTriplet);
			}
			u8Channel_Kind = 0;
			u8Video_Test_state = 0;
			u8Audio_Test_state = 0;

			u16CAS_ID = CasGetCurrentCardId(0);
/**********************************************************************************/			
			hdc=BeginPaint(hwnd);
			MV_Draw_FAC_MenuBar(hdc);
			EndPaint(hwnd,hdc);			

			MV_USB_Test(hwnd);
			
			hdc=BeginPaint(hwnd);
			MV_Network_Test(hdc, FALSE);
			EndPaint(hwnd,hdc);
			
			MV_CAS_Test(hwnd, u16CAS_ID);
			
#ifdef SUPPORT_CI
			if ( MW_CI_Get_Cam_Information(strTemp) != 0 )
				MV_CI_Test(hwnd, 1);
			else
				MV_CI_Test(hwnd, 0);
#endif	
			Factory_Focus_Item = CSAPP_FAC_LIVE;
			Current_Item = CSAPP_FAC_LIVE;
/**********************************************************************************/
			break;
		
		case MSG_TIMER:
			if (wparam == CHECK_SIGNAL_TIMER_ID)
				MV_USB_Test(hwnd);
			//else if (wparam == CHECK_EPG_UPDATE_ID)
				//MV_CI_Test(hwnd);
			break;
			
		case MSG_USB_MSG:
			break;

		case MSG_SMART_CARD_INSERT:
			u16CAS_ID = (U16)wparam;
			MV_CAS_Test(hwnd, u16CAS_ID);
			break;

		case MSG_SMART_CARD_REMOVE:
			MV_CAS_Test(hwnd, 0);
			break;
		
		case MSG_PAINT:
			printf("\n\n========== PAINT =================\n\n");
			hdc=BeginPaint(hwnd);
			MV_Draw_FAC_MenuBar(hdc);
			EndPaint(hwnd,hdc);

			Factory_Focus_Item = CSAPP_FAC_LIVE;

			MV_USB_Test(hwnd);
			
			hdc=BeginPaint(hwnd);
			MV_Network_Test(hdc, FALSE);
			EndPaint(hwnd,hdc);
			
			MV_CAS_Test(hwnd, u16CAS_ID);
			
#ifdef SUPPORT_CI
			if ( MW_CI_Get_Cam_Information(strTemp) != 0 )
				MV_CI_Test(hwnd, 1);
			else
				MV_CI_Test(hwnd, 0);
#endif
			return 0;

#ifdef SUPPORT_CI
		case MSG_CI_MMI_UPDATE:
			switch(wparam)
			{
				case MSG_PARAM_CI_Insert_Notify:
					if ( lparam == 5 )
						MV_CI_Test(hwnd, 0);
					else
						MV_CI_Test(hwnd, 1);
					break;
				case MSG_PARAM_CI_App_Info_Changed:
					if ( lparam == 1 )
						MV_CI_Test(hwnd, 1);
					else
						MV_CI_Test(hwnd, 0);
					break;
				default:
					break;
			}
			break;
#endif
			
		case MSG_CLOSE:
			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
			
		case MSG_KEYDOWN:
            if( wparam == CSAPP_KEY_IDLE )
            {
        		hdc=BeginPaint(hwnd);
				MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
				
				Current_Item = CSAPP_FAC_POWER;
				Power_State = FOCUS;
				
				MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
				EndPaint(hwnd,hdc);

				Check_Front_KeyEnd(hwnd);
        		break;
            }

			switch(wparam)
			{
/*
				case CSAPP_KEY_PAUSE:
					//MV_Fac_End_cfg(KIND_OFF);
					CFG_Factory_Mode = FALSE;
					CSApp_Factory_Applets = CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
                	break;
*/
				case CSAPP_KEY_RIGHT:
					hdc=BeginPaint(hwnd);
					MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					Current_Item = CSAPP_FAC_RIGHT;
					Right_State = FOCUS;
					
					MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					Check_Front_KeyEnd(hwnd);
					break;
					
				case CSAPP_KEY_LEFT:
					hdc=BeginPaint(hwnd);
					MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					Current_Item = CSAPP_FAC_LEFT;
					Left_State = FOCUS;
					
					MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					Check_Front_KeyEnd(hwnd);
					break;
					
				case CSAPP_KEY_UP:
					if ( Current_Item > CSAPP_FAC_SYSTEM )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
						Current_Item = CSAPP_FAC_UP;
						Up_State = FOCUS;
						
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						Check_Front_KeyEnd(hwnd);
					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						
						if ( Current_Item < CSAPP_FAC_POWER )
							MV_Draw_Fac_Unfocus(hwnd, Current_Item);
						
						if(Current_Item > 0)
						{
							Current_Item--;
							u8Channel_Kind = 0;
						}
						
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_DOWN:
					if ( Current_Item > CSAPP_FAC_SYSTEM )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
						Current_Item = CSAPP_FAC_DOWN;
						Down_State = FOCUS;
						
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						Check_Front_KeyEnd(hwnd);
					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						
						if ( Current_Item < CSAPP_FAC_POWER )
							MV_Draw_Fac_Unfocus(hwnd, Current_Item);

						if(Current_Item == CSAPP_FAC_MAX - 1)
						{
							u8Channel_Kind = 0;
							Current_Item = 0;
						}
						else
							Current_Item++;

						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_ENTER:
					if ( Current_Item > CSAPP_FAC_SYSTEM )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
						Current_Item = CSAPP_FAC_OK;
						Ok_State = FOCUS;
						
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						Check_Front_KeyEnd(hwnd);
					} else {
						if ( Factory_Focus_Item == CSAPP_FAC_LIVE && Current_Item == CSAPP_FAC_LIVE )
						{
							u8Channel_Kind++;
							if ( u8Channel_Kind == MV_DB_GetALLServiceNumber() )
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
								EndPaint(hwnd,hdc);
								
								MV_Draw_Fac_Unfocus(hwnd, Current_Item);
								u8Channel_Kind = 0;
								Current_Item = CSAPP_FAC_VIDEO;
								
								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
								EndPaint(hwnd,hdc);
							}
							else 
							{
								tCS_DB_ServiceManageData 	service_index;

								CS_DB_SetNextService_OrderIndex();
								CS_MW_StopService(TRUE);
								CS_DB_GetCurrentList_ServiceData( &service_index, CS_DB_GetCurrentService_OrderIndex());
								CS_MW_PlayServiceByIdx(service_index.Service_Index, RE_TUNNING);
							}
						} else if ( Factory_Focus_Item == CSAPP_FAC_VIDEO && Current_Item == CSAPP_FAC_VIDEO ) {
							u8Video_Test_state++;
							if ( u8Video_Test_state == CSAPP_VT_MAX )
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
								EndPaint(hwnd,hdc);
								
								MV_Draw_Fac_Unfocus(hwnd, Current_Item);
								u8Video_Test_state = 0;
								Current_Item = CSAPP_FAC_AUDIO;

								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
								EndPaint(hwnd,hdc);
							}
						} else if ( Factory_Focus_Item == CSAPP_FAC_AUDIO && Current_Item == CSAPP_FAC_AUDIO ) {
							u8Audio_Test_state++;
							if ( u8Audio_Test_state == CSAPP_AT_MAX )
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
								EndPaint(hwnd,hdc);
								
								MV_Draw_Fac_Unfocus(hwnd, Current_Item);
								u8Audio_Test_state = 0;
								Current_Item = CSAPP_FAC_SYSTEM;

								hdc=BeginPaint(hwnd);
								MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
								EndPaint(hwnd,hdc);
							}
						} else if ( Current_Item == CSAPP_FAC_SYSTEM ) {
							hdc=BeginPaint(hwnd);
							MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
							hdc=BeginPaint(hwnd);
							
							MV_Draw_Fac_Unfocus(hwnd, Current_Item);
							Current_Item = CSAPP_FAC_POWER;

							hdc=BeginPaint(hwnd);
							MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						} 

						switch(Current_Item)
						{
							case CSAPP_FAC_LIVE:
								Factory_Focus_Item = CSAPP_FAC_LIVE;
								hdc=BeginPaint(hwnd);
								MV_Live_Test(hdc);
								EndPaint(hwnd,hdc);
								break;
							case CSAPP_FAC_VIDEO:
								Factory_Focus_Item = CSAPP_FAC_VIDEO;							
								switch(u8Video_Test_state)
								{
									case CSAPP_VT_RGB_OFF:
										CS_MW_SetAspectRatio(0);
										CS_MW_SetVideoOutput(1);
										break;
									case CSAPP_VT_RGB_ON:
										CS_MW_SetVideoOutput(0);
										break;
									case CSAPP_VT_SCART_4_3:
										CS_MW_SetVideoOutput(1);
										CS_MW_SetAspectRatio(0);
									// ScartAspecChange(0);
										break;
									case CSAPP_VT_SCART_16_9:
										CS_MW_SetAspectRatio(1);
									// ScartAspecChange(1);
										break;
									case CSAPP_VT_SCART_OFF:
										ScartSbOnOff();
										break;
									case CSAPP_VT_SCART_ON:
										ScartSbOnOff();
										break;
									default:
										break;
								}
								hdc=BeginPaint(hwnd);
								MV_Video_Test(hdc);
								EndPaint(hwnd,hdc);
								break;
							case CSAPP_FAC_AUDIO:
								Factory_Focus_Item = CSAPP_FAC_AUDIO;
								switch(u8Audio_Test_state)
								{
									case CSAPP_AT_VOL_MAX:
										CS_AV_AudioSetVolume(kCS_DBU_MAX_VOLUME);
										break;
									case CSAPP_AT_VOL_MIN:
										CS_AV_AudioSetVolume(0);
										break;
									default:
										break;
								}
								hdc=BeginPaint(hwnd);
								MV_Audio_Test(hdc);
								EndPaint(hwnd,hdc);
								break;
#if 0
							case CSAPP_FAC_USB:
								Factory_Focus_Item = CSAPP_FAC_USB;
								MV_USB_Test(hwnd);
								break;
							case CSAPP_FAC_NET:
								Factory_Focus_Item = CSAPP_FAC_NET;
								MV_Network_Test(hwnd);
								break;
							case CSAPP_FAC_SMART:
								Factory_Focus_Item = CSAPP_FAC_SMART;
								MV_CI_Test(hwnd);
								break;
#endif
							default:
								break;
						}
					}
					break;
				case CSAPP_KEY_MENU:
					if ( Current_Item > CSAPP_FAC_SYSTEM )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
						Current_Item = CSAPP_FAC_MENU;
						Menu_State = FOCUS;
						
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						Check_Front_KeyEnd(hwnd);
					}
					break;
				case CSAPP_KEY_ESC:
					if ( Current_Item > CSAPP_FAC_SYSTEM )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_FACMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
						Current_Item = CSAPP_FAC_EXIT;
						Exit_State = FOCUS;
						
						MV_Draw_FACMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
						Check_Front_KeyEnd(hwnd);
					}
					break;
                                			
				default:				
					break;
			}
			break;
	}

	//printf("\n======= 0x%04x============= CHECK =====\n", message);
	return DefaultMainWinProc(hwnd, message, wparam, lparam);
		
}

