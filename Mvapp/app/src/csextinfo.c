#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csextinfo.h"
#include "ui_common.h"
#include "sattp.h"
#include "dvbtuner.h"
#include "eit_engine.h"
#include "database.h"
#include "date_time.h"
#include "csepg.h"

static CSAPP_Applet_t		CSApp_ExtInfo_Applets;
static U16					u16Service_Index;
static MV_stServiceInfo 	stServiceData;
static TunerSignalState_t 	Siganl_State;
static BITMAP				btCapture;
static BOOL					b8Big_Signal_Flag = FALSE;

enum
{
	EPG_DRAW_INIT=0,	
	EPG_DRAW_PAGE,
	EPG_DRAW_SCROLL_UP,
	EPG_DRAW_SCROLL_DOWN,
	EPG_DRAW_UPDATE,
	EPG_DRAW_NULL
};

extern U32					*Tuner_HandleId;

extern void Draw_Number_Icon(HDC hdc, int x, int y, char* text);
extern void EpgDrawDetailedBox(HDC hdc, U8 mode, U8 type);

static int ExtInfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_Extend_Information_Back(HDC hdc)
{
	RECT		Temp_Rect;
	
	Temp_Rect.top 		= EXTINFO_TITLE_Y;
	Temp_Rect.bottom	= Temp_Rect.top + EXTINFO_TITLE_DY;
	Temp_Rect.left		= EXTINFO_TITLE_X;
	Temp_Rect.right		= Temp_Rect.left + EXTINFO_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_CH_INFO);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(EXTINFO_MAIN_X), ScalerHeigthPixel(EXTINFO_MAIN_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(EXTINFO_MAIN_X + EXTINFO_MAIN_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(EXTINFO_MAIN_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(EXTINFO_MAIN_X), ScalerHeigthPixel(EXTINFO_MAIN_Y + EXTINFO_MAIN_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(EXTINFO_MAIN_X + EXTINFO_MAIN_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(EXTINFO_MAIN_Y + EXTINFO_MAIN_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(EXTINFO_MAIN_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(EXTINFO_MAIN_Y),ScalerWidthPixel(EXTINFO_MAIN_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(EXTINFO_MAIN_DY));
	FillBox(hdc,ScalerWidthPixel(EXTINFO_MAIN_X), ScalerHeigthPixel(EXTINFO_MAIN_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(EXTINFO_MAIN_DX),ScalerHeigthPixel(EXTINFO_MAIN_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
}

void MV_Draw_Extend_Information_Channel(HDC hdc)
{
	U32						u32Position_X = EXTINFO_CH_ICON_X;
	tMWStream				SUBTTXstream;
	char					Temp_Str[64];
	MV_stSatInfo 			Temp_SatInfo;
	MV_stTPInfo 			Temp_TPInfo;
	/* By KB Kim 2011.01.20 */
	U8                  	tvRadio;

	tCS_EIT_Event_t			present;
	tCS_EIT_Event_t			follow;
	tCS_EIT_Error			epgResult;
	U16						EpgCharPerLine = 38;
	char					*EpgCurrentDetailInfo= NULL;
	CSVID_SequenceHeader	B_hdr;
	
	MV_DB_GetServiceDataByIndex(&stServiceData, u16Service_Index);

/* Channel Name ********************************************/
	sprintf(Temp_Str, "%s", stServiceData.acServiceName);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc, CSAPP_WHITE_COLOR);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_NAME_Y), Temp_Str);	
	
/* AC3 ********************************************/	
	if(stServiceData.u8AC3Flag == TRUE)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_DOLBY_FO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_DOLBY_FO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_DOLBY_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON]);

/* Video Type ********************************************/	
	u32Position_X += MV_BMP[MVBMP_INFO_DOLBY_FO_ICON].bmWidth;

	B_hdr = MV_Get_Seq_Header();
	if ( B_hdr.h >= 720 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_HD_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_HD_UNFO_ICON]);

/* Scramble ********************************************/	
	u32Position_X += MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON].bmWidth;
	
	if(stServiceData.u8Scramble == TRUE)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_SCRAM_FO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_SCRAM_FO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_SCRAM_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_SCRAM_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_SCRAM_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_SCRAM_UNFO_ICON]);

/* TTX ********************************************/	
	u32Position_X += MV_BMP[MVBMP_INFO_SCRAM_FO_ICON].bmWidth;
	CS_MW_GetTeletextStream(&SUBTTXstream);
	
	if(SUBTTXstream.Number == 0)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_TTX_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_TTX_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_TTX_UNFO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_TTX_FO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_TTX_FO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_TTX_FO_ICON]);

/* SUBTitle ********************************************/	
	u32Position_X += MV_BMP[MVBMP_INFO_TTX_UNFO_ICON].bmWidth;
	CS_MW_GetSubtitleStream(&SUBTTXstream);

	if(SUBTTXstream.Number == 0)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_SUBT_FO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_SUBT_FO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_SUBT_FO_ICON]);

/* Favorite ********************************************/
	u32Position_X += MV_BMP[MVBMP_INFO_SUBT_FO_ICON].bmWidth;
	
	/* By KB Kim 2011.01.20 */
	if (stServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
	{
		tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
	}
	else
	{
		tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	}
	if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, stServiceData.u16ChIndex) < MV_MAX_FAV_KIND )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_FAV_FO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_FAV_FO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_FAV_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(u32Position_X),ScalerHeigthPixel(EXTINFO_CH_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_FAV_UNFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_FAV_UNFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_FAV_UNFO_ICON]);

/* Video Resolution ******************************************/
	sprintf(Temp_Str, "%d x %d - %d", B_hdr.w, B_hdr.h, B_hdr.frame_rate);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_CAS_Y), Temp_Str);

/* Satellite Name ********************************************/
	MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, stServiceData.u16ChIndex);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_SAT_Y), Temp_SatInfo.acSatelliteName);	

/* TP Data ********************************************/
	MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, stServiceData.u16ChIndex);
	if ( Temp_TPInfo.u8Polar_H == P_H)
		sprintf(Temp_Str, "%d/%s/%d", Temp_TPInfo.u16TPFrequency, "H", Temp_TPInfo.u16SymbolRate);
	else
		sprintf(Temp_Str, "%d/%s/%d", Temp_TPInfo.u16TPFrequency, "V", Temp_TPInfo.u16SymbolRate);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_TP_Y), Temp_Str);	

/* Video PID ********************************************/
	sprintf(Temp_Str, "%s : %d", CS_MW_LoadStringByIdx(CSAPP_STR_VPID), stServiceData.u16VideoPid);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_VPID_Y), Temp_Str);	

/* Audio PID ********************************************/
	sprintf(Temp_Str, "%s : %d", CS_MW_LoadStringByIdx(CSAPP_STR_APID), stServiceData.u16AudioPid);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_APID_Y), Temp_Str);

/* PCR PID ********************************************/
	sprintf(Temp_Str, "%s : %d", CS_MW_LoadStringByIdx(CSAPP_STR_PPID), stServiceData.u16PCRPid);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_PPID_Y), Temp_Str);

/* Program ID ********************************************/
	sprintf(Temp_Str, "Service ID : %d", stServiceData.u16ServiceId);
	CS_MW_TextOut(hdc, ScalerWidthPixel(EXTINFO_CH_NAME_X), ScalerHeigthPixel(EXTINFO_CH_PGMPID_Y), Temp_Str);

/* Extended Description **********************************/
	epgResult = CS_EIT_Get_PF_Event(stServiceData.u16TransponderIndex , stServiceData.u16ServiceId, &present, &follow);
	if (epgResult == eCS_EIT_NO_ERROR)
	{
		U32			Epg_Desc_Line = 0 ;
		U32 		pointer = 0;
		U32 		lineCounter = 0;
		RECT		lRect;

		Epg_Desc_Line = MvConvertTextforWindow(present.description_data, &EpgCurrentDetailInfo, EpgCharPerLine, present.descriptionLength);

		SetTextColor(hdc, MVAPP_YELLOW_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		lRect.top = EXTINFO_CH_NAME_Y;
		lRect.bottom = lRect.top + 30;
		lRect.left = EXTINFO_SIGNAL_X;
		lRect.right = lRect.left + EXTINFO_SIGNAL_DX;
		MV_MW_DrawText_Static( hdc, present.event_name, -1, &lRect, DT_LEFT );
		// printf("%s\n", present.event_name);

		lRect.top = EXTINFO_CH_NAME_Y + 40;
		lRect.bottom = lRect.top + 30;
		SetTextColor(hdc, CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		while( (lineCounter < Epg_Desc_Line) && (lineCounter < 8) )
		{
			MV_MW_DrawText_Static(hdc, (EpgCurrentDetailInfo + pointer), -1, &lRect, DT_LEFT);
			// printf("%s\n", (EpgCurrentDetailInfo + pointer));
			
			lRect.top = lRect.top + 22; 
			lRect.bottom = lRect.top + 22;
			
			pointer += EpgCharPerLine;
			lineCounter++;
		}
	}
}

void Show_ExtInfo_Signal(HDC hdc)
{	
	RECT	Temp_Rect;

	TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);
	//printf("\n ====== %d % ==== %d % ======\n", Siganl_State.Strength, Siganl_State.Quality);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
	Temp_Rect.top 		= EXTINFO_SIGNAL_Y1;
	Temp_Rect.bottom	= Temp_Rect.top + EXTINFO_SIGNAL_DY;
	Temp_Rect.left		= EXTINFO_SIGNAL_X - 10;
	Temp_Rect.right		= EXTINFO_SIGNAL_X + EXTINFO_SIGNAL_DX + 10;
	
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Strength, EN_ITEM_CHEDIT_SIGNAL_LEVEL);

	Temp_Rect.top 		= EXTINFO_SIGNAL_Y2;
	Temp_Rect.bottom	= Temp_Rect.top + EXTINFO_SIGNAL_DY;
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Quality, EN_ITEM_CHEDIT_SIGNAL_LEVEL);
}

//#define USE_BMP
void Show_ExtInfo_BIG_Signal(HDC hdc)
{	
	U16		u16Signal_length;
	char	Temp_Str[10];

	TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);
	//printf("\n ====== %d % ==== %d % ======\n", Siganl_State.Strength, Siganl_State.Quality);

#ifdef USE_BMP	
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y),ScalerWidthPixel(BIG_SIGNAL_BACK_DX),ScalerHeigthPixel(BIG_SIGNAL_BACK_DY));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1), ScalerWidthPixel(BIG_SIGNAL_DX), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_GRAY_SIGNAL]);

	u16Signal_length = (BIG_SIGNAL_DX * Siganl_State.Strength)/100;

	if ( Siganl_State.Strength < 50 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_RED_SIGNAL]);
	else if ( Siganl_State.Strength >= 50 && Siganl_State.Strength < 80 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_ORANGE_SIGNAL]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_GREEN_SIGNAL]);

	
	sprintf(Temp_Str, "%d", Siganl_State.Strength);
	Draw_Number_Icon(hdc, BIG_SIGNAL_VALUE_X, BIG_SIGNAL_Y1 + 10, Temp_Str);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2), ScalerWidthPixel(BIG_SIGNAL_DX), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_GRAY_SIGNAL]);

	u16Signal_length = (BIG_SIGNAL_DX * Siganl_State.Quality)/100;

	if ( Siganl_State.Strength < 50 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_RED_SIGNAL]);
	else if ( Siganl_State.Strength >= 50 && Siganl_State.Strength < 80 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_ORANGE_SIGNAL]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2), ScalerWidthPixel( u16Signal_length ), ScalerHeigthPixel(BIG_SIGNAL_DY), &MV_BMP[MVBMP_GREEN_SIGNAL]);

	sprintf(Temp_Str, "%d", Siganl_State.Quality);
	Draw_Number_Icon(hdc, BIG_SIGNAL_VALUE_X, BIG_SIGNAL_Y2 + 10, Temp_Str);
#else
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y),ScalerWidthPixel(BIG_SIGNAL_BACK_DX),ScalerHeigthPixel(BIG_SIGNAL_BACK_DY));

	SetBrushColor(hdc, MVAPP_BLACK_COLOR);
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1),ScalerWidthPixel(BIG_SIGNAL_DX),ScalerHeigthPixel(BIG_SIGNAL_DY));

	u16Signal_length = (BIG_SIGNAL_DX * Siganl_State.Strength)/100;

	if ( Siganl_State.Strength < 50 )
		SetBrushColor(hdc, MVAPP_RED_COLOR);
	else if ( Siganl_State.Strength >= 50 && Siganl_State.Strength < 75 )
		SetBrushColor(hdc, MVAPP_ORANGE_COLOR);
	else
		SetBrushColor(hdc, MVAPP_LIGHT_GREEN_COLOR);

	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y1),ScalerWidthPixel(u16Signal_length),ScalerHeigthPixel(BIG_SIGNAL_DY));

	
	sprintf(Temp_Str, "%d", Siganl_State.Strength);
	Draw_Number_Icon(hdc, BIG_SIGNAL_VALUE_X, BIG_SIGNAL_Y1 + 10, Temp_Str);
/*-----------------------------------------------------------------------------------------------------------*/
	SetBrushColor(hdc, MVAPP_BLACK_COLOR);
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2),ScalerWidthPixel(BIG_SIGNAL_DX),ScalerHeigthPixel(BIG_SIGNAL_DY));

	u16Signal_length = (BIG_SIGNAL_DX * Siganl_State.Quality)/100;

	if ( Siganl_State.Quality < 50 )
		SetBrushColor(hdc, MVAPP_RED_COLOR);
	else if ( Siganl_State.Quality >= 50 && Siganl_State.Quality < 75 )
		SetBrushColor(hdc, MVAPP_ORANGE_COLOR);
	else
		SetBrushColor(hdc, MVAPP_LIGHT_GREEN_COLOR);

	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_X), ScalerHeigthPixel(BIG_SIGNAL_Y2),ScalerWidthPixel(u16Signal_length),ScalerHeigthPixel(BIG_SIGNAL_DY));

	sprintf(Temp_Str, "%d", Siganl_State.Quality);
	Draw_Number_Icon(hdc, BIG_SIGNAL_VALUE_X, BIG_SIGNAL_Y2 + 10, Temp_Str);

#endif
}

void MV_Draw_Big_Signal_Back(HDC hdc)
{
	b8Big_Signal_Flag = TRUE;
	memset(&btCapture, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y), ScalerWidthPixel(BIG_SIGNAL_BACK_DX), ScalerHeigthPixel(BIG_SIGNAL_BACK_DY), &btCapture);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X + BIG_SIGNAL_BACK_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y + BIG_SIGNAL_BACK_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X + BIG_SIGNAL_BACK_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y + BIG_SIGNAL_BACK_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_BACK_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y),ScalerWidthPixel(BIG_SIGNAL_BACK_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(BIG_SIGNAL_BACK_DY));
	FillBox(hdc,ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(BIG_SIGNAL_BACK_DX),ScalerHeigthPixel(BIG_SIGNAL_BACK_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	Show_ExtInfo_BIG_Signal(hdc);
}

void MV_Draw_Big_Signal_Close(HDC hdc)
{
	MV_FillBoxWithBitmap(hdc, ScalerWidthPixel(BIG_SIGNAL_BACK_X), ScalerHeigthPixel(BIG_SIGNAL_BACK_Y), ScalerWidthPixel(BIG_SIGNAL_BACK_DX), ScalerHeigthPixel(BIG_SIGNAL_BACK_DY), &btCapture);
	UnloadBitmap (&btCapture);
	b8Big_Signal_Flag = FALSE;
}

CSAPP_Applet_t CSApp_ExtInfo(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_ExtInfo_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "ExtInfo";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = ExtInfo_Msg_cb;
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

	return CSApp_ExtInfo_Applets;   
}


static int ExtInfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
   
	switch(message)
   	{
		case MSG_CREATE:
			u16Service_Index = CS_DB_GetCurrentServiceIndex();
			break;
			
		case MSG_PAINT:
			hdc=BeginPaint(hwnd);
			MV_Draw_Extend_Information_Back(hdc);
			MV_Draw_Extend_Information_Channel(hdc);
			Show_ExtInfo_Signal(hdc);
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			EndPaint(hwnd,hdc);				
			return 0;

		case MSG_TIMER:
			if ( b8Big_Signal_Flag == FALSE )
			{
				hdc = BeginPaint(hwnd);
				Show_ExtInfo_Signal(hdc);
				EndPaint(hwnd,hdc);
			} else {
				hdc = BeginPaint(hwnd);
				Show_ExtInfo_BIG_Signal(hdc);
				EndPaint(hwnd,hdc);
			}
			break;
		
		case MSG_KEYDOWN:
			if ( b8Big_Signal_Flag == FALSE )
			{
				switch(wparam)
				{
					case CSAPP_KEY_INFO:
					case CSAPP_KEY_ENTER:
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						CSApp_ExtInfo_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_F1:
						hdc = BeginPaint(hwnd);
						KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
						MV_Draw_Big_Signal_Back(hdc);
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_ExtInfo_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					default:
						break;
				}
			} else {
				switch(wparam)
				{
					case CSAPP_KEY_INFO:
					case CSAPP_KEY_ENTER:
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_F1:
						hdc = BeginPaint(hwnd);
						KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
						MV_Draw_Big_Signal_Close(hdc);
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_ExtInfo_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					default:
						break;
				}
			}
			break;
		
	   	case MSG_CLOSE:
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;

	   	default:
			break;
   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}



