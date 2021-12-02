#include "linuxos.h"

#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mvstorageinfo.h"
#include "csmpr_usb.h"
#include "ui_common.h"
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <pwd.h> 
#include <grp.h>

#define		FAT_FORMAT		TRUE
#define		EXT_FORMAT		FALSE

char *SCSIInfo = "/proc/scsi/usb-storage";
char USBfile[256];

//Get HDD File System Type
// 	shell command : cat /proc/mounts | grep '/mnt/usb/disk1'

static CSAPP_Applet_t		CSApp_USBInfo_Applets;

static U32					USBInfoItemIdx[CSAPP_SINFO_ITEM_MAX]={
									CSAPP_STR_USB_SIZE,
									CSAPP_STR_USB_USE,
									CSAPP_STR_USB_UNUSE,
									CSAPP_STR_USB_TYPE,
									CSAPP_STR_USB_VENDER,
									CSAPP_STR_USB_PRODUCT,
									CSAPP_STR_USB_SERIAL,
									CSAPP_STR_USB_FORMAT
							};

static char					USB_Check_String[7][100] = {
								"   Host scsi0",
								"       Vendor",
								"      Product",
								"Serial Number",
								"     Protocol",
								"    Transport",
								"       Quirks"
							};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static char					USB_Type[50];
static char					USB_Vender[50];
static char					USB_Product[50];
static char					USB_Serial[50];
static char					USB_Protocl[50];
static char					USB_Transport[50];
static char					USB_Quirks[50];
static struct f_size		stfile_size;
static U8					u8ThisHDD = 0;
static BOOL					Format_Window_Status = FALSE;
static BOOL					Format_State = FAT_FORMAT;

//extern struct passwd *getpwuid(uid_t uid);
//extern struct group *getgrgid(gid_t gid);

void MV_Parser_USB_Value( char *Temp, char *tempSection )
{
	U16		i, j;
	
	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( tempSection[i] == ':' )
			break;
	}
	
	j = i+2;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;
		
		Temp[i] = tempSection[i+j];
	}
}

void MV_Parser_USB_DataValue( U8 u8IPIndex , char *Temp )
{
	switch(u8IPIndex)
	{
		case CSAPP_USB_TYPE:
			memcpy(&USB_Type, Temp, strlen(Temp));
			break;
			
		case CSAPP_USB_VENDER:
			memcpy(&USB_Vender, Temp, strlen(Temp));
			break;
			
		case CSAPP_USB_PRODUCT:
			memcpy(&USB_Product, Temp, strlen(Temp));
			break;
			
		case CSAPP_USB_SERIAL:
			memcpy(&USB_Serial, Temp, strlen(Temp));
			break;
			
		case CSAPP_USB_PROTOCOL:
			memcpy(&USB_Protocl, Temp, strlen(Temp));
			break;

		case CSAPP_USB_TRANSPORT:
			memcpy(&USB_Transport, Temp, strlen(Temp));
			break;
			
		case CSAPP_USB_QUIRKS:
			memcpy(&USB_Quirks, Temp, strlen(Temp));
			break;
			
		default:
			break;
	}
}

U8 MV_USBData_Namecheck(char *tempSection)
{
	int 	i;
	char	Temp[20];
	
	for ( i = 0 ; i < CSAPP_USBINFO_MAX ; i++ )
	{
		memset (Temp, 0, sizeof(char) * 20);
		strncpy(Temp, tempSection, strlen(USB_Check_String[i]));

		if ( strcmp ( USB_Check_String[i], Temp ) == 0 )
			break;
	}

	return i;
}

MV_File_Return MV_Load_USBData_Test(void)
{
	DIR				*pDir = NULL;
	struct dirent 	*pDirEnt;
	struct stat		statbuffer;
    struct passwd 	*my_passwd; 
    struct group  	*my_group; 

	if( (pDir=opendir( "/mnt/usb/disk1" )) == NULL )
	{
		closedir( pDir );
		printf( "[UsbCon_Mount]:GetUsbDevFileName(): Failed to open dir.\n" );
		return FILE_READ_FAIL;
	}

	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		printf("pDirEnt->d_name = %lld : %lld : %s : %d\n", pDirEnt->d_ino, pDirEnt->d_off, pDirEnt->d_name, pDirEnt->d_reclen);
	}

	rewinddir( pDir );
	
	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		lstat(pDirEnt->d_name, &statbuffer );
		
		my_passwd = getpwuid(statbuffer.st_uid); 
	    my_group  = getgrgid(statbuffer.st_gid); 
#if 0
		printf("================== %s ==================== \n", pDirEnt->d_name );
	    printf("OWNER : %s\n", my_passwd->pw_name); 
	    printf("GROUP : %s\n", my_group->gr_name); 
		printf("statbuffer.st_ino = %ld \n", statbuffer.st_ino );
		printf("statbuffer.st_mode = %d \n", statbuffer.st_mode);
		printf("statbuffer.st_mode = %d \n", S_ISDIR(statbuffer.st_mode) );
		printf("statbuffer.st_nlink = %d \n", statbuffer.st_nlink );
		printf("statbuffer.st_size = %d - %ld \n", S_ISREG(statbuffer.st_mode), statbuffer.st_size );
		printf("statbuffer.st_atime = %ld \n", statbuffer.st_atime );
		printf("statbuffer.st_mtime = %ld \n", statbuffer.st_mtime );
		printf("statbuffer.st_ctime = %ld \n", statbuffer.st_ctime );		
#endif
	}
	closedir( pDir );
	return FILE_OK;
}

MV_File_Return MV_Load_USBData_File(void)
{
	FILE* 			fp;
    char 			tempSection [DHCP_FILE_MAX_COL + 2];
	char			Temp[100];
	DIR				*pDir = NULL;
	struct dirent 	*pDirEnt;
	char 			Filename[40];

	if( (pDir=opendir( SCSIInfo )) == NULL )
	{
//		printf( "[UsbCon_Mount]:GetUsbDevFileName(): Failed to open dir.\n" );
		closedir( pDir );
		return FILE_READ_FAIL;
	}

	memset(&Temp, 0x00, sizeof(char) * 100 );
	memset(&Filename, 0x00, sizeof(char) * 40 );
	
	while( (pDirEnt=readdir( pDir )) != NULL )
	{
//		printf("pDirEnt->d_name = %s \n",pDirEnt->d_name);
		if( pDirEnt->d_name[0] != '.')
			sprintf(Temp, pDirEnt->d_name);
	}
	
	sprintf(Filename, "%s/%s", SCSIInfo, Temp);
//	printf( "[UsbCon_Mount]:MV_Load_USBData_File(): %s \n", Filename );

	if (!(fp = fopen(Filename, "r")))
	{
		closedir( pDir );
		return FILE_NOFILE;
	}

//	printf( "[UsbCon_Mount]:MV_Load_USBData_File(): %s \n", Filename );
	
	while (!feof(fp)) {
		memset (tempSection, 0, sizeof(char) * (DHCP_FILE_MAX_COL + 2));
		memset (Temp, 0, sizeof(char) * 100);
		
        if (!fgets(tempSection, DHCP_FILE_MAX_COL, fp)) {
			closedir( pDir );
			return FILE_READ_FAIL;
        }

		MV_Parser_USB_Value(Temp, tempSection);
		MV_Parser_USB_DataValue( MV_USBData_Namecheck(tempSection) , Temp);
    }
	
	fclose (fp);
	closedir( pDir );
	return FILE_OK;
}

#if 0
MOUNTP *dfopen(void)
{
	MOUNTP 		*MP;

	// /proc/mounts 파일을 연다.
	MP = (MOUNTP *)malloc(sizeof(MOUNTP));
	if(!(MP->fp = fopen(MMOUNT, "r")))
	{
		return NULL;
	}
	else
		return MP;
}

MOUNTP *dfget(MOUNTP *MP)
{
	char 			buf[256];
	struct 	statfs 	lstatfs;
	struct 	stat 	lstat; 
	int 			is_root = 0;

    // /proc/mounts로 부터 마운트된 파티션의 정보를 얻어온다.
	while(fgets(buf, 255, MP->fp))
	{
		is_root = 0;
		sscanf(buf, "%s%s%s",MP->devname, MP->mountdir, MP->fstype);
		
		if (strcmp(MP->mountdir,"/") == 0) 
			is_root=1;
		
		if (stat(MP->devname, &lstat) == 0 || is_root)
        {
			if ((strstr(buf, MP->mountdir) && S_ISBLK(lstat.st_mode)) || is_root)
			{
				// 파일시스템의 총 할당된 크기와 사용량을 구한다.
				statfs(MP->mountdir, &lstatfs);
				
				//printf("\n======= %u - %u %d=======\n\n", lstatfs.f_blocks, lstatfs.f_bsize, sizeof(long));
				
				MP->size.blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
				MP->size.avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
				return MP;
			}
		}
	}
	rewind(MP->fp);
	return NULL;
}

int dfclose(MOUNTP *MP)
{
	fclose(MP->fp);
	return 0;
}
#endif

void MV_Calculate_Size(long long llong, char *temp)
{
	if ( (llong/1048576) > 0 )
	{
		sprintf(temp, "%.2f Gb", ((float)llong/1048576));
	} else if ( (llong/1024) > 0 ) {
		sprintf(temp, "%.2f Mb", ((float)llong/1024));
	} else {
		sprintf(temp, "%lld Kb", llong);
	}
}

void MV_Calculate_Size2(long long llong, char *temp)
{
	printf("%.2f , %lld ====== %.2f ========\n", (float)llong, llong, ((float)llong/1048576));
	if ( (llong/1048576) > 0 )
	{
		sprintf(temp, "%.2f Gb", ((float)llong/1048576));
	} else if ( (llong/1024) > 0 ) {
		sprintf(temp, "%.2f Mb", ((float)llong/1024));
	} else {
		sprintf(temp, "%lld Kb", llong);
	}
}

void MV_Draw_SInfoMenuBar(HDC hdc, int esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP + 4) * ( esItem + 1 );
	RECT	TmpRect;
	char	temp_str[100];

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);		
	MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	sprintf(temp_str, "%s :", CS_MW_LoadStringByIdx(USBInfoItemIdx[esItem]));
	CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_RIGHT);
	//MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(USBInfoItemIdx[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + 10);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX - 10;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		case CSAPP_SINFO_TOTAL:
			MV_Calculate_Size(stfile_size.blocks, temp_str);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);		
			break;
		case CSAPP_SINFO_USED:
			MV_Calculate_Size((stfile_size.blocks - stfile_size.avail), temp_str);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_SINFO_UNUSED:
			MV_Calculate_Size(stfile_size.avail, temp_str);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_SINFO_TYPE:
			CS_MW_DrawText(hdc, USB_Type, -1, &TmpRect, DT_LEFT);		
			break;
		case CSAPP_SINFO_VENDER:
			CS_MW_DrawText(hdc, USB_Vender, -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_SINFO_PRODUCT:
			CS_MW_DrawText(hdc, USB_Product, -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_SINFO_SERIAL:
			CS_MW_DrawText(hdc, USB_Serial, -1, &TmpRect, DT_LEFT);	
			break;
		case CSAPP_SINFO_FORMAT:
			sprintf(temp_str, "%s", stPartition_data[u8ThisHDD].acPartition_Type/*, stPartition_data[u8ThisHDD].acPartition_Data, stPartition_data[u8ThisHDD].acPartition_Dir*/);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);	
			break;
		default:
			break;
	}
}

void MV_Draw_SInfo_Part_Info(HDC hdc)
{
	RECT	Usage_Rect;
	char	acTemp_str[256];
	
	Usage_Rect.top = MV_INSTALL_MENU_Y;
	Usage_Rect.bottom = Usage_Rect.top + MV_INSTALL_MENU_HEIGHT + 10;
	Usage_Rect.left = MV_INSTALL_MENU_X + 200;
	Usage_Rect.right = (ScreenWidth - MV_INSTALL_MENU_X*2) - 50;
	
	if ( acUsbDevCount > 1 )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(Usage_Rect.left),ScalerHeigthPixel(Usage_Rect.top), ScalerWidthPixel(Usage_Rect.right - Usage_Rect.left),ScalerHeigthPixel(Usage_Rect.bottom - Usage_Rect.top) );
		sprintf(acTemp_str, "<<  %s (%d/%d)  >>", CS_MW_LoadStringByIdx(CSAPP_STR_PARTITION), u8ThisHDD + 1, acUsbDevCount);
		CS_MW_DrawText(hdc, acTemp_str, -1, &Usage_Rect, DT_CENTER);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(Usage_Rect.left),ScalerHeigthPixel(Usage_Rect.top), ScalerWidthPixel(Usage_Rect.right - Usage_Rect.left),ScalerHeigthPixel(Usage_Rect.bottom - Usage_Rect.top) );
		sprintf(acTemp_str, "%s (1/1)", CS_MW_LoadStringByIdx(CSAPP_STR_PARTITION));
		CS_MW_DrawText(hdc, acTemp_str, -1, &Usage_Rect, DT_CENTER);
	}
}

void MV_Draw_SInfo_MenuBar(HDC hdc)
{
	U16 	i = 0;
	RECT	Usage_Rect;
	U16 	LevelValue = 0;

	MV_Draw_SInfo_Part_Info(hdc);

	Usage_Rect.top = STORAGE_USAGE_Y;
	Usage_Rect.bottom = Usage_Rect.top + MV_INSTALL_MENU_HEIGHT + 10;
	Usage_Rect.left = MV_INSTALL_MENU_X + 200;
	Usage_Rect.right = (ScreenWidth - MV_INSTALL_MENU_X*2) - 50;

	LevelValue = (U16)(( stfile_size.blocks - stfile_size.avail ) / ( stfile_size.blocks / 100 ));

	//printf("===( %ld - %ld ) = %ld === %ld =======\n", stfile_size.blocks, stfile_size.avail, ( stfile_size.blocks - stfile_size.avail ), (( stfile_size.blocks - stfile_size.avail ) * 100));
	//printf("=============== %ld ===================\n", stfile_size.blocks);

	for( i = 0 ; i < CSAPP_SINFO_ITEM_MAX ; i++ )
	{
		MV_Draw_SInfoMenuBar(hdc, i);
	}
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_Draw_LevelBar(hdc, &Usage_Rect, LevelValue, EN_ITEM_SIGNAL_LEVEL);
}

void MV_Draw_Format_Window(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	Format_Window_Status = TRUE;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WARNING_TOP),ScalerWidthPixel(WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WARNING_DY));
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_DX),ScalerHeigthPixel(WARNING_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	Temp_Rect.top 	= WARNING_TOP + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_LEFT + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + WARNING_DX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_USB_FORMAT);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY) );

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X + 40), ScalerHeigthPixel(WARNING_ITEM_Y + MV_INSTALL_MENU_BAR_H), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(WARNING_ITEM_X + 40 + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(WARNING_ITEM_Y + MV_INSTALL_MENU_BAR_H), "FAT32 Format"/*CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY)*/);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X + 40), ScalerHeigthPixel(WARNING_ITEM_Y + (MV_INSTALL_MENU_BAR_H*2)), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(WARNING_ITEM_X + 40 + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2), ScalerHeigthPixel(WARNING_ITEM_Y + (MV_INSTALL_MENU_BAR_H*2)), "EXT2 Format"/*CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY)*/);

	MV_EndPaint(hwnd,hdc);
}

void MV_Format_Window_Close( HWND hwnd )
{
	HDC		hdc;

	Format_Window_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

CSAPP_Applet_t MVApp_Storage_Info(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_USBInfo_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "storageinfo";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = Storageinfo_Msg_cb;
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

	return CSApp_USBInfo_Applets;   
}

int Storageinfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
   
	switch(message)
	   	{
			case MSG_CREATE:
				u8ThisHDD = 0;
				Format_State = FAT_FORMAT;
				sprintf(USBfile, "%s", stPartition_data[u8ThisHDD].acPartition_Dir);
				//MV_Load_USBData_Test();
				break;
				
			case MSG_PAINT:
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_MEDIA, EN_ITEM_FOCUS_STORAGE_INFO);
				
				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
				EndPaint(hwnd,hdc);
				
				MV_Get_USB_Info(u8ThisHDD, &stfile_size);
				MV_Load_USBData_File();

				hdc=BeginPaint(hwnd);
				Close_Msg_Window(hdc);
				EndPaint(hwnd,hdc);

				hdc=BeginPaint(hwnd);
				MV_Draw_SInfo_MenuBar(hdc);
				//MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_SYSINFO);
				MV_Media_draw_help_banner(hdc, EN_ITEM_FOCUS_STORAGE_INFO);
				EndPaint(hwnd,hdc);
				
				return 0;

			case MSG_TIMER:
				break;

			case MSG_USB_MSG:
				if ( UsbCon_GetStatus() != USB_STATUS_MOUNTED )
				{	
					CSApp_USBInfo_Applets=CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
				}
				break;
				
			case MSG_KEYDOWN:
				/************************************** Confirm Process *****************************************/
				if ( MV_Check_Confirm_Window() == TRUE )
				{
					MV_Confirm_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
					{
						if ( wparam == CSAPP_KEY_ENTER )
						{
							if ( MV_Check_YesNo() == TRUE )
							{
								char 	ShellCommand[256];

								memset(ShellCommand, 0x00, 256);

								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
								
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

//SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
								Disc_Ani_Init();

								if ( Format_State == FAT_FORMAT )
								{
									sprintf(ShellCommand, "umount %s", stPartition_data[u8ThisHDD].acPartition_Dir);
									printf("===>>> %s\n", ShellCommand);
									system(ShellCommand);
									
									sprintf(ShellCommand, "mkfs.vfat /dev/%s", stPartition_data[u8ThisHDD].acPartition_Data);

									if ( system(ShellCommand) == 0 )
										printf("===>>> Success Format FAT32 : %s\n", ShellCommand);
									else
										printf("\n===>>> Fail Format FAT32 : %s\n\n", ShellCommand);

									sprintf(ShellCommand, "mount -t vfat /dev/%s %s", stPartition_data[u8ThisHDD].acPartition_Data, stPartition_data[u8ThisHDD].acPartition_Dir);
									printf("===>>> %s\n", ShellCommand);

									if ( system(ShellCommand) == 0 )
									{
										MV_Get_USB_Format_Type(u8ThisHDD);
										sprintf(USBfile, "%s", stPartition_data[u8ThisHDD].acPartition_Dir);
										
										MV_Get_USB_Info(u8ThisHDD, &stfile_size);
										MV_Load_USBData_File();

										hdc=BeginPaint(hwnd);
//KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
										Disc_Ani_Stop();
										
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);

										hdc=BeginPaint(hwnd);
										MV_Draw_SInfo_MenuBar(hdc);
										EndPaint(hwnd,hdc);
									} else {
										hdc=BeginPaint(hwnd);
//KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
										Disc_Ani_Stop();
										
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);
									}
								}
								else
								{
									sprintf(ShellCommand, "umount %s", stPartition_data[u8ThisHDD].acPartition_Dir);
									printf("===>>> %s\n", ShellCommand);
									system(ShellCommand);
									
									sprintf(ShellCommand, "mkfs.ext2 /dev/%s", stPartition_data[u8ThisHDD].acPartition_Data);

									if ( system(ShellCommand) == 0 )
										printf("===>>> Success Format EXT2 : %s\n", ShellCommand);
									else
										printf("\n===>>> Fail Format EXT2 : %s\n\n", ShellCommand);

									sprintf(ShellCommand, "mount -t ext2 /dev/%s %s", stPartition_data[u8ThisHDD].acPartition_Data, stPartition_data[u8ThisHDD].acPartition_Dir);
									printf("===>>> %s\n", ShellCommand);

									if ( system(ShellCommand) == 0 )
									{
										MV_Get_USB_Format_Type(u8ThisHDD);
										sprintf(USBfile, "%s", stPartition_data[u8ThisHDD].acPartition_Dir);
										
										MV_Get_USB_Info(u8ThisHDD, &stfile_size);
										MV_Load_USBData_File();

										hdc=BeginPaint(hwnd);
//KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
										Disc_Ani_Stop();
										
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);

										hdc=BeginPaint(hwnd);
										MV_Draw_SInfo_MenuBar(hdc);
										EndPaint(hwnd,hdc);
									} else {
										hdc=BeginPaint(hwnd);
//KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
										Disc_Ani_Stop();
										
										Close_Msg_Window(hdc);
										EndPaint(hwnd,hdc);
									}
								}
							} else {
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
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
						if ( Format_Window_Status == TRUE )
						{
							Format_State = FAT_FORMAT;
							MV_Format_Window_Close( hwnd );
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
						} else {
							if ( acUsbDevCount > 0 )
								MV_Draw_Format_Window(hwnd);
						}
						break;
					case CSAPP_KEY_GREEN:
						if ( Format_Window_Status == TRUE )
						{
							Format_State = EXT_FORMAT;
							MV_Format_Window_Close( hwnd );
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
						}
						break;
					case CSAPP_KEY_RIGHT:
						u8ThisHDD++;
						if ( u8ThisHDD >= acUsbDevCount )
							u8ThisHDD = 0;

						sprintf(USBfile, "%s", stPartition_data[u8ThisHDD].acPartition_Dir);
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
						EndPaint(hwnd,hdc);
						
						MV_Get_USB_Info(u8ThisHDD, &stfile_size);
						MV_Load_USBData_File();

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);

						hdc=BeginPaint(hwnd);
						MV_Draw_SInfo_MenuBar(hdc);
						EndPaint(hwnd,hdc);
						break;
					case CSAPP_KEY_LEFT:
						if ( u8ThisHDD == 0 )
							u8ThisHDD = acUsbDevCount - 1;
						else
							u8ThisHDD--;

						sprintf(USBfile, "%s", stPartition_data[u8ThisHDD].acPartition_Dir);
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
						EndPaint(hwnd,hdc);
						
						MV_Get_USB_Info(u8ThisHDD, &stfile_size);
						MV_Load_USBData_File();

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);

						hdc=BeginPaint(hwnd);
						MV_Draw_SInfo_MenuBar(hdc);
						EndPaint(hwnd,hdc);
						break;
					case CSAPP_KEY_ESC:
						if ( Format_Window_Status == TRUE )
						{
							MV_Format_Window_Close( hwnd );
						}
						else
						{
							CSApp_USBInfo_Applets=CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
						
					case CSAPP_KEY_ENTER:
					case CSAPP_KEY_MENU:
						if ( Format_Window_Status == TRUE )
						{
							MV_Format_Window_Close( hwnd );
						}
						else
						{
							CSApp_USBInfo_Applets=b8Last_App_Status;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_IDLE:
						CSApp_USBInfo_Applets = CSApp_Applet_Sleep;
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



