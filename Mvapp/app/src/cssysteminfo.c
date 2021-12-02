#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

static CSAPP_Applet_t		CSApp_SysInfo_Applets;

static U32					SystemSettingItemIdx[CSAPP_INFO_ITEM_MAX]={
								CSAPP_STR_MODEL_TYPE,
								CSAPP_STR_HW_VERSION,
								CSAPP_STR_UBOOT_VERSION,
								CSAPP_STR_KERNEL_VERSION,
								CSAPP_STR_ROOTFS_VERSION,
								CSAPP_STR_MAIN_VERSION,
								CSAPP_STR_DEFAULT_DATA,
								CSAPP_STR_DEFAULT_DB,
								CSAPP_STR_VIDEO_BG,
								CSAPP_STR_AUDIO_BG
							};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

/*
static char *				pInfoText[CSAPP_INFO_ITEM_MAX] = {
								MODEL_TYPE,
								HW_VERSION,
								UBOOT_VERSION,
								KERNEL_VERSION,
								ROOTFS_VERSION,
								EPP_CHECK_STR,
								DEFAULT_DATA_VERSION,
								DEFAULT_DB_VERSION,
								VIDEO_BG,
								AUDIO_BG
							};
*/
static char					pInfoText[CSAPP_INFO_ITEM_MAX][100];

static int Sysinfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Parser_SW_Value( char *Temp, char *tempSection )
{
	U16		i;
	
	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( tempSection[i] == ';' )
			break;
		else
			Temp[i] = tempSection[i];
	}
	//printf("===== Parser : %s \n", Temp);
}

MV_CFG_RETURN MV_Load_Version_File(void)
{
	int				i = 0;
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL + 2];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;

	if (!(fp = fopen(SW_VERSION_FILE, "r")))
	{
         ret = CFG_NOFILE;
		 //printf("=== NO FILE =====\n");
		 return ret;
	}

	memset (pInfoText, 0, CSAPP_INFO_ITEM_MAX * 100);

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);
		
        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			//printf("=== READ FAIL =====\n");
			return ret;
        }

		MV_Parser_SW_Value(Temp, tempSection);

		strcpy(pInfoText[i], Temp);
		//printf("%d : %s =====>  %s \n", i, Temp, pInfoText[i]);
		i++;
    }
	
	fclose (fp);
	return ret;
}

void MV_Draw_InfoMenuBar(HDC hdc, U8 u8Focuskind, eMV_Sysinfo_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( esItem + 2 );
	RECT	TmpRect;
	char	Temp_Str[100];

	memset ( Temp_Str, 0x00, 100 );
	if ( u8Focuskind == MV_UNFOCUS )
	{
		if ( esItem%2 == 0 )
		{
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		} else {
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		}
	}
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);		
	
	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX - 20;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	sprintf( Temp_Str, "%s :", CS_MW_LoadStringByIdx(SystemSettingItemIdx[esItem]));
	CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_RIGHT);
	//MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SystemSettingItemIdx[esItem]));

	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);		
	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);
#if 0
	switch(esItem)
	{
		case CSAPP_INFO_TYPE:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);		
			break;
		case CSAPP_INFO_BOOT:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_INFO_KERNEL:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_INFO_ROOTFS:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);		
			break;
		case CSAPP_INFO_MAIN:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_INFO_DEF_DATA:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_INFO_DEF_DB:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);		
			break;
		case CSAPP_INFO_VBG:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_INFO_ABG:
			CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);	
			break;
		default:
			break;
	}
#else
	if ( esItem == CSAPP_INFO_MAIN )
	{
		sprintf( Temp_Str, "%s", pInfoText[esItem]);
		CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_LEFT);
	}
	else
		CS_MW_DrawText(hdc, pInfoText[esItem], -1, &TmpRect, DT_LEFT);
#endif
}

void MV_Draw_Info_MenuBar(HDC hdc)
{
	U16 	i = 0;
	int 	y_gap = MV_INSTALL_MENU_Y;
	RECT	TmpRect;
	char	Temp_Str[100];

	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2 * 2);
	sprintf( Temp_Str, "%s", pInfoText[CSAPP_INFO_MAIN]);
	MV_MW_DrawBigText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
	
	for( i = 0 ; i < CSAPP_INFO_ITEM_MAX ; i++ )
	{
		MV_Draw_InfoMenuBar(hdc, MV_UNFOCUS, i);
	}
}

CSAPP_Applet_t CSApp_Sysinfo(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_SysInfo_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "sysinfo";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = Sysinfo_Msg_cb;
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

	return CSApp_SysInfo_Applets;   
}


static int Sysinfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
   
	switch(message)
	   	{
			case MSG_CREATE:
				MV_Load_Version_File();
				break;
				
			case MSG_PAINT:

				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_SYSINFO);

				hdc=BeginPaint(hwnd);
				MV_Draw_Info_MenuBar(hdc);
				MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_SYSINFO);
				EndPaint(hwnd,hdc);				
				return 0;
			
			case MSG_KEYDOWN:
				switch(wparam)
				{
					case CSAPP_KEY_ESC:
						CSApp_SysInfo_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
					
					case CSAPP_KEY_ENTER:	
					case CSAPP_KEY_MENU:
						CSApp_SysInfo_Applets=b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_SysInfo_Applets = CSApp_Applet_Sleep;
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



