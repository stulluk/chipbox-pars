#include "linuxos.h"

#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mvrecfile.h"
#include "csmpr_usb.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"
#include "csmpr_parser.h"
#include "demux.h"
#include "ui_common.h"
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <pwd.h>
#include <grp.h>

#define LIST_MAX_ITEM		10
#define kNbSecondPerMinute 	60
#define kNbSecondPerHour 	3600
#define kNbMinutePerHour 	60
#define kNbHourPerDay		24

static 	CSAPP_Applet_t		CSApp_RecFile_Applets;

static 	stFile_db			stFileDB;
static 	U16					u16Current_index;
static 	U16					u16Current_Focus;
static 	U16					u16Current_Page;
static 	U16					u16Prev_Page;
static 	BOOL				IsThereFile = FALSE;

extern char	PVR_Check_String[PVR_DATA_KIND][PVR_DATA_LEN];

st_pvr_data	PVR_Rec_Data;

extern void MV_Calculate_Size(long long llong, char *temp);

void MV_Draw_Rec_MenuTitle(HDC hdc)
{
	RECT 	TmpRect;
	char	TmpStr[10];
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(CHEDIT_LIST_LEFT - 10),ScalerHeigthPixel( MV_INSTALL_MENU_Y - 10), ScalerWidthPixel(CHEDIT_LIST_DX + 20 ),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	TmpRect.left	= ScalerWidthPixel(CHEDIT_LIST_LEFT + 20);
	TmpRect.right	= TmpRect.left + (CHEDIT_LIST_DX - 30);
	TmpRect.top		= ScalerHeigthPixel(MV_INSTALL_MENU_Y - 7);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	sprintf(TmpStr, "%d/%d", u16Current_index + 1 , stFileDB.u16file_Count);
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NAME), -1, &TmpRect, DT_LEFT);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_RIGHT);
}

void MV_Draw_Rec_FileInfo(HDC hdc, U16 u16Index)
{
	RECT			TmpRect;
	struct stat		statbuffer;
	char			TmpStr[256];
	char			TmpStr2[256];
	struct tm 		*tm_ptr;
	CSMPR_FILEINFO  fileInfo;

	sprintf(TmpStr, "%s/%s", RECfile, stFileDB.acFileName[u16Index]);
	get_file_info_simple( TmpStr, &fileInfo );
	MV_PVR_CFG_File_Parser(&PVR_Rec_Data, TmpStr);
	
	//CS_PVR_Player_Start(TmpStr);
	if( stat(TmpStr, &statbuffer ) != 0 )
		printf("STAT ERROR========================\n");
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LIST_LEFT),ScalerHeigthPixel( MV_PIG_LIST_TOP + MV_PIG_LIST_DY + 10 ), ScalerWidthPixel(MV_PIG_LIST_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H * 3) );

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	TmpRect.left	=ScalerWidthPixel(MV_PIG_LIST_LEFT + 30);
	TmpRect.right	=TmpRect.left + MV_PIG_LIST_DX - 80;
	TmpRect.top		=ScalerHeigthPixel(MV_PIG_LIST_TOP + MV_PIG_LIST_DY + 10) + 2;
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	sprintf(TmpStr, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_NAME), stFileDB.acFileName[u16Index]);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	//MV_Calculate_Size( statbuffer.st_size, TmpStr2);
	MV_Calculate_Size( ( fileInfo.total_size / 1024 ), TmpStr2);
	
	sprintf(TmpStr, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_SIZE), TmpStr2);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	//tm_ptr = gmtime(&statbuffer.st_mtime);
	tm_ptr = gmtime(&PVR_Rec_Data.PVR_Start_OS);
	sprintf(TmpStr, "%s : %02d/%02d/%04d %02d:%02d:%02d", 
					CS_MW_LoadStringByIdx(CSAPP_STR_RECORD_TIME),
                    tm_ptr->tm_mday,
                    tm_ptr->tm_mon+1,
                    tm_ptr->tm_year+1900,
                    tm_ptr->tm_hour,
                    tm_ptr->tm_min,
                    tm_ptr->tm_sec);

	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
	//CS_PVR_Player_Stop();
}


void MV_Draw_Rec_StorageInfo(HDC hdc)
{
	RECT			TmpRect;
	char			TmpStr[256];
	U32				u32Play_Duration;
	struct tm 		*tm_ptr;
	int				Int_Hour = 0, Int_Min = 0, Int_Sec = 0;

	memset (&PVR_Rec_Data, 0x00, sizeof(st_pvr_data));
	memset (TmpStr, 0x00, sizeof(256));

	sprintf(TmpStr, "%s/%s", RECfile, stFileDB.acFileName[u16Current_index]);
	MV_PVR_CFG_File_Parser(&PVR_Rec_Data, TmpStr);
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel( MV_PIG_TOP + MV_PIG_DY + 10 ), ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H * 3) );

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	TmpRect.left	=ScalerWidthPixel(MV_PIG_LEFT + 30);
	TmpRect.right	=TmpRect.left + MV_PIG_DX - 80;
	TmpRect.top		=ScalerHeigthPixel(MV_PIG_TOP + MV_PIG_DY + 10) + 2;
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	sprintf(TmpStr, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_PROGRAM), PVR_Rec_Data.PVR_Title);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT | DT_INTERNAL | DT_SINGLELINE);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	u32Play_Duration = PVR_Rec_Data.PVR_End_OS - PVR_Rec_Data.PVR_Start_OS;

	Int_Hour = u32Play_Duration/kNbSecondPerHour;
	Int_Min = (u32Play_Duration%kNbSecondPerHour)/kNbSecondPerMinute;
	Int_Sec = (u32Play_Duration%kNbSecondPerHour)%kNbSecondPerMinute;

#if 0
	sprintf(TmpStr, "Duration : %02d:%02d:%02d [%d Sec]",
                    Int_Hour,
                    Int_Min,
                    Int_Sec,
                    u32Play_Duration);
#else
	sprintf(TmpStr, "%s : %02d:%02d:%02d",
					CS_MW_LoadStringByIdx(CSAPP_STR_DURATION), 
                    Int_Hour,
                    Int_Min,
                    Int_Sec);
#endif

//printf("1. %s ====\n", TmpStr);

	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	tm_ptr = gmtime(&PVR_Rec_Data.PVR_End_OS);
	sprintf(TmpStr, "%s : %02d/%02d/%04d %02d:%02d:%02d", 
					CS_MW_LoadStringByIdx(CSAPP_STR_END_TIME),
                    tm_ptr->tm_mday,
                    tm_ptr->tm_mon+1,
                    tm_ptr->tm_year+1900,
                    tm_ptr->tm_hour,
                    tm_ptr->tm_min,
                    tm_ptr->tm_sec);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
}

void MV_Draw_RecMenuBar(HDC hdc, int esItem, U8 u8Kind)
{
	RECT				TmpRect;
	struct stat			statbuffer;
	char				TmpStr[256];
	CSMPR_FILEINFO  	fileInfo;
	
	
	if ( esItem == 0xFFFF )
	{
		int 		y_gap = MV_INSTALL_MENU_Y + MV_INSTALL_MENU_BAR_H * ( 4 + 1 );

		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X);
		TmpRect.right	=TmpRect.left + (CHEDIT_LIST_DX - 20);
		TmpRect.top		=ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NO_FILE), -1, &TmpRect, DT_CENTER);
	} else {
		int 			y_gap = MV_INSTALL_MENU_Y + MV_INSTALL_MENU_BAR_H * ( esItem + 1 );		
		
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		if( u8Kind == FOCUS ) {
			FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_LIST_LEFT), ScalerHeigthPixel(y_gap), ScalerWidthPixel(CHEDIT_LIST_DX - 20), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H), &MV_BMP[MVBMP_CHLIST_SELBAR]);
		} else {
			if ( esItem % 2 == 0 )
			{
				MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
				MV_FillBox( hdc, ScalerWidthPixel(CHEDIT_LIST_LEFT),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(CHEDIT_LIST_DX - 20),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			} else {
				MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(CHEDIT_LIST_LEFT),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(CHEDIT_LIST_DX - 20),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			}
		}

		if ( ( esItem + ( u16Current_Page * LIST_MAX_ITEM) ) < stFileDB.u16file_Count )
		{
			TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X);
			TmpRect.right	=TmpRect.left + (CHEDIT_LIST_DX - 220);
			TmpRect.top		=ScalerHeigthPixel(y_gap+4);
			TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

			sprintf(TmpStr, "%s/%s", RECfile, stFileDB.acFileName[esItem + ( u16Current_Page * LIST_MAX_ITEM)]);
			if( stat(TmpStr, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");

			if( S_ISDIR(statbuffer.st_mode) )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_OK_ICON].bmWidth ), ScalerHeigthPixel(y_gap), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);

			get_file_info_simple( TmpStr, &fileInfo );
			
			CS_MW_DrawText(hdc, stFileDB.acFileName[esItem + ( u16Current_Page * LIST_MAX_ITEM)], -1, &TmpRect, DT_LEFT);

			if( !(S_ISDIR(statbuffer.st_mode)) )
			{
				TmpRect.left	=TmpRect.right;
				TmpRect.right	=TmpRect.left + 170;

				//MV_Calculate_Size( statbuffer.st_size, TmpStr);
				MV_Calculate_Size( ( fileInfo.total_size / 1024 ), TmpStr);
				//sprintf(TmpStr, "%d", statbuffer.st_size);
				CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_RIGHT);
			}
		}
		
		if ( u8Kind == FOCUS )
		{
			TmpRect.top = CHEDIT_SCROLL_TOP - 6;
			TmpRect.left = CHEDIT_SCROLL_LEFT;
			TmpRect.right = CHEDIT_SCROLL_RIGHT;
			TmpRect.bottom = CHEDIT_SCROLL_BOTTOM + 6;
			MV_Draw_ScrollBar(hdc, TmpRect, u16Current_index, stFileDB.u16file_Count - 1, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
		}
	}
}

void MV_Draw_Rec_MenuBar(HDC hdc)
{
	U16 	i = 0;

	if ( IsThereFile == TRUE )
	{
		for( i = 0 ; i < LIST_MAX_ITEM ; i++ )
		{
			if ( i == u16Current_Focus )
				MV_Draw_RecMenuBar(hdc, i, FOCUS);
			else
				MV_Draw_RecMenuBar(hdc, i, UNFOCUS);
		}
	} else {
		MV_Draw_RecMenuBar(hdc, 0xFFFF, UNFOCUS);
	}
}

CSAPP_Applet_t CSApp_RecFile(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_RecFile_Applets = CSApp_Applet_Error;

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

	CreateInfo.dwStyle		= WS_VISIBLE;
	CreateInfo.dwExStyle	= WS_EX_NONE;
	CreateInfo.spCaption	= "RecFile";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = RecFile_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_RecFile_Applets;   
}

int RecFile_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
   
	switch(message)
	   	{
			case MSG_CREATE:
				u16Current_index = 0;
				u16Current_Page = 0;
				u16Current_Focus = 0;
				IsThereFile = TRUE;
				memset(&stFileDB, 0x00, sizeof(stFile_db));
				
				if ( MV_Load_TS_fileData(&stFileDB) == FILE_READ_FAIL )
					IsThereFile = FALSE;
				break;
				
			case MSG_PAINT:
				CS_MW_SetSmallWindow(ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_TOP),ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_PIG_DY));
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_MEDIA, EN_ITEM_FOCUS_RECORD_FILE);
				
				hdc = BeginPaint(hwnd);
				MV_Draw_Rec_MenuTitle(hdc);
				MV_Draw_Rec_MenuBar(hdc);
				MV_Draw_Rec_FileInfo(hdc, u16Current_index);
				MV_Draw_Rec_StorageInfo(hdc);
				//MV_Media_draw_help_banner(hdc, EN_ITEM_FOCUS_RECORD_FILE);
				EndPaint(hwnd,hdc);
			
				return 0;

			case MSG_USB_MSG:
				if ( UsbCon_GetStatus() != USB_STATUS_MOUNTED )
				{	
					CSApp_RecFile_Applets=CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
				}
				break;
				
			case MSG_KEYDOWN:
				if ( MV_Check_Confirm_Window() == TRUE )
				{
					MV_Confirm_Proc(hwnd, wparam);

					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
					{
						if ( wparam == CSAPP_KEY_ENTER )
						{
							char 		acPlay_File[256];
							long long	Temp_Position = 0;

							memset( acPlay_File, 0x00, 256 );
							
							if( CSMPR_Player_GetStatus() != CSMPR_PLAY_STOP )
								CSMPR_Player_Stop();
							else
								CS_AV_ProgramStop();

							sprintf(acPlay_File, "%s/%s", RECfile, stFileDB.acFileName[u16Current_index]);
								
							if ( MV_Check_YesNo() == TRUE )
							{	
								Temp_Position = PVR_Rec_Data.PVR_Last_Posion;
							} else {
								Temp_Position = 0;
							}
							
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);

							//printf("\n==== Temp_Position : %lld : %lld ==========\n", Temp_Position, PVR_Rec_Data.PVR_Last_Posion);
							if ( CSMPR_Player_Start(acPlay_File, Temp_Position) == CSMPR_ERROR )
							{
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_PLAY_ERROR);
								EndPaint(hwnd,hdc);

								usleep( 1000 * 1000 );

								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
								break;
							} else {

								CS_MW_SetNormalWindow();
								
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
							
								CSApp_RecFile_Applets=CSApp_Applet_Pvr_Player;
								SendMessage(hwnd,MSG_CLOSE,0,0);
							}
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}
					}
					
					if (wparam != CSAPP_KEY_IDLE)
					{
						break;
					}
				}
				
				switch(wparam)
				{
					case CSAPP_KEY_RED:
						break;
						
					case CSAPP_KEY_BLUE:
						break;

					case CSAPP_KEY_UP:
						hdc = BeginPaint(hwnd);
						u16Prev_Page = u16Current_Page;
						MV_Draw_RecMenuBar(hdc, u16Current_Focus, UNFOCUS);

						if ( u16Current_index <= 0 )
							u16Current_index = stFileDB.u16file_Count - 1;
						else
							u16Current_index--;

						u16Current_Focus = get_focus_line(&u16Current_Page, u16Current_index, LIST_MAX_ITEM);

						if ( u16Prev_Page != u16Current_Page )
							MV_Draw_Rec_MenuBar(hdc);
						else
							MV_Draw_RecMenuBar(hdc, u16Current_Focus, FOCUS);
						//EndPaint(hwnd,hdc);

						//hdc = BeginPaint(hwnd);
						MV_Draw_Rec_MenuTitle(hdc);
						MV_Draw_Rec_FileInfo(hdc, u16Current_index);
						MV_Draw_Rec_StorageInfo(hdc);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_DOWN:
						hdc = BeginPaint(hwnd);
						u16Prev_Page = u16Current_Page;
						MV_Draw_RecMenuBar(hdc, u16Current_Focus, UNFOCUS);

						if ( u16Current_index >= stFileDB.u16file_Count - 1 )
							u16Current_index = 0;
						else
							u16Current_index++;

						u16Current_Focus = get_focus_line(&u16Current_Page, u16Current_index, LIST_MAX_ITEM);

						if ( u16Prev_Page != u16Current_Page )
							MV_Draw_Rec_MenuBar(hdc);
						else
							MV_Draw_RecMenuBar(hdc, u16Current_Focus, FOCUS);
						
						//EndPaint(hwnd,hdc);

						//hdc = BeginPaint(hwnd);
						MV_Draw_Rec_MenuTitle(hdc);
						MV_Draw_Rec_FileInfo(hdc, u16Current_index);
						MV_Draw_Rec_StorageInfo(hdc);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_ENTER:
						if ( strlen(stFileDB.acFileName[u16Current_index]) == 0 )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_SEL_FILE);
							EndPaint(hwnd,hdc);

							usleep( 1000 * 1000 );

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
						} 
						else
						{
							char 		acPlay_File[256];

							memset( acPlay_File, 0x00, 256 );
							sprintf(acPlay_File, "%s/%s", RECfile, stFileDB.acFileName[u16Current_index]);

							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);

							//printf("\n=== PVR_Rec_Data.PVR_Last_Posion : %lld ========\n", PVR_Rec_Data.PVR_Last_Posion);
							if ( PVR_Rec_Data.PVR_Last_Posion != 0 )
							{
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								
								MV_Draw_Confirm_Window(hwnd, CSAPP_STR_CONTINUE);
							} else {								
								if( CSMPR_Player_GetStatus() != CSMPR_PLAY_STOP )
									CSMPR_Player_Stop();
								else
									CS_AV_ProgramStop();
								
								if ( CSMPR_Player_Start(acPlay_File, 0) == CSMPR_ERROR )
								{
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);

									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_PLAY_ERROR);
									EndPaint(hwnd,hdc);

									usleep( 1000 * 1000 );

									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);
									CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);

									break;
								} else {
									CS_MW_SetNormalWindow();
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);
									
									CSApp_RecFile_Applets=CSApp_Applet_Pvr_Player;
									SendMessage(hwnd,MSG_CLOSE,0,0);
								}
							}
						}
						break;
						
					case CSAPP_KEY_ESC:
						CS_MW_SetNormalWindow();
						CSApp_RecFile_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_MENU:
						CS_MW_SetNormalWindow();
						if ( get_prev_windown_status() == CSApp_Applet_Desktop )
						{
							CSApp_RecFile_Applets=CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						} else {
							CSApp_RecFile_Applets=b8Last_App_Status;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_IDLE:
						CSApp_RecFile_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
						
					default:
						break;
				}
				break;
			
		   	case MSG_CLOSE:
				DestroyMainWindow(hwnd);
				PostQuitMessage(hwnd);
				break;

		   	default:
				break;
	   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

#define			MOVIE_FILE_KIND	11
#define			IMAGE_FILE_KIND	4
#define			EXT_LENGTH		5
char	Chech_MovieFile_Str[MOVIE_FILE_KIND][EXT_LENGTH] = {
								"avi",
								"mkv",
								"mpg",
								"mpeg",
								"wmv",
								"asf",
								"m1v",
								"m1v",
								"rm",
								"mov",
								"mp4"
							};

char	Check_Picture_Str[IMAGE_FILE_KIND][EXT_LENGTH] = {
								"jpg",
								"png",
								"gif",
								"bmp"
							};

BOOL Check_Movie(char *Extention)
{
	int		i;

	for ( i = 0 ; i < MOVIE_FILE_KIND ; i++ )
	{
		if ( strcmp(Chech_MovieFile_Str[i], Extention ) == 0 )
			return TRUE;
	}

	return FALSE;
}

BOOL Check_Picture(char *Extention)
{
	int		i;

	for ( i = 0 ; i < IMAGE_FILE_KIND ; i++ )
	{
		if ( strcmp(Check_Picture_Str[i], Extention ) == 0 )
			return TRUE;
	}

	return FALSE;
}

U8 Check_Picture_Kind(char *Extention)
{
	int		i;

	for ( i = 0 ; i < IMAGE_FILE_KIND ; i++ )
	{
		if ( strcmp(Check_Picture_Str[i], Extention ) == 0 )
			return i;
	}

	return 0xFF;
}

eMV_Filetype MV_Check_Filetype(char *Extention)
{
	eMV_Filetype	Return_Val;

	if ( strcmp(Extention, "txt") == 0 )
		Return_Val = MVAPP_FILE_TEXT;
	else if ( strcmp(Extention, "mp3") == 0 )
		Return_Val = MVAPP_FILE_MUSIC;
	else if ( strcmp(Extention, "ts") == 0 )
		Return_Val = MVAPP_FILE_TS;
	else if ( Check_Movie(Extention) == TRUE )
		Return_Val = MVAPP_FILE_MOVIE;
	else if ( Check_Picture(Extention) == TRUE )
		Return_Val = MVAPP_FILE_PIC;
	else
		Return_Val = MVAPP_FILE_NORMAL;

	return Return_Val;
}



