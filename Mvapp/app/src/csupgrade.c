#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csupgrade.h"
#include "csreset.h"

// #define	TMP_FILE			"/tmp/www/AllFlash_32M.tar" /* By KB Kim for Plugin Setting : 2011.05.07 */
#define TMP_FOLDER			"/tmp/www/"
#define TMP_EXE_FILE		"/tmp/www/DumpWrite32M"
//#define TMP_SIZE			29103616

#if 0
#define	SYSTEM_COMMAND_KIND	9
#define UPGRADE_STATUS		8
static char					System_Command[SYSTEM_COMMAND_KIND][128] = {
								"rm /tmp/www/*.tar",
								"rm /tmp/*.log",
								"mtd_debug erase /dev/mtd0 0x80000 0x20000",
								"mtd_debug write /dev/mtd0 0x80000 0x20000 /tmp/www/bootlogo.bin",
								"flashcp /tmp/www/kernel.bin /dev/mtd1",
								"flashcp /tmp/www/plugin.bin /dev/mtd4",
								"flashcp /tmp/www/Work0.bin /dev/mtd7",
								"flashcp /tmp/www/Work1.bin /dev/mtd9",
								"flashcp /tmp/www/rootfs.bin /dev/mtd2"
							};
static U16					System_Command_State[UPGRADE_STATUS] = {
								CSAPP_STR_SYSTEM_READY,
								CSAPP_STR_BOOT_BLOCK_UP,
								CSAPP_STR_KERNEL_UP,
								CSAPP_STR_PLUGIN_UP,
								CSAPP_STR_APP1_UP,
								CSAPP_STR_APP2_UP,
								CSAPP_STR_ROOT_FILE_UP,
								CSAPP_STR_FINISH_UP
							};
static U16					System_Command_UnState[UPGRADE_STATUS] = {
								CSAPP_STR_SYSTEM_READY,
								CSAPP_STR_BOOT_BLOCK,
								CSAPP_STR_KERNEL,
								CSAPP_STR_PLUGIN,
								CSAPP_STR_APP1,
								CSAPP_STR_APP2,
								CSAPP_STR_ROOT_FILE,
								CSAPP_STR_FINISH
							};
#else
#define	SYSTEM_COMMAND_KIND	8
#define UPGRADE_STATUS		7
static char					System_Command[SYSTEM_COMMAND_KIND][128] = {
								"rm /tmp/www/*.tar",
								"rm /tmp/*.log",
								"mtd_debug erase /dev/mtd0 0x80000 0x20000",
								"mtd_debug write /dev/mtd0 0x80000 0x20000 /tmp/www/bootlogo.bin",
								"flashcp /tmp/www/kernel.bin /dev/mtd1",
								"flashcp /tmp/www/plugin.bin /dev/mtd4",
								"flashcp /tmp/www/Work0.bin /dev/mtd7",
								"flashcp /tmp/www/rootfs.bin /dev/mtd2"
							};
static U16					System_Command_State[UPGRADE_STATUS] = {
								CSAPP_STR_SYSTEM_READY,
								CSAPP_STR_BOOT_BLOCK_UP,
								CSAPP_STR_KERNEL_UP,
								CSAPP_STR_PLUGIN_UP,
								CSAPP_STR_APP1_UP,
								CSAPP_STR_ROOT_FILE_UP,
								CSAPP_STR_FINISH_UP
							};
static U16					System_Command_UnState[UPGRADE_STATUS] = {
								CSAPP_STR_SYSTEM_READY,
								CSAPP_STR_BOOT_BLOCK,
								CSAPP_STR_KERNEL,
								CSAPP_STR_PLUGIN,
								CSAPP_STR_APP1,
								CSAPP_STR_ROOT_FILE,
								CSAPP_STR_FINISH
							};
#endif

static U8					Sys_Com_Status = 0;

static CSAPP_Applet_t		CSApp_Upgreade_Applets;
static EN_UPGRADE_METHOD	upgrade_item = EN_UPGRADE_KIND_FULL_SYSTEM;
static EN_UPGRADE_METHOD	upgrade_method = EN_UPGRADE_METHOD_USB;
static BOOL					Method_Flag = TRUE;

/* By KB Kim for Plugin Setting : 2011.05.07 */
static pthread_t  			hUpgrade_TaskHandle;

#ifndef PLUGIN_MENU /* By KB Kim for Plugin Setting : 2011.05.07 */
static BOOL					Confirm_Window_Flag = FALSE;
static BOOL					Confirm_YesNo = FALSE;
static BOOL					Download_Status = FALSE;
static stMV_Upgrade_Info	stUpgrade_Data[256];
static U8					u8Upgrade_Data_Count = 0;
/**************************************************************************/
static BOOL					Wget_File_List = FALSE;
static U16					u16Wget_Focus;
static U16					u16Current_WgetPage;
static U16					u16Prev_WgetPage;
static U16					u16Current_Wgetindex;
static pthread_t  			hProgress_TaskHandle;
static pthread_t			hDownload_TaskHandle;
static long 				TMP_SIZE = 1;
#endif

char Upgrade_USB[EN_UPGRADE_KIND_MAX][256] = {
	"Full system",
	"Database",
	"Default Database"
};

U16 Upgrade_Category[EN_UPGRADE_METHOD_MAX] = {
	CSAPP_STR_USB,
	CSAPP_STR_NETWORK
};

extern void MV_Calculate_Size(long long llong, char *temp);

static int Upgreade_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

int MV_Upgrade_Seq(void)
{
    //printf("int MV_Upgrade_Seq(void)\n");
    system("killall -s 9 getip.sh");

	int 	system_result = 0;
	int		Sys_Com_Kind = 0;

	Sys_Com_Status = 0;

	for ( Sys_Com_Kind = 0 ; Sys_Com_Kind < SYSTEM_COMMAND_KIND ; Sys_Com_Kind++ )
	{
		if ( Sys_Com_Kind == 14 )
			usleep(2000*1000);

		switch(Sys_Com_Kind)
		{
			case 0:
			case 1:
				Sys_Com_Status = 0;
				break;
			case 2:
			case 3:
				Sys_Com_Status = 1;
				break;
			case 4:
				Sys_Com_Status = 2;
				break;
			case 5:
				Sys_Com_Status = 3;
				break;
			case 6:
				Sys_Com_Status = 4;
				break;
			case 7:
				Sys_Com_Status = 5;
				break;
			case 8:
				Sys_Com_Status = 6;
				break;
			default:
				Sys_Com_Status = 7;
				break;
		}

		if ( Sys_Com_Kind == 2 )
		{
			if (access( "/tmp/www/uboot.bin" , 0 ) == 0 )
			{
				system("echo UBoot Upgrade");
				//system("flashcp /tmp/www/uboot.bin /dev/mtd0");
			}
		}

		system_result = system(System_Command[Sys_Com_Kind]);

		if ( system_result != 0 && Sys_Com_Kind > 1 )
			return system_result;
	}
	usleep(2000*1000);
	return system_result;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
#ifndef PLUGIN_MENU
/* By KB Kim for Plugin Setting : 2011.05.07 */
void MV_SetNetUpgradeMode(eMV_NET_Upgrade_Items mode)
{
	CurrentNetUpgradeMode = mode;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
eMV_NET_Upgrade_Items MV_GetNetUpgradeMode(void)
{
	return CurrentNetUpgradeMode;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
BOOL MV_NetCheck_Confirm_Window(void)
{
	return Confirm_Window_Flag;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
BOOL MV_NetCheck_YesNo(void)
{
	return Confirm_YesNo;
}

void MV_NetDown_Change_Confirm(void)
{
	if ( Confirm_YesNo == TRUE )
		Confirm_YesNo = FALSE;
	else
		Confirm_YesNo = TRUE;
}

void Restore_NetDown_Confirm_Window(HDC hdc)
{
	Confirm_Window_Flag = FALSE;
	Confirm_YesNo = FALSE;
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(UPGRADE_WARNING_DX), ScalerHeigthPixel(UPGRADE_WARNING_DY), &btMsg_Con_cap);
	UnloadBitmap (&btMsg_Con_cap);
}

void MV_Draw_NetDown_Confirm_Button(HDC hdc)
{
	RECT 	Temp_Rect_Button;
	RECT 	Temp_Rect_Blank;
	RECT	Yes_Button = {	UPGRADE_WARNING_ITEM_X + 30,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							UPGRADE_WARNING_ITEM_X + 30 + 200,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};
	RECT	No_Button = {	UPGRADE_WARNING_ITEM_X + 30 + 200 + 20,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							UPGRADE_WARNING_ITEM_X + 30 + 200 + 20 + 200,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};

	if ( Confirm_YesNo == TRUE )
	{
		Temp_Rect_Button = Yes_Button;
		Temp_Rect_Blank = No_Button;
	} else {
		Temp_Rect_Button = No_Button;
		Temp_Rect_Blank = Yes_Button;
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel((Temp_Rect_Button.right - Temp_Rect_Button.left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.top), ScalerWidthPixel(Temp_Rect_Blank.right - Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.bottom - Temp_Rect_Blank.top) );

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	Yes_Button.top += 4;
	No_Button.top += 4;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OK), -1, &Yes_Button, DT_CENTER);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_EDIT), -1, &No_Button, DT_CENTER);
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
void MV_Draw_NetDown_Confirm(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	char	Temp_Str[64];

	memset(Temp_Str, 0x00, 64);

	Confirm_Window_Flag = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(UPGRADE_WARNING_DX), ScalerHeigthPixel(UPGRADE_WARNING_DY), &btMsg_Con_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT + UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP + UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT + UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP + UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(UPGRADE_WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP),ScalerWidthPixel(UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(UPGRADE_WARNING_DY));
	FillBox(hdc,ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(UPGRADE_WARNING_DX),ScalerHeigthPixel(UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= UPGRADE_WARNING_TOP + UPGRADE_WARNING_OUT_GAP + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= UPGRADE_WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + UPGRADE_WARNING_ITEM_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_ATTENTION);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_WARNING_ITEM_X), ScalerHeigthPixel(UPGRADE_WARNING_ITEM_Y), ScalerWidthPixel(UPGRADE_WARNING_ITEM_DX), ScalerHeigthPixel(UPGRADE_WARNING_ITEM_DY) );

	Temp_Rect.top 	= UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= UPGRADE_WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + UPGRADE_WARNING_ITEM_DX;

	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		sprintf(Temp_Str, "%s", CS_DBU_Get_Webaddr());
	}
	CS_MW_DrawText (hdc, Temp_Str, -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);

	MV_Draw_NetDown_Confirm_Button(hdc);
	MV_EndPaint(hwnd,hdc);
}

BOOL MV_NetDown_Confirm_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
		case CSAPP_KEY_RIGHT:
        case CSAPP_KEY_UP:
		case CSAPP_KEY_LEFT:
        	hdc = BeginPaint(hwnd);
			MV_NetDown_Change_Confirm();
			MV_Draw_NetDown_Confirm_Button(hdc);
			EndPaint(hwnd,hdc);
        	break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			Confirm_YesNo = FALSE;
			hdc = BeginPaint(hwnd);
			Restore_NetDown_Confirm_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
BOOL MV_CheckWgetListStatus(void)
{
	return Wget_File_List;
}

void MV_Close_Wget_Window(HDC hdc)
{
	Wget_File_List = FALSE;
	FillBoxWithBitmap(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);
	UnloadBitmap (&btFile_cap);
}

MV_CFG_RETURN MV_Upgrade_Get_Wget_File(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;
	U16				i = 0, j = 0, k = 0, l = 0;

	/* By KB Kim for Plugin Setting : 2011.05.07 */
	u8Upgrade_Data_Count = 0;
	memset( stUpgrade_Data, 0x00, sizeof(stMV_Upgrade_Info) * 256 );

	if (!(fp = fopen(UPGRADE_CFG_FILE, "r")))
	{
         ret = CFG_NOFILE;
		 return ret;
	}

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			return ret;
        }

		//printf("%s ===>\n", tempSection);

		for ( i = 0 ; i < strlen(tempSection) ; i++ )
		{
			if ( tempSection[i] == ';' || tempSection[i] == '\n')
			{
				switch(l)
				{
					case 0:
						stUpgrade_Data[u8Upgrade_Data_Count].u8Set_Version = atoi(Temp);
						break;

					case 1:
						stUpgrade_Data[u8Upgrade_Data_Count].u8SW_Version = atoi(Temp);
						break;

					case 2:
						sprintf(stUpgrade_Data[u8Upgrade_Data_Count].acFile_Location, "%s", Temp);

						for ( j = 0 ; j < strlen(Temp) ; j++ )
							if ( Temp[j] == '/' )
								strncpy(stUpgrade_Data[u8Upgrade_Data_Count].acFile_Name, &Temp[j+1], strlen(Temp) - ( j + 1 ) );
						break;

					case 3:
						sprintf(stUpgrade_Data[u8Upgrade_Data_Count].acFile_size, "%s", Temp);
						break;

					case 4:
						sprintf(stUpgrade_Data[u8Upgrade_Data_Count].acFile_date, "%s", Temp);
						break;

					default:
						break;
				}

				memset (Temp, 0, sizeof(char) * 100);
				k = 0;
				l++;
			}
			else
			{
				Temp[k] = tempSection[i];
				k++;
			}
		}
		l = 0;
		u8Upgrade_Data_Count++;
    }

	fclose (fp);
	return ret;
}

void MV_Draw_WgetListBar(HDC hdc, int esItem, U8 u8Kind)
{
	int 			y_gap = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_HEIGHT * esItem;
	RECT			TmpRect;

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if( u8Kind == FOCUS ) {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(y_gap), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	} else {
		if ( esItem % 2 == 0 )
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		} else {
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		}
	}

	TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_NAME_X + MV_BMP[MVBMP_TS_FILE].bmWidth + 10);
	TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_NAME_DX - ( MV_BMP[MVBMP_TS_FILE].bmWidth + 10 );
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);
	CS_MW_DrawText(hdc, stUpgrade_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].acFile_Name, -1, &TmpRect, DT_LEFT);

	TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_DATE_X);
	TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_DATE_DX;
	CS_MW_DrawText(hdc, stUpgrade_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].acFile_date, -1, &TmpRect, DT_LEFT);

	if ( u8Kind == FOCUS )
	{
		TmpRect.top = FILE_WINDOW_ITEM_Y;
		TmpRect.left = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX;
		TmpRect.right = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX;
		TmpRect.bottom = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_DY;
		MV_Draw_ScrollBar(hdc, TmpRect, u16Current_Wgetindex, u8Upgrade_Data_Count, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Wget_List(HWND hwnd)
{
	U16 	i = 0;
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);

	for( i = 0 ; i < FILE_LIST_MAX_ITEM ; i++ )
	{
		if ( i == u16Wget_Focus )
			MV_Draw_WgetListBar(hdc, i, FOCUS);
		else
			MV_Draw_WgetListBar(hdc, i, UNFOCUS);
	}

	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Wget_FileList(HWND hwnd) /* By KB Kim for Plugin Setting : 2011.05.07 */
{
	HDC 	hdc;
	RECT	Temp_Rect;

	u16Wget_Focus = 0;
	u16Current_WgetPage = 0;
	u16Prev_WgetPage = 0;
	u16Current_Wgetindex = 0;

	Wget_File_List = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y),ScalerWidthPixel(FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(FILE_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(FILE_WINDOW_DX),ScalerHeigthPixel(FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= FILE_WINDOW_TITLE_Y;
	Temp_Rect.bottom	= FILE_WINDOW_TITLE_Y + FILE_WINDOW_ITEM_HEIGHT;
	Temp_Rect.left	= FILE_WINDOW_TITLE_X;
	Temp_Rect.right	= Temp_Rect.left + FILE_WINDOW_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_FILE_EXPLORE);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(FILE_WINDOW_ITEM_Y), ScalerWidthPixel(FILE_WINDOW_ITEM_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_DY) );

	MV_EndPaint(hwnd,hdc);

	MV_Draw_Wget_List(hwnd);
}

BOOL MV_WgetList_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
			hdc = BeginPaint(hwnd);
			u16Prev_WgetPage = u16Current_WgetPage;
			MV_Draw_WgetListBar(hdc, u16Wget_Focus, UNFOCUS);

			if ( u16Current_Wgetindex >= u8Upgrade_Data_Count - 1 )
				u16Current_Wgetindex = 0;
			else
				u16Current_Wgetindex++;

			u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_WgetPage != u16Current_WgetPage )
				MV_Draw_Wget_List(hdc);
			else
				MV_Draw_WgetListBar(hdc, u16Wget_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

        case CSAPP_KEY_UP:
			hdc = BeginPaint(hwnd);
			u16Prev_WgetPage = u16Current_WgetPage;
			MV_Draw_WgetListBar(hdc, u16Wget_Focus, UNFOCUS);

			if ( u16Current_Wgetindex <= 0 )
				u16Current_Wgetindex = u8Upgrade_Data_Count - 1;
			else
				u16Current_Wgetindex--;

			u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_WgetPage != u16Current_WgetPage )
				MV_Draw_Wget_List(hdc);
			else
				MV_Draw_WgetListBar(hdc, u16Wget_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			{
				U8	current_page, total_page;

				if(u8Upgrade_Data_Count == 0)
					break;

				current_page = getpage((u16Current_Wgetindex+1), FILE_LIST_MAX_ITEM);
				total_page = getpage(u8Upgrade_Data_Count, FILE_LIST_MAX_ITEM);

				u16Current_Wgetindex += FILE_LIST_MAX_ITEM;
				if(u16Current_Wgetindex > u8Upgrade_Data_Count-1)
				{
					if(current_page < total_page)
						u16Current_Wgetindex = u8Upgrade_Data_Count-1;
					else
						u16Current_Wgetindex = 0;
				}

				u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_Wget_List(hdc);
				EndPaint(hwnd,hdc);
			}
			break;

		case CSAPP_KEY_LEFT:
			{
				if(u8Upgrade_Data_Count == 0)
					break;

				if(u16Current_Wgetindex < FILE_LIST_MAX_ITEM)
				{
					u16Current_Wgetindex = (getpage(u8Upgrade_Data_Count, FILE_LIST_MAX_ITEM)+1)*FILE_LIST_MAX_ITEM + u16Current_Wgetindex-FILE_LIST_MAX_ITEM;
				}
				else
					u16Current_Wgetindex -= FILE_LIST_MAX_ITEM;

				if(u16Current_Wgetindex > u8Upgrade_Data_Count-1)
					u16Current_Wgetindex = u8Upgrade_Data_Count-1;

				u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_Wget_List(hdc);
				EndPaint(hwnd,hdc);
			}
        	break;

        case CSAPP_KEY_ENTER:
			break;

        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			hdc = BeginPaint(hwnd);
			MV_Close_Wget_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

int Show_Progress(HWND hwnd)
{
	HDC 			hdc;
	struct stat		statbuffer;
	U8				file_Percent;
	RECT			TmpRect;
	char 			TmpStr[20];

	memset(TmpStr, 0x00, 20 );

	if( stat(TMP_UPGRADE_FILE, &statbuffer ) != 0 )
	{
		printf("STAT ERROR========================\n");
	}

	file_Percent = (U32)( statbuffer.st_size / ( TMP_SIZE / 100 ));

	//printf("==== %d , ( %ld * 100 ) / %ld ,, %ld\n", file_Percent, statbuffer.st_size, TMP_SIZE, (( statbuffer.st_size * 100 ) / TMP_SIZE));

	TmpRect.left = UPGRADE_MESSAGE_X ;
	TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
	TmpRect.top = UPGRADE_MESSAGE_Y + 40;
	TmpRect.bottom = TmpRect.top + 30;

	sprintf(TmpStr, "%ld / %ld", statbuffer.st_size, TMP_SIZE);

	hdc=BeginPaint(hwnd);

	if ( file_Percent <= 100 )
	{
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y + 40), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(30) );
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		CS_MW_DrawText (hdc, TmpStr, -1, &TmpRect, DT_CENTER);

		TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX - 70;
		TmpRect.top = UPGRADE_MESSAGE_Y + 80;
		TmpRect.bottom = TmpRect.top + 14;
		MV_Draw_LevelBar(hdc, &TmpRect, file_Percent, EN_TTEM_PROGRESS_NO_IMG_MP3);
	}

	EndPaint(hwnd,hdc);

	return 0;
}

void *Progress_Task( void *param )
{
	HWND	hwnd;

	hwnd = GetActiveWindow();

	while(1)
	{
		if ( Show_Progress(hwnd) != 0 )
		{
			printf("\n=========== ERROR ==================\n");
			break;
		}
		usleep( 400*1000 );
	}

	return ( param );
}

int Progress_Init(BOOL b8Type)
{
	if ( b8Type == TRUE )
		pthread_create( &hProgress_TaskHandle, NULL, Progress_Task, NULL );
#if 0 /* By KB Kim for Plugin Setting : 2011.05.07 */
	else
		pthread_create( &hUpgrade_TaskHandle, NULL, Upgrade_Task, NULL );
#endif
	return( 0 );
}

void Progress_Stop(void)
{
	pthread_cancel( hProgress_TaskHandle );
}

BOOL MV_CheckDownLoadStatus(void)
{
	return Download_Status;
}

void Download_Stop(void)
{
	Download_Status = FALSE;
	pthread_cancel( hDownload_TaskHandle );
}

void *Download_Task( void *param )
{
	char 		ShellCommand[256];
	HWND		hwnd;
	int			ShellResult = 0;

	hwnd = GetActiveWindow();

	Download_Status = TRUE;

	MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD);

	TMP_SIZE = atoi(stUpgrade_Data[u16Current_Wgetindex].acFile_size);
	sprintf(ShellCommand, "wget -q -O %s \"http://%s/chipbox/%s\"",TMP_UPGRADE_FILE , CS_DBU_Get_Webaddr(), stUpgrade_Data[u16Current_Wgetindex].acFile_Location);
	printf("Download_Task : %s\n", ShellCommand);
	ShellResult = system(ShellCommand);

	BroadcastMessage (MSG_DOWNLOAD_COMPLETE, ShellResult, 0);

	return ( param );
}

int Download_Init(void)
{
	Download_Status = FALSE;
	pthread_create( &hDownload_TaskHandle, NULL, Download_Task, NULL );
	return( 0 );
}

void MV_Draw_Msg_Download(HWND hwnd, U32 u32Message)
{
	HDC 		hdc;
	RECT		rc1;

	rc1.left = UPGRADE_MESSAGE_X;
	rc1.top = UPGRADE_MESSAGE_Y + 10;
	rc1.right = rc1.left + UPGRADE_MESSAGE_DX;
	rc1.bottom = rc1.top + MV_INSTALL_MENU_BAR_H * 2;

	hdc=BeginPaint(hwnd);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY) );

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(u32Message), -1, &rc1, DT_CENTER);
	EndPaint(hwnd,hdc);

	if ( u32Message == CSAPP_STR_DOWNLOAD )
		Progress_Init(TRUE);
#if 0 /* By KB Kim for Plugin Setting : 2011.05.07 */
	else if ( u32Message == CSAPP_STR_UPGRADE )
		Progress_Init(FALSE);
#endif
}

void MV_Close_Msg_Download(HWND hwnd)
{
	HDC 		hdc;

	Progress_Stop();
	hdc=BeginPaint(hwnd);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY) );
	EndPaint(hwnd,hdc);
}
#endif

void *Upgrade_Task( void *param )
{
	HWND	hwnd;
	HDC		hdc;
	int		i = 0, j = 0;
	RECT	TmpRect;
	U8		old_status = 0;

	hwnd = GetActiveWindow();

	while(1)
	{
		hdc=BeginPaint(hwnd);

		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );

		/* By KB Kim for Plugin Setting : 2011.05.07 */
		MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y2 - 60), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY2 + 60) );

		for ( j = 0 ;  j < UPGRADE_STATUS ; j ++ )
		{
			TmpRect.left = UPGRADE_MESSAGE_X;
			TmpRect.top = UPGRADE_MESSAGE_Y2 + (MV_INSTALL_MENU_BAR_H * j) + 10;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			TmpRect.bottom = TmpRect.top + MV_INSTALL_MENU_BAR_H;

			if ( j == Sys_Com_Status )
			{
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(System_Command_State[j]), -1, &TmpRect, DT_CENTER);
			} else {
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(System_Command_UnState[j]), -1, &TmpRect, DT_CENTER);
			}
		}

		if ( i % 10 < 5 )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			TmpRect.left = UPGRADE_MESSAGE_X ;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			/* By KB Kim for Plugin Setting : 2011.05.07 */
			TmpRect.top = UPGRADE_MESSAGE_Y2 - 60;
			TmpRect.bottom = TmpRect.top + 60;

			CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_IF_UPGRADE_FAIL), -1, &TmpRect, DT_CENTER);

			TmpRect.left = UPGRADE_MESSAGE_X ;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			TmpRect.top = UPGRADE_MESSAGE_Y + 40;
			TmpRect.bottom = TmpRect.top + 30;

			CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_UPGRADING_WAIT), -1, &TmpRect, DT_CENTER);
		}

		TmpRect.top = UPGRADE_MESSAGE_Y + 70;
		TmpRect.bottom = TmpRect.top + 30;

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		switch ( i % 3 )
		{
			case 0:
				CS_MW_DrawText (hdc, "/", -1, &TmpRect, DT_CENTER);
				break;
			case 1:
				CS_MW_DrawText (hdc, "=", -1, &TmpRect, DT_CENTER);
				break;
			case 2:
				CS_MW_DrawText (hdc, "\\", -1, &TmpRect, DT_CENTER);
				break;
		}
		i++;

		EndPaint(hwnd,hdc);
		old_status = Sys_Com_Status;
		usleep( 100*1000 );
	}
	return ( param );
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
int Upgrade_Init(void)
{
	pthread_create( &hUpgrade_TaskHandle, NULL, Upgrade_Task, NULL );
	return( 0 );
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
void Upgrade_Stop(void)
{
	pthread_cancel( hUpgrade_TaskHandle );
}

void MV_Upgrade_Draw_Leftline(HDC hdc)
{
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_LEFT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H*5) );

	MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
	switch(upgrade_method)
	{
		case EN_UPGRADE_METHOD_USB:
		default:
			MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_LEFT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
			break;
		case EN_UPGRADE_METHOD_NETWORK:
			MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_LEFT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y2), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
			break;
	}
}

void MV_Upgrade_Draw_Midline(HDC hdc)
{
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H*5) );

	MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
	switch(upgrade_item)
	{
		case EN_UPGRADE_KIND_FULL_SYSTEM:
		default:
			switch(upgrade_method)
			{
				case EN_UPGRADE_METHOD_USB:
				default:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
					break;
				case EN_UPGRADE_METHOD_NETWORK:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H + UPGRADE_LINE_OFFSET) );
					break;
			}
			break;
		case EN_UPGRADE_KIND_DATABASE:
			switch(upgrade_method)
			{
				case EN_UPGRADE_METHOD_USB:
				default:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H + UPGRADE_LINE_OFFSET) );
					break;
				case EN_UPGRADE_METHOD_NETWORK:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y2), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(/*MV_INSTALL_MENU_BAR_H*2 + */UPGRADE_LINE_OFFSET) );
					break;
			}
			break;
		case EN_UPGRADE_KIND_DEFAULT_DATA:
			switch(upgrade_method)
			{
				case EN_UPGRADE_METHOD_USB:
				default:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H*2 + UPGRADE_LINE_OFFSET) );
					break;
				case EN_UPGRADE_METHOD_NETWORK:
					MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MID_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y2), ScalerWidthPixel(UPGRADE_LINE_OFFSET), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H + UPGRADE_LINE_OFFSET) );
					break;
			}
			break;
	}
}

void MV_Upgrade_Draw_Rightline(HDC hdc)
{
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_RIGHT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H*5) );

	MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
	switch(upgrade_item)
	{
		case EN_UPGRADE_KIND_FULL_SYSTEM:
		default:
			MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_RIGHT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y1), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
			break;
		case EN_UPGRADE_KIND_DATABASE:
			MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_RIGHT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y2), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
			break;
		case EN_UPGRADE_KIND_DEFAULT_DATA:
			MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_RIGHT_LINE_X), ScalerHeigthPixel(UPGRADE_LEFT_LINE_Y3), ScalerWidthPixel(UPGRADE_LEFT_LINE_DX), ScalerHeigthPixel(UPGRADE_LINE_OFFSET) );
			break;
	}
}


void MV_Upgrade_Draw_Menu(HDC hdc, EN_UPGRADE_METHOD enfocus, BOOL bfocus)
{
	RECT	rc1;

	int 	y_gap = UPGRADE_CATEGORY_Y + MV_INSTALL_MENU_BAR_H * enfocus;
	int		start_x = UPGRADE_CATEGORY_X;
	int 	mid_width = UPGRADE_CATEGORY_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int 	right_x = UPGRADE_CATEGORY_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	if ( bfocus == FOCUS )
	{
		if ( Method_Flag == TRUE )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap + 4 ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight),&MV_BMP[MVBMP_YELLOW_BUTTON]);
		} else {
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(UPGRADE_CATEGORY_DX + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) + 10),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		}
	} else {
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(UPGRADE_CATEGORY_DX + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) + 10),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}

	rc1.left = start_x + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth);
	rc1.top = y_gap + 4;
	rc1.right = rc1.left + UPGRADE_CATEGORY_DX - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth);
	rc1.bottom = rc1.top + MV_INSTALL_MENU_BAR_H - 4;

	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(Upgrade_Category[enfocus]), -1, &rc1, DT_LEFT);
}

void MV_Upgrade_Draw_Menu_Item(HDC hdc, EN_UPGRADE_METHOD enfocus, BOOL bfocus)
{
	RECT	rc1;
	int 	y_gap = UPGRADE_KIND_Y + MV_INSTALL_MENU_BAR_H * enfocus;
	int		start_x = UPGRADE_KIND_X;
	int 	mid_width = UPGRADE_KIND_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int 	right_x = UPGRADE_KIND_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	if ( bfocus == FOCUS )
	{
		if ( Method_Flag == FALSE )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap + 4 ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight),&MV_BMP[MVBMP_YELLOW_BUTTON]);
		} else {
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(UPGRADE_CATEGORY_DX + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) + 10),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		}
	} else {
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(start_x - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) - 10),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(UPGRADE_KIND_DX + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth) + 10),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}

	rc1.left = start_x + ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth);
	rc1.top = y_gap + 4;
	rc1.right = rc1.left + UPGRADE_KIND_DX - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth);
	rc1.bottom = rc1.top + MV_INSTALL_MENU_BAR_H - 4;

	CS_MW_DrawText (hdc, Upgrade_USB[enfocus], -1, &rc1, DT_LEFT);
}

void MV_Upgrade_Draw_FullMenu(HDC hdc)
{
	EN_UPGRADE_METHOD	i = EN_UPGRADE_METHOD_USB;

	for ( i = EN_UPGRADE_METHOD_USB ; i < EN_UPGRADE_METHOD_MAX ; i++ )
	{
		if ( i == upgrade_method )
			MV_Upgrade_Draw_Menu(hdc, i, FOCUS);
		else
			MV_Upgrade_Draw_Menu(hdc, i, UNFOCUS);
	}
}

void MV_Upgrade_Draw_FullMenu_Item(HDC hdc)
{
	EN_UPGRADE_METHOD	i = EN_UPGRADE_METHOD_USB;

	for ( i = EN_UPGRADE_KIND_FULL_SYSTEM ; i < EN_UPGRADE_KIND_MAX ; i++ )
	{
		if ( i == upgrade_item )
			MV_Upgrade_Draw_Menu_Item(hdc, i, FOCUS);
		else
			MV_Upgrade_Draw_Menu_Item(hdc, i, UNFOCUS);
	}
}

void MV_Upgrade_Draw_Line(HDC hdc)
{
	MV_Upgrade_Draw_Leftline(hdc);
	MV_Upgrade_Draw_Midline(hdc);
	MV_Upgrade_Draw_Rightline(hdc);
}

CSAPP_Applet_t	CSApp_Upgreade(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Upgreade_Applets = CSApp_Applet_Error;

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

	CreateInfo.dwStyle	 = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "upgreade window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Upgreade_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = CSAPP_BLACK_COLOR;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	return CSApp_Upgreade_Applets;

}


static int Upgreade_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	RECT 				rc1;
	char 				ShellCommand[256];
	int					ShellResult = 0;

	switch(message)
	{
		case MSG_CREATE:
			upgrade_method = EN_UPGRADE_METHOD_USB;
			upgrade_item = EN_UPGRADE_KIND_FULL_SYSTEM;
			/* By KB Kim for Plugin Setting : 2011.05.07 */
			MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
			break;

		case MSG_PAINT:
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_UPGRADE);

				hdc=BeginPaint(hwnd);

				FillBoxWithBitmap(hdc,ScalerWidthPixel(UPGRADE_TITLE_X), ScalerHeigthPixel(UPGRADE_TITLE_Y), ScalerWidthPixel(UPGRADE_TITLE_DX), ScalerHeigthPixel(UPGRADE_TITLE_DY), &MV_BMP[MVBMP_CHLIST_SELBAR]);
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				rc1.left = ScalerWidthPixel(UPGRADE_TITLE_X);
				rc1.top = ScalerHeigthPixel(UPGRADE_TITLE_Y) + 4;
				rc1.right = ScalerWidthPixel(UPGRADE_TITLE_X+UPGRADE_TITLE_DX);
				rc1.bottom = ScalerHeigthPixel(UPGRADE_TITLE_Y+UPGRADE_TITLE_DY);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_UPGRADE), -1, &rc1, DT_CENTER | DT_VCENTER);

				MV_Upgrade_Draw_FullMenu(hdc);
#ifdef USE_TWO_STEP
				MV_Upgrade_Draw_FullMenu_Item(hdc);
				MV_Upgrade_Draw_Line(hdc);
#endif
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_DOWNLOAD_COMPLETE:
			if ( wparam != 0 )
			{
				MV_Close_Msg_Download(hwnd);
				printf("FULL System == Get Wget File Fail >>>>>>>>>>\n");
				printf("%s\n", ShellCommand);
				MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD_FAIL);
				usleep(2000*1000);
				MV_Close_Msg_Download(hwnd);
				/* By KB Kim for Plugin Setting : 2011.05.07 */
				MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
			}
			else
			{
				MV_Close_Msg_Download(hwnd);
				MV_Draw_Confirm_Window(hwnd, CSAPP_STR_WANT_TO_UPGRADE);
			}

			Download_Stop();
			break;

		case MSG_KEYDOWN:
			if ( MV_Get_StringKeypad_Status() == TRUE )
			{
				MV_StringKeypad_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					CS_DBU_Set_Webaddr(MV_Get_StringEdited_String());
					CS_DBU_SaveUserSettingDataInHW();
					/* By KB Kim for Plugin Setting : 2011.05.07 */
					MV_SetNetUpgradeMode(NET_UPGRADE_MAIN);
					MV_Draw_NetDown_Confirm(hwnd);
				}
				break;
			}

			else if ( MV_Check_File_Window() == TRUE )
			{
				MV_FileList_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_File_Check_Enter() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							MV_Close_File_Window(hdc);
							EndPaint(hwnd,hdc);

							MV_Draw_Msg_Download(hwnd, CSAPP_STR_UPGRADE);

							/* By KB Kim for Plugin Setting : 2011.05.07 */
							Upgrade_Init();

							sprintf(ShellCommand, "tar -xf %s -C %s", MV_Get_FileName(), TMP_FOLDER);
							printf("%s\n", ShellCommand);
							ShellResult = system(ShellCommand);

							if ( ShellResult == 0 )
							{
								CS_AV_ProgramStop();
								ShellResult = MV_Upgrade_Seq();

								//sprintf(ShellCommand, TMP_EXE_FILE);
								//ShellResult = system(ShellCommand);

								if ( ShellResult == 0 )
								{
									// printf("Reboot System Now FbStartWatchdog(1);\n");
									FbStartWatchdog(1);
									// system("reboot");
								} else {
									MV_Close_Msg_Download(hwnd);
									printf("FULL System == DumpWrite32M Fail >>>>>>>>>>\n");
								}
							} else {
								MV_Close_Msg_Download(hwnd);
								MV_Draw_Msg_Download(hwnd, CSAPP_STR_UPGRADE_FAIL);
								printf("FULL System == tar -xvf Fail >>>>>>>>>>\n");
								usleep(1000*1000);
								MV_Close_Msg_Download(hwnd);
							}

							/* By KB Kim for Plugin Setting : 2011.05.07 */
							Upgrade_Stop();
							break;
						} else {

							hdc = BeginPaint(hwnd);
							MV_Close_File_Window(hdc);
							EndPaint(hwnd,hdc);

							printf("FULL System == %s , %s >>>>>>>>>>\n", MV_Get_FileName(), MV_Get_FileExt());
						}
					} else {
						hdc = BeginPaint(hwnd);
						MV_Close_File_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}

				if ( wparam != CSAPP_KEY_IDLE )
					break;
			}

			else if ( MV_NetCheck_Confirm_Window() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_NetDown_Confirm_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						printf("\nKEY_ENTER : ===============\n");
						if ( MV_NetCheck_YesNo() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
						{
							hdc = BeginPaint(hwnd);
							Restore_NetDown_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);

							sprintf(ShellCommand, "wget -q -O /tmp/sw_list.txt \"http://%s/chipbox/send_ver.php?stb_id=1\"", CS_DBU_Get_Webaddr());
							ShellResult = system(ShellCommand);

							if ( ShellResult != 0 )
							{
								printf("FULL System == Get Wget Fail >>>>>>>>>>\n");
								printf("%s\n", ShellCommand);
								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								MV_Draw_Msg_Download(hwnd, CSAPP_STR_GET_LIST_FAIL);
								/* By KB Kim for Plugin Setting : 2011.05.07 */
								MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
								usleep(1000*1000);
								MV_Close_Msg_Download(hwnd);
								break;
							}
							else
							{
								printf("FULL System == Get Wget Success >>>>>>>>>>\n");
								MV_Upgrade_Get_Wget_File();

								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);

								MV_Draw_Wget_FileList(hwnd);
							}
						} else {
							hdc = BeginPaint(hwnd);
							Restore_NetDown_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							MV_Draw_StringKeypad(hwnd, CS_DBU_Get_Webaddr(), 34);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_NetDown_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						/* By KB Kim for Plugin Setting : 2011.05.07 */
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}
				}
				break;
			}
			else if (MV_CheckWgetListStatus() == TRUE) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_WgetList_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						hdc = BeginPaint(hwnd);
						MV_Close_Wget_Window(hdc);
						EndPaint(hwnd,hdc);

						Download_Init();
/*
						MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD);

						TMP_SIZE = atoi(stUpgrade_Data[u16Current_Wgetindex].acFile_size);
						sprintf(ShellCommand, "wget -q -O %s \"http://%s/chipbox/%s\"",TMP_UPGRADE_FILE , CS_DBU_Get_Webaddr(), stUpgrade_Data[u16Current_Wgetindex].acFile_Location);
						ShellResult = system(ShellCommand);

						if ( ShellResult != 0 )
						{
							MV_Close_Msg_Download(hwnd);
							printf("FULL System == Get Wget File Fail >>>>>>>>>>\n");
							printf("%s\n", ShellCommand);
							MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD_FAIL);
							usleep(1000*1000);
							MV_Close_Msg_Download(hwnd);
						}
						else
						{
							MV_Close_Msg_Download(hwnd);
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_WANT_TO_UPGRADE);
						}
*/
					} else {
						hdc = BeginPaint(hwnd);
						MV_Close_Wget_Window(hdc);
						EndPaint(hwnd,hdc);
						/* By KB Kim for Plugin Setting : 2011.05.07 */
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}
				}
				break;
			}
			else if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							printf("FULL System == Get Wget File Success >>>>>>>>>>\n");

							sprintf(ShellCommand, "cd %s", TMP_FOLDER);
							ShellResult = system(ShellCommand);

							MV_Draw_Msg_Download(hwnd, CSAPP_STR_UPGRADE);

							/* By KB Kim for Plugin Setting : 2011.05.07 */
							Upgrade_Init();

							sprintf(ShellCommand, "tar -xf %s -C %s", TMP_UPGRADE_FILE, TMP_FOLDER);
							ShellResult = system(ShellCommand);

							if ( ShellResult == 0 )
							{
								CS_AV_ProgramStop();
								ShellResult = MV_Upgrade_Seq();

								if ( ShellResult == 0 )
								{
									// printf("Reboot System Now FbStartWatchdog(2);\n");
									FbStartWatchdog(1);
									// system("reboot");
								} else {
									MV_Close_Msg_Download(hwnd);
									printf("FULL System == DumpWrite32M Fail >>>>>>>>>>\n");
								}
							} else {
								MV_Close_Msg_Download(hwnd);
								MV_Draw_Msg_Download(hwnd, CSAPP_STR_UPGRADE_FAIL);
								printf("FULL System == tar -xvf Fail >>>>>>>>>>\n");
								usleep(1000*1000);
								MV_Close_Msg_Download(hwnd);
							}

							/* By KB Kim for Plugin Setting : 2011.05.07 */
							Upgrade_Stop();
						}else {
							system("rm /tmp/www/AllFlash_32M.tar");
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
					}

					/* By KB Kim for Plugin Setting : 2011.05.07 */
					MV_SetNetUpgradeMode(NET_UPGRADE_KIND);

				}

				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}

			/* By KB Kim for Plugin Setting : 2011.05.07 */
			if ( MV_CheckDownLoadStatus() == TRUE )
			{
				switch(wparam)
				{
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						system("killall wget");
						system("rm /tmp/www/AllFlash_32M.tar");
						MV_Close_Msg_Download(hwnd);
						Download_Stop();
						break;

					case CSAPP_KEY_IDLE:
						CSApp_Upgreade_Applets = CSApp_Applet_Sleep;
						Download_Stop(); /* By KB Kim for Plugin Setting : 2011.05.07 */
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
				}
				break;
			}

			switch(wparam)
			{
				case CSAPP_KEY_ESC:
#ifdef USE_TWO_STEP
					if ( Method_Flag == FALSE )
					{
						Method_Flag = TRUE;

						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_FullMenu(hdc);
						MV_Upgrade_Draw_FullMenu_Item(hdc);
						MV_Upgrade_Draw_Line(hdc);
						EndPaint(hwnd,hdc);
					} else {
#endif
						CSApp_Upgreade_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
#ifdef USE_TWO_STEP
					}
#endif
					break;

				case CSAPP_KEY_MENU:
#ifdef USE_TWO_STEP
					if ( Method_Flag == FALSE )
					{
						Method_Flag = TRUE;

						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_FullMenu(hdc);
						MV_Upgrade_Draw_FullMenu_Item(hdc);
						MV_Upgrade_Draw_Line(hdc);
						EndPaint(hwnd,hdc);
					} else {
#endif
						CSApp_Upgreade_Applets = b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
#ifdef USE_TWO_STEP
					}
#endif
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Upgreade_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

				case CSAPP_KEY_UP:
#ifdef USE_TWO_STEP
					if ( Method_Flag == TRUE )
					{
#endif
						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Menu(hdc, upgrade_method, UNFOCUS);
						EndPaint(hwnd,hdc);

						if ( upgrade_method == EN_UPGRADE_METHOD_USB )
							upgrade_method = EN_UPGRADE_METHOD_MAX - 1;
						else
							upgrade_method--;

						hdc=BeginPaint(hwnd);
#ifdef USE_TWO_STEP
						MV_Upgrade_Draw_Line(hdc);
#endif
						MV_Upgrade_Draw_Menu(hdc, upgrade_method, FOCUS);
#ifdef USE_TWO_STEP
						MV_Upgrade_Draw_FullMenu_Item(hdc);
#endif
						EndPaint(hwnd,hdc);
#ifdef USE_TWO_STEP
					} else {
						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, UNFOCUS);
						EndPaint(hwnd,hdc);

						if ( upgrade_item == EN_UPGRADE_KIND_FULL_SYSTEM )
							upgrade_item = EN_UPGRADE_KIND_MAX - 1;
						else
							upgrade_item--;

						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Line(hdc);
						MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, FOCUS);
						EndPaint(hwnd,hdc);
					}
#endif
					break;

				case CSAPP_KEY_DOWN:
#ifdef USE_TWO_STEP
					if ( Method_Flag == TRUE )
					{
#endif
						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Menu(hdc, upgrade_method, UNFOCUS);
						EndPaint(hwnd,hdc);

						if ( upgrade_method == EN_UPGRADE_METHOD_MAX - 1)
							upgrade_method = EN_UPGRADE_METHOD_USB ;
						else
							upgrade_method++;

						hdc=BeginPaint(hwnd);
#ifdef USE_TWO_STEP
						MV_Upgrade_Draw_Line(hdc);
#endif
						MV_Upgrade_Draw_Menu(hdc, upgrade_method, FOCUS);
#ifdef USE_TWO_STEP
						MV_Upgrade_Draw_FullMenu_Item(hdc);
#endif
						EndPaint(hwnd,hdc);
#ifdef USE_TWO_STEP
					} else {
						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, UNFOCUS);
						EndPaint(hwnd,hdc);

						if ( upgrade_item == EN_UPGRADE_KIND_MAX - 1 )
							upgrade_item = EN_UPGRADE_KIND_FULL_SYSTEM;
						else
							upgrade_item++;

						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_Line(hdc);
						MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, FOCUS);
						EndPaint(hwnd,hdc);
					}
#endif
					break;

#ifdef USE_TWO_STEP
				case CSAPP_KEY_LEFT:
				case CSAPP_KEY_RIGHT:
					if ( Method_Flag == TRUE )
						Method_Flag = FALSE;
					else
						Method_Flag = TRUE;

					hdc=BeginPaint(hwnd);
					MV_Upgrade_Draw_FullMenu(hdc);
					MV_Upgrade_Draw_FullMenu_Item(hdc);
					MV_Upgrade_Draw_Line(hdc);
					EndPaint(hwnd,hdc);
					break;
#endif

				case CSAPP_KEY_ENTER:
#ifdef USE_TWO_STEP
					if ( Method_Flag == TRUE )
					{
						Method_Flag = FALSE;

						hdc=BeginPaint(hwnd);
						MV_Upgrade_Draw_FullMenu(hdc);
						MV_Upgrade_Draw_FullMenu_Item(hdc);
						MV_Upgrade_Draw_Line(hdc);
						EndPaint(hwnd,hdc);
					} else {
#endif
/*
						memset(acCurrent_Dir, 0x00, MAX_FILE_NAME_LENGTH);
						strncpy( acCurrent_Dir, USB_ROOT, strlen(USB_ROOT));
						MV_Upgrade_Load_fileData(acCurrent_Dir);
						MV_Upgrade_File_Search();
*/
						if ( upgrade_method == EN_UPGRADE_METHOD_USB )
							MV_Draw_File_Window(hwnd);
						else
						{
							/* By KB Kim for Plugin Setting : 2011.05.07 */
							MV_SetNetUpgradeMode(NET_UPGRADE_MAIN);
							MV_Draw_NetDown_Confirm(hwnd);
						}
#ifdef USE_TWO_STEP
					}
#endif
					break;

#ifdef USE_TWO_STEP
				case CSAPP_KEY_RED:
					hdc=BeginPaint(hwnd);
					MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, UNFOCUS);
					EndPaint(hwnd,hdc);

					if ( upgrade_item == EN_UPGRADE_KIND_MAX - 1 )
						upgrade_item = EN_UPGRADE_KIND_FULL_SYSTEM;
					else
						upgrade_item++;

					hdc=BeginPaint(hwnd);
					MV_Upgrade_Draw_Line(hdc);
					MV_Upgrade_Draw_Menu_Item(hdc, upgrade_item, FOCUS);
					EndPaint(hwnd,hdc);
					break;
#endif
				case CSAPP_KEY_F2:
					MV_Draw_StringKeypad(hwnd, CS_DBU_Get_Webaddr(), 34);
					break;

				default:
					break;
			}
			break;

		case MSG_CLOSE:
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;

		default:
			break;
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}








