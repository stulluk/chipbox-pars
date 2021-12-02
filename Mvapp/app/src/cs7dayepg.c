#include "linuxos.h"

#include "eit_engine.h"
#include "database.h"
#include "date_time.h"

#include "mwsvc.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mv_menu_ctr.h"
#include "ui_common.h"
#include "mvosapi.h"
#include "mvtimer.h"

#ifdef DAILY_EPG

#ifdef DEBUG_ON
	// #define EPG_DEBUG_ON
#endif // #ifdef DEBUG_ON

#define EPG_MENU_BAR_H						25
#define EPG_MENU_BUTTON_H					36
#define EPG_LIST_X							MV_MENU_BACK_X
#define EPG_LIST_Y							( MV_MENU_BACK_Y + MV_EPG_CN_PIG_DY + 32 )
#define EPG_DETAIL_Y						( MV_MENU_BACK_Y + 54 )
#define EPG_CHANNEL_NAME_WIDTH      		320
#define EPG_SIMPLE_TIME_WIDTH       		250
#define EPG_SIMPLE_DATA_WIDTH       		486
#define	EPG_DETAIL_DX						420
#define EPG_LIST_DX							( EPG_CHANNEL_NAME_WIDTH + EPG_SIMPLE_TIME_WIDTH + EPG_SIMPLE_DATA_WIDTH )
#define	EPG_LIST_SCROLL_X					( EPG_LIST_X + EPG_LIST_DX + SCROLL_BAR_DX )
#define EPG_DATA_DX							( EPG_SIMPLE_TIME_WIDTH + EPG_SIMPLE_DATA_WIDTH )
#define	EPG_HELP_ICON_X						( EPG_LIST_X + EPG_DETAIL_DX + MV_EPG_CN_PIG_DX + 80 )
#define EPG_HELP_ICON_DX					270
#define EPG_SCROLL_BAR_WIDTH                20
#define	SERVICES_NUM_PER_PAGE_SIMPLE		10
#define EPG_CHANNEL_CHANGE_TIMER            500
#define EPG_CHANNEL_CHANGE_MAX_COUNT        2
#define EPG_CHANNEL_CHANGE_CHECK            1000

#define EPG_DETAIL_WINDOW_X					TIMER_WINDOW_ITEM_X
#define EPG_DETAIL_WINDOW_Y					TIMER_WINDOW_Y
#define EPG_DETAIL_CONTENT_Y				TIMER_WINDOW_CONT_Y
#define EPG_DETAIL_CONTENT_DY				( TIMER_WINDOW_ITEM_DY * 8 )
#define EPG_DETAIL_CONTENT_DX				TIMER_WINDOW_CONT_DX


#define EPG_INFO_UPDATE_TIMER               1000
#define	EPG_NUM_PER_PAGE					10

#define EPG_UP_DOWN_SCROLL

enum
{
	EPG_DRAW_INIT=0,	
	EPG_DRAW_PAGE,
	EPG_DRAW_SCROLL_UP,
	EPG_DRAW_SCROLL_DOWN,
	EPG_DRAW_UPDATE,
	EPG_DRAW_NULL
};

enum
{
	CH_LIST = 0,
	PMG_LIST
};

const U16	EpgWeekIdx[7]={
						CSAPP_STR_EPG_SUN,
						CSAPP_STR_EPG_MON,
						CSAPP_STR_EPG_TUSE,
						CSAPP_STR_EPG_WED,
						CSAPP_STR_EPG_THUS,
						CSAPP_STR_EPG_FRI,
						CSAPP_STR_EPG_SAT
						};

static CSAPP_Applet_t	        CSApp_7DayEpg_Applets;
tCS_DB_ServiceListTriplet		Daily_EpgTriplet;     // was chedit_triplet
tCS_DBU_Service     			Daily_EpgBackTriplet; // was back_triplet
static tCS_DT_Date              Daily_CurrentDate;
static tCS_DT_Time              Daily_CurrentTime;
static tCS_DT_Time              Daily_EventStartTime;
static tCS_DT_Time              Daily_EventDuration;
static U16                      Daily_EventStartMjd;
static U16                      Daily_CurrentMjd;
static BITMAP	                TitleBmp;
static BITMAP	                Timer_Cap_Bmp;
/*
EpgChannel_t                   *FirstEpgChannel = NULL;
*/
tCS_DT_OffsetPolarity   		Daily_EpgUtcOffsetPolarity;
U16                     		Daily_EpgUtcOffset;

static char						*Daily_EpgCurrentDetailInfo= NULL;
static char						*Daily_EpgEventTitle = NULL;
static U32						Daily_EpgDetailInfoLine = 0;
static U32						Daily_EpgNameLine = 0;
static U32						Daily_EpgCharPerLine;
static U16						Daily_EpgTotalService = 0;	 // was Total_Service
static U16						Daily_EpgCurrentService = 0;   // was chlist_Current_Service
static U16						Daily_EpgCurrentPage = 0; // was chlist_Current_Page
static U16						Daily_EpgPrevPageStart = 0; // was chlist_Prev_Page
static U16						Daily_EpgCurrentFocus = 0; // was chlist_Current_Focus
static U16						Daily_EpgCurrentPageStartIndex = 0;

static U16						Daily_7dayEpgTotalService = 0;	 // was Total_Service
static U16						Daily_7dayEpgCurrentService = 0;   // was chlist_Current_Service
static U16						Daily_7dayEpgCurrentPage = 0; // was chlist_Current_Page
static U16						Daily_7dayEpgPrevPageStart = 0; // was chlist_Prev_Page
static U16						Daily_7dayEpgCurrentFocus = 0; // was chlist_Current_Focus
static U16						Daily_7dayEpgCurrentPageStartIndex = 0;

static U8						Daily_date_time_client_id = 0;
static U8						Daily_ColorRow;
static U8						Daily_ColorCol;
static U8						Daily_EpgSatIndex = 0;  // was u8Sat_index
static U8						Daily_EpgCurrentCol = 0; 
static U8						u8Select_Day = 0;
static BOOL						b8Screen_Kind = CH_LIST;
static BOOL						b8Timer_Window_Status = FALSE;
static BOOL						b8Detail_Window_Status = FALSE;
static U8						Current_Item = 0;
static MV_stServiceInfo			ServiceData;
static tCS_TIMER_JobInfo		stEpgTimerJob;
static tCS_EIT_Event_List_t		MvScEventList;  /* By KB Kim 2011.05.28 */
static BOOL						b8Check_Exit = FALSE;

void MV_Draw_Daily_Epg_Item(HDC hdc, int index, U16 u16Focusindex);
void Daily_EpgDrawCnBox(HDC hdc, U8 serviceNumberPerPage, U8 type);

U32 Daily_AdjustYPosition(U32 yPosition, U32 boxH, U32 sourceH)
{
	U32 yCal;

	yCal = yPosition;
	
	if (boxH > sourceH)
	{
		yCal += (boxH - sourceH) / 2;
	}
	else if (boxH < sourceH)
	{
		yCal -= (sourceH - boxH) / 2;
	}

	return yCal;
}

U8 Daily_TimeAdd(tCS_DT_Time timeA, tCS_DT_Time timeB, tCS_DT_Time *timeResult)
{
	U8 dayOver;
	U8 minAdd;

	minAdd = timeA.minute + timeB.minute;
	timeResult->minute = minAdd % 60;
	dayOver            = (minAdd / 60) + timeA.hour + timeB.hour;
	timeResult->hour   = dayOver % 24;
	dayOver            = dayOver / 24;

	return dayOver;
}

void Daily_EpgDrawButton(HDC hdc)
{
	RECT		rc1;
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(EPG_HELP_ICON_X - 10), ScalerHeigthPixel(EPG_DETAIL_Y - 10), ScalerWidthPixel(EPG_HELP_ICON_DX + 20), ScalerHeigthPixel(MV_EPG_CN_PIG_DY + 20));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(EPG_HELP_ICON_X), ScalerHeigthPixel(EPG_DETAIL_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(EPG_HELP_ICON_X), ScalerHeigthPixel(EPG_DETAIL_Y + EPG_MENU_BUTTON_H), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
//	FillBoxWithBitmap(hdc,ScalerWidthPixel(EPG_HELP_ICON_X), ScalerHeigthPixel(EPG_DETAIL_Y + EPG_MENU_BUTTON_H * 2), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(EPG_HELP_ICON_X), ScalerHeigthPixel(EPG_DETAIL_Y + EPG_MENU_BUTTON_H * 2), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(EPG_HELP_ICON_X), ScalerHeigthPixel(EPG_DETAIL_Y + EPG_MENU_BUTTON_H * 3), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);

	rc1.top = EPG_DETAIL_Y;
	rc1.left = EPG_HELP_ICON_X + 10 + MV_BMP[MVBMP_RED_BUTTON].bmWidth;
	rc1.bottom = rc1.top + EPG_MENU_BUTTON_H;
	rc1.right = rc1.left + EPG_HELP_ICON_DX;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_TIMER), -1, &rc1, DT_LEFT);
/*
	rc1.top += EPG_MENU_BUTTON_H;
	rc1.bottom = rc1.top + EPG_MENU_BUTTON_H;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_DETAIL_INFO), -1, &rc1, DT_LEFT);
*/
	rc1.top += EPG_MENU_BUTTON_H;
	rc1.bottom = rc1.top + EPG_MENU_BUTTON_H;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_CHANGE_FOCUS), -1, &rc1, DT_LEFT);

	rc1.top += EPG_MENU_BUTTON_H;
	rc1.bottom = rc1.top + EPG_MENU_BUTTON_H;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_CHANGE_FOCUS), -1, &rc1, DT_LEFT);

	rc1.top += EPG_MENU_BUTTON_H;
	rc1.bottom = rc1.top + EPG_MENU_BUTTON_H;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_EPG), -1, &rc1, DT_LEFT);
}

void Daily_EpgDrawDetailedBox(HDC hdc, U8 mode, U8 type)
{
	U32 boxH = 20;
	U32 boxW = 20;
	U32 boxY = 10;
	U32 boxX = 10;
	U32 pointer;
	U32 maxLine;
	U32 direction = 0;
	static U32 startLine;
	static U32 numberOfLine;
	static U32 lineCounter;
	RECT       charRect;
	RECT       scrollRect;
	BITMAP	   scrollBmp;
	
	if (mode == EN_EPG_TYPE_SIMPLE)
	{
		boxX = EPG_LIST_X + 10;
		boxY = EPG_DETAIL_Y; /* 54 + 20 + 14 */
		boxH = MV_EPG_CN_PIG_DY;
		boxW = EPG_DETAIL_DX - SCROLL_BAR_DX;
		maxLine = boxH / MV_FONT_HEIGHT;
	}
	else
	{
		boxX = EPG_LIST_X + 10;
		boxY = EPG_DETAIL_Y;
		boxH = MV_EPG_CN_PIG_DY;
		boxW = EPG_DETAIL_DX - SCROLL_BAR_DX;
		maxLine = boxH / MV_FONT_HEIGHT;
	}

	charRect.left   = boxX;
	charRect.right  = charRect.left + boxW;
	charRect.top    = boxY;
	charRect.bottom = charRect.top + MV_FONT_HEIGHT;

	switch(type)
	{
		case EPG_DRAW_INIT :
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX - 10), ScalerHeigthPixel(boxY - 10), ScalerWidthPixel(boxW + SCROLL_BAR_DX + 20), ScalerHeigthPixel(boxH + 20));
			break;
		case EPG_DRAW_PAGE :
			startLine = 0;
			numberOfLine = maxLine - Daily_EpgNameLine;
			if (numberOfLine > Daily_EpgDetailInfoLine)
			{
				numberOfLine = Daily_EpgDetailInfoLine;
			}
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < Daily_EpgNameLine)
			{
				MV_MW_DrawText_Static(hdc, (Daily_EpgEventTitle + pointer), -1, &charRect, DT_LEFT);
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += Daily_EpgCharPerLine;
				lineCounter++;
			}
			
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < numberOfLine)
			{
				MV_MW_DrawText_Static(hdc, (Daily_EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += Daily_EpgCharPerLine;
				lineCounter++;
			}
			break;
		case EPG_DRAW_SCROLL_UP :
			if ((startLine + numberOfLine) >= Daily_EpgDetailInfoLine)
			{
				break;
			}
			direction = 1;
		case EPG_DRAW_SCROLL_DOWN :
			if (Daily_EpgDetailInfoLine <= numberOfLine)
			{
				break;
			}

			if ((startLine == 0) && (direction == 0))
			{
				break;
			}
			boxY = boxY + (MV_FONT_HEIGHT * Daily_EpgNameLine);
			boxH = MV_FONT_HEIGHT * (numberOfLine - 1);
			memset(&scrollBmp, 0x00, sizeof(BITMAP));
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			direction = 1 - direction;
			FillBoxWithBitmap(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			UnloadBitmap(&scrollBmp);
			if (direction)
			{
				/* EPG_DRAW_SCROLL_DOWN */
				// boxY = boxY + (MV_FONT_HEIGHT * Daily_EpgNameLine);
				startLine--;
				pointer = Daily_EpgCharPerLine * startLine;
			}
			else
			{
				/* EPG_DRAW_SCROLL_UP */
				boxY = boxY + (MV_FONT_HEIGHT * (numberOfLine - 1));
				pointer = Daily_EpgCharPerLine * (startLine + numberOfLine);
				startLine++;
			}

			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(MV_FONT_HEIGHT));
			charRect.top    = boxY;
			charRect.bottom = charRect.top + MV_FONT_HEIGHT;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc, (Daily_EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
			break;
		default :
			return;
	}

	boxY = EPG_DETAIL_Y; 
	boxH = MV_EPG_CN_PIG_DY;
	scrollRect.left   = boxX + boxW;
	scrollRect.right  = scrollRect.left + SCROLL_BAR_DX;
	scrollRect.top    = boxY;
	scrollRect.bottom = scrollRect.top + boxH;
	MV_Draw_PageScrollBar(hdc, scrollRect, startLine, Daily_EpgDetailInfoLine, (maxLine - Daily_EpgNameLine));

}

void MV_Draw_Daily_Epg_Item_Channel (HDC hdc, int index, U16 u16Focusindex, U8 FocusKind, U8 mode)
{
	char	                    buff1[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	RECT                        scrollRect;
	RECT                        charRect;
	U32                         positionX;
	U32                         positionY;
	U32                         nameWidth;
	U32                         boxH;
	U32                         boxW;
	U8                          colorCount;
	U32                         channelCollor[2]  = {MVAPP_DARKBLUE_COLOR, MVAPP_DARK_GRAY_COLOR};
	tCS_DB_ServiceManageData	channelItemData;
	MV_stServiceInfo			channelServiceData;

	Daily_7dayEpgCurrentService = 0;
	
	if (mode == EN_EPG_TYPE_SIMPLE)
	{
		positionX = MV_MENU_BACK_X + 10;
		positionY = EPG_LIST_Y + EPG_MENU_BAR_H * (index +1) + 46;
		boxW      = EPG_CHANNEL_NAME_WIDTH - 2;
		nameWidth = boxW - 40;
	}
	else
	{
		positionX = MV_MENU_BACK_X + 10;
		positionY = EPG_LIST_Y + EPG_MENU_BAR_H * (index +1) + 46;
		boxW      = EPG_CHANNEL_NAME_WIDTH - 2;
		nameWidth = boxW - 40;
	}
	boxH = EPG_MENU_BAR_H - 2;
	
	colorCount = (Daily_ColorRow + index) % 2;
		
	// printf("%04d : %d : %d : %s ======\n", u16Focusindex, channelItemData.Service_Index, channelServiceData.u16ChIndex, channelServiceData.acServiceName);

	if ( FocusKind == FOCUS)
	{
		if ( b8Screen_Kind == CH_LIST )
		{
			FillBoxWithBitmap (hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &MV_BMP[MVBMP_CHLIST_SELBAR]);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		else
		{
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
	}
	else
	{
		SetBrushColor(hdc, channelCollor[colorCount]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	}

	if ( FocusKind != NOTFOCUS )
	{
		charRect.left   = positionX + 10;
		charRect.top    = Daily_AdjustYPosition(positionY, boxH, MV_FONT_HEIGHT);
		charRect.right  = charRect.left + nameWidth;
		charRect.bottom = charRect.top + MV_FONT_HEIGHT;
		
		memset(buff1, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
		CS_DB_GetCurrentList_ServiceData(&channelItemData, u16Focusindex);
		MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);

#ifdef DEBUG_TEST
		//printf(" %d === %d ========= 1 ========== %d \n", u16Focusindex, channelItemData.Service_Index, u32Count_Loop);
#else
		//printf("============= 1 ========== %04d \n", u16Focusindex + 1);
#endif
		sprintf(buff1, "%04d %s", u16Focusindex + 1, channelServiceData.acServiceName);

		MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_LEFT);

		if(channelServiceData.u8Scramble)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(charRect.right + 8), ScalerHeigthPixel(positionY), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth), ScalerHeigthPixel(boxH), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(charRect.right + 8), ScalerHeigthPixel(positionY), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON].bmWidth), ScalerHeigthPixel(boxH), &MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON]);
		}

		if ( FocusKind == FOCUS )
		{		
			positionX = MV_MENU_BACK_X + 10;
			positionY = EPG_LIST_Y + EPG_MENU_BAR_H * (index +1) + 70;
			scrollRect.top = EPG_LIST_Y + 70;
			scrollRect.left = EPG_LIST_SCROLL_X - 10;
			scrollRect.right = scrollRect.left + SCROLL_BAR_DX;
			scrollRect.bottom = scrollRect.top + EPG_MENU_BAR_H * 10 -2;
			MV_Draw_ScrollBar(hdc, scrollRect, u16Focusindex, Daily_EpgTotalService, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
		}
	}

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X + EPG_CHANNEL_NAME_WIDTH + 10), ScalerHeigthPixel(EPG_LIST_Y + EPG_MENU_BAR_H + 46), ScalerWidthPixel(EPG_DATA_DX), ScalerHeigthPixel(EPG_MENU_BAR_H * 10));

	if ( FocusKind == FOCUS )
	{
		/*
		MvScEventList.NumberOfEvent = 0;
		MvScEventList.StartPoint = 0;
		MvScEventList.EventArray    = NULL;
		*/
		Mv_GetScEventList (channelServiceData.u16TransponderIndex, channelServiceData.u16ServiceId, u8Select_Day, &MvScEventList);
		//printf("Mv_GetScEventList Event Number1 : %d\n", MvScEventList.NumberOfEvent);

		if(MvScEventList.NumberOfEvent > 0)
		{
			Daily_7dayEpgTotalService = MvScEventList.NumberOfEvent;
			if ( u8Select_Day != 0 )
				Daily_7dayEpgCurrentService = 0;
			else
				Daily_7dayEpgCurrentService = MvScEventList.StartPoint;  
			Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);
			Daily_7dayEpgCurrentPageStartIndex = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE;
		}
		MV_Draw_Daily_Epg_Item(hdc, index, u16Focusindex);
	}
}

void MV_Draw_Daily_Epg_Data(HDC hdc, int index, U16 u16Focusindex, U8 u8Mode, U8 u8FocusKind)
{
	char	                    buff1[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	RECT                        charRect;
	U32                         positionX = 0;
	U32                         positionY = 0;
	U32                         boxH;
	U32                         boxW = 0;
	U32                         boxW2 = 0;
	U32                         tmpNumber;
	U8                          colorCount;
	U32                         channelCollor[2]  = {MVAPP_DARKBLUE_COLOR, MVAPP_DARK_GRAY_COLOR};
	tCS_DB_ServiceManageData	channelItemData;
	MV_stServiceInfo			channelServiceData;
	tCS_EIT_Event_t            *current;
	tCS_DT_Time                 endTime;
	tCS_DT_Time                 startTime;
	U16							u16Focus;

	u16Focus = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE + index;

	// printf("MV_Draw_Daily_Epg_Data : Current Page [%d], index[%d], Focus[%d]\n", Daily_7dayEpgCurrentPage, index, u16Focus);
	
	CS_DB_GetCurrentList_ServiceData(&channelItemData, u16Focusindex);
	MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);
	//epgResult = CS_EIT_Get_PF_Event(channelServiceData.u16TransponderIndex , channelServiceData.u16ServiceId, &present, &follow);
	
	boxW      = EPG_SIMPLE_TIME_WIDTH;
	boxW2	  = EPG_SIMPLE_DATA_WIDTH;
	positionY = EPG_LIST_Y + EPG_MENU_BAR_H + 46;
	Daily_EpgCharPerLine = (EPG_DETAIL_DX - SCROLL_BAR_DX) / MV_FONT_WIDTH;
	
	tmpNumber = (boxW - 12) / MV_FONT_WIDTH - 1;
	boxH = EPG_MENU_BAR_H - 2;

	if ( u8Mode == TRUE )
	{
		current = &MvScEventList.EventArray[u16Focus];

		colorCount = index % 2;
		positionX = MV_MENU_BACK_X + EPG_CHANNEL_NAME_WIDTH + 10;
		
		charRect.left   = positionX + 6;
		charRect.top    = positionY + ( EPG_MENU_BAR_H * index );
		charRect.right  = charRect.left + boxW - 12;
		charRect.bottom = charRect.top + MV_FONT_HEIGHT;

		memset(buff1, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
		
		if (current->nameLength > 0)
		{	
			startTime = CS_DT_UTCtoHM(current->start_time_utc);
			endTime = CS_DT_UTCtoHM(current->end_time_utc);
			if ( startTime.hour < 24 && startTime.minute < 60 && endTime.hour < 24 && endTime.minute < 60 )
				sprintf(buff1, "%02d:%02d - %02d:%02d", startTime.hour, startTime.minute, endTime.hour, endTime.minute);
			else
				sprintf(buff1, "-");
		}
		else
		{
			sprintf(buff1, "-");
		}
	
		if ( b8Screen_Kind == CH_LIST )
		{
			SetBrushColor(hdc, channelCollor[colorCount]);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(charRect.top), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		} else if ( b8Screen_Kind == PMG_LIST && u8FocusKind == FOCUS ) {
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(charRect.top), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		} else {
			SetBrushColor(hdc, channelCollor[colorCount]);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(charRect.top), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		}

		MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_CENTER);
		
		memset(buff1, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);

		if (current->nameLength > 0 && current->nameLength < kCS_EIT_MAX_EVENT_NAME_LENGTH)
		{
			if (current->parental_rating > 0)
			{
				sprintf(buff1, "%s (%s) - <%d>", current->event_name, current->short_desc_language, current->parental_rating);
			}
			else
				sprintf(buff1, "%s (%s)", current->event_name, current->short_desc_language);
		}
		else
		{
			sprintf(buff1, "-");
		}
			
		positionX += boxW;
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(charRect.top), ScalerWidthPixel(boxW2 - 2), ScalerHeigthPixel(boxH));

		charRect.left   = positionX + 6;
		charRect.right  = charRect.left + boxW2 - 12;

		MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_LEFT);

		if ( u8FocusKind == FOCUS )
		{
			if (current->nameLength > 0 && current->nameLength < kCS_EIT_MAX_EVENT_NAME_LENGTH)
			{
				Daily_EpgNameLine = MvConvertTextforWindow(buff1, &Daily_EpgEventTitle, Daily_EpgCharPerLine, (current->nameLength + 14));
				if (current->descriptionLength > 0)
				{
					Daily_EpgDetailInfoLine = MvConvertTextforWindow(current->description_data, &Daily_EpgCurrentDetailInfo, Daily_EpgCharPerLine, current->descriptionLength);
					//printf("============= 1 ==========> EpgDetailInfoLine[%d] \n", Daily_EpgDetailInfoLine);
				}
				else
				{
					Daily_EpgDetailInfoLine = 0;
				}

				Daily_EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_PAGE);
			}
		}
	} 
	else
	{
		colorCount = index % 2;
		positionY    = positionY + ( EPG_MENU_BAR_H * index );
		positionX = MV_MENU_BACK_X + EPG_CHANNEL_NAME_WIDTH + 10;
		sprintf(buff1, "-");

		SetBrushColor(hdc, channelCollor[colorCount]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));

		// SetTextColor(hdc,CSAPP_WHITE_COLOR);
		// SetBkMode(hdc,BM_TRANSPARENT);
		// MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_CENTER);
		
		positionX += boxW;
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW2 - 2), ScalerHeigthPixel(boxH));

		// charRect.left   = positionX + 6;
		// charRect.right  = charRect.left + boxW2 - 12;
		// MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_CENTER);
		/*

		if (current->nameLength > 0 && current->nameLength < kCS_EIT_MAX_EVENT_NAME_LENGTH)
			Daily_EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_INIT);
			*/
	}
}

void MV_Draw_Daily_Epg_Item(HDC hdc, int index, U16 u16Focusindex)
{
	int							i = 0;
	U16							u16Focus;

	/* By KB Kim 2011.05.28 */
	if ( MvScEventList.NumberOfEvent == 0 )
	{
		i = index;   /// Just .. Worning
	} else {
		for ( i = 0 ; i < EPG_NUM_PER_PAGE ; i++ )
		{
			u16Focus = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE + i;
			if ( MvScEventList.NumberOfEvent <= u16Focus )
			{
				MV_Draw_Daily_Epg_Data(hdc, i, u16Focusindex, FALSE, UNFOCUS);
			}
			else
			{
				if( Daily_7dayEpgCurrentService == u16Focus && b8Screen_Kind == PMG_LIST )
					MV_Draw_Daily_Epg_Data(hdc, i, u16Focusindex, TRUE, FOCUS);
				else if ( i == 0 && b8Screen_Kind == CH_LIST )
					MV_Draw_Daily_Epg_Data(hdc, i, u16Focusindex, TRUE, FOCUS);
				else
					MV_Draw_Daily_Epg_Data(hdc, i, u16Focusindex, TRUE, UNFOCUS);
			}
		}
	}
}

void Daily_EpgDrawCnBox(HDC hdc, U8 serviceNumberPerPage, U8 type)
{
	U8      		scrollDirection = 0;
	U8      		colorCount;
	U8      		row;
	U16     		index;
	U32     		positionX;
	U32    			positionY;
	U32     		boxH;
	U32     		boxW;
	RECT    		charRect;
	RECT    		scrollRect;
	BITMAP			scrollBmp;
	char			Temp_Str[50];
	int				i = 0;
	tCS_DT_Date		Temp_CurrentDate;

	memset(Temp_Str, 0x00, 50 );
	boxH = EPG_MENU_BAR_H - 2;

	switch(type)
	{
		case EPG_DRAW_INIT :
			Daily_ColorRow = 0;
			Daily_ColorCol = 0;
			colorCount = 0;
			positionX = MV_MENU_BACK_X + 10;
			positionY = EPG_LIST_Y + 46;
			// MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR);
			// MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(EPG_CHANNEL_NAME_WIDTH), ScalerHeigthPixel(boxH));
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(EPG_CHANNEL_NAME_WIDTH - 2), ScalerHeigthPixel(boxH));
			charRect.top    = positionY;
			charRect.bottom = charRect.top + boxH;
			charRect.left   = positionX;
			charRect.right  = charRect.left + EPG_CHANNEL_NAME_WIDTH - 2;
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			sprintf(Temp_Str, "%02d/%02d/%04d - %s   ", Daily_CurrentDate.day, Daily_CurrentDate.month, Daily_CurrentDate.year, CS_MW_LoadStringByIdx(EpgWeekIdx[CS_DT_CalculateWeekDay(Daily_CurrentMjd)]));
			MV_MW_DrawText_Static(hdc, Temp_Str, -1, &charRect, DT_RIGHT);

			positionX += EPG_CHANNEL_NAME_WIDTH;
			
			for ( i = 0 ; i < 7 ; i++ )
			{
				Temp_CurrentDate = CS_DT_MJDtoYMD(Daily_CurrentMjd + i);
				
				if ( i == u8Select_Day )
				{
					SetTextColor(hdc,CSAPP_BLACK_COLOR);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY - 4), ScalerWidthPixel(EPG_DATA_DX/7 - 2), ScalerHeigthPixel(boxH + 4));
				}
				else 
				{
					SetTextColor(hdc,MVAPP_GRAY_COLOR);
					MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY - 4), ScalerWidthPixel(EPG_DATA_DX/7 - 2), ScalerHeigthPixel(boxH + 4));
				}
				charRect.top    = positionY;
				charRect.bottom = charRect.top + boxH;
				charRect.left   = positionX;
				charRect.right  = charRect.left + EPG_DATA_DX/7 - 2;
				
				SetBkMode(hdc,BM_TRANSPARENT);
				sprintf(Temp_Str, "%02d/%02d", Temp_CurrentDate.day, Temp_CurrentDate.month);
				MV_MW_DrawText_Static(hdc,  Temp_Str, -1, &charRect, DT_CENTER);
				
				positionX += EPG_DATA_DX/7;
			
			}
		case EPG_DRAW_PAGE :
			for (row = 0; row < serviceNumberPerPage; row++)
			{
				index = Daily_EpgCurrentPageStartIndex + row;

				if(index < Daily_EpgTotalService)
				{
					if(Daily_EpgCurrentFocus == row)
						MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, FOCUS, EN_EPG_TYPE_SIMPLE);
					else
						MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, UNFOCUS, EN_EPG_TYPE_SIMPLE);
				}
				else
				{
					MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, NOTFOCUS, EN_EPG_TYPE_SIMPLE);
				}
			}

			positionX = MV_MENU_BACK_X + 10 + EPG_CHANNEL_NAME_WIDTH;
			positionY = EPG_LIST_Y + 46;

			for ( i = 0 ; i < 7 ; i++ )
			{
				Temp_CurrentDate = CS_DT_MJDtoYMD(Daily_CurrentMjd + i);
				
				if ( i == u8Select_Day )
				{
					SetTextColor(hdc,CSAPP_BLACK_COLOR);
					MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY - 4), ScalerWidthPixel(EPG_DATA_DX/7 - 2), ScalerHeigthPixel(boxH + 4));
				}
				else 
				{
					SetTextColor(hdc,MVAPP_GRAY_COLOR);
					MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY - 4), ScalerWidthPixel(EPG_DATA_DX/7 - 2), ScalerHeigthPixel(boxH + 4));
				}
				charRect.top    = positionY;
				charRect.bottom = charRect.top + boxH;
				charRect.left   = positionX;
				charRect.right  = charRect.left + EPG_DATA_DX/7 - 2;
				
				SetBkMode(hdc,BM_TRANSPARENT);
				sprintf(Temp_Str, "%02d/%02d", Temp_CurrentDate.day, Temp_CurrentDate.month);
				MV_MW_DrawText_Static(hdc,  Temp_Str, -1, &charRect, DT_CENTER);
				
				positionX += EPG_DATA_DX/7;
			
			}

			if (Daily_EpgTotalService == 0)
			{
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				positionX = MV_MENU_BACK_X + 10;
				positionY = EPG_LIST_Y + EPG_MENU_BAR_H * (index +1) + 70;
				scrollRect.top = EPG_LIST_Y + 70;
				scrollRect.left = EPG_LIST_SCROLL_X - 10;
				scrollRect.right = scrollRect.left + SCROLL_BAR_DX;
				scrollRect.bottom = scrollRect.top + EPG_MENU_BAR_H * 10 - 2;
				CS_MW_TextOut(hdc, ScalerWidthPixel(positionX + EPG_CHANNEL_NAME_WIDTH + 20), ScalerHeigthPixel(scrollRect.top + EPG_MENU_BAR_H),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));
				MV_Draw_ScrollBar(hdc, scrollRect, Daily_EpgCurrentService, Daily_EpgTotalService, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
			}
			break;
		case EPG_DRAW_SCROLL_UP   :
			scrollDirection = 1;
		case EPG_DRAW_SCROLL_DOWN :
			positionX = MV_MENU_BACK_X + 10;
			positionY = EPG_LIST_Y + EPG_MENU_BAR_H + 46;
			boxW      = EPG_CHANNEL_NAME_WIDTH + (EPG_SIMPLE_DATA_WIDTH + EPG_SIMPLE_TIME_WIDTH) - 2;
			boxH      = EPG_MENU_BAR_H * (serviceNumberPerPage - 1);
			memset(&scrollBmp, 0x00, sizeof(BITMAP));
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY + (EPG_MENU_BAR_H * scrollDirection)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			scrollDirection = 1 - scrollDirection;
			FillBoxWithBitmap(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY + (EPG_MENU_BAR_H * scrollDirection)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			UnloadBitmap(&scrollBmp);
			Daily_ColorRow = 1- Daily_ColorRow;
			break;
		case EPG_DRAW_UPDATE     :
			for (row = 0; row < serviceNumberPerPage; row++)
			{
				index = Daily_EpgCurrentPageStartIndex + row;

				if(index < Daily_EpgTotalService)
				{
					if(Daily_EpgCurrentFocus == row)
						MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, FOCUS, EN_EPG_TYPE_SIMPLE);
					else
						MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, UNFOCUS, EN_EPG_TYPE_SIMPLE);
				}
				else
				{
					MV_Draw_Daily_Epg_Item_Channel(hdc, row, index, NOTFOCUS, EN_EPG_TYPE_SIMPLE);
					//break;
				}
			}
			break;
		default :
			break;
	}
}

U32 MV_Set_Daily_EPG_Current_List(U32 epgType, U8 satIndex)
{
	U32 returnType;

	returnType = epgType;
	
	switch ( epgType )
	{
		case CSApp_Applet_Daily_TV_EPG:
			Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_Daily_Radio_EPG:
			Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_Daily_FAV_TV_EPG:
			Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			break;
		case CSApp_Applet_Daily_FAV_Radio_EPG:
			Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			break;
		case CSApp_Applet_Daily_SAT_TV_EPG:
			if ( satIndex == 255 )
			{
				returnType = CSApp_Applet_Daily_TV_EPG;
				Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
				Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			}
			break;
		case CSApp_Applet_Daily_SAT_Radio_EPG:
			if ( satIndex == 255 )
			{
				returnType = CSApp_Applet_Daily_Radio_EPG;
				Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				Daily_EpgTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
				Daily_EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			}
			break;
		default:
			break;
	}
	
	Daily_EpgTotalService = CS_DB_GetListServiceNumber(Daily_EpgTriplet);
	// printf("=== CH EDIT TOTAL : %d , %d , %d ====\n", Daily_EpgTriplet.sCS_DB_ServiceListType, Daily_EpgTriplet.sCS_DB_ServiceListTypeValue, Daily_EpgTotalService);

	if( Daily_EpgTotalService > 0 )
	{
		CS_DB_SetCurrentList(Daily_EpgTriplet, FALSE);
	}

	return returnType;
}

void MV_Draw_Daily_Epg_Title(HDC hdc, U32 serviceType)
{
	char 			title[50];
	char			tempStr[20];
	RECT 			tmpRect;
	MV_stSatInfo	tempSatData;
	
	memset(title, 0, 50);
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(EPG_MENU_BAR_H) );
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	switch(serviceType)
	{
		case CSApp_Applet_Daily_TV_EPG:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
		case CSApp_Applet_Daily_Radio_EPG:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_RD_LIST));
			break;
		case CSApp_Applet_Daily_FAV_TV_EPG:
			MV_DB_Get_Favorite_Name(tempStr, Daily_EpgTriplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s TV" ,tempStr);
			break;
		case CSApp_Applet_Daily_FAV_Radio_EPG:
			MV_DB_Get_Favorite_Name(tempStr, Daily_EpgTriplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s RADIO" ,tempStr);
			break;
		case CSApp_Applet_Daily_SAT_TV_EPG:
			MV_GetSatelliteData_ByIndex(&tempSatData, Daily_EpgSatIndex);
			sprintf(title, "%s TV" ,tempSatData.acSatelliteName);
			break;
		case CSApp_Applet_Daily_SAT_Radio_EPG:
			MV_GetSatelliteData_ByIndex(&tempSatData, Daily_EpgSatIndex);
			sprintf(title, "%s RADIO" ,tempSatData.acSatelliteName);
			break;
		default:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
	}

	tmpRect.left	= ScalerWidthPixel(MV_MENU_BACK_X);
	tmpRect.right	= tmpRect.left + ScalerWidthPixel(MV_MENU_BACK_DX);
	tmpRect.top		= ScalerWidthPixel(MV_MENU_BACK_Y);
	tmpRect.top		= Daily_AdjustYPosition(tmpRect.top, EPG_MENU_BAR_H, MV_FONT_HEIGHT);
	tmpRect.bottom	= tmpRect.top + ScalerHeigthPixel(MV_FONT_HEIGHT);

	MV_MW_DrawText_Static(hdc, title, -1, &tmpRect, DT_CENTER);
}

static int Daily_Epg_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

static void    Daily_epg_date_time_callback(tCS_DT_UpdateEvent event, U16 currentMjd, U16 currentUtc)
{
	if (event == eCS_DT_MINUTE)
	{
		#if 0
		printf ("Daily_epg_date_time_callback : event[0x%x], MJDTime[%d] = %02d/%02d/%02d, Time[0x%04X] = %02d:%02d\n",
				event, currentMjd, Daily_CurrentDate.year, Daily_CurrentDate.month, Daily_CurrentDate.day,
				currentUtc, Daily_CurrentTime.hour, Daily_CurrentTime.minute);
		#endif
		Daily_CurrentMjd  = currentMjd;
		Daily_CurrentDate = CS_DT_MJDtoYMD(currentMjd);
		Daily_CurrentTime = CS_DT_UTCtoHM(currentUtc);
		BroadcastMessage (MSG_TIME_UPDATE, 0, 0);
	}
}

void Daily_EpgChangeChannel(HWND hwnd)
{
	tCS_DB_ServiceManageData	    channelItemData;
	MV_stServiceInfo			    channelServiceData;
	tCS_DBU_Service                 serviceTriplet;
	U8                              playChannel;
	
	CS_DB_GetCurrentList_ServiceData(&channelItemData, Daily_EpgCurrentService);
	MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);

	if ((CS_MW_GetServicesLockStatus()) && (channelItemData.Lock_Flag == eCS_DB_LOCKED)/*(channelServiceData.u8Lock== eCS_DB_LOCKED)*/
		  && (Daily_EpgCurrentService != CS_APP_GetLastUnlockServiceIndex())/* && !playOn*/)
	{
		CS_MW_StopService(TRUE);
		MV_Draw_Password_Window(hwnd);
		playChannel = 0;
	}
	else
	{
		playChannel = 1;
	}

	if (playChannel)
	{
		if ( b8Check_Exit == TRUE )
		{
			CS_MW_PlayServiceByIdx(channelItemData.Service_Index, RE_TUNNING);
			b8Check_Exit = FALSE;
		}
		else
			CS_MW_PlayServiceByIdx(channelItemData.Service_Index, NOT_TUNNING);
		
		CS_DB_SetLastServiceTriplet();
		CS_DB_SetCurrentService_OrderIndex(Daily_EpgCurrentService);
		CS_DB_GetCurrentListTriplet(&(serviceTriplet.sCS_DBU_ServiceList));
		serviceTriplet.sCS_DBU_ServiceIndex =  Daily_EpgCurrentService;
		CS_DBU_SaveCurrentService(serviceTriplet);
		Daily_EpgBackTriplet = serviceTriplet;
		FbSendFndDisplayNum((unsigned)Daily_EpgCurrentService+1);
	}
}

void MvDraw_Daily_EpgCurrentTime(HDC hdc)
{
	char    buf[20];
	RECT	etcRect;
	U32     boxW;
	U32     boxH;

	boxW = MV_FONT_WIDTH * 15;
	boxH = MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight + 8;

	etcRect.top    = MV_MENU_BACK_Y - 6 ;
	etcRect.bottom = etcRect.top + boxH;
	etcRect.left   = MV_MENU_BACK_X + MV_MENU_BACK_DX - boxW;
	etcRect.right  = MV_MENU_BACK_X + MV_MENU_BACK_DX;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(etcRect.left), ScalerHeigthPixel(etcRect.top), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &TitleBmp);
	memset(buf, 0x00, 20);
	sprintf(buf, "%02d/%02d  %02d:%02d", Daily_CurrentDate.month, Daily_CurrentDate.day, Daily_CurrentTime.hour, Daily_CurrentTime.minute);
	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	etcRect.top = etcRect.top + 16 - ( CS_DBU_GetFont_Size() - 10 )/2;
	etcRect.left = etcRect.left + 2;
	MV_MW_DrawText_Static(hdc, buf, -1, &etcRect, DT_CENTER);
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	etcRect.top = etcRect.top - 2;
	etcRect.left = etcRect.left - 2;
	MV_MW_DrawText_Static(hdc, buf, -1, &etcRect, DT_CENTER);
}

void Daily_Epg_Detail_Contents(HDC hdc, U8 mode, U8 type)
{
	U32 			boxH = 20;
	U32 			boxW = 20;
	U32 			boxY = 10;
	U32 			boxX = 10;
	U32 			pointer;
	U32 			maxLine;
	U32 			direction = 0;
	static U32 		startLine;
	static U32 		numberOfLine;
	static U32 		lineCounter;
	RECT       		charRect;
	RECT       		scrollRect;
	BITMAP	   		scrollBmp;
	
	if (mode == EN_EPG_TYPE_SIMPLE)
	{	
		boxX = EPG_DETAIL_WINDOW_X;
		boxY = EPG_DETAIL_CONTENT_Y; /* 54 + 20 + 14 */
		boxH = EPG_DETAIL_CONTENT_DY;
		boxW = EPG_DETAIL_CONTENT_DX - SCROLL_BAR_DX;
		maxLine = boxH / MV_FONT_HEIGHT;
	} else {
		boxX = EPG_DETAIL_WINDOW_X;
		boxY = EPG_DETAIL_CONTENT_Y; /* 54 + 20 + 14 */
		boxH = EPG_DETAIL_CONTENT_DY;
		boxW = EPG_DETAIL_CONTENT_DX - SCROLL_BAR_DX;
		maxLine = boxH / MV_FONT_HEIGHT;
	}

	charRect.left   = boxX;
	charRect.right  = charRect.left + boxW;
	charRect.top    = boxY;
	charRect.bottom = charRect.top + MV_FONT_HEIGHT;

	switch(type)
	{
		case EPG_DRAW_INIT :
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX - 10), ScalerHeigthPixel(boxY - 10), ScalerWidthPixel(boxW + SCROLL_BAR_DX + 20), ScalerHeigthPixel(boxH + 20));
			break;
		case EPG_DRAW_PAGE :
			startLine = 0;
			numberOfLine = maxLine - Daily_EpgNameLine;
			if (numberOfLine > Daily_EpgDetailInfoLine)
			{
				numberOfLine = Daily_EpgDetailInfoLine;
			}
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < Daily_EpgNameLine)
			{
				MV_MW_DrawText_Static(hdc, (Daily_EpgEventTitle + pointer), -1, &charRect, DT_LEFT);
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += Daily_EpgCharPerLine;
				lineCounter++;
			}
			
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < numberOfLine)
			{
				MV_MW_DrawText_Static(hdc, (Daily_EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += Daily_EpgCharPerLine;
				lineCounter++;
			}
			break;
		case EPG_DRAW_SCROLL_UP :
			if ((startLine + numberOfLine) >= Daily_EpgDetailInfoLine)
			{
				break;
			}
			direction = 1;
		case EPG_DRAW_SCROLL_DOWN :
			if (Daily_EpgDetailInfoLine <= numberOfLine)
			{
				break;
			}

			if ((startLine == 0) && (direction == 0))
			{
				break;
			}
			boxY = boxY + (MV_FONT_HEIGHT * Daily_EpgNameLine);
			boxH = MV_FONT_HEIGHT * (numberOfLine - 1);
			memset(&scrollBmp, 0x00, sizeof(BITMAP));
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			direction = 1 - direction;
			FillBoxWithBitmap(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			UnloadBitmap(&scrollBmp);
			if (direction)
			{
				/* EPG_DRAW_SCROLL_DOWN */
				// boxY = boxY + (MV_FONT_HEIGHT * Daily_EpgNameLine);
				startLine--;
				pointer = Daily_EpgCharPerLine * startLine;
			}
			else
			{
				/* EPG_DRAW_SCROLL_UP */
				boxY = boxY + (MV_FONT_HEIGHT * (numberOfLine - 1));
				pointer = Daily_EpgCharPerLine * (startLine + numberOfLine);
				startLine++;
			}

			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(MV_FONT_HEIGHT));
			charRect.top    = boxY;
			charRect.bottom = charRect.top + MV_FONT_HEIGHT;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc, (Daily_EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
			break;
		default :
			return;
	}

	boxY = EPG_DETAIL_CONTENT_Y; 
	boxH = EPG_DETAIL_CONTENT_DY;
	scrollRect.left   = boxX + boxW;
	scrollRect.right  = scrollRect.left + SCROLL_BAR_DX;
	scrollRect.top    = boxY;
	scrollRect.bottom = scrollRect.top + boxH;
	MV_Draw_PageScrollBar(hdc, scrollRect, startLine, Daily_EpgDetailInfoLine, (maxLine - Daily_EpgNameLine));

}

void Daily_Epg_Detail_Window(HDC hdc)
{
	RECT	rc1;

	b8Detail_Window_Status = TRUE;
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(TIMER_WINDOW_DX), ScalerHeigthPixel(TIMER_WINDOW_DY), &Timer_Cap_Bmp);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y),ScalerWidthPixel(TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(TIMER_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(TIMER_WINDOW_DX),ScalerHeigthPixel(TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
/*
	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_X + 30), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_X + 220), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	rc1.top = TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30;
	rc1.left = TIMER_WINDOW_X + 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth;
	rc1.bottom = rc1.top + TIMER_WINDOW_ITEM_DY;
	rc1.right = rc1.left + 200;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY), -1, &rc1, DT_LEFT);
	
	rc1.left = TIMER_WINDOW_X + 250 + MV_BMP[MVBMP_RED_BUTTON].bmWidth;
	rc1.right = rc1.left + 200;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_TYPE), -1, &rc1, DT_LEFT);
*/	
	rc1.top = TIMER_WINDOW_TITLE_Y;
	rc1.left = TIMER_WINDOW_ITEM_X;
	rc1.bottom = TIMER_WINDOW_TITLE_Y + TIMER_WINDOW_ITEM_DY;
	rc1.right = rc1.left + TIMER_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_DETAIL_INFO);
	Daily_Epg_Detail_Contents(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_PAGE);
}

BOOL Daily_Epg_Detail_Window_proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;
	
	switch (u8Key)
    {
		case CSAPP_KEY_UP:
			hdc = BeginPaint(hwnd);
			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_DOWN:
			hdc = BeginPaint(hwnd);
			EndPaint(hwnd,hdc);
			break;
			
        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;
    }

	return FALSE;
}

void Daily_Epg_Timer_Draw_SelectBar(HDC hdc, int y_gap)
{
	int mid_width = TIMER_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = TIMER_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
}

void Daily_Epg_Timer_Draw_Job_Item(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 						y_gap = TIMER_WINDOW_CONT_Y + TIMER_WINDOW_ITEM_DY * esItem;
	RECT						TmpRect;
	char						temp_str[256];
	tCS_DT_Date					Temp_YMD;
	tCS_DT_Time					Temp_HM;
	tCS_DB_ServiceManageData 	item_data;
	
	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		Daily_Epg_Timer_Draw_SelectBar(hdc, y_gap);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(TIMER_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(TIMER_WINDOW_CONT_DX),ScalerHeigthPixel(TIMER_WINDOW_ITEM_DY) );
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X + TIMER_WINDOW_CONT_DX - ( MV_BMP[MVBMP_Y_ENTER].bmWidth + 10 )), ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight), &MV_BMP[MVBMP_Y_ENTER]);
	
	memset( temp_str, 0x00, 256);

	CS_TIMER_GetJobInfo(esItem, &stEpgTimerJob);

	if ( stEpgTimerJob.CS_Timer_Status == eCS_TIMER_Enable )
	{
		TmpRect.left	= TIMER_WINDOW_ITEM_NO_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NO_DX;
		TmpRect.top		= y_gap + 4;
		TmpRect.bottom	= TmpRect.top + TIMER_WINDOW_ITEM_DY;
		sprintf(temp_str, "%d", esItem + 1);
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_NAME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NAME_DX;
		
		CS_DB_GetCurrentList_ServiceData(&item_data, stEpgTimerJob.CS_Wakeup_Service.Service_Index);
		MV_DB_GetServiceDataByIndex(&ServiceData, item_data.Service_Index);		
		sprintf(temp_str, "%s", ServiceData.acServiceName);
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_LEFT);

		TmpRect.left	= TIMER_WINDOW_ITEM_DATE_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_DATE_DX;
		Temp_YMD = CS_DT_MJDtoYMD(stEpgTimerJob.CS_Begin_MDJ);
		sprintf(temp_str, "%02d/%02d/%04d", Temp_YMD.day, Temp_YMD.month, Temp_YMD.year);
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_TIME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_TIME_DX;
		Temp_HM = CS_DT_UTCtoHM(stEpgTimerJob.CS_Begin_UTC);
		sprintf(temp_str, "%02d:%02d", Temp_HM.hour, Temp_HM.minute);
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_TYPE_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_TYPE_DX;
		if (stEpgTimerJob.CS_Timer_Type == eCS_TIMER_Wakeup)
			sprintf(temp_str, "Turn ON");
		else if (stEpgTimerJob.CS_Timer_Type == eCS_TIMER_Sleep)
			sprintf(temp_str, "Turn OFF");
		else if (stEpgTimerJob.CS_Timer_Type == eCS_TIMER_Record)
			sprintf(temp_str, "Record");
		else 
			sprintf(temp_str, CS_MW_LoadStringByIdx(CSAPP_STR_DURATION));
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_CENTER);
	} else {	
		TmpRect.left	= TIMER_WINDOW_ITEM_NO_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NO_DX;
		TmpRect.top		= y_gap + 4;
		TmpRect.bottom	= TmpRect.top + TIMER_WINDOW_ITEM_DY;
		sprintf(temp_str, "%d", esItem + 1);
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_CENTER);
		
		TmpRect.left	= TIMER_WINDOW_ITEM_NAME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NAME_DX;	
		sprintf(temp_str, "Off");
		MV_MW_DrawText_Static(hdc, temp_str, -1, &TmpRect, DT_LEFT);
	}		 
}

void Daily_Epg_Timer_Draw_Job_List(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < kCS_TIMER_MAX_NO_OF_JOB - 1 ; i++ )
	{
		if( Current_Item == i )
		{
			Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, i);
		} else {
			Daily_Epg_Timer_Draw_Job_Item(hdc, UNFOCUS, i);
		}
	}
}

void Daily_Epg_Timer_Close(HDC hdc)
{
	b8Timer_Window_Status = FALSE;
	b8Detail_Window_Status = FALSE;
	FillBoxWithBitmap(hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(TIMER_WINDOW_DX), ScalerHeigthPixel(TIMER_WINDOW_DY), &Timer_Cap_Bmp);
}

void Daily_Epg_Timer_Draw(HDC hdc)
{
	RECT	rc1;

	b8Timer_Window_Status = TRUE;
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(TIMER_WINDOW_DX), ScalerHeigthPixel(TIMER_WINDOW_DY), &Timer_Cap_Bmp);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y),ScalerWidthPixel(TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(TIMER_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(TIMER_WINDOW_DX),ScalerHeigthPixel(TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_X + 30), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_X + 220), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	rc1.top = TIMER_WINDOW_Y + TIMER_WINDOW_DY - 30;
	rc1.left = TIMER_WINDOW_X + 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth;
	rc1.bottom = rc1.top + TIMER_WINDOW_ITEM_DY;
	rc1.right = rc1.left + 200;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY), -1, &rc1, DT_LEFT);
	
	rc1.left = TIMER_WINDOW_X + 250 + MV_BMP[MVBMP_RED_BUTTON].bmWidth;
	rc1.right = rc1.left + 200;
	MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_TYPE), -1, &rc1, DT_LEFT);
		
	rc1.top = TIMER_WINDOW_TITLE_Y;
	rc1.left = TIMER_WINDOW_ITEM_X;
	rc1.bottom = TIMER_WINDOW_TITLE_Y + TIMER_WINDOW_ITEM_DY;
	rc1.right = rc1.left + TIMER_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_TIMER);
	Daily_Epg_Timer_Draw_Job_List(hdc);
}

tCS_TIMER_Error Daily_Epg_Timer_Upload_Data(U8	u8Mo_Number, BOOL Update_kind)
{
	tCS_DT_Time					st_StartHM;
	tCS_DB_ServiceManageData	channelItemData;
	MV_stServiceInfo			channelServiceData;
	tCS_EIT_Event_t            *current;
	U16							DurationUTC;
	tCS_DB_ServiceListTriplet	ListTriplet;
	U16							CurrentService;

	if ( Update_kind )
	{
		CS_DB_GetCurrentList_ServiceData(&channelItemData, Daily_EpgCurrentService);
		MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);

#if 0
		if ( Daily_7dayEpgCurrentService == 0 )
			current = &present;
		else
			current = &follow;
#endif 
		current = &MvScEventList.EventArray[Daily_7dayEpgCurrentService];

		if ( current->nameLength > 0 )
		{
			st_StartHM = CS_DT_UTCtoHM(current->start_time_utc);
			DurationUTC = current->end_time_utc - current->start_time_utc;
						
			CS_DB_GetCurrentListTriplet(&ListTriplet);
			CurrentService = CS_DB_GetCurrentService_OrderIndex();
			CS_DB_GetCurrentList_ServiceData(&channelItemData, CurrentService);
			MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);
			
			if ( ListTriplet.sCS_DB_ServiceListType == eCS_DB_TV_LIST || ListTriplet.sCS_DB_ServiceListType == eCS_DB_FAV_TV_LIST || ListTriplet.sCS_DB_ServiceListType == eCS_DB_SAT_TV_LIST )
				stEpgTimerJob.CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_TV;
			else
				stEpgTimerJob.CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_RADIO;

			stEpgTimerJob.CS_Wakeup_Service.SList_Value = 0;
			CurrentService = MV_DB_Get_ServiceAllList_Index(stEpgTimerJob.CS_Wakeup_Service.SList_Type, channelServiceData.u16ChIndex);
			stEpgTimerJob.CS_Wakeup_Service.Service_Index = CurrentService;

			stEpgTimerJob.CS_Timer_Status = eCS_TIMER_Enable;
			stEpgTimerJob.CS_Timer_Type = eCS_TIMER_Record;
			stEpgTimerJob.CS_Timer_Cycle = eCS_TIMER_Onetime;
			stEpgTimerJob.CS_Begin_MDJ = current->start_date_mjd;
			stEpgTimerJob.CS_Begin_UTC = current->start_time_utc;
			stEpgTimerJob.CS_Begin_Weekday = CS_DT_CalculateWeekDay(current->start_date_mjd);
			stEpgTimerJob.CS_Duration_UTC = DurationUTC;
		} else {
			stEpgTimerJob.CS_Timer_Status = eCS_TIMER_Disable;
		}
	} else {
		stEpgTimerJob.CS_Timer_Status = eCS_TIMER_Disable;
	}
	
	return CS_TIMER_CheckandSaveJobInfo(&stEpgTimerJob, u8Mo_Number);
}

tCS_TIMER_Error Daily_Epg_Timer_Change_Data(U8	u8Mo_Number)
{
	CS_TIMER_GetJobInfo(u8Mo_Number, &stEpgTimerJob);

	if ( stEpgTimerJob.CS_Timer_Type == eCS_TIMER_Record )
		stEpgTimerJob.CS_Timer_Type = eCS_TIMER_Wakeup;
	else
		stEpgTimerJob.CS_Timer_Type = eCS_TIMER_Record;
	
	return CS_TIMER_CheckandSaveJobInfo(&stEpgTimerJob, u8Mo_Number);
}

BOOL Daily_Epg_Timer_proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	if ( MV_Check_Confirm_Window() == TRUE )
	{
		MV_Confirm_Proc(hwnd, u8Key);
		
		if ( u8Key == CSAPP_KEY_ESC || u8Key == CSAPP_KEY_MENU || u8Key == CSAPP_KEY_ENTER )
		{
			if ( u8Key == CSAPP_KEY_ENTER )
			{
				if ( MV_Check_YesNo() == TRUE )
				{
					Daily_Epg_Timer_Upload_Data(Current_Item, FALSE);
					hdc = BeginPaint(hwnd);
					Restore_Confirm_Window(hdc);
					Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
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
		
		return TRUE;
	}
	
	switch (u8Key)
    {
		case CSAPP_KEY_UP:
			hdc = BeginPaint(hwnd);
			Daily_Epg_Timer_Draw_Job_Item(hdc, UNFOCUS, Current_Item);
			
			if ( Current_Item == 0 )
				Current_Item = kCS_TIMER_MAX_NO_OF_JOB - 2;
			else
				Current_Item--;

			Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_DOWN:
			hdc = BeginPaint(hwnd);
			Daily_Epg_Timer_Draw_Job_Item(hdc, UNFOCUS, Current_Item);
			
			if ( Current_Item == kCS_TIMER_MAX_NO_OF_JOB - 2 )
				Current_Item = 0;
			else
				Current_Item++;

			Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
			EndPaint(hwnd,hdc);
			break;
			
        case CSAPP_KEY_ENTER:
			Daily_Epg_Timer_Upload_Data(Current_Item, TRUE);
			hdc = BeginPaint(hwnd);
			Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RED:
			MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
			break;

		case CSAPP_KEY_GREEN:
			Daily_Epg_Timer_Change_Data(Current_Item);
			hdc = BeginPaint(hwnd);
			Daily_Epg_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
			EndPaint(hwnd,hdc);
			break;
			
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;
    }

	return FALSE;
}

CSAPP_Applet_t CSApp_Daily_Epg(CSAPP_Applet_t   slist_type)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_7DayEpg_Applets = CSApp_Applet_Error;
		
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

	
	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "Daily epg window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Daily_Epg_Msg_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_black;
	CreateInfo.dwAddData 		= slist_type;
	CreateInfo.hHosting 		= HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_7DayEpg_Applets;
	
}

static int Daily_Epg_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 							hdc;
	RECT			                etcRect;
	// tCS_DB_ServiceManageData	    channelItemData;
	// MV_stServiceInfo			    channelServiceData;
	// tCS_DBU_Service                 serviceTriplet;
	tCS_DT_Time                     currentTime;
	tCS_DT_Time                     startTime;
	static U32                      servicelistType;
    static U32                      prevServiceType = 0;
    static U32                      prevServiceIndex = 0;
	static U32                      epgChBoxY;
	static U32                      epgChBoxH;
	static U32                      epgLBoxX;
	static U32                      epgBBoxY;
	static U32                      epgBBoxH;
	static U32                      channelChangeCounter;
	static U32                      channelChangeTime;
	static U8                       channelChanged;
	static U8                       channelChangeRequest;
	static U8                       popupModeOn = FALSE;
	static U16                      prevMjd;
	U8                              channelChangeOn = 0;
	U32                             currentMilliTime;
	U32                             tmpX;
	U32                             tmpY;
	U16                             currentUtc;
	tCS_DBU_Service 				ServiceTriplet;
	
	tCS_DB_ServiceManageData	channelItemData;
	MV_stServiceInfo			channelServiceData;

	if (( MV_Get_PopUp_Window_Status() == TRUE ) && (message != MSG_TIME_UPDATE))
	{
		if (message == MSG_KEYDOWN)
		{
			popupModeOn = MV_PopUp_Proc(hwnd, wparam);

			if (wparam == CSAPP_KEY_ENTER)
			{
				U8	u8Result_Value;

				u8Result_Value = MV_Get_PopUp_Window_Result();

				switch(MV_Get_PopUp_Window_Kind())
				{
					case eMV_TITLE_FAV:
					{
						U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;

						prevServiceIndex = Daily_EpgSatIndex;
						switch(servicelistType)
						{
							case CSApp_Applet_Daily_TV_EPG:
							case CSApp_Applet_Daily_FAV_TV_EPG:
							case CSApp_Applet_Daily_SAT_TV_EPG:
								servicelistType = CSApp_Applet_Daily_FAV_TV_EPG;
								u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
								break;
							case CSApp_Applet_Daily_Radio_EPG:
							case CSApp_Applet_Daily_FAV_Radio_EPG:
							case CSApp_Applet_Daily_SAT_Radio_EPG:
								servicelistType = CSApp_Applet_Daily_FAV_Radio_EPG;
								u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
								break;
							default:
								break;
						}

						Daily_EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value);
						servicelistType = MV_Set_Daily_EPG_Current_List(servicelistType, Daily_EpgSatIndex);
						if ((servicelistType == prevServiceType) && (prevServiceIndex == Daily_EpgSatIndex))
						{
							/* Same List */
							break;
						}
						prevServiceType = servicelistType;
						Daily_EpgBackTriplet = CS_DB_GetLastServiceTriplet();
						Daily_EpgTotalService = CS_DB_GetListServiceNumber(Daily_EpgTriplet);
						// printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, Daily_EpgSatIndex);
						if(Daily_EpgTotalService>0)
						{
							Daily_EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						hdc = BeginPaint(hwnd);
						MV_Draw_Daily_Epg_Title(hdc, servicelistType);
						MvDraw_Daily_EpgCurrentTime(hdc);
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						break;
					}
							
					case eMV_TITLE_SAT_FAV:
					{
						U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
						U8      satCount = 0;
															
						prevServiceIndex = Daily_EpgSatIndex;
						switch(servicelistType)
						{
							case CSApp_Applet_Daily_TV_EPG:
							case CSApp_Applet_Daily_FAV_TV_EPG:
							case CSApp_Applet_Daily_SAT_TV_EPG:
								u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
								satCount = MV_Get_Searched_SatCount();
								// printf("MV_Get_Searched_SatCount [%d]\n", satCount);
								if ( u8Result_Value == 0 )
								{
									servicelistType = CSApp_Applet_Daily_TV_EPG;
									Daily_EpgSatIndex = 255;
								}
								else if (u8Result_Value <= satCount)
								{
									servicelistType = CSApp_Applet_Daily_SAT_TV_EPG;
									Daily_EpgSatIndex = MV_Get_Satindex_by_Seq(u8Result_Value);
								}
								else
								{
									servicelistType = CSApp_Applet_Daily_FAV_TV_EPG;
									Daily_EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - satCount - 1);
								}
								break;
							case CSApp_Applet_Daily_Radio_EPG:
							case CSApp_Applet_Daily_FAV_Radio_EPG:
							case CSApp_Applet_Daily_SAT_Radio_EPG:
								u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
								satCount = MV_Get_Searched_SatCount();
								if ( u8Result_Value == 0 )
								{
									servicelistType = CSApp_Applet_Daily_Radio_EPG;
									Daily_EpgSatIndex = 255;
								}
								else if (u8Result_Value <= satCount)
								{
									servicelistType = CSApp_Applet_Daily_SAT_Radio_EPG;
									Daily_EpgSatIndex = MV_Get_Satindex_by_Seq(u8Result_Value);
								}
								else
								{
									servicelistType = CSApp_Applet_Daily_FAV_Radio_EPG;
									Daily_EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - satCount - 1);
								}
								break;
								break;
							default:
								break;
						}

						servicelistType = MV_Set_Daily_EPG_Current_List(servicelistType, Daily_EpgSatIndex);
						if ((servicelistType == CSApp_Applet_Daily_TV_EPG) || (servicelistType == CSApp_Applet_Daily_Radio_EPG))
						{
							if (servicelistType == prevServiceType)
							{
								break;
							}
						}
						else if ((servicelistType == prevServiceType) && (prevServiceIndex == Daily_EpgSatIndex))
						{
							/* Same List */
							break;
						}
						prevServiceType = servicelistType;
						Daily_EpgBackTriplet = CS_DB_GetLastServiceTriplet();
						Daily_EpgTotalService = CS_DB_GetListServiceNumber(Daily_EpgTriplet);
						// printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, Daily_EpgSatIndex);
						if(Daily_EpgTotalService>0)
						{
							Daily_EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						hdc = BeginPaint(hwnd);
						MV_Draw_Daily_Epg_Title(hdc, servicelistType);
						MvDraw_Daily_EpgCurrentTime(hdc);
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						break;
					}
	
					default :
						break;
				}
			}
			else if (wparam == CSAPP_KEY_IDLE)
			{
				CSApp_7DayEpg_Applets = CSApp_Applet_Sleep;
				SendMessage(hwnd,MSG_CLOSE,0,0);
			}
			else if(wparam == CSAPP_KEY_TV_AV)
			{
				ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
			}
		}

		return DefaultMainWinProc(hwnd, message, wparam, lparam);	
	}
	/*
	else if(PinDlg_GetStatus() == TRUE)
	{
		PinDlg_TreatKey(hwnd, wparam, &input_count, input_keys);
	}
	*/
	
	switch (message)
	{
		case MSG_CREATE:
			memset(&TitleBmp, 0x00, sizeof(BITMAP));
			epgChBoxY = MV_MENU_BACK_Y + 54;
			epgChBoxH = EPG_MENU_BAR_H * 11;
			epgLBoxX = EPG_LIST_X + EPG_DETAIL_DX + 40;
			epgBBoxY = epgChBoxY + epgChBoxH + 48;
			epgBBoxH = CSAPP_OSD_MAX_HEIGHT - epgBBoxY - 106;

			// EpgFocusCn    = 0;
			b8Check_Exit = FALSE;
			u8Select_Day = 0;
			Daily_EpgCurrentCol = 0;
			Daily_7dayEpgCurrentService = 0;
			b8Screen_Kind = CH_LIST;
			b8Timer_Window_Status = FALSE;
			
			/* By KB Kim 2011.05.28 */
			MvScEventList.NumberOfEvent = 0;
			MvScEventList.StartPoint    = 0;
			MvScEventList.EventArray    = NULL;
			
			CS_DT_Register_DateTime_Notify(&Daily_date_time_client_id, eCS_DT_MINUTE, Daily_epg_date_time_callback);
			servicelistType = GetWindowAdditionalData(hwnd);
			//Daily_EpgSatIndex = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
			CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));					
			Daily_EpgSatIndex = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
			
			servicelistType = MV_Set_Daily_EPG_Current_List(servicelistType, Daily_EpgSatIndex);
			Daily_EpgBackTriplet = CS_DB_GetLastServiceTriplet();
			
			Daily_EpgTotalService = CS_DB_GetListServiceNumber(Daily_EpgTriplet);
			// printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, Daily_EpgSatIndex);
			if(Daily_EpgTotalService>0)
			{
				Daily_EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();  
				Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
				Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
				// prevService = Daily_EpgCurrentService;
			}
			CS_DT_GetUTCOffset(&Daily_EpgUtcOffset, &Daily_EpgUtcOffsetPolarity);
#if 0
			Daily_CurrentMjd  = CS_DT_GetLocalMJD();
			currentUtc  = CS_DT_GetLocalUTC();
			Daily_CurrentDate = CS_DT_MJDtoYMD(Daily_CurrentMjd);
			Daily_CurrentTime = CS_DT_UTCtoHM(currentUtc);
#else
			MV_OS_Get_Time_to_MJD_UTC_Date_Time(&Daily_CurrentMjd, &currentUtc, &Daily_CurrentDate, &Daily_CurrentTime);
#endif
			prevMjd     = Daily_CurrentMjd;
			channelChangeCounter = 0;
			channelChangeTime    = 0;
			channelChanged       = 0;
			channelChangeRequest = 0;
			currentMilliTime     = 0;
			
			/* By KB Kim 2011.05.28 */
			CS_DB_GetCurrentList_ServiceData(&channelItemData, Daily_EpgCurrentService);
			MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);
			Mv_GetScEventList (channelServiceData.u16TransponderIndex, channelServiceData.u16ServiceId, 0, &MvScEventList);

			if(MvScEventList.NumberOfEvent > 0)
			{
				Daily_7dayEpgTotalService = MvScEventList.NumberOfEvent;
				Daily_7dayEpgCurrentService = MvScEventList.StartPoint;  
				Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);
				Daily_7dayEpgCurrentPageStartIndex = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE;
			}

			break;

		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_EPG, EN_EPG_TYPE_SCHEDULED);
			tmpX = MV_FONT_WIDTH * 15;
			tmpY = MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight + 8;
			etcRect.top    = MV_MENU_BACK_Y - 6;
			etcRect.bottom = etcRect.top + tmpY;
			etcRect.left   = MV_MENU_BACK_X + MV_MENU_BACK_DX - tmpX;
			etcRect.right  = MV_MENU_BACK_X + MV_MENU_BACK_DX;

			CS_MW_SetSmallWindow((epgLBoxX - 6),(epgChBoxY - 6),(MV_EPG_CN_PIG_DX + 12),(MV_EPG_CN_PIG_DY + 12));
			
			hdc=BeginPaint(hwnd);			
			Daily_EpgDrawButton(hdc);
			MV_Draw_Daily_Epg_Title(hdc, servicelistType);
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(etcRect.left), ScalerHeigthPixel(etcRect.top), ScalerWidthPixel(tmpX), ScalerHeigthPixel(tmpY), &TitleBmp);
			MvDraw_Daily_EpgCurrentTime(hdc);
			Daily_EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_INIT);
			EndPaint(hwnd,hdc);
			hdc=BeginPaint(hwnd);
			Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);
			MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
			EndPaint(hwnd,hdc);
			break;

		case MSG_TIMER:
			if (wparam == CHANNELCHANGE_TIMER_ID)
			{
				if (channelChangeRequest)
				{
					channelChangeCounter++;
					if (channelChangeCounter >= EPG_CHANNEL_CHANGE_MAX_COUNT)
					{
						if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
						{
							KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
						}
						channelChangeCounter = 0;
						channelChangeRequest = 0;
						channelChanged       = 0;
						SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
					}
				}
				else
				{
					if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
					{
						KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
					}
					channelChangeCounter = 0;
					channelChangeRequest = 0;
					channelChanged       = 0;
				}
			}
			break;

		case MSG_NOW_NEXT_UPDATE :
			if ( b8Timer_Window_Status == FALSE && MV_Get_Password_Flag() == FALSE )
			{
				hdc=BeginPaint(hwnd);
				//Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_UPDATE);
				//printf("==== MSG_NOW_NEXT_UPDATE ====\n");
				MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
				EndPaint(hwnd,hdc);
			}
			break;

		case MSG_EPG_DRAW_DESC :
			break;
                        
		case MSG_TIME_UPDATE :
			etcRect.left   = MV_MENU_BACK_X;
			etcRect.right  = MV_MENU_BACK_X + MV_EPG_CN_BAR_DX + 20;
			etcRect.top    = EPG_LIST_Y + (EPG_MENU_BAR_H * 11) + 72;
			etcRect.bottom = etcRect.top + 20;
			currentTime = Daily_CurrentTime;
			startTime   = Daily_EventStartTime;
			if ((Daily_EventDuration.hour > 0) || (Daily_EventDuration.minute > 0))
			{
				// printf("=======  MSG_TIME_UPDATE : Daily_EventStartMjd[%d] Daily_CurrentMjd[%d] \n", Daily_EventStartMjd, Daily_CurrentMjd);
				if (Daily_EventStartMjd < Daily_CurrentMjd)
				{
					currentTime.hour += 24;
				}
				else if (Daily_EventStartMjd > Daily_CurrentMjd)
				{
					startTime.hour += 24;
				}
			}
		
			hdc=BeginPaint(hwnd);
			MvDraw_Daily_EpgCurrentTime(hdc);
			EndPaint(hwnd,hdc);
			break;
			   
		case MSG_VIDEO_FORFMAT_UPDATE:
			break;
		case MSG_CHECK_SERVICE_LOCK:
			if (channelChangeRequest)
			{
				/* Re requested  Channel change : Reset Counter */
				channelChangeCounter = 0;
			}
			else
			{
				if (channelChanged)
				{
					currentMilliTime = OsTimeNowMilli();
					if (OsTimeDiff(currentMilliTime, channelChangeTime) < EPG_CHANNEL_CHANGE_CHECK)
					{
						SetTimer(hwnd, CHANNELCHANGE_TIMER_ID, EPG_CHANNEL_CHANGE_TIMER);
						channelChangeRequest = 1;
						channelChangeCounter = 0;
						channelChanged       = 0;

						break;
					}
				}

				Daily_EpgChangeChannel(hwnd);
				channelChangeRequest = 0;
				channelChanged       = 1;
				channelChangeTime    = OsTimeNowMilli();
			}
			break;

		case MSG_PLAYSERVICE:
#if 0
			CS_DB_GetCurrentList_ServiceData(&channelItemData, Daily_EpgCurrentService);
			CS_MW_PlayServiceByIdx(channelItemData.Service_Index, NOT_TUNNING);
			CS_DB_SetCurrentService_OrderIndex(Daily_EpgCurrentService);
			CS_DB_GetCurrentListTriplet(&(serviceTriplet.sCS_DBU_ServiceList));
			serviceTriplet.sCS_DBU_ServiceIndex =  Daily_EpgCurrentService;
			CS_DBU_SaveCurrentService(serviceTriplet);
			Daily_EpgBackTriplet = serviceTriplet;
			if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
			{
				FbSendFndDisplayNum((unsigned)Daily_EpgCurrentService+1);
			}
			else
			{
				FbSendFndDisplayNum((unsigned)channelItemData.LCN);
			}
#else
			Daily_EpgChangeChannel(hwnd);
#endif
			break;

		case MSG_CLOSE:
			if (channelChangeRequest)
			{
				if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
				{
					KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
				}
				channelChangeCounter = 0;
				channelChangeRequest = 0;
				channelChanged       = 0;
			}
			
			/* By KB Kim 2011.05.28 */
			if (MvScEventList.EventArray != NULL)
			{
				OsMemoryFree(MvScEventList.EventArray);
			}
			MvScEventList.NumberOfEvent = 0;
			MvScEventList.StartPoint    = 0;
			MvScEventList.EventArray    = NULL;
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			CS_DT_Unregister_DateTime_Notify(Daily_date_time_client_id);
			UnloadBitmap(&TitleBmp);
			hdc=BeginPaint(hwnd);
			MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(0), ScalerHeigthPixel(0), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT) );
			EndPaint(hwnd,hdc);

#ifdef  SCREEN_CHANGE_DARK
			CS_AV_SetOSDAlpha(0);
#endif
			DestroyMainWindow (hwnd);
			PostQuitMessage (hwnd);
			break;

		case MSG_MOTOR_MOVING: /* For Motor Control By KB Kim 2011.05.22 */
			if (wparam)
			{
				Mv_MotorMovingDisplay();
			}
			else
			{
				Motor_Moving_Stop();
			}
			break;

		case MSG_PIN_INPUT:
			break;
			
		case MSG_KEYDOWN:
			{
				if (MV_Get_Password_Flag() == TRUE)
				{			
					MV_Password_Proc(hwnd, wparam);
					switch(wparam)
					{
						case CSAPP_KEY_0:
						case CSAPP_KEY_1:
						case CSAPP_KEY_2:
						case CSAPP_KEY_3:
						case CSAPP_KEY_4:
						case CSAPP_KEY_5:
						case CSAPP_KEY_6:
						case CSAPP_KEY_7:
						case CSAPP_KEY_8:
						case CSAPP_KEY_9:
							if(MV_Password_Retrun_Value() == TRUE)
							{
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(Daily_EpgCurrentService);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							break;
							
						case CSAPP_KEY_ENTER:
							if(MV_Password_Retrun_Value() == TRUE)
							{
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(Daily_EpgCurrentService);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							break;

						case CSAPP_KEY_ESC:
						case CSAPP_KEY_MENU:
							MV_Password_Set_Flag(FALSE);
							b8Check_Exit = TRUE;
							hdc = BeginPaint(hwnd);
							MV_Restore_PopUp_Window( hdc );
							EndPaint(hwnd,hdc);
							break;
					}
					break;
				}
				
				if ( b8Detail_Window_Status == TRUE )
				{
					if ( Daily_Epg_Detail_Window_proc(hwnd, wparam) == FALSE )
					{
						switch(wparam)
						{
							case CSAPP_KEY_MENU:
							case CSAPP_KEY_ESC:
								hdc = BeginPaint(hwnd);
								Daily_Epg_Timer_Close(hdc);
								EndPaint(hwnd,hdc);
								break;
							
							default:
								break;
						}
					}
					break;
				}
				
				if ( b8Timer_Window_Status == TRUE )
				{
					if ( Daily_Epg_Timer_proc(hwnd, wparam) == FALSE )
					{
						switch(wparam)
						{
							case CSAPP_KEY_MENU:
							case CSAPP_KEY_ESC:
								hdc = BeginPaint(hwnd);
								Daily_Epg_Timer_Close(hdc);
								EndPaint(hwnd,hdc);
								break;
							
							default:
								break;
						}
					}
					break;
				}
				
				switch(wparam)
				{
					case CSAPP_KEY_DOWN :
						if ( b8Screen_Kind == CH_LIST )
						{
								
							if(Daily_EpgTotalService < 1)
								break;
							hdc = BeginPaint(hwnd);
							MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, UNFOCUS, EN_EPG_TYPE_SIMPLE);
							Daily_EpgPrevPageStart = Daily_EpgCurrentPageStartIndex;

#ifdef EPG_UP_DOWN_SCROLL
							if(Daily_EpgCurrentService == Daily_EpgTotalService-1)
							{
								Daily_EpgCurrentService = 0;
								Daily_EpgCurrentFocus = 0;
								Daily_EpgCurrentPageStartIndex = 0;
								if  (Daily_EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
								{
									Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
									EndPaint(hwnd,hdc);
									channelChangeOn = 1;
									break;
								}
							}
							else
							{
								Daily_EpgCurrentService++;
								Daily_EpgCurrentFocus++;
								if (Daily_EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
								{
									if (Daily_EpgCurrentFocus >= SERVICES_NUM_PER_PAGE_SIMPLE)
									{
										Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_SCROLL_UP);
										Daily_EpgCurrentFocus = SERVICES_NUM_PER_PAGE_SIMPLE - 1;
										Daily_EpgCurrentPageStartIndex++;
									}
								}
							}

							MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE);
#else  // #ifdef EPG_UP_DOWN_SCROLL
							if(Daily_EpgCurrentService == Daily_EpgTotalService-1)
							{
								Daily_EpgCurrentService = 0;
							}
							else
							{
								Daily_EpgCurrentService++;
							}
							Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							if (Daily_EpgPrevPageStart != Daily_EpgCurrentPageStartIndex)
							{
								Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
							}
							else
							{
								MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE);
							}
#endif // #ifdef EPG_UP_DOWN_SCROLL
							EndPaint(hwnd,hdc);
							// SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							CS_APP_SetLastUnlockServiceIndex(0xffff);
							channelChangeOn = 1;
						} else {
							hdc = BeginPaint(hwnd);
							MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, UNFOCUS);

							if ( MvScEventList.NumberOfEvent > 0 )
							{
								Daily_7dayEpgPrevPageStart = Daily_7dayEpgCurrentPage;

								if ( Daily_7dayEpgCurrentService == MvScEventList.NumberOfEvent - 1)
									Daily_7dayEpgCurrentService = 0;
								else
									Daily_7dayEpgCurrentService++;

								Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);

								if ( Daily_7dayEpgPrevPageStart != Daily_7dayEpgCurrentPage )
								{
									// Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);
									MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
								}
								else
									MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, FOCUS);
							} else {
								if ( Daily_7dayEpgCurrentService == 0 )
									Daily_7dayEpgCurrentService = 1;
								else
									Daily_7dayEpgCurrentService = 0;

								MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, FOCUS);
							}
							EndPaint(hwnd,hdc);
						}
						break;
					case CSAPP_KEY_UP:
						if ( b8Screen_Kind == CH_LIST )
						{
							if(Daily_EpgTotalService < 1)
								break;
							hdc = BeginPaint(hwnd);
							MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, UNFOCUS, EN_EPG_TYPE_SIMPLE);
							Daily_EpgPrevPageStart = Daily_EpgCurrentPageStartIndex;

#ifdef EPG_UP_DOWN_SCROLL
							if(Daily_EpgCurrentService == 0)
							{
								Daily_EpgCurrentService = Daily_EpgTotalService-1;
								
								if  (Daily_EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
								{
									Daily_EpgCurrentPageStartIndex = Daily_EpgTotalService - 10;
									Daily_EpgCurrentFocus = SERVICES_NUM_PER_PAGE_SIMPLE - 1;
									Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
									EndPaint(hwnd,hdc);
									channelChangeOn = 1;
									break;
								}
								else
								{
									Daily_EpgCurrentFocus = Daily_EpgCurrentService;
									Daily_EpgCurrentPageStartIndex = 0;
								}
							}
							else
							{
								Daily_EpgCurrentService--;
								if (Daily_EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
								{
									if (Daily_EpgCurrentFocus == 0)
									{
										Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_SCROLL_DOWN);
										Daily_EpgCurrentFocus = 0;
										Daily_EpgCurrentPageStartIndex--;
									}
									else
									{
										Daily_EpgCurrentFocus--;
									}
								}
								else
								{
									Daily_EpgCurrentFocus--;
								}
							}

							MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE);
#else  // #ifdef EPG_UP_DOWN_SCROLL
							if(Daily_EpgCurrentService == 0)
							{
								Daily_EpgCurrentService = Daily_EpgTotalService-1;
							}
							else
							{
								Daily_EpgCurrentService--;
							}
							Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							if (Daily_EpgPrevPageStart != Daily_EpgCurrentPageStartIndex)
							{
								Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
							}
							else
							{
								MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE);
							}
#endif // #ifdef EPG_UP_DOWN_SCROLL
							EndPaint(hwnd,hdc);
							//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							CS_APP_SetLastUnlockServiceIndex(0xffff);
							channelChangeOn = 1;
						} else {
							hdc = BeginPaint(hwnd);
							MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, UNFOCUS);

							if ( MvScEventList.NumberOfEvent > 0 )
							{
								Daily_7dayEpgPrevPageStart = Daily_7dayEpgCurrentPage;

								if ( Daily_7dayEpgCurrentService == 0 )
									Daily_7dayEpgCurrentService = MvScEventList.NumberOfEvent - 1;
								else
									Daily_7dayEpgCurrentService--;

								Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);

								if ( Daily_7dayEpgPrevPageStart != Daily_7dayEpgCurrentPage )
								{
									// Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);
									MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
								}
								else
									MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, FOCUS);
							} else {
								if ( Daily_7dayEpgCurrentService == 0 )
									Daily_7dayEpgCurrentService = 1;
								else
									Daily_7dayEpgCurrentService = 0;
								
								MV_Draw_Daily_Epg_Data(hdc, Daily_7dayEpgCurrentFocus, Daily_EpgCurrentService, TRUE, FOCUS);
							}
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_PG_UP:
						if(Daily_EpgTotalService <= SERVICES_NUM_PER_PAGE_SIMPLE)
							break;

#ifdef EPG_UP_DOWN_SCROLL
						if (Daily_EpgCurrentPageStartIndex == 0)
						{
							Daily_EpgCurrentPageStartIndex = Daily_EpgTotalService - 10;
						}
						else if (Daily_EpgCurrentPageStartIndex < SERVICES_NUM_PER_PAGE_SIMPLE)
						{
							Daily_EpgCurrentPageStartIndex = 0;
						}
						else
						{
							Daily_EpgCurrentPageStartIndex -= SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						Daily_EpgCurrentService = Daily_EpgCurrentPageStartIndex + Daily_EpgCurrentFocus;
#else  // #ifdef EPG_UP_DOWN_SCROLL
						if (Daily_EpgCurrentService <= SERVICES_NUM_PER_PAGE_SIMPLE)
						{
							U16 tmpFocus;

							Daily_EpgCurrentService = Daily_EpgTotalService - 1;
							tmpFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							if (tmpFocus < Daily_EpgCurrentFocus)
							{
								Daily_EpgCurrentFocus = tmpFocus;
							}
							Daily_EpgCurrentService = Daily_EpgCurrentPageStartIndex + Daily_EpgCurrentFocus;
						}
						else
						{
							Daily_EpgCurrentService -= SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
#endif // #ifdef EPG_UP_DOWN_SCROLL
						hdc = BeginPaint(hwnd);
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						channelChangeOn = 1;
						break;

					case CSAPP_KEY_PG_DOWN:
						if(Daily_EpgTotalService <= SERVICES_NUM_PER_PAGE_SIMPLE)
							break;
#ifdef EPG_UP_DOWN_SCROLL
						Daily_EpgCurrentPageStartIndex += SERVICES_NUM_PER_PAGE_SIMPLE;
						if (Daily_EpgCurrentPageStartIndex >= Daily_EpgTotalService)
						{
							Daily_EpgCurrentPageStartIndex = 0;
						}

						Daily_EpgCurrentService = Daily_EpgCurrentPageStartIndex + Daily_EpgCurrentFocus;
						if (Daily_EpgCurrentService >= Daily_EpgTotalService)
						{
							Daily_EpgCurrentService = Daily_EpgTotalService - 1;
							Daily_EpgCurrentFocus   = Daily_EpgCurrentService - Daily_EpgCurrentPageStartIndex;
						}
#else  // #ifdef EPG_UP_DOWN_SCROLL
						Daily_EpgCurrentService += SERVICES_NUM_PER_PAGE_SIMPLE;
						if (Daily_EpgCurrentService >= Daily_EpgTotalService)
						{
							Daily_EpgCurrentService = Daily_EpgCurrentFocus;
						}
						Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
#endif // #ifdef EPG_UP_DOWN_SCROLL
						hdc = BeginPaint(hwnd);
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						// SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						channelChangeOn = 1;
						break;

					case CSAPP_KEY_LEFT:
						if ( u8Select_Day == 0 )
							u8Select_Day = 6;
						else
							u8Select_Day--;
						
						hdc=BeginPaint(hwnd);

						/*
						MvScEventList.NumberOfEvent = 0;
						MvScEventList.StartPoint = 0;
						MvScEventList.EventArray    = NULL;
						*/
						Mv_GetScEventList (channelServiceData.u16TransponderIndex, channelServiceData.u16ServiceId, u8Select_Day, &MvScEventList);
						//printf("Mv_GetScEventList Event Number3 : %d\n", MvScEventList.NumberOfEvent);

						if(MvScEventList.NumberOfEvent > 0)
						{
							Daily_7dayEpgTotalService = MvScEventList.NumberOfEvent;
							if ( u8Select_Day != 0 )
								Daily_7dayEpgCurrentService = 0;
							else
								Daily_7dayEpgCurrentService = MvScEventList.StartPoint;  
							Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);
							Daily_7dayEpgCurrentPageStartIndex = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE;
						}
						
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);
						if(MvScEventList.NumberOfEvent > 0)
							MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_RIGHT:
						if ( u8Select_Day == 6 )
							u8Select_Day = 0;
						else
							u8Select_Day++;
						
						hdc=BeginPaint(hwnd);
						
						/*
						MvScEventList.NumberOfEvent = 0;
						MvScEventList.StartPoint = 0;
						MvScEventList.EventArray    = NULL;
						*/
						Mv_GetScEventList (channelServiceData.u16TransponderIndex, channelServiceData.u16ServiceId, u8Select_Day, &MvScEventList);
						//printf("Mv_GetScEventList Event Number4 : %d\n", MvScEventList.NumberOfEvent);

						if(MvScEventList.NumberOfEvent > 0)
						{
							Daily_7dayEpgTotalService = MvScEventList.NumberOfEvent;
							if ( u8Select_Day != 0 )
								Daily_7dayEpgCurrentService = 0;
							else
								Daily_7dayEpgCurrentService = MvScEventList.StartPoint; 
							Daily_7dayEpgCurrentFocus = get_focus_line(&Daily_7dayEpgCurrentPage, Daily_7dayEpgCurrentService, EPG_NUM_PER_PAGE);
							Daily_7dayEpgCurrentPageStartIndex = Daily_7dayEpgCurrentPage * EPG_NUM_PER_PAGE;
						}
						
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);
						if(MvScEventList.NumberOfEvent > 0)
							MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_SAT:
						{
							U8 Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
							
							if (channelChangeRequest)
							{
								if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
								{
									KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
								}
								channelChangeCounter = 0;
								channelChangeRequest = 0;
								channelChanged       = 0;
							}						

							switch(servicelistType)
							{								
								case CSApp_Applet_Daily_FAV_Radio_EPG:
								case CSApp_Applet_Daily_SAT_Radio_EPG:
								case CSApp_Applet_Daily_Radio_EPG:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case CSApp_Applet_Daily_TV_EPG:
								case CSApp_Applet_Daily_FAV_TV_EPG:
								case CSApp_Applet_Daily_SAT_TV_EPG:
									Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								default:
									break;								
							}
							
							MV_Set_Save_Flag(FALSE);
							MV_Draw_SatFavlist_Window(hwnd, Service_Type);
							popupModeOn = TRUE;
							// MV_Draw_Satlist_Window(hwnd);
						}
						break;

					case CSAPP_KEY_FAVOLIST:
						{
							U8 Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
							
							if (channelChangeRequest)
							{
								if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
								{
									KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
								}
								channelChangeCounter = 0;
								channelChangeRequest = 0;
								channelChanged       = 0;
							}
							MV_Set_Save_Flag(FALSE);

							switch(servicelistType)
							{								
								case CSApp_Applet_Daily_FAV_Radio_EPG:
								case CSApp_Applet_Daily_SAT_Radio_EPG:
								case CSApp_Applet_Daily_Radio_EPG:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case CSApp_Applet_Daily_TV_EPG:
								case CSApp_Applet_Daily_FAV_TV_EPG:
								case CSApp_Applet_Daily_SAT_TV_EPG:
									Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								default:
									break;								
							}
						
							MV_Draw_Favlist_Window(hwnd, Service_Type, TRUE);
							popupModeOn = TRUE;
						}
						break;

					case CSAPP_KEY_TVRADIO:
						switch( servicelistType )
						{
							case CSApp_Applet_Daily_TV_EPG:
								servicelistType = CSApp_Applet_Daily_Radio_EPG;
								break;
							case CSApp_Applet_Daily_Radio_EPG:
								servicelistType = CSApp_Applet_Daily_TV_EPG;
								break;
							case CSApp_Applet_Daily_FAV_TV_EPG:
								servicelistType = CSApp_Applet_Daily_FAV_Radio_EPG;
								break;
							case CSApp_Applet_Daily_FAV_Radio_EPG:
								servicelistType = CSApp_Applet_Daily_FAV_TV_EPG;
								break;
							case CSApp_Applet_Daily_SAT_TV_EPG:
								servicelistType = CSApp_Applet_Daily_SAT_Radio_EPG;
								break;
							case CSApp_Applet_Daily_SAT_Radio_EPG:
								servicelistType = CSApp_Applet_Daily_SAT_TV_EPG;
								break;
							default:
								break;
						}

						servicelistType = MV_Set_Daily_EPG_Current_List(servicelistType, Daily_EpgSatIndex);
						if (servicelistType == prevServiceType)
						{
							/* No Changes */
							break;
						}
						prevServiceType = servicelistType;

						if(Daily_EpgTotalService>0)
						{
							Daily_EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							Daily_EpgCurrentFocus = get_focus_line(&Daily_EpgCurrentPage, Daily_EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							Daily_EpgCurrentPageStartIndex = Daily_EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							channelChangeOn = 1;
						}
						
						hdc = BeginPaint(hwnd);
						MV_Draw_Daily_Epg_Title(hdc, servicelistType);
						MvDraw_Daily_EpgCurrentTime(hdc);
						Daily_EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						
						break;
						
					case CSAPP_KEY_CH_UP:
					case CSAPP_KEY_REW:
						hdc=BeginPaint(hwnd);
						Daily_EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_SCROLL_DOWN);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_CH_DOWN:
					case CSAPP_KEY_FF:
						hdc=BeginPaint(hwnd);
						Daily_EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_SCROLL_UP);
						EndPaint(hwnd,hdc);
						break;
/*
					case CSAPP_KEY_GREEN:
						if ( b8Screen_Kind == PMG_LIST )
						{
							hdc=BeginPaint(hwnd);
							Daily_Epg_Detail_Window(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
*/
					case CSAPP_KEY_RED:
						if ( b8Screen_Kind == PMG_LIST )
						{
							Current_Item = 0;
							hdc=BeginPaint(hwnd);
							Daily_Epg_Timer_Draw(hdc);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_ENTER:
					case CSAPP_KEY_YELLOW:
						if( b8Screen_Kind == CH_LIST )
							if ( MvScEventList.NumberOfEvent > 0 )
								b8Screen_Kind = PMG_LIST;
							else
								b8Screen_Kind = CH_LIST;
						else
							b8Screen_Kind = CH_LIST;
						
						hdc=BeginPaint(hwnd);
						MV_Draw_Daily_Epg_Item_Channel(hdc, Daily_EpgCurrentFocus, Daily_EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE);
						MV_Draw_Daily_Epg_Item(hdc ,Daily_EpgCurrentFocus, Daily_EpgCurrentService);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_IDLE:
						CSApp_7DayEpg_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_BLUE:
						switch( servicelistType )
						{
							case CSApp_Applet_Daily_TV_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_TV_EPG;
								break;
							case CSApp_Applet_Daily_Radio_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_Radio_EPG;
								break;
							case CSApp_Applet_Daily_FAV_TV_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_FAV_TV_EPG;
								break;
							case CSApp_Applet_Daily_FAV_Radio_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_FAV_Radio_EPG;
								break;
							case CSApp_Applet_Daily_SAT_TV_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_SAT_TV_EPG;
								break;
							case CSApp_Applet_Daily_SAT_Radio_EPG:
								CSApp_7DayEpg_Applets = CSApp_Applet_SAT_Radio_EPG;
								break;
							default:
								break;
						}
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						break;
						
					case CSAPP_KEY_ESC:
						CSApp_7DayEpg_Applets = CSApp_Applet_Desktop;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						break;
					default:
						break;
				}
			}

			if (channelChangeOn)
			{
				channelChangeOn = 0;
				SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
			}
			else
			{
				channelChanged = 0;
			}
			break;
		default:
        		break;
    	}
	
	return DefaultMainWinProc(hwnd, message, wparam, lparam);	
}

#endif

