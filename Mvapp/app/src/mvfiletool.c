#include "linuxos.h"

#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mvrecfile.h"
#include "csmpr_usb.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"
#include "ui_common.h"
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <pwd.h>
#include <grp.h>

#define LIST_MAX_ITEM				10
#define	MAX_FILE_NAME_LENGTH		255
#define	Volume_timer_Max			( 2*1000 )
#define	MEDIA_MUTE_ICON_X			( MV_PIG_RIGHT - 70	)
#define	MEDIA_MUTE_ICON_Y			( MV_PIG_TOP + 20 )
#define VOLUME_X					390//CHEDIT_LIST_LEFT + 30
#define	VOLUME_Y					650
#define	VOLUME_DX					500//450
#define	VOLUME_DY					40
#define	VOLUME_TEXT_X				( VOLUME_X + VOLUME_DX )
#define	VOLUME_TEXT_DX				50
#define	VOLUME_FULL_DX 				( MV_BMP[MVBMP_VOLUME_ICON].bmWidth + VOLUME_DX + VOLUME_TEXT_DX )

static 	CSAPP_Applet_t		CSApp_Filetools_Applets;

static 	stFile_db			stFileDB;
static 	U16					u16Current_index;
static 	U16					u16Current_Focus;
static 	U16					u16Current_Page;
static 	U16					u16Prev_Page;
static 	char 				acCurrent_Dir[MAX_FILE_NAME_LENGTH];
static 	BOOL				File_Delete_Flag = FALSE;
static	BOOL				File_Rename_Flag = FALSE;
static 	char				acCopy_Source[256];
static 	char				acCopy_File[256];
static 	char				acCopy_Dir[256];
static 	BOOL				bCopy_Flag = FALSE;
static 	BOOL				bMove_Flag = FALSE;
static 	U8					u8ThisHDD = 0;
static 	tCSDesktopVolume 	PVR_Volume;
BITMAP						btCap_bmp;
BITMAP						btCap_Volume;

extern void MV_Calculate_Size(long long llong, char *temp);


void MV_Tools_VolumeScroll(BOOL right)
{
	PVR_Volume.Volume=CS_DBU_GetVolume();

	if(CS_AV_Audio_GetMuteStatus() == TRUE)
		CS_AV_Audio_SetMuteStatus(FALSE);
	
	if(right == FALSE)
	{
		if(PVR_Volume.Volume<=0)
			PVR_Volume.Volume = 0;
		else
			PVR_Volume.Volume--;
	}
	else
	{
		PVR_Volume.Volume++;
		if(PVR_Volume.Volume > kCS_DBU_MAX_VOLUME)
			PVR_Volume.Volume = kCS_DBU_MAX_VOLUME;
	}

	if(PVR_Volume.Volume == 0)
	{
		CS_DBU_SetMuteStatus(eCS_DBU_ON);
		CS_AV_Audio_SetMuteStatus(TRUE);
		CS_DBU_SaveMuteStatus();
	}
	else
	{
		if(CS_AV_Audio_GetMuteStatus()==TRUE)
		{
			CS_DBU_SetMuteStatus(eCS_DBU_OFF);
			CS_AV_Audio_SetMuteStatus(FALSE);
			CS_DBU_SaveMuteStatus();
		}
	}
	
	PVR_Volume.Draw=TRUE;
	CS_DBU_SetVolume(PVR_Volume.Volume);

	CS_AV_AudioSetVolume(PVR_Volume.Volume);
	
	CS_DBU_SaveVolume();
	CS_DBU_SaveMuteStatus();
    
}

void MV_Tools_PaintVolume(HDC hdc)
{
	int      	num;
	char		acText[5];

	
	if ( btCap_Volume.bmHeight == 0 )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY), &btCap_Volume);
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY), &btCap_Volume);
	}
	
	if( PVR_Volume.Draw != TRUE && PVR_Volume.OnScreen == TRUE )
	{
		PVR_Volume.OnScreen=FALSE;
#if 0		
		SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY));
#endif
		return;
	}
	
	PVR_Volume.OnScreen = TRUE;
	num = PVR_Volume.Volume;

	FillBoxWithBitmap (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth), ScalerHeigthPixel(VOLUME_Y), ScalerWidthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmHeight), &MV_BMP[MVBMP_VOLUME_ICON]);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_DX + VOLUME_TEXT_DX),ScalerHeigthPixel(VOLUME_DY));
	MV_SetBrushColor(hdc,MVAPP_YELLOW_COLOR);
	FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y+5),ScalerWidthPixel((VOLUME_DX * ((num*100)/kCS_DBU_MAX_VOLUME))/100),ScalerHeigthPixel(VOLUME_DY-10));

	sprintf(acText, "%d", num);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(VOLUME_TEXT_X + 10),ScalerHeigthPixel(VOLUME_Y + 7), acText);
}

void MV_Tools_PaintMute(HDC hdc)
{
	if(CS_AV_Audio_GetMuteStatus() == FALSE)
	{
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(MEDIA_MUTE_ICON_X), ScalerHeigthPixel(MEDIA_MUTE_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight));
	}
	else
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MEDIA_MUTE_ICON_X), ScalerHeigthPixel(MEDIA_MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight), &MV_BMP[MVBMP_MUTE_ICON]);
	}
}

void MV_Sort_Files(void)
{
	int 			i = 0, j = 0;
	U16				Dir_count = 0;
	U16				File_count = 0;
	stFile_db_temp	stSort_Temp;
	stFile_db		stFileDB_Dir;
	stFile_db		stFileDB_File;
	struct stat		statbuffer;
	char			TmpStr[MAX_FILE_NAME_LENGTH];

	memset ( &stSort_Temp, 0x00, sizeof(stFile_db_temp));
	memset ( &stFileDB_Dir, 0x00, sizeof(stFile_db));
	memset ( &stFileDB_File, 0x00, sizeof(stFile_db));

	for ( i = 0 ; i < stFileDB.u16file_Count ; i++ )
	{
		sprintf(TmpStr, "%s/%s", acCurrent_Dir, stFileDB.acFileName[i]);
		
		if( stat(TmpStr, &statbuffer ) != 0 )
			printf("STAT ERROR========================\n");

		if( S_ISDIR(statbuffer.st_mode) )
		{
			strcpy(stFileDB_Dir.acFileName[Dir_count], stFileDB.acFileName[i]);
			strcpy(stFileDB_Dir.acFileExt[Dir_count], stFileDB.acFileExt[i]);
			Dir_count++;
			//printf("DIRECTORY : %d : %s <- %s \n", Dir_count, stFileDB_Dir.acFileName[Dir_count-1] , stFileDB.acFileName[i]);
		} else {
			strcpy(stFileDB_File.acFileName[File_count], stFileDB.acFileName[i]);
			strcpy(stFileDB_File.acFileExt[File_count], stFileDB.acFileExt[i]);
			File_count++;
			//printf("FILE : %d : %s <- %s\n", File_count, stFileDB_File.acFileName[File_count-1] , stFileDB.acFileName[i]);
		}
	}
	stFileDB_Dir.u16file_Count = Dir_count;
	stFileDB_File.u16file_Count = File_count;

	i = 0 ;
	
	while ( i < stFileDB_Dir.u16file_Count )
	{
		for ( j = i ; j < stFileDB_Dir.u16file_Count - 1 ; j++ )
		{
			if ( strcasecmp( stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]) > 0 )
			{
				//printf("=== %s => %s \n", stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]);
				strcpy(stSort_Temp.acFileName, stFileDB_Dir.acFileName[j]);
				strcpy(stSort_Temp.acFileExt, stFileDB_Dir.acFileExt[j]);

				strcpy(stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]);
				strcpy(stFileDB_Dir.acFileExt[j], stFileDB_Dir.acFileExt[j+1]);

				strcpy(stFileDB_Dir.acFileName[j+1], stSort_Temp.acFileName);
				strcpy(stFileDB_Dir.acFileExt[j+1], stSort_Temp.acFileExt);
			}
		}
		i++;
	}

	i = 0;
	
	while ( i < stFileDB_File.u16file_Count )
	{
		for ( j = i ; j < stFileDB_File.u16file_Count - 1 ; j++ )
		{
			if ( strcasecmp( stFileDB_File.acFileName[j], stFileDB_File.acFileName[j+1]) > 0 )
			{
				strcpy(stSort_Temp.acFileName, stFileDB_File.acFileName[j]);
				strcpy(stSort_Temp.acFileExt, stFileDB_File.acFileExt[j]);

				strcpy(stFileDB_File.acFileName[j], stFileDB_File.acFileName[j+1]);
				strcpy(stFileDB_File.acFileExt[j], stFileDB_File.acFileExt[j+1]);

				strcpy(stFileDB_File.acFileName[j+1], stSort_Temp.acFileName);
				strcpy(stFileDB_File.acFileExt[j+1], stSort_Temp.acFileExt);
			}
		}
		i++;
	}

	//printf("\n=== DIR : %d === FILE : %d ===\n", stFileDB_Dir.u16file_Count, stFileDB_File.u16file_Count);

	memset ( &stFileDB, 0x00, sizeof(stFile_db));

	for ( i = 0; i < stFileDB_Dir.u16file_Count ; i++ )
	{
		strcpy(stFileDB.acFileName[i] , stFileDB_Dir.acFileName[i]);
		strcpy(stFileDB.acFileExt[i] , stFileDB_Dir.acFileExt[i]);
		//printf("DIRECTORY : %d : %s -> %s \n", i, stFileDB_Dir.acFileName[i] , stFileDB.acFileName[i]);
	}

	for ( i = 0; i < stFileDB_File.u16file_Count ; i++ )
	{
		strcpy(stFileDB.acFileName[stFileDB_Dir.u16file_Count + i] , stFileDB_File.acFileName[i]);
		strcpy(stFileDB.acFileExt[stFileDB_Dir.u16file_Count + i] , stFileDB_File.acFileExt[i]);
		//printf("FILE : %d : %s -> %s \n", i, stFileDB_File.acFileName[i] , stFileDB.acFileName[stFileDB_Dir.u16file_Count + i]);
	}

	stFileDB.u16file_Count = stFileDB_Dir.u16file_Count + stFileDB_File.u16file_Count;
}

MV_File_Return MV_Load_fileData(char *Read_Dir)
{
	DIR				*pDir = NULL;
	struct dirent 	*pDirEnt;
	struct stat		statbuffer;
	U16				i;
	char			TmpStr[MAX_FILE_NAME_LENGTH];
	
	memset(&stFileDB, 0x00, sizeof(stFile_db));

	if( (pDir=opendir( Read_Dir )) == NULL )
	{
		closedir( pDir );
		printf( "[UsbCon_Mount]:GetUsbDevFileName(): Failed to open dir.\n" );
		return FILE_READ_FAIL;
	}

	if ( strcmp(Read_Dir, USB_ROOT) == 0 )
	{
		while( (pDirEnt=readdir( pDir )) != NULL )
		{
			sprintf(TmpStr, "%s/%s", acCurrent_Dir, pDirEnt->d_name);
			if( stat(TmpStr, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");
			
			if( strcmp( pDirEnt->d_name, "." ) != 0 && strcmp( pDirEnt->d_name, "..") != 0 )
			{
				strcpy(stFileDB.acFileName[stFileDB.u16file_Count], pDirEnt->d_name);

				for ( i = strlen(pDirEnt->d_name) ; i > 0 ; i-- )
				{
					if ( pDirEnt->d_name[i] == '.' )
						break;
				}

				if( S_ISDIR(statbuffer.st_mode) )
					memset(stFileDB.acFileExt[stFileDB.u16file_Count], 0x00, 10);
				else if ( i < strlen(pDirEnt->d_name) - 1 && i > 1 )
				{
					//printf("==== %d , %s , %c , %d ====\n", i, pDirEnt->d_name, pDirEnt->d_name[i+1], strlen(pDirEnt->d_name));
					strncpy(stFileDB.acFileExt[stFileDB.u16file_Count], &pDirEnt->d_name[i+1], strlen(pDirEnt->d_name) - i);
					//printf("==== %s : EXT : %d : %s =======\n", stFileDB.acFileName[stFileDB.u16file_Count], i, stFileDB.acFileExt[stFileDB.u16file_Count]);
				}

				stFileDB.u16file_Count++;
			}
		}
	} else {
		while( (pDirEnt=readdir( pDir )) != NULL )
		{
			sprintf(TmpStr, "%s/%s", acCurrent_Dir, pDirEnt->d_name);
			if( stat(TmpStr, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");
			
			if( strcmp( pDirEnt->d_name, "." ) != 0 )
			{
				strcpy(stFileDB.acFileName[stFileDB.u16file_Count], pDirEnt->d_name);

				if( strcmp( pDirEnt->d_name, ".." ) != 0 )
				{
					for ( i = strlen(pDirEnt->d_name) ; i > 0 ; i-- )
					{
						if ( pDirEnt->d_name[i] == '.' )
							break;
					}

					if( S_ISDIR(statbuffer.st_mode) )
						memset(stFileDB.acFileExt[stFileDB.u16file_Count], 0x00, 10);
					else if ( i < strlen(pDirEnt->d_name) - 1 && i > 1 )
					{
						//printf("==== %d , %s , %c , %d ====\n", i, pDirEnt->d_name, pDirEnt->d_name[i+1], strlen(pDirEnt->d_name));
						strncpy(stFileDB.acFileExt[stFileDB.u16file_Count], &pDirEnt->d_name[i+1], strlen(pDirEnt->d_name) - i);
						//printf("==== %s : EXT : %d : %s =======\n", stFileDB.acFileName[stFileDB.u16file_Count], i, stFileDB.acFileExt[stFileDB.u16file_Count]);
					}
				}
				
				stFileDB.u16file_Count++;
			}
		}
	}

	MV_Sort_Files();
	
	closedir( pDir );
	return FILE_OK;
}

void MV_Draw_FileTool_MenuTitle(HDC hdc)
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

void MV_Draw_FileTool_Partition_Info(HDC hdc)
{
	RECT 	TmpRect;
	char	TmpStr[50];
	char 	TempStr[10];
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT), ScalerHeigthPixel( MV_INSTALL_MENU_Y - 10 ), ScalerWidthPixel(MV_PIG_LIST_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	TmpRect.left	= MV_PIG_LEFT + 20;
	TmpRect.right	= TmpRect.left + (MV_PIG_LIST_DX - 20);
	TmpRect.top		= MV_INSTALL_MENU_Y - 7;
	TmpRect.bottom	= TmpRect.top + MV_INSTALL_MENU_BAR_H;

	strncpy(TempStr, &stPartition_data[u8ThisHDD].acPartition_Dir[strlen(stPartition_data[u8ThisHDD].acPartition_Dir) - 5], 5);
	sprintf(TmpStr, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_PARTITION), TempStr);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
}


void MV_Draw_FileTool_FileInfo(HDC hdc, U16 u16Index)
{
	RECT			TmpRect;
	struct stat		statbuffer;
	char			TmpStr[MAX_FILE_NAME_LENGTH];
	char			TmpStr2[MAX_FILE_NAME_LENGTH];
	struct 			tm *tm_ptr;

	sprintf(TmpStr, "%s/%s", acCurrent_Dir, stFileDB.acFileName[u16Index]);
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

	MV_Calculate_Size( ( statbuffer.st_size / 1024 ), TmpStr2);
	sprintf(TmpStr, "%s : %s [ %lld Byte ]", CS_MW_LoadStringByIdx(CSAPP_STR_SIZE), TmpStr2, statbuffer.st_size);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	tm_ptr = gmtime(&statbuffer.st_mtime);

	sprintf(TmpStr, "%s : %d-%d-%d %d:%d:%d", CS_MW_LoadStringByIdx(CSAPP_STR_TIME), tm_ptr->tm_year+1900,
                    tm_ptr->tm_mon+1,
                    tm_ptr->tm_mday,
                    tm_ptr->tm_hour,
                    tm_ptr->tm_min,
                    tm_ptr->tm_sec);

	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
	//CS_PVR_Player_Stop();
}


void MV_Draw_FileTool_StorageInfo(HDC hdc)
{
//	RECT			TmpRect;
	char			TmpStr[MAX_FILE_NAME_LENGTH];
	char			Temp_Str[MAX_FILE_NAME_LENGTH];
	char			File_Str[MAX_FILE_NAME_LENGTH];
	//struct tm 		*tm_ptr;

	memset (TmpStr, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset (Temp_Str, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	memset (File_Str, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel( MV_PIG_TOP + MV_PIG_DY + 10 ), ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H * 3) );

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
#if 0 
	TmpRect.left	=ScalerWidthPixel(MV_PIG_LEFT + 30);
	TmpRect.right	=TmpRect.left + MV_PIG_DX - 80;
	TmpRect.top		=ScalerHeigthPixel(MV_PIG_TOP + MV_PIG_DY + 10) + 2;
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	sprintf(TmpStr, "Program : %s", PVR_Rec_Data.PVR_Title);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);

	TmpRect.top		=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H);

	sprintf(TmpStr, "Record : %s", PVR_Rec_Data.PVR_Start_UTC);
	CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
#endif
}

void MV_Draw_FileToolMenuBar(HDC hdc, int esItem, U8 u8Kind)
{
	int 			y_gap = MV_INSTALL_MENU_Y + MV_INSTALL_MENU_BAR_H * ( esItem + 1 );
	RECT			TmpRect;
	struct stat		statbuffer;
	char			TmpStr[MAX_FILE_NAME_LENGTH];

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
		TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X) + 6;
		TmpRect.right	=TmpRect.left + (CHEDIT_LIST_DX - 210);
		TmpRect.top		=ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

		sprintf(TmpStr, "%s/%s", acCurrent_Dir, stFileDB.acFileName[esItem + ( u16Current_Page * LIST_MAX_ITEM)]);
		if( stat(TmpStr, &statbuffer ) != 0 )
			printf("STAT ERROR========================\n");

		if( S_ISDIR(statbuffer.st_mode) )
		{
			if ( strcmp ( stFileDB.acFileName[esItem + ( u16Current_Page * LIST_MAX_ITEM)], ".." ) != 0 )				
				FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_DIR_FOLDER].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_DIR_FOLDER].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_DIR_FOLDER].bmHeight), &MV_BMP[MVBMP_DIR_FOLDER]);
		}
		else
		{
			switch( MV_Check_Filetype(stFileDB.acFileExt[esItem + ( u16Current_Page * LIST_MAX_ITEM)] ) )
			{
				case MVAPP_FILE_TS:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_TS_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_TS_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_TS_FILE].bmHeight), &MV_BMP[MVBMP_TS_FILE]);
					break;
				case MVAPP_FILE_MOVIE:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_MOVIE_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_MOVIE_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MOVIE_FILE].bmHeight), &MV_BMP[MVBMP_MOVIE_FILE]);
					break;
				case MVAPP_FILE_MUSIC:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_MUSIC_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_MUSIC_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUSIC_FILE].bmHeight), &MV_BMP[MVBMP_MUSIC_FILE]);
					break;
				case MVAPP_FILE_PIC:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_IMAGE_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_IMAGE_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_IMAGE_FILE].bmHeight), &MV_BMP[MVBMP_IMAGE_FILE]);
					break;
				case MVAPP_FILE_TEXT:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_TEXT_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_TEXT_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_TEXT_FILE].bmHeight), &MV_BMP[MVBMP_TEXT_FILE]);
					break;
				default :
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_NORMAL_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_NORMAL_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_NORMAL_FILE].bmHeight), &MV_BMP[MVBMP_NORMAL_FILE]);
					break;
			}
		}

		TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X) + 10;
		CS_MW_DrawText(hdc, stFileDB.acFileName[esItem + ( u16Current_Page * LIST_MAX_ITEM)], -1, &TmpRect, DT_LEFT);

		if( !(S_ISDIR(statbuffer.st_mode)) )
		{
			TmpRect.left	=TmpRect.right;
			TmpRect.right	=TmpRect.left + 160;

			MV_Calculate_Size( ( statbuffer.st_size / 1024 ), TmpStr);
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

void MV_Draw_FileTool_Help_Icon(HDC hdc)
{
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
#if 1
	if ( btCap_bmp.bmHeight == 0 )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(MV_HELP_ICON_X),ScalerHeigthPixel( MV_HELP_ICON_Y - 10 ), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH - MV_HELP_ICON_X*2), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H), &btCap_bmp);
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MV_HELP_ICON_X),ScalerHeigthPixel( MV_HELP_ICON_Y - 10 ), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH - MV_HELP_ICON_X*2), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H), &btCap_bmp);
	}
#else
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_HELP_ICON_X),ScalerHeigthPixel( MV_HELP_ICON_Y - 10 ), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH - MV_HELP_ICON_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
#endif
	
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));

	if ( bCopy_Flag == FALSE )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_COPY));
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_PASTE));
	}

	if ( bMove_Flag == FALSE )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_MOVE));
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), CS_MW_LoadStringByIdx(CSAPP_STR_PASTE));
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y + 20), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y + 20), CS_MW_LoadStringByIdx(CSAPP_STR_CHANGE_PARTITION));
}

void MV_Draw_FileTool_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < LIST_MAX_ITEM ; i++ )
	{
		if ( i == u16Current_Focus )
			MV_Draw_FileToolMenuBar(hdc, i, FOCUS);
		else
			MV_Draw_FileToolMenuBar(hdc, i, UNFOCUS);
	}
}

void MV_Under_Dir(void)
{
	int		i = 0;

	for ( i = strlen(acCurrent_Dir) ; i > 0 ; i-- )
	{
		if ( acCurrent_Dir[i] == '/' )
		{
			acCurrent_Dir[i] = 0x00;
			break;
		} else {
			acCurrent_Dir[i] = 0x00;
		}
	}
}

CSAPP_Applet_t CSApp_FileTools(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Filetools_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "File Tools";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = FileTool_Msg_cb;
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

	return CSApp_Filetools_Applets;   
}

int FileTool_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc=0;
   
	switch(message)
	   	{
			case MSG_CREATE:
				memset(&btCap_bmp, 0x00, sizeof(BITMAP));
				memset(&btCap_Volume, 0x00, sizeof(BITMAP));
				u16Current_index = 0;
				u16Current_Page = 0;
				u16Current_Focus = 0;
				u8ThisHDD = 0;
				bCopy_Flag = FALSE;
				bMove_Flag = FALSE;
				File_Delete_Flag = FALSE;
				File_Rename_Flag = FALSE;
				memset(acCurrent_Dir, 0x00, MAX_FILE_NAME_LENGTH);
				memset(acCopy_Source, 0x00, 256);
				memset(acCopy_Dir, 0x00, 256);
				memset(acCopy_File, 0x00, 256);
				strncpy( acCurrent_Dir, stPartition_data[u8ThisHDD].acPartition_Dir, strlen(stPartition_data[u8ThisHDD].acPartition_Dir));
				MV_Load_fileData(acCurrent_Dir);
				break;
				
			case MSG_PAINT:
				CS_MW_SetSmallWindow(ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_TOP),ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_PIG_DY));
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_MEDIA, EN_ITEM_FOCUS_FILE_TOOL);
				
				hdc = BeginPaint(hwnd);
				MV_Draw_FileTool_MenuTitle(hdc);
				MV_Draw_FileTool_MenuBar(hdc);
				MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
				MV_Draw_FileTool_StorageInfo(hdc);
				MV_Draw_FileTool_Partition_Info(hdc);
				MV_Draw_FileTool_Help_Icon(hdc);
				EndPaint(hwnd,hdc);
			
				return 0;

			case MSG_USB_MSG:
				if ( UsbCon_GetStatus() != USB_STATUS_MOUNTED )
				{	
					CSApp_Filetools_Applets=CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
				}
				break;

			case MSG_TIMER:
				if(wparam == DESKTOP_VOLUME_TIMER_ID)
				{
					PVR_Volume.Draw = FALSE;
				
					hdc=MV_BeginPaint(hwnd);
					MV_Tools_PaintVolume(hdc);
					MV_EndPaint(hwnd,hdc);

					if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
						KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
				} 
				break;
				
			case MSG_KEYDOWN:
				if ( MV_Check_Confirm_Window() == TRUE )
				{
					struct stat		statbuffer;
					
					MV_Confirm_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
					{
						if ( wparam == CSAPP_KEY_ENTER )
						{
							if ( MV_Check_YesNo() == TRUE )
							{
								if ( File_Delete_Flag == TRUE )
								{
									char	Temp_Str[MAX_FILE_NAME_LENGTH];

									memset(Temp_Str, 0x00, MAX_FILE_NAME_LENGTH);
									sprintf(Temp_Str, "%s/%s", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);
									if( stat(Temp_Str, &statbuffer ) != 0 )
									{
										printf("MAIN STAT ERROR========================\n");
										break;
									}

									if( S_ISDIR(statbuffer.st_mode) == 0 )
									{
										memset(Temp_Str, 0x00, MAX_FILE_NAME_LENGTH);
										sprintf(Temp_Str, "rm %s/'%s'", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);
										system(Temp_Str);

										if ( u16Current_index == stFileDB.u16file_Count )
											u16Current_index--;

										if ( strcmp ( stFileDB.acFileName[u16Current_index], "..") == 0 )
											break;

										MV_Load_fileData(acCurrent_Dir);

										hdc = BeginPaint(hwnd);
										Restore_Confirm_Window(hdc);
										MV_Draw_FileTool_MenuTitle(hdc);
										MV_Draw_FileTool_MenuBar(hdc);
										MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
										MV_Draw_FileTool_StorageInfo(hdc);
										EndPaint(hwnd,hdc);
									}
									File_Delete_Flag = FALSE;
								}
							}else {
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
								File_Delete_Flag = FALSE;
							}
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							File_Delete_Flag = FALSE;
						}
					}
					
					if (wparam != CSAPP_KEY_IDLE)
					{
						break;
					}
				}

				if ( Get_Keypad_Status() == TRUE ) 
				{
					char			sReturn_str[MAX_FILE_NAME_LENGTH];
					char			Temp_Str[MAX_FILE_NAME_LENGTH];
					struct stat		statbuffer;

					memset( sReturn_str, 0x00, MAX_FILE_NAME_LENGTH );
					
					UI_Keypad_Proc(hwnd, wparam);
					
					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
					{
						if ( wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
						{
							if ( Get_Keypad_is_Save() == TRUE )
							{
								Get_Save_Str(sReturn_str);
								
								sprintf(Temp_Str, "%s/%s", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);

								if( stat(Temp_Str, &statbuffer ) != 0 )
								{
									printf("MAIN STAT ERROR========================\n");
									break;
								}
								
								if( S_ISDIR(statbuffer.st_mode) )
									sprintf(Temp_Str, "mv %s/%s/ %s/%s/", acCurrent_Dir, stFileDB.acFileName[u16Current_index], acCurrent_Dir, sReturn_str);
								else
									sprintf(Temp_Str, "mv %s/%s %s/%s.%s", acCurrent_Dir, stFileDB.acFileName[u16Current_index], acCurrent_Dir, sReturn_str, stFileDB.acFileExt[u16Current_index]);

								printf("=== File Rename : %s\n", Temp_Str);
								system(Temp_Str);

								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								MV_Load_fileData(acCurrent_Dir);
								MV_Draw_FileTool_MenuBar(hdc);
								EndPaint(hwnd,hdc);
							}
						} 
						File_Rename_Flag = FALSE;
					}
					break;
				}
				
				switch(wparam)
				{
					case CSAPP_KEY_F1:
						if ( acUsbDevCount > 0 )
						{ 
							if ( u8ThisHDD == 0 )
								u8ThisHDD = acUsbDevCount - 1;
							else
								u8ThisHDD--;

							u16Current_index = 0;
							u16Current_Page = 0;
							u16Current_Focus = 0;
							memset(acCurrent_Dir, 0x00, MAX_FILE_NAME_LENGTH);
							strncpy( acCurrent_Dir, stPartition_data[u8ThisHDD].acPartition_Dir, strlen(stPartition_data[u8ThisHDD].acPartition_Dir));
							printf("==== %s \n", acCurrent_Dir);
							MV_Load_fileData(acCurrent_Dir);
							
							hdc = BeginPaint(hwnd);
							MV_Draw_FileTool_MenuTitle(hdc);
							MV_Draw_FileTool_MenuBar(hdc);
							MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
							MV_Draw_FileTool_Partition_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_F2:
						if ( acUsbDevCount > 0 )
						{
							u8ThisHDD++;
							if ( u8ThisHDD >= acUsbDevCount )
								u8ThisHDD = 0;

							u16Current_index = 0;
							u16Current_Page = 0;
							u16Current_Focus = 0;
							memset(acCurrent_Dir, 0x00, MAX_FILE_NAME_LENGTH);
							strncpy( acCurrent_Dir, stPartition_data[u8ThisHDD].acPartition_Dir, strlen(stPartition_data[u8ThisHDD].acPartition_Dir));
							MV_Load_fileData(acCurrent_Dir);
							
							hdc = BeginPaint(hwnd);
							MV_Draw_FileTool_MenuTitle(hdc);
							MV_Draw_FileTool_MenuBar(hdc);
							MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
							MV_Draw_FileTool_Partition_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
						
					case CSAPP_KEY_RED:
						File_Delete_Flag = TRUE;
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
						break;

					case CSAPP_KEY_GREEN:
						File_Rename_Flag = TRUE;
						MV_Draw_Keypad(hwnd, stFileDB.acFileName[u16Current_index], MAX_FILE_NAME_LENGTH);
						break;

					case CSAPP_KEY_YELLOW:
						if ( bMove_Flag == FALSE )
						{
							if ( bCopy_Flag == FALSE )
							{
								bCopy_Flag = TRUE;
								memset(acCopy_Source, 0x00, 256);
								memset(acCopy_Dir, 0x00, 256);
								memset(acCopy_File, 0x00, 256);
								sprintf(acCopy_Source, "%s/'%s'", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);
								strncpy(acCopy_Dir, acCurrent_Dir, strlen(acCurrent_Dir));
								strcpy(acCopy_File, stFileDB.acFileName[u16Current_index]);
								hdc = BeginPaint(hwnd);
								MV_Draw_FileTool_Help_Icon(hdc);
								EndPaint(hwnd,hdc);
							} else {
								char Command_Str[256];
								char Temp_Str[256];
								
								bCopy_Flag = FALSE;

								sprintf(Temp_Str, "%s/%s", acCurrent_Dir, acCopy_File);
								//printf("Temp_Str : %s ====> \n", Temp_Str);
								//printf("acCopy_Dir : %s ====> \n", acCopy_Dir);
								//printf("acCurrent_Dir : %s : %d ====> \n", acCurrent_Dir, strcmp(acCopy_Dir, acCurrent_Dir));
								if ( access(Temp_Str, F_OK) == 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_SAME_FILE);
									EndPaint(hwnd,hdc);
									
									usleep( 2000*1000 );
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
								else if ( strcmp(acCopy_Dir, acCurrent_Dir) == 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_SAME_DIR);
									EndPaint(hwnd,hdc);

									usleep( 2000*1000 );
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
								else
								{									
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
									EndPaint(hwnd,hdc);

									Disc_Ani_Init();

									sprintf(Command_Str, "cp %s %s", acCopy_Source, acCurrent_Dir);
									//printf("\n= %s ===>\n\n", Command_Str);
									system(Command_Str);

									Disc_Ani_Stop();
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Load_fileData(acCurrent_Dir);
									MV_Draw_FileTool_MenuBar(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
							}
						}
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_BLUE:
						if ( bCopy_Flag == FALSE )
						{
							if ( bMove_Flag == FALSE )
							{
								bMove_Flag = TRUE;
								memset(acCopy_Source, 0x00, 256);
								memset(acCopy_Dir, 0x00, 256);
								memset(acCopy_File, 0x00, 256);
								sprintf(acCopy_Source, "%s/'%s'", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);
								strncpy(acCopy_Dir, acCurrent_Dir, strlen(acCurrent_Dir));
								strcpy(acCopy_File, stFileDB.acFileName[u16Current_index]);						
								hdc = BeginPaint(hwnd);
								MV_Draw_FileTool_Help_Icon(hdc);
								EndPaint(hwnd,hdc);
							} else {
								char Command_Str[256];
								char Temp_Str[256];
								
								bMove_Flag = FALSE;

								sprintf(Temp_Str, "%s/%s", acCurrent_Dir, acCopy_File);
								//printf("Temp_Str : %s ====> \n", Temp_Str);
								//printf("acCopy_Dir : %s ====> \n", acCopy_Dir);
								//printf("acCurrent_Dir : %s ====> \n", acCurrent_Dir);
								if ( access(Temp_Str, F_OK) == 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_SAME_FILE);
									EndPaint(hwnd,hdc);
									
									usleep( 2000*1000 );
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
								else if ( strcmp(acCopy_Dir, acCurrent_Dir) == 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_SAME_DIR);
									EndPaint(hwnd,hdc);

									usleep( 2000*1000 );
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
								else
								{									
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
									EndPaint(hwnd,hdc);

									sprintf(Command_Str, "mv %s '%s'", acCopy_Source, acCurrent_Dir);
									system(Command_Str);
									
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									MV_Load_fileData(acCurrent_Dir);
									MV_Draw_FileTool_MenuBar(hdc);
									MV_Draw_FileTool_Help_Icon(hdc);
									EndPaint(hwnd,hdc);
								}
							}
						}
						break;

					case CSAPP_KEY_UP:
						hdc = BeginPaint(hwnd);
						u16Prev_Page = u16Current_Page;
						MV_Draw_FileToolMenuBar(hdc, u16Current_Focus, UNFOCUS);

						if ( u16Current_index <= 0 )
							u16Current_index = stFileDB.u16file_Count - 1;
						else
							u16Current_index--;

						u16Current_Focus = get_focus_line(&u16Current_Page, u16Current_index, LIST_MAX_ITEM);

						if ( u16Prev_Page != u16Current_Page )
							MV_Draw_FileTool_MenuBar(hdc);
						else
							MV_Draw_FileToolMenuBar(hdc, u16Current_Focus, FOCUS);
						//EndPaint(hwnd,hdc);

						//hdc = BeginPaint(hwnd);
						MV_Draw_FileTool_MenuTitle(hdc);
						MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
						MV_Draw_FileTool_StorageInfo(hdc);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_DOWN:
						hdc = BeginPaint(hwnd);
						u16Prev_Page = u16Current_Page;
						MV_Draw_FileToolMenuBar(hdc, u16Current_Focus, UNFOCUS);

						if ( u16Current_index >= stFileDB.u16file_Count - 1 )
							u16Current_index = 0;
						else
							u16Current_index++;

						u16Current_Focus = get_focus_line(&u16Current_Page, u16Current_index, LIST_MAX_ITEM);

						if ( u16Prev_Page != u16Current_Page )
							MV_Draw_FileTool_MenuBar(hdc);
						else
							MV_Draw_FileToolMenuBar(hdc, u16Current_Focus, FOCUS);
						
						//EndPaint(hwnd,hdc);

						//hdc = BeginPaint(hwnd);
						MV_Draw_FileTool_MenuTitle(hdc);
						MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
						MV_Draw_FileTool_StorageInfo(hdc);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_ENTER:
						{
							struct stat		statbuffer;
							char			Temp_str[MAX_FILE_NAME_LENGTH];

							memset(Temp_str, 0x00, MAX_FILE_NAME_LENGTH );
								
							sprintf(Temp_str, "%s/%s", acCurrent_Dir, stFileDB.acFileName[u16Current_index]);

							//printf("\n\n===== Dir : %s\n\n", Temp_str);
							
							if( stat(Temp_str, &statbuffer ) != 0 )
							{
								printf("MAIN STAT ERROR========================\n");
								break;
							}

							if( S_ISDIR(statbuffer.st_mode) )
							{
								if ( strcmp( stFileDB.acFileName[u16Current_index] , "..") == 0 )
								{
									MV_Under_Dir();
									MV_Load_fileData(acCurrent_Dir);
								} else {
									strncpy(acCurrent_Dir, Temp_str, strlen(Temp_str));
									MV_Load_fileData(acCurrent_Dir);
								}
								
								u16Current_index = 0;
								u16Current_Page = 0;
								u16Current_Focus = 0;
								
								hdc = BeginPaint(hwnd);
								MV_Draw_FileTool_MenuTitle(hdc);
								MV_Draw_FileTool_MenuBar(hdc);
								MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
								MV_Draw_FileTool_StorageInfo(hdc);
								EndPaint(hwnd,hdc);
							}
						}
						break;

					case CSAPP_KEY_MUTE:
						if(CS_AV_Audio_GetMuteStatus() == FALSE)
							CS_AV_Audio_SetMuteStatus(TRUE);
						else
							CS_AV_Audio_SetMuteStatus(FALSE);
						
						CS_DBU_SaveMuteStatus();
						
						hdc=MV_BeginPaint(hwnd);
						MV_Tools_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_VOL_DOWN:
					case CSAPP_KEY_LEFT:
						MV_Tools_VolumeScroll(FALSE);

						hdc=MV_BeginPaint(hwnd);
						MV_Tools_PaintVolume(hdc);
						MV_Tools_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
						break;

					case CSAPP_KEY_VOL_UP:
					case CSAPP_KEY_RIGHT:
						MV_Tools_VolumeScroll(TRUE);

						hdc=MV_BeginPaint(hwnd);
						MV_Tools_PaintVolume(hdc);
						MV_Tools_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
						break;
						
					case CSAPP_KEY_ESC:
						if ( PVR_Volume.OnScreen == TRUE )
						{
							PVR_Volume.Draw = FALSE;
							
							hdc=MV_BeginPaint(hwnd);
							MV_Tools_PaintVolume(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
								KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

							break;
						}
						else if ( strcmp(acCurrent_Dir, stPartition_data[u8ThisHDD].acPartition_Dir) != 0 )
						{
							MV_Under_Dir();
							MV_Load_fileData(acCurrent_Dir);
							
							u16Current_index = 0;
							u16Current_Page = 0;
							u16Current_Focus = 0;
							
							hdc = BeginPaint(hwnd);
							MV_Draw_FileTool_MenuTitle(hdc);
							MV_Draw_FileTool_MenuBar(hdc);
							MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
							MV_Draw_FileTool_StorageInfo(hdc);
							EndPaint(hwnd,hdc);
							break;
						}
						
						CSApp_Filetools_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_MENU:
						if ( PVR_Volume.OnScreen == TRUE )
						{
							PVR_Volume.Draw = FALSE;
							
							hdc=MV_BeginPaint(hwnd);
							MV_Tools_PaintVolume(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
								KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

							break;
						}
						else if ( strcmp(acCurrent_Dir, stPartition_data[u8ThisHDD].acPartition_Dir) != 0 )
						{
							MV_Under_Dir();
							MV_Load_fileData(acCurrent_Dir);
							
							u16Current_index = 0;
							u16Current_Page = 0;
							u16Current_Focus = 0;
							
							hdc = BeginPaint(hwnd);
							MV_Draw_FileTool_MenuTitle(hdc);
							MV_Draw_FileTool_MenuBar(hdc);
							MV_Draw_FileTool_FileInfo(hdc, u16Current_index);
							MV_Draw_FileTool_StorageInfo(hdc);
							EndPaint(hwnd,hdc);
							break;
						}
						
						CSApp_Filetools_Applets=b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_Filetools_Applets = CSApp_Applet_Sleep;
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
				UnloadBitmap(&btCap_bmp);
				UnloadBitmap(&btCap_Volume);
				CS_MW_SetNormalWindow();
				DestroyMainWindow(hwnd);
				PostQuitMessage(hwnd);
				break;

		   	default:
				break;
	   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}



