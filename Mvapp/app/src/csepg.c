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

#ifdef DEBUG_ON
	// #define EPG_DEBUG_ON
#endif // #ifdef DEBUG_ON

#define EPG_CHANNEL_NAME_WIDTH      		220
#define EPG_SIMPLE_DATA_WIDTH       		240
#define EPG_SCROLL_BAR_WIDTH                20
#define	SERVICES_NUM_PER_PAGE_SIMPLE		10
#define EPG_CHANNEL_CHANGE_TIMER            500
#define EPG_CHANNEL_CHANGE_MAX_COUNT        2
#define EPG_CHANNEL_CHANGE_CHECK            1000
#define EPG_INFO_UPDATE_TIMER               1000  /* By KB Kim 2011.05.28 */

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

static CSAPP_Applet_t	        CSApp_Epg_Applets;
tCS_DB_ServiceListTriplet		EpgTriplet;     // was chedit_triplet
tCS_DBU_Service     			EpgBackTriplet; // was back_triplet
static tCS_DT_Date              CurrentDate;
static tCS_DT_Time              CurrentTime;
static tCS_DT_Time              EventStartTime;
static tCS_DT_Time              EventDuration;
static U16                      EventStartMjd;
static U16                      CurrentMjd;
static BITMAP	                TitleBmp;
/*
EpgChannel_t                   *FirstEpgChannel = NULL;
*/
tCS_DT_OffsetPolarity   EpgUtcOffsetPolarity;
U16                     EpgUtcOffset;

static char            *EpgCurrentDetailInfo= NULL;
static char            *EpgEventTitle = NULL;
static U32              EpgDetailInfoLine = 0;
static U32              EpgNameLine = 0;
static U32              EpgCharPerLine;
static U16				EpgTotalService = 0;	 // was Total_Service
static U16				EpgCurrentService = 0;   // was chlist_Current_Service
static U16				EpgCurrentPage = 0; // was chlist_Current_Page
static U16				EpgPrevPageStart = 0; // was chlist_Prev_Page
static U16				EpgCurrentFocus = 0; // was chlist_Current_Focus
static U16              EpgCurrentPageStartIndex = 0;
static U8               date_time_client_id = 0;
static U8               ColorRow;
static U8               ColorCol;
static U8				EpgSatIndex = 0;  // was u8Sat_index
static U8				EpgCurrentCol = 0; 
static U8               EpgCurrentAge = 0;
static U8               EpgCurrentProgram = 0;
static U8               EpgUpdated = 0;  /* By KB Kim 2011.05.28 */
static BOOL				b8Check_Exit = FALSE;

BOOL AddEpgChannel(U16 channelIndex, U16 serviceId)
{
	U16 Temp_warnign1;
	U16	Temp_warning2;

	Temp_warnign1 = channelIndex;
	Temp_warning2 = serviceId;
	#if 0
	EpgChannel_t *currentEpgChannel;
	EpgChannel_t *prevEpgChannel;

	currentEpgChannel = FirstEpgChannel;
	prevEpgChannel = FirstEpgChannel;
	while (currentEpgChannel != NULL)
	{
		prevEpgChannel = currentEpgChannel;
		currentEpgChannel = currentEpgChannel->NextChannel;
	}
	currentEpgChannel = OsMemoryAllocate(sizeof(EpgChannel_t));
	if (currentEpgChannel == NULL)
	{
#ifdef EPG_DEBUG_ON
		OsDebugPrtf("AddEpgChannel Error : Cannot allocate memory for new channel\n");
#endif // #ifdef EPG_DEBUG_ON
		return FALSE;
	}
	currentEpgChannel->NextChannel  = NULL;
	currentEpgChannel->ChannelIndex = channelIndex;
	currentEpgChannel->ServiceId    = serviceId;
	currentEpgChannel->FirstEvent   = NULL;

	if (prevEpgChannel == NULL)
	{
		FirstEpgChannel = currentEpgChannel;
	}
	else
	{
		prevEpgChannel->NextChannel = currentEpgChannel;
	}
	#endif

	return TRUE;
}

U32 AdjustYPosition(U32 yPosition, U32 boxH, U32 sourceH)
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

U8 TimeAdd(tCS_DT_Time timeA, tCS_DT_Time timeB, tCS_DT_Time *timeResult)
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

void EpgDrawProgress(HDC hdc, RECT barRect, tCS_DT_Time startTime, tCS_DT_Time duration, tCS_DT_Time currentTime, U8 type)
{
	static U32  prevProcWidth = 0;
	U32  timeY;
	U16  sTime;
	U16  eTime;
	U16  cTime;
	U16  tTmeDiff;
	U16  timeProcess;
	U32  width;
	U32  Height;
	U32  processY;
	U32  processH;
	U32  charWidth;
	U32  charHeight;
	U32  progWidth;
	U8   endDayOver   = 0;
	U8   startDayOver = 0;
	U8   startDayBehind = 0;
	RECT timeRect;
	tCS_DT_Time endTime;
	char timeS[10];
	char timeE[10];

	charWidth = MV_FONT_WIDTH * 7;
	Height = barRect.bottom - barRect.top;
	width  = barRect.right - barRect.left - (charWidth * 2);
	timeY = AdjustYPosition(barRect.top, Height, MV_FONT_HEIGHT);
	tTmeDiff = 0;
	if (MV_FONT_HEIGHT < Height)
	{
		charHeight = Height;
		processY = barRect.top + 2;
		processH = charHeight - 4;
	}
	else
	{
		charHeight = MV_FONT_HEIGHT;
		processY = timeY + 2;
		processH = charHeight - 4;
	}

	if ((type == EPG_DRAW_PAGE) || (type == EPG_DRAW_UPDATE))
	{
		timeProcess = 0;
		sTime    = ((U16)startTime.hour * 60) + (U16)startTime.minute;
		tTmeDiff = ((U16)duration.hour * 60) + (U16)duration.minute;
		eTime = sTime + tTmeDiff;
		cTime = ((U16)currentTime.hour * 60) + (U16)currentTime.minute;

		if ((tTmeDiff == 0) || (cTime <= sTime))
		{
			progWidth = 0;
		}
		else if (cTime >= eTime)
		{
			progWidth= width;
		}
		else
		{
			timeProcess = cTime - sTime;
			progWidth = (width * timeProcess) / tTmeDiff;
		}
	}
	else
	{
		progWidth = 0;
	}

	if (type == EPG_DRAW_UPDATE)
	{
		if (progWidth == prevProcWidth)
		{
			return;
		}
	}
	else
	{
		MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR);
		MV_FillBox( hdc, ScalerWidthPixel(barRect.left), ScalerHeigthPixel(timeY), ScalerWidthPixel(width + (charWidth * 2)), ScalerHeigthPixel(charHeight));
	}

	if ((type == EPG_DRAW_PAGE) && (tTmeDiff > 0))
	{
		memset(timeS, 0x00, 10);
		memset(timeE, 0x00, 10);

		// printf("=======  EpgDrawProgress : currentTime.hour[%02d]\n", currentTime.hour);
		if (currentTime.hour >= 24)
		{
			startDayBehind = 1;
		}
		else if (startTime.hour >= 24)
		{
			startTime.hour -= 24;
			startDayOver = 1;
		}
		endDayOver = TimeAdd(startTime, duration, &endTime);
		timeRect.top    = timeY;
		timeRect.bottom = timeY + charHeight;
		timeRect.left   = barRect.left;
		timeRect.right  = timeRect.left + charWidth;
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		if (startDayBehind)
		{
			sprintf(timeS, "-%02d:%02d", startTime.hour, startTime.minute);
			if (endDayOver)
			{
				sprintf(timeE, "%02d:%02d", endTime.hour, endTime.minute);
			}
			else
			{
				sprintf(timeE, "-%02d:%02d", endTime.hour, endTime.minute);
			}
		}
		else if (startDayOver)
		{
			sprintf(timeS, "%02d:%02d*", startTime.hour, startTime.minute);
			sprintf(timeE, "%02d:%02d*", endTime.hour, endTime.minute);
		}
		else
		{
		sprintf(timeS, "%02d:%02d", startTime.hour, startTime.minute);
			if (endDayOver)
		{
				sprintf(timeE, "%02d:%02d*", endTime.hour, endTime.minute);
		}
		else
		{
				sprintf(timeE, "%02d:%02d", endTime.hour, endTime.minute);
			}
		}
		MV_MW_DrawText_Static(hdc, timeS, -1, &timeRect, DT_CENTER);
		timeRect.left   = barRect.left + width + charWidth;
		timeRect.right  = timeRect.left + charWidth;
		MV_MW_DrawText_Static(hdc, timeE, -1, &timeRect, DT_CENTER);
	}
	
	if (progWidth == width)
	{
		MV_SetBrushColor( hdc, MVAPP_LIGHT_GREEN_COLOR);
		MV_FillBox( hdc, ScalerWidthPixel(barRect.left + charWidth), ScalerHeigthPixel(processY), ScalerWidthPixel(width), ScalerHeigthPixel(processH));
	}
	else if (progWidth == 0)
	{
		MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR);
		MV_FillBox( hdc, ScalerWidthPixel(barRect.left + charWidth), ScalerHeigthPixel(processY), ScalerWidthPixel(width), ScalerHeigthPixel(processH));
	}
	else
	{
		MV_SetBrushColor( hdc, MVAPP_LIGHT_GREEN_COLOR);
		MV_FillBox( hdc, ScalerWidthPixel(barRect.left + charWidth), ScalerHeigthPixel(processY), ScalerWidthPixel(progWidth), ScalerHeigthPixel(processH) );
		MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR);
		MV_FillBox( hdc, ScalerWidthPixel(barRect.left + progWidth + charWidth), ScalerHeigthPixel(processY), ScalerWidthPixel(width - progWidth), ScalerHeigthPixel(processH));
	}

	prevProcWidth = progWidth;
}

void EpgDrawDetailedBox(HDC hdc, U8 mode, U8 type)
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
		boxX = MV_MENU_BACK_X + MV_MENU_BACK_DX - MV_EPG_CN_PIG_DX - 10;
		boxY = MV_MENU_BACK_Y + MV_EPG_CN_PIG_DY + 88; /* 54 + 20 + 14 */
		boxH = CSAPP_OSD_MAX_HEIGHT - boxY - 86;
		boxW = MV_EPG_CN_PIG_DX - SCROLL_BAR_DX;
		maxLine = boxH / MV_FONT_HEIGHT;
	}
	else
	{
		boxX = MV_MENU_BACK_X + MV_MENU_BACK_DX - MV_EPG_CN_PIG_DX - 10;
		boxY = MV_MENU_BACK_Y + MV_EPG_CN_PIG_DY + 88; /* 54 + 20 + 14 */
		boxH = CSAPP_OSD_MAX_HEIGHT - boxY - 86;
		boxW = MV_EPG_CN_PIG_DX - SCROLL_BAR_DX;
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
			numberOfLine = maxLine - EpgNameLine;
			if (numberOfLine > EpgDetailInfoLine)
			{
				numberOfLine = EpgDetailInfoLine;
			}
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < EpgNameLine)
			{
				MV_MW_DrawText_Static(hdc, (EpgEventTitle + pointer), -1, &charRect, DT_LEFT);
				// printf("%s\n", (EpgEventTitle + pointer));
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += EpgCharPerLine;
				lineCounter++;
			}
			
			lineCounter = 0;
			pointer     = 0;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			while(lineCounter < numberOfLine)
			{
				MV_MW_DrawText_Static(hdc, (EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
				// printf("%s\n", (EpgCurrentDetailInfo + pointer));
				charRect.top    = charRect.bottom;
				charRect.bottom = charRect.top + MV_FONT_HEIGHT;
				pointer += EpgCharPerLine;
				lineCounter++;
			}
			break;
		case EPG_DRAW_SCROLL_UP :
			if ((startLine + numberOfLine) >= EpgDetailInfoLine)
			{
				break;
			}
			direction = 1;
		case EPG_DRAW_SCROLL_DOWN :
			if (EpgDetailInfoLine <= numberOfLine)
			{
				break;
			}

			if ((startLine == 0) && (direction == 0))
			{
				break;
			}
			boxY = boxY + (MV_FONT_HEIGHT * EpgNameLine);
			boxH = MV_FONT_HEIGHT * (numberOfLine - 1);
			memset(&scrollBmp, 0x00, sizeof(BITMAP));
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			direction = 1 - direction;
			FillBoxWithBitmap(hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY + (MV_FONT_HEIGHT * direction)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			UnloadBitmap(&scrollBmp);
			if (direction)
			{
				/* EPG_DRAW_SCROLL_DOWN */
				// boxY = boxY + (MV_FONT_HEIGHT * EpgNameLine);
				startLine--;
				pointer = EpgCharPerLine * startLine;
			}
			else
			{
				/* EPG_DRAW_SCROLL_UP */
				boxY = boxY + (MV_FONT_HEIGHT * (numberOfLine - 1));
				pointer = EpgCharPerLine * (startLine + numberOfLine);
				startLine++;
			}

			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(boxX), ScalerHeigthPixel(boxY), ScalerWidthPixel(boxW), ScalerHeigthPixel(MV_FONT_HEIGHT));
			charRect.top    = boxY;
			charRect.bottom = charRect.top + MV_FONT_HEIGHT;
			SetTextColor(hdc, CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc, (EpgCurrentDetailInfo + pointer), -1, &charRect, DT_LEFT);
			// printf("%s\n", (EpgCurrentDetailInfo + pointer));
			break;
		default :
			return;
	}

	boxY = MV_MENU_BACK_Y + MV_EPG_CN_PIG_DY + 88; 
	boxH = CSAPP_OSD_MAX_HEIGHT - boxY - 86;
	scrollRect.left   = boxX + boxW;
	scrollRect.right  = scrollRect.left + SCROLL_BAR_DX;
	scrollRect.top    = boxY;
	scrollRect.bottom = scrollRect.top + boxH;
	MV_Draw_PageScrollBar(hdc, scrollRect, startLine, EpgDetailInfoLine, (maxLine - EpgNameLine));

}

void EpgDrawChannelInfo(HDC hdc, MV_stServiceInfo channelServiceData, U8 mode)
{
	U32          epgBBoxX;
	U32          epgBBoxW;
	U32          epgBBoxY;
	U32          epgBBoxH;
	MV_stTPInfo	 tpdata;
	MV_stSatInfo satData;
	char		 buff[100];

	memset(buff, 0x00, 100);
	MV_DB_GetTPDataByIndex(&tpdata, channelServiceData.u16TransponderIndex);
	MV_GetSatelliteData_ByIndex(&satData, MV_DB_Get_SatIndex_By_TPindex(channelServiceData.u16TransponderIndex));
	
	if (mode == EN_EPG_TYPE_SIMPLE)
	{
		epgBBoxX = MV_MENU_BACK_X;
		epgBBoxW = MV_EPG_CN_BAR_DX + 20;
		epgBBoxY = MV_MENU_BACK_Y + (MV_INSTALL_MENU_BAR_H * 11) + 102;
		epgBBoxH = CSAPP_OSD_MAX_HEIGHT - epgBBoxY - 106;
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(epgBBoxX), ScalerHeigthPixel(epgBBoxY), ScalerWidthPixel(epgBBoxW), ScalerHeigthPixel(epgBBoxH) );
		FillBoxWithBitmap (hdc, ScalerWidthPixel(epgBBoxX + 20), ScalerHeigthPixel(epgBBoxY + 20), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_INFO_ICON]);

		epgBBoxY += 10;
		epgBBoxX += (30 + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth);
		SetTextColor(hdc, CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		sprintf(buff, "%s", channelServiceData.acServiceName);
		CS_MW_TextOut(hdc,ScalerWidthPixel(epgBBoxX),ScalerHeigthPixel(epgBBoxY), buff);

		epgBBoxY += 30;
		memset(buff, 0x00, 100);
		if ( tpdata.u8Polar_H == 1 )
			sprintf(buff, "%s - %d / H / %d", satData.acSatelliteName, tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		else
			sprintf(buff, "%s - %d / V / %d", satData.acSatelliteName, tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(epgBBoxX),ScalerHeigthPixel(epgBBoxY), buff);

		epgBBoxX = MV_MENU_BACK_X + epgBBoxW -(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth * 3) - 18;
		epgBBoxY -= 30;
		if(channelServiceData.u8Scramble)
		{
			FillBoxWithBitmap (hdc, ScalerWidthPixel(epgBBoxX), ScalerHeigthPixel(epgBBoxY), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
		}

		epgBBoxX += (MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth + 4);
		if(channelServiceData.u8Lock)
		{
			FillBoxWithBitmap (hdc, ScalerWidthPixel(epgBBoxX), ScalerHeigthPixel(epgBBoxY), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_LOCK_ICON]);
		}

		epgBBoxX += (MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmWidth + 4);
		if(channelServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE)
		{
			FillBoxWithBitmap (hdc, ScalerWidthPixel(epgBBoxX), ScalerHeigthPixel(epgBBoxY), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_HD_ICON]);
		}
	}
}

void MV_Draw_Epg_Item_Channel (HDC hdc, int index, U16 u16Focusindex, U8 FocusKind, U8 mode)
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

	if (mode == EN_EPG_TYPE_SIMPLE)
	{
		positionX = MV_MENU_BACK_X + 10;
		positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
		boxW      = EPG_CHANNEL_NAME_WIDTH - 2;
		nameWidth = boxW - 40;
	}
	else
	{
		positionX = MV_MENU_BACK_X + 10;
		positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
		boxW      = EPG_CHANNEL_NAME_WIDTH - 2;
		nameWidth = boxW - 40;
	}
	boxH = MV_INSTALL_MENU_BAR_H - 2;
	
	colorCount = (ColorRow + index) % 2;
		
	// printf("%04d : %d : %d : %s ======\n", u16Focusindex, channelItemData.Service_Index, channelServiceData.u16ChIndex, channelServiceData.acServiceName);

	if ( FocusKind == FOCUS)
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	}
	else
	{
		SetBrushColor(hdc, channelCollor[colorCount]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH));
	}

	if ( FocusKind != NOTFOCUS )
	{
		charRect.left   = positionX + 10;
		charRect.top    = AdjustYPosition(positionY, boxH, MV_FONT_HEIGHT);
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

		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
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
			EpgDrawChannelInfo(hdc, channelServiceData, mode);
			positionX = MV_MENU_BACK_X + 10;
			positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
			scrollRect.top = MV_MENU_BACK_Y + 54;
			scrollRect.left = positionX + MV_EPG_CN_BAR_DX - SCROLL_BAR_DX;
			scrollRect.right = scrollRect.left + SCROLL_BAR_DX;
			scrollRect.bottom = scrollRect.top + MV_INSTALL_MENU_BAR_H * 11 -2;
			MV_Draw_ScrollBar(hdc, scrollRect, u16Focusindex, EpgTotalService, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
		}
	}
}

void MV_Draw_Epg_Item(HDC hdc, int index, U16 u16Focusindex, U8 FocusKind, U8 mode, U8 type)
{
	char	                    buff1[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	char	                    buff2[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	char	                    buff3[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	RECT                        charRect;
	RECT                        etcRect;
	U32                         positionX;
	U32                         positionY;
	U32                         boxH;
	U32                         boxW;
	U32                         tmpNumber;
	U8                          colorCount;
	U32                         channelCollor[2]  = {MVAPP_DARKBLUE_COLOR, MVAPP_DARK_GRAY_COLOR};
	U32                         epgBackColor[2];
	U32                         epgCharColor[2];
	tCS_DB_ServiceManageData	channelItemData;
	MV_stServiceInfo			channelServiceData;
	tCS_EIT_Event_t             present;
	tCS_EIT_Event_t             follow;
	tCS_EIT_Event_t            *current;
	tCS_EIT_Error               epgResult;
	tCS_DT_Time                 currentTime;
	tCS_DT_Time                 startTime;
	
	if (mode == EN_EPG_TYPE_SIMPLE)
	{
		positionX = MV_MENU_BACK_X + EPG_CHANNEL_NAME_WIDTH + 10;
		positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
		boxW      = EPG_SIMPLE_DATA_WIDTH;
		EpgCharPerLine = (MV_EPG_CN_PIG_DX - SCROLL_BAR_DX) / MV_FONT_WIDTH;
	}
	else
	{
		positionX = MV_MENU_BACK_X + EPG_CHANNEL_NAME_WIDTH + 10;
		positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
		boxW      = EPG_SIMPLE_DATA_WIDTH;
		EpgCharPerLine = (MV_EPG_CN_PIG_DX - SCROLL_BAR_DX) / MV_FONT_WIDTH;
	}
	tmpNumber = (boxW - 12) / MV_FONT_WIDTH - 1;
	boxH = MV_INSTALL_MENU_BAR_H - 2;
	colorCount = (ColorRow + index) % 2;
	if (FocusKind == NOTFOCUS)
	{
		SetBrushColor(hdc, channelCollor[colorCount]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		positionX += boxW;
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));

		return;
	}
	
	charRect.left   = positionX + 6;
	charRect.top    = AdjustYPosition(positionY, boxH, MV_FONT_HEIGHT);
	charRect.right  = charRect.left + boxW - 12;
	charRect.bottom = charRect.top + MV_FONT_HEIGHT;
	
	CS_DB_GetCurrentList_ServiceData(&channelItemData, u16Focusindex);
	MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);
	epgResult = CS_EIT_Get_PF_Event(channelServiceData.u16TransponderIndex , channelServiceData.u16ServiceId, &present, &follow);

	memset(buff1, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
	memset(buff2, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
	memset(buff3, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
	if (epgResult == eCS_EIT_NO_ERROR)
	{
		if (present.nameLength > 0)
		{
			sprintf(buff1, "%s", present.event_name);
			memset (buff1 + tmpNumber, 0x00, (kCS_EIT_MAX_EVENT_NAME_LENGTH - tmpNumber - 2));
		}
		else
		{
			sprintf(buff1, "-");
		}
		if (follow.nameLength > 0)
		{
			sprintf(buff2, "%s", follow.event_name);
			memset (buff2 + tmpNumber, 0x00, (kCS_EIT_MAX_EVENT_NAME_LENGTH - tmpNumber - 2));
		}
		else
		{
			sprintf(buff2, "-");
		}
	}
	else
	{
		/* No EPG Data comming */
		sprintf(buff1, "-");
		sprintf(buff2, "-");
	}

	if ( FocusKind == FOCUS )
	{
		if (EpgCurrentCol)
		{
			epgBackColor[0] = channelCollor[colorCount];
			epgBackColor[1] = MVAPP_YELLOW_COLOR;
			
			epgCharColor[0] = CSAPP_WHITE_COLOR;
			epgCharColor[1] = MVAPP_BLACK_GRAY_COLOR;
			current = &follow;
		}
		else
		{
			epgBackColor[0] = MVAPP_YELLOW_COLOR;
			epgBackColor[1] = channelCollor[colorCount];
			
			epgCharColor[0] = MVAPP_BLACK_GRAY_COLOR;
			epgCharColor[1] = CSAPP_WHITE_COLOR;
			current = &present;
		}
		
		SetBrushColor(hdc, epgBackColor[0]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		SetTextColor(hdc, epgCharColor[0]);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_CENTER);
		// CS_MW_DrawText(hdc, buff1, -1, &charRect, DT_LEFT);

		positionX += boxW;
		SetBrushColor(hdc, epgBackColor[1]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		charRect.left   = positionX + 6;
		charRect.right  = charRect.left + boxW - 12;
		SetTextColor(hdc, epgCharColor[1]);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_MW_DrawText_Static(hdc, buff2, -1, &charRect, DT_CENTER);
		// CS_MW_DrawText(hdc, buff2, -1, &charRect, DT_LEFT);

		etcRect.left   = MV_MENU_BACK_X;
		etcRect.right  = MV_MENU_BACK_X + MV_EPG_CN_BAR_DX + 20;
		etcRect.top    = MV_MENU_BACK_Y + (MV_INSTALL_MENU_BAR_H * 11) + 72;
		etcRect.bottom = etcRect.top + 20;
		currentTime = CurrentTime;
		
		if (epgResult == eCS_EIT_NO_ERROR)
		{
			EventStartTime = CS_DT_UTCtoHM(current->start_time_utc);
			EventDuration  = CS_DT_UTCtoHM(current->duration_utc);
			EventStartMjd  = current->start_date_mjd;
			startTime = EventStartTime;
			if(CurrentMjd > EventStartMjd)
			{
				currentTime.hour += 24;
			}
			else if (EventStartMjd > CurrentMjd)
			{
				startTime.hour += 24;
			}
						
			if (current->nameLength > 0)
			{
				if (current->parental_rating > 0)
				{
					sprintf(buff3, "%s (%s) - <%d>", current->event_name, current->short_desc_language, current->parental_rating);
				}
				else
				{
				sprintf(buff3, "%s (%s)", current->event_name, current->short_desc_language);
			}
			}
			else
			{
				sprintf(buff3, " (%s)", current->short_desc_language);
			}
			EpgNameLine = MvConvertTextforWindow(buff3, &EpgEventTitle, EpgCharPerLine, (current->nameLength + 14));

			if (current->descriptionLength > 0)
			{
				EpgDetailInfoLine = MvConvertTextforWindow(current->description_data, &EpgCurrentDetailInfo, EpgCharPerLine, current->descriptionLength);
				// printf("============= 1 ==========> EpgDetailInfoLine[%d] \n", EpgDetailInfoLine);
			}
			else
			{
				EpgDetailInfoLine = 0;
			}

			EpgCurrentAge = current->parental_rating;
			EpgCurrentProgram = (current->contentData >> 4) & 0x0f;
		}
		else
		{
			EventStartTime.hour   = 0;
			EventStartTime.minute = 0;
			EventDuration.hour    = 0;
			EventDuration.minute  = 0;
			startTime = EventStartTime;

			memset(buff3, 0x00, kCS_EIT_MAX_EVENT_NAME_LENGTH);
			sprintf(buff3, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_NO_DATA));
			EpgNameLine = MvConvertTextforWindow(buff3, &EpgEventTitle, EpgCharPerLine, (sizeof(buff3) + 2));
			EpgNameLine = 1;
			EpgDetailInfoLine = 0;
		}

		EpgDrawProgress(hdc, etcRect, startTime, EventDuration, currentTime, EPG_DRAW_PAGE);
		
		EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_PAGE);
	}
	else
	{
		SetBrushColor(hdc, channelCollor[colorCount]);
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		positionX += boxW;
		FillBox(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(boxW - 2), ScalerHeigthPixel(boxH));
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_MW_DrawText_Static(hdc, buff1, -1, &charRect, DT_CENTER);
		// CS_MW_DrawText(hdc, buff1, -1, &charRect, DT_LEFT);
		charRect.left   = positionX + 6;
		charRect.right  = charRect.left + boxW - 12;
		MV_MW_DrawText_Static(hdc, buff2, -1, &charRect, DT_CENTER);
		// CS_MW_DrawText(hdc, buff2, -1, &charRect, DT_LEFT);
	}
	
	if (type)
	{
		/* Draw channel information */
		MV_Draw_Epg_Item_Channel (hdc, index, u16Focusindex, FocusKind, mode);
	}
	
}

void EpgDrawCnBox(HDC hdc, U8 serviceNumberPerPage, U8 type)
{
	U8      scrollDirection = 0;
	U8      colorCount;
	U8      row;
	U16     index;
	U32     positionX;
	U32     positionY;
	U32     boxH;
	U32     boxW;
	RECT    charRect;
	RECT    scrollRect;
	BITMAP	scrollBmp;
	// U32 ChannelCollor[2]  = {MVAPP_BLACK_GRAY_COLOR, MVAPP_DARK_GRAY_COLOR};
	// U32 InnerCollor [2] = { MVAPP_DARK_CYAN_COLOR, MVAPP_BLACK_CYAN_COLOR };	

	boxH = MV_INSTALL_MENU_BAR_H - 2;

	switch(type)
	{
		case EPG_DRAW_INIT :
			ColorRow = 0;
			ColorCol = 0;
			colorCount = 0;
			positionX = MV_MENU_BACK_X + 10;
			positionY = MV_MENU_BACK_Y + 54;
			// MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR);
			// MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(EPG_CHANNEL_NAME_WIDTH), ScalerHeigthPixel(boxH));
			positionX += EPG_CHANNEL_NAME_WIDTH;
			MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR /* ChannelCollor[colorCount] */);
			MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(EPG_SIMPLE_DATA_WIDTH - 2), ScalerHeigthPixel(boxH));
			charRect.top    = positionY;
			charRect.bottom = charRect.top + boxH;
			charRect.left   = positionX;
			charRect.right  = charRect.left + EPG_SIMPLE_DATA_WIDTH - 2;
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc,  CS_MW_LoadStringByIdx(CSAPP_STR_EPG_CURRENT), -1, &charRect, DT_CENTER);
			
			positionX += EPG_SIMPLE_DATA_WIDTH;
			MV_SetBrushColor( hdc, MVAPP_DARK_GRAY_COLOR /* ChannelCollor[1 - colorCount] */);
			MV_FillBox( hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY), ScalerWidthPixel(EPG_SIMPLE_DATA_WIDTH - 2), ScalerHeigthPixel(boxH));
			charRect.left   = positionX;
			charRect.right  = charRect.left + EPG_SIMPLE_DATA_WIDTH - 2;
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc,  CS_MW_LoadStringByIdx(CSAPP_STR_EPG_NEXT), -1, &charRect, DT_CENTER);
		case EPG_DRAW_PAGE :
			for (row = 0; row < serviceNumberPerPage; row++)
			{
				index = EpgCurrentPageStartIndex + row;

				if(index < EpgTotalService)
				{
					if(EpgCurrentFocus == row)
						MV_Draw_Epg_Item(hdc, row, index, FOCUS, EN_EPG_TYPE_SIMPLE, 1);
					else
						MV_Draw_Epg_Item(hdc, row, index, UNFOCUS, EN_EPG_TYPE_SIMPLE, 1);
				}
				else
				{
					MV_Draw_Epg_Item(hdc, row, index, NOTFOCUS, EN_EPG_TYPE_SIMPLE, 1);
				}
			}

			if (EpgTotalService == 0)
			{
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				positionX = MV_MENU_BACK_X + 10;
				positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H * (index +1) + 54;
				scrollRect.top = MV_MENU_BACK_Y + 54;
				scrollRect.left = positionX + MV_EPG_CN_BAR_DX - SCROLL_BAR_DX;
				scrollRect.right = scrollRect.left + SCROLL_BAR_DX;
				scrollRect.bottom = scrollRect.top + MV_INSTALL_MENU_BAR_H * 11 -2;
				CS_MW_TextOut(hdc, ScalerWidthPixel(positionX + EPG_CHANNEL_NAME_WIDTH + 20), ScalerHeigthPixel(scrollRect.top + MV_INSTALL_MENU_BAR_H),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));
				MV_Draw_ScrollBar(hdc, scrollRect, EpgCurrentService, EpgTotalService, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
			}
			break;
		case EPG_DRAW_SCROLL_UP   :
			scrollDirection = 1;
		case EPG_DRAW_SCROLL_DOWN :
			positionX = MV_MENU_BACK_X + 10;
			positionY = MV_MENU_BACK_Y + MV_INSTALL_MENU_BAR_H + 54;
			boxW      = EPG_CHANNEL_NAME_WIDTH + (EPG_SIMPLE_DATA_WIDTH * 2) - 2;
			boxH      = MV_INSTALL_MENU_BAR_H * (serviceNumberPerPage - 1);
			memset(&scrollBmp, 0x00, sizeof(BITMAP));
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY + (MV_INSTALL_MENU_BAR_H * scrollDirection)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			scrollDirection = 1 - scrollDirection;
			FillBoxWithBitmap(hdc, ScalerWidthPixel(positionX), ScalerHeigthPixel(positionY + (MV_INSTALL_MENU_BAR_H * scrollDirection)), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &scrollBmp);
			UnloadBitmap(&scrollBmp);
			ColorRow = 1- ColorRow;
			break;
		case EPG_DRAW_UPDATE     :
			for (row = 0; row < serviceNumberPerPage; row++)
			{
				index = EpgCurrentPageStartIndex + row;

				if(index < EpgTotalService)
				{
					if(EpgCurrentFocus == row)
						MV_Draw_Epg_Item(hdc, row, index, FOCUS, EN_EPG_TYPE_SIMPLE, 0);
					else
						MV_Draw_Epg_Item(hdc, row, index, UNFOCUS, EN_EPG_TYPE_SIMPLE, 0);
				}
				else
				{
					break;
				}
			}
			break;
		default :
			break;
	}
}

U32 MV_Set_EPG_Current_List(U32 epgType, U8 satIndex)
{
	U32 returnType;

	returnType = epgType;
	
	switch ( epgType )
	{
		case CSApp_Applet_TV_EPG:
			EpgTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_Radio_EPG:
			EpgTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_FAV_TV_EPG:
			EpgTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			break;
		case CSApp_Applet_FAV_Radio_EPG:
			EpgTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			break;
		case CSApp_Applet_SAT_TV_EPG:
			if ( satIndex == 255 )
			{
				returnType = CSApp_Applet_TV_EPG;
				EpgTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				EpgTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
				EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			}
			break;
		case CSApp_Applet_SAT_Radio_EPG:
			if ( satIndex == 255 )
			{
				returnType = CSApp_Applet_Radio_EPG;
				EpgTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				EpgTriplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				EpgTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
				EpgTriplet.sCS_DB_ServiceListTypeValue = satIndex;
			}
			break;
		default:
			break;
	}
	
	EpgTotalService = CS_DB_GetListServiceNumber(EpgTriplet);
	// printf("=== CH EDIT TOTAL : %d , %d , %d ====\n", EpgTriplet.sCS_DB_ServiceListType, EpgTriplet.sCS_DB_ServiceListTypeValue, EpgTotalService);

	if( EpgTotalService > 0 )
	{
		CS_DB_SetCurrentList(EpgTriplet, FALSE);
	}

	return returnType;
}

void MV_Draw_Epg_Title(HDC hdc, U32 serviceType)
{
	char 			title[50];
	char			tempStr[20];
	RECT 			tmpRect;
	MV_stSatInfo	tempSatData;
	
	memset(title, 0, 50);
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	switch(serviceType)
	{
		case CSApp_Applet_TV_EPG:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
		case CSApp_Applet_Radio_EPG:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_RD_LIST));
			break;
		case CSApp_Applet_FAV_TV_EPG:
			MV_DB_Get_Favorite_Name(tempStr, EpgTriplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s TV" ,tempStr);
			break;
		case CSApp_Applet_FAV_Radio_EPG:
			MV_DB_Get_Favorite_Name(tempStr, EpgTriplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s RADIO" ,tempStr);
			break;
		case CSApp_Applet_SAT_TV_EPG:
			MV_GetSatelliteData_ByIndex(&tempSatData, EpgSatIndex);
			sprintf(title, "%s TV" ,tempSatData.acSatelliteName);
			break;
		case CSApp_Applet_SAT_Radio_EPG:
			MV_GetSatelliteData_ByIndex(&tempSatData, EpgSatIndex);
			sprintf(title, "%s RADIO" ,tempSatData.acSatelliteName);
			break;
		default:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
	}

	tmpRect.left	= ScalerWidthPixel(MV_MENU_BACK_X);
	tmpRect.right	= tmpRect.left + ScalerWidthPixel(MV_MENU_BACK_DX);
	tmpRect.top		= ScalerWidthPixel(MV_MENU_BACK_Y);
	tmpRect.top		= AdjustYPosition(tmpRect.top, MV_INSTALL_MENU_BAR_H, MV_FONT_HEIGHT);
	tmpRect.bottom	= tmpRect.top + ScalerHeigthPixel(MV_FONT_HEIGHT);

	MV_MW_DrawText_Static(hdc, title, -1, &tmpRect, DT_CENTER);
}

static int Epg_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

static void    epg_date_time_callback(tCS_DT_UpdateEvent event, U16 currentMjd, U16 currentUtc)
{
	if (event == eCS_DT_MINUTE)
	{
		#if 0
		printf ("epg_date_time_callback : event[0x%x], MJDTime[%d] = %02d/%02d/%02d, Time[0x%04X] = %02d:%02d\n",
				event, currentMjd, CurrentDate.year, CurrentDate.month, CurrentDate.day,
				currentUtc, CurrentTime.hour, CurrentTime.minute);
		#endif
		CurrentMjd  = currentMjd;
		CurrentDate = CS_DT_MJDtoYMD(currentMjd);
		CurrentTime = CS_DT_UTCtoHM(currentUtc);
		BroadcastMessage (MSG_TIME_UPDATE, 0, 0);
	}
}

void EpgChangeChannel(HWND hwnd, U8 playOn)
{
	printf("%d\n", playOn); //Just to get rid of unused variable warning
	tCS_DB_ServiceManageData	    channelItemData;
	MV_stServiceInfo			    channelServiceData;
	tCS_DBU_Service                 serviceTriplet;
	U8                              playChannel;
	
	CS_DB_GetCurrentList_ServiceData(&channelItemData, EpgCurrentService);
	MV_DB_GetServiceDataByIndex(&channelServiceData, channelItemData.Service_Index);

	//printf("== Current : %d , Unlock : %d , ChItem : %d - %d, ChData : %d - %d\n", EpgCurrentService, CS_APP_GetLastUnlockServiceIndex(), channelItemData.Service_Index, channelItemData.Lock_Flag, channelServiceData.u16ChIndex, channelServiceData.u8Lock);
	if ((CS_MW_GetServicesLockStatus()) && (channelItemData.Lock_Flag == eCS_DB_LOCKED)/*(channelServiceData.u8Lock== eCS_DB_LOCKED)*/
		  && (EpgCurrentService != CS_APP_GetLastUnlockServiceIndex())/* && !playOn*/)
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
		CS_DB_SetCurrentService_OrderIndex(EpgCurrentService);
		CS_DB_GetCurrentListTriplet(&(serviceTriplet.sCS_DBU_ServiceList));
		serviceTriplet.sCS_DBU_ServiceIndex =  EpgCurrentService;
		CS_DBU_SaveCurrentService(serviceTriplet);
		EpgBackTriplet = serviceTriplet;
		FbSendFndDisplayNum((unsigned)EpgCurrentService+1);
	}
}

void MvDrawEpgCurrentTime(HDC hdc)
{
	char    buf[20];
	RECT	etcRect;
	U32     boxW;
	U32     boxH;

	boxW = MV_FONT_WIDTH * 15;
	boxH = MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight + 8;

	etcRect.top    = MV_MENU_BACK_Y;
	etcRect.bottom = etcRect.top + boxH;
	etcRect.left   = MV_MENU_BACK_X + MV_MENU_BACK_DX - boxW;
	etcRect.right  = MV_MENU_BACK_X + MV_MENU_BACK_DX;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(etcRect.left), ScalerHeigthPixel(etcRect.top), ScalerWidthPixel(boxW), ScalerHeigthPixel(boxH), &TitleBmp);
	memset(buf, 0x00, 20);
	sprintf(buf, "%02d/%02d  %02d:%02d", CurrentDate.month, CurrentDate.day, CurrentTime.hour, CurrentTime.minute);
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

CSAPP_Applet_t CSApp_Epg(CSAPP_Applet_t   slist_type)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Epg_Applets = CSApp_Applet_Error;
		
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
	CreateInfo.spCaption 		= "epgcn window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Epg_Msg_cb;
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

	return CSApp_Epg_Applets;
	
}

static int Epg_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
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

						prevServiceIndex = EpgSatIndex;

						if ( u8Result_Value == 0 )
							EpgSatIndex = 255;
						else
							EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - 1);
						
						switch(servicelistType)
						{
							case CSApp_Applet_TV_EPG:
							case CSApp_Applet_FAV_TV_EPG:
							case CSApp_Applet_SAT_TV_EPG:
								if ( EpgSatIndex == 255 )
									servicelistType = CSApp_Applet_TV_EPG;
								else
									servicelistType = CSApp_Applet_FAV_TV_EPG;
								u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
								break;
							case CSApp_Applet_Radio_EPG:
							case CSApp_Applet_FAV_Radio_EPG:
							case CSApp_Applet_SAT_Radio_EPG:
								if ( EpgSatIndex == 255 )
									servicelistType = CSApp_Applet_Radio_EPG;
								else
									servicelistType = CSApp_Applet_FAV_Radio_EPG;
								u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
								break;
							default:
								break;
						}

						servicelistType = MV_Set_EPG_Current_List(servicelistType, EpgSatIndex);
						if ((servicelistType == prevServiceType) && (prevServiceIndex == EpgSatIndex))
						{
							/* Same List */
							break;
						}
						prevServiceType = servicelistType;
						EpgBackTriplet = CS_DB_GetLastServiceTriplet();
						EpgTotalService = CS_DB_GetListServiceNumber(EpgTriplet);
						// printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, EpgSatIndex);
						if(EpgTotalService>0)
						{
							EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						hdc = BeginPaint(hwnd);
						MV_Draw_Epg_Title(hdc, servicelistType);
						MvDrawEpgCurrentTime(hdc);
						EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						break;
					}
							
					case eMV_TITLE_SAT_FAV:
					{
						U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
						U8      satCount = 0;
															
						prevServiceIndex = EpgSatIndex;
						switch(servicelistType)
						{
							case CSApp_Applet_TV_EPG:
							case CSApp_Applet_FAV_TV_EPG:
							case CSApp_Applet_SAT_TV_EPG:
								u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
								satCount = MV_Get_Searched_SatCount();
								// printf("MV_Get_Searched_SatCount [%d]\n", satCount);
								if ( u8Result_Value == 0 )
								{
									servicelistType = CSApp_Applet_TV_EPG;
									EpgSatIndex = 255;
								}
								else if (u8Result_Value <= satCount)
								{
									servicelistType = CSApp_Applet_SAT_TV_EPG;
									EpgSatIndex = MV_Get_Satindex_by_Seq(u8Result_Value);
								}
								else
								{
									servicelistType = CSApp_Applet_FAV_TV_EPG;
									EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - satCount - 1);
								}
								break;
							case CSApp_Applet_Radio_EPG:
							case CSApp_Applet_FAV_Radio_EPG:
							case CSApp_Applet_SAT_Radio_EPG:
								u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
								satCount = MV_Get_Searched_SatCount();
								if ( u8Result_Value == 0 )
								{
									servicelistType = CSApp_Applet_Radio_EPG;
									EpgSatIndex = 255;
								}
								else if (u8Result_Value <= satCount)
								{
									servicelistType = CSApp_Applet_SAT_Radio_EPG;
									EpgSatIndex = MV_Get_Satindex_by_Seq(u8Result_Value);
								}
								else
								{
									servicelistType = CSApp_Applet_FAV_Radio_EPG;
									EpgSatIndex = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - satCount - 1);
								}
								break;
								break;
							default:
								break;
						}

						servicelistType = MV_Set_EPG_Current_List(servicelistType, EpgSatIndex);
						if ((servicelistType == CSApp_Applet_TV_EPG) || (servicelistType == CSApp_Applet_Radio_EPG))
						{
							if (servicelistType == prevServiceType)
							{
								break;
							}
						}
						else if ((servicelistType == prevServiceType) && (prevServiceIndex == EpgSatIndex))
						{
							/* Same List */
							break;
						}
						prevServiceType = servicelistType;
						EpgBackTriplet = CS_DB_GetLastServiceTriplet();
						EpgTotalService = CS_DB_GetListServiceNumber(EpgTriplet);
						// printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, EpgSatIndex);
						if(EpgTotalService>0)
						{
							EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						hdc = BeginPaint(hwnd);
						MV_Draw_Epg_Title(hdc, servicelistType);
						MvDrawEpgCurrentTime(hdc);
						EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
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
				CSApp_Epg_Applets = CSApp_Applet_Sleep;
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
			epgChBoxH = MV_INSTALL_MENU_BAR_H * 11;
			epgLBoxX = MV_MENU_BACK_X + MV_MENU_BACK_DX - MV_EPG_CN_PIG_DX - 10;
			epgBBoxY = epgChBoxY + epgChBoxH + 48;
			epgBBoxH = CSAPP_OSD_MAX_HEIGHT - epgBBoxY - 106;

			// EpgFocusCn    = 0;
			EpgCurrentCol = 0;
			b8Check_Exit = FALSE;
			
			CS_DT_Register_DateTime_Notify(&date_time_client_id, eCS_DT_MINUTE, epg_date_time_callback);
			/* By KB Kim 2011.05.28 */
			EpgUpdated = 0;
			SetTimer(hwnd, MSG_EPG_SEC_UPDATE, EPG_INFO_UPDATE_TIMER);
			
			servicelistType = GetWindowAdditionalData(hwnd);
			//EpgSatIndex = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
			CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));					
			EpgSatIndex = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
			
			servicelistType = MV_Set_EPG_Current_List(servicelistType, EpgSatIndex);
			EpgBackTriplet = CS_DB_GetLastServiceTriplet();
			
			EpgTotalService = CS_DB_GetListServiceNumber(EpgTriplet);
			//printf("servicelistType [0x%X / 0x%X] , SatIndex [%d]\n", servicelistType, CSApp_Applet_TV_EPG, EpgSatIndex);
			if(EpgTotalService>0)
			{
				EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();  
				EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
				EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
				// prevService = EpgCurrentService;
			}
			CS_DT_GetUTCOffset(&EpgUtcOffset, &EpgUtcOffsetPolarity);
#if 0
			CurrentMjd  = CS_DT_GetLocalMJD();
			currentUtc  = CS_DT_GetLocalUTC();
			CurrentDate = CS_DT_MJDtoYMD(CurrentMjd);
			CurrentTime = CS_DT_UTCtoHM(currentUtc);
#else
			MV_OS_Get_Time_to_MJD_UTC_Date_Time(&CurrentMjd, &currentUtc, &CurrentDate, &CurrentTime);
#endif
			prevMjd     = CurrentMjd;
			channelChangeCounter = 0;
			channelChangeTime    = 0;
			channelChanged       = 0;
			channelChangeRequest = 0;
			currentMilliTime     = 0;
			break;

		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_EPG, EN_EPG_TYPE_SIMPLE);
			tmpX = MV_FONT_WIDTH * 15;
			tmpY = MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight + 8;
			etcRect.top    = MV_MENU_BACK_Y;
			etcRect.bottom = etcRect.top + tmpY;
			etcRect.left   = MV_MENU_BACK_X + MV_MENU_BACK_DX - tmpX;
			etcRect.right  = MV_MENU_BACK_X + MV_MENU_BACK_DX;

			CS_MW_SetSmallWindow((epgLBoxX - 6),(epgChBoxY - 6),(MV_EPG_CN_PIG_DX + 12),(MV_EPG_CN_PIG_DY + 12));
			
			hdc=BeginPaint(hwnd);
			MV_Draw_Epg_Title(hdc, servicelistType);
			MV_GetBitmapFromDC(hdc, ScalerWidthPixel(etcRect.left), ScalerHeigthPixel(etcRect.top), ScalerWidthPixel(tmpX), ScalerHeigthPixel(tmpY), &TitleBmp);
			MvDrawEpgCurrentTime(hdc);
			
			EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_INIT);
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(epgBBoxY), ScalerWidthPixel(MV_EPG_CN_BAR_DX + 20), ScalerHeigthPixel(epgBBoxH) );
			etcRect.top    = epgBBoxY - 30;
			etcRect.bottom = etcRect.top + 20;
			etcRect.left   = MV_MENU_BACK_X;
			etcRect.right  = MV_MENU_BACK_X + MV_EPG_CN_BAR_DX + 20;
			EpgDrawProgress(hdc, etcRect, CurrentTime, CurrentTime, CurrentTime, EPG_DRAW_INIT);

			tmpY = CSAPP_OSD_MAX_HEIGHT - MV_BMP[MVBMP_RED_BUTTON].bmHeight - 66;
			// FillBoxWithBitmap(hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(tmpY), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			// CS_MW_TextOut(hdc,ScalerWidthPixel(MV_MENU_BACK_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(tmpY), CS_MW_LoadStringByIdx(CSAPP_STR_EPG_SCHEDULED));
			tmpX = MV_MENU_BACK_X + (MV_EPG_CN_BAR_DX / 2) + 10;
			// FillBoxWithBitmap(hdc,ScalerWidthPixel(tmpX), ScalerHeigthPixel(tmpY), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			// CS_MW_TextOut(hdc,ScalerWidthPixel(tmpX + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2), ScalerHeigthPixel(tmpY), CS_MW_LoadStringByIdx(CSAPP_STR_TIMER));

			EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_INIT);

#ifdef DAILY_EPG
			etcRect.top    = MV_MENU_BACK_Y + (MV_INSTALL_MENU_BAR_H * 11) + 182;
			etcRect.bottom = etcRect.top + 30;
			etcRect.left   = MV_MENU_BACK_X + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10;
			etcRect.right  = MV_MENU_BACK_X + 200;
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(etcRect.top), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);

			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_EPG_SCHEDULED), -1, &etcRect, DT_LEFT);
			
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			etcRect.top = etcRect.top - 2;
			etcRect.left = etcRect.left - 2;
			MV_MW_DrawText_Static(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_EPG_SCHEDULED), -1, &etcRect, DT_LEFT);
#endif

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
			/* By KB Kim 2011.05.28 */
			else if (wparam == MSG_EPG_SEC_UPDATE && (MV_Get_Password_Flag() == FALSE))
			{
				if ((EpgUpdated) && (!popupModeOn))
				{
					EpgUpdated = 0;
					hdc=BeginPaint(hwnd);
					EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_UPDATE);
					EndPaint(hwnd,hdc);
				}
			}
			break;

		case MSG_NOW_NEXT_UPDATE : /* By KB Kim 2011.05.28 */
			if (MV_Get_Password_Flag() == FALSE)
				EpgUpdated = 1;
			break;

		case MSG_EPG_DRAW_DESC :
			break;
                        
		case MSG_TIME_UPDATE :
			etcRect.left   = MV_MENU_BACK_X;
			etcRect.right  = MV_MENU_BACK_X + MV_EPG_CN_BAR_DX + 20;
			etcRect.top    = MV_MENU_BACK_Y + (MV_INSTALL_MENU_BAR_H * 11) + 72;
			etcRect.bottom = etcRect.top + 20;
			currentTime = CurrentTime;
			startTime   = EventStartTime;
			if ((EventDuration.hour > 0) || (EventDuration.minute > 0))
			{
				// printf("=======  MSG_TIME_UPDATE : EventStartMjd[%d] CurrentMjd[%d] \n", EventStartMjd, CurrentMjd);
				if (EventStartMjd < CurrentMjd)
				{
					currentTime.hour += 24;
				}
				else if (EventStartMjd > CurrentMjd)
				{
					startTime.hour += 24;
				}
			}
		
			hdc=BeginPaint(hwnd);
			MvDrawEpgCurrentTime(hdc);
			if (!popupModeOn)
			{
				if (prevMjd == CurrentMjd)
				{
					EpgDrawProgress(hdc, etcRect, startTime, EventDuration, currentTime, EPG_DRAW_UPDATE);
				}
				else
				{
					prevMjd = CurrentMjd;
					EpgDrawProgress(hdc, etcRect, startTime, EventDuration, currentTime, EPG_DRAW_PAGE);
				}
			}
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

				EpgChangeChannel(hwnd, 0);
				channelChangeRequest = 0;
				channelChanged       = 1;
				channelChangeTime    = OsTimeNowMilli();
			}
			break;

		case MSG_PLAYSERVICE:
#if 0
			CS_DB_GetCurrentList_ServiceData(&channelItemData, EpgCurrentService);
			CS_MW_PlayServiceByIdx(channelItemData.Service_Index, NOT_TUNNING);
			CS_DB_SetCurrentService_OrderIndex(EpgCurrentService);
			CS_DB_GetCurrentListTriplet(&(serviceTriplet.sCS_DBU_ServiceList));
			serviceTriplet.sCS_DBU_ServiceIndex =  EpgCurrentService;
			CS_DBU_SaveCurrentService(serviceTriplet);
			EpgBackTriplet = serviceTriplet;
			if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
			{
				FbSendFndDisplayNum((unsigned)EpgCurrentService+1);
			}
			else
			{
				FbSendFndDisplayNum((unsigned)channelItemData.LCN);
			}
#else
			EpgChangeChannel(hwnd, 1);
#endif
			break;

		case MSG_CLOSE:
			if(IsTimerInstalled(hwnd, MSG_EPG_SEC_UPDATE))
			{
				KillTimer (hwnd, MSG_EPG_SEC_UPDATE);
			}
			
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
			CS_DT_Unregister_DateTime_Notify(date_time_client_id);

			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
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

								CS_APP_SetLastUnlockServiceIndex(EpgCurrentService);
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

								CS_APP_SetLastUnlockServiceIndex(EpgCurrentService);
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
				
				switch(wparam)
				{
					case CSAPP_KEY_DOWN :
						if(EpgTotalService < 1)
							break;
						hdc = BeginPaint(hwnd);
						MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, UNFOCUS, EN_EPG_TYPE_SIMPLE, 1);
						EpgPrevPageStart = EpgCurrentPageStartIndex;

#ifdef EPG_UP_DOWN_SCROLL
						if(EpgCurrentService == EpgTotalService-1)
						{
							EpgCurrentService = 0;
							EpgCurrentFocus = 0;
							EpgCurrentPageStartIndex = 0;
							if  (EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
							{
								EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
								EndPaint(hwnd,hdc);
								channelChangeOn = 1;
								break;
							}
						}
						else
						{
							EpgCurrentService++;
							EpgCurrentFocus++;
							if (EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
							{
								if (EpgCurrentFocus >= SERVICES_NUM_PER_PAGE_SIMPLE)
								{
									EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_SCROLL_UP);
									EpgCurrentFocus = SERVICES_NUM_PER_PAGE_SIMPLE - 1;
									EpgCurrentPageStartIndex++;
								}
							}
						}

						MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE, 1);
#else  // #ifdef EPG_UP_DOWN_SCROLL
						if(EpgCurrentService == EpgTotalService-1)
						{
							EpgCurrentService = 0;
						}
						else
						{
							EpgCurrentService++;
						}
						EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						if (EpgPrevPageStart != EpgCurrentPageStartIndex)
						{
							EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						}
						else
						{
							MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE, 1);
						}
#endif // #ifdef EPG_UP_DOWN_SCROLL
						EndPaint(hwnd,hdc);
						// SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						CS_APP_SetLastUnlockServiceIndex(0xffff);
						channelChangeOn = 1;
						break;
					case CSAPP_KEY_UP:
						if(EpgTotalService < 1)
							break;
						hdc = BeginPaint(hwnd);
						MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, UNFOCUS, EN_EPG_TYPE_SIMPLE, 1);
						EpgPrevPageStart = EpgCurrentPageStartIndex;

#ifdef EPG_UP_DOWN_SCROLL
						if(EpgCurrentService == 0)
						{
							EpgCurrentService = EpgTotalService-1;
							
							if  (EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
							{
								EpgCurrentPageStartIndex = EpgTotalService - 10;
								EpgCurrentFocus = SERVICES_NUM_PER_PAGE_SIMPLE - 1;
								EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
								EndPaint(hwnd,hdc);
								channelChangeOn = 1;
								break;
							}
							else
							{
								EpgCurrentFocus = EpgCurrentService;
								EpgCurrentPageStartIndex = 0;
							}
						}
						else
						{
							EpgCurrentService--;
							if (EpgTotalService > SERVICES_NUM_PER_PAGE_SIMPLE)
							{
								if (EpgCurrentFocus == 0)
								{
									EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_SCROLL_DOWN);
									EpgCurrentFocus = 0;
									EpgCurrentPageStartIndex--;
								}
								else
								{
									EpgCurrentFocus--;
								}
							}
							else
							{
								EpgCurrentFocus--;
							}
						}

						MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE, 1);
#else  // #ifdef EPG_UP_DOWN_SCROLL
						if(EpgCurrentService == 0)
						{
							EpgCurrentService = EpgTotalService-1;
						}
						else
						{
							EpgCurrentService--;
						}
						EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
						if (EpgPrevPageStart != EpgCurrentPageStartIndex)
						{
							EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						}
						else
						{
							MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE, 1);
						}
#endif // #ifdef EPG_UP_DOWN_SCROLL
						EndPaint(hwnd,hdc);
						//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						CS_APP_SetLastUnlockServiceIndex(0xffff);
						channelChangeOn = 1;
						break;

					case CSAPP_KEY_PG_UP:
						if(EpgTotalService <= SERVICES_NUM_PER_PAGE_SIMPLE)
							break;

#ifdef EPG_UP_DOWN_SCROLL
						if (EpgCurrentPageStartIndex == 0)
						{
							EpgCurrentPageStartIndex = EpgTotalService - 10;
						}
						else if (EpgCurrentPageStartIndex < SERVICES_NUM_PER_PAGE_SIMPLE)
						{
							EpgCurrentPageStartIndex = 0;
						}
						else
						{
							EpgCurrentPageStartIndex -= SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						EpgCurrentService = EpgCurrentPageStartIndex + EpgCurrentFocus;
#else  // #ifdef EPG_UP_DOWN_SCROLL
						if (EpgCurrentService <= SERVICES_NUM_PER_PAGE_SIMPLE)
						{
							U16 tmpFocus;

							EpgCurrentService = EpgTotalService - 1;
							tmpFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							if (tmpFocus < EpgCurrentFocus)
							{
								EpgCurrentFocus = tmpFocus;
							}
							EpgCurrentService = EpgCurrentPageStartIndex + EpgCurrentFocus;
						}
						else
						{
							EpgCurrentService -= SERVICES_NUM_PER_PAGE_SIMPLE;
						}
						EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
#endif // #ifdef EPG_UP_DOWN_SCROLL
						hdc = BeginPaint(hwnd);
						EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						channelChangeOn = 1;
						break;

					case CSAPP_KEY_PG_DOWN:
						if(EpgTotalService <= SERVICES_NUM_PER_PAGE_SIMPLE)
							break;
#ifdef EPG_UP_DOWN_SCROLL
						EpgCurrentPageStartIndex += SERVICES_NUM_PER_PAGE_SIMPLE;
						if (EpgCurrentPageStartIndex >= EpgTotalService)
						{
							EpgCurrentPageStartIndex = 0;
						}

						EpgCurrentService = EpgCurrentPageStartIndex + EpgCurrentFocus;
						if (EpgCurrentService >= EpgTotalService)
						{
							EpgCurrentService = EpgTotalService - 1;
							EpgCurrentFocus   = EpgCurrentService - EpgCurrentPageStartIndex;
						}
#else  // #ifdef EPG_UP_DOWN_SCROLL
						EpgCurrentService += SERVICES_NUM_PER_PAGE_SIMPLE;
						if (EpgCurrentService >= EpgTotalService)
						{
							EpgCurrentService = EpgCurrentFocus;
						}
						EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
						EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
#endif // #ifdef EPG_UP_DOWN_SCROLL
						hdc = BeginPaint(hwnd);
						EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						// SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						channelChangeOn = 1;
						break;

					case CSAPP_KEY_RIGHT:
					case CSAPP_KEY_LEFT:
						EpgCurrentCol = 1 - EpgCurrentCol;
						hdc=BeginPaint(hwnd);
						MV_Draw_Epg_Item(hdc, EpgCurrentFocus, EpgCurrentService, FOCUS, EN_EPG_TYPE_SIMPLE, 0);
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
								case CSApp_Applet_FAV_Radio_EPG:
								case CSApp_Applet_SAT_Radio_EPG:
								case CSApp_Applet_Radio_EPG:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case CSApp_Applet_TV_EPG:
								case CSApp_Applet_FAV_TV_EPG:
								case CSApp_Applet_SAT_TV_EPG:
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
								case CSApp_Applet_FAV_Radio_EPG:
								case CSApp_Applet_SAT_Radio_EPG:
								case CSApp_Applet_Radio_EPG:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case CSApp_Applet_TV_EPG:
								case CSApp_Applet_FAV_TV_EPG:
								case CSApp_Applet_SAT_TV_EPG:
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
							case CSApp_Applet_TV_EPG:
								servicelistType = CSApp_Applet_Radio_EPG;
								break;
							case CSApp_Applet_Radio_EPG:
								servicelistType = CSApp_Applet_TV_EPG;
								break;
							case CSApp_Applet_FAV_TV_EPG:
								servicelistType = CSApp_Applet_FAV_Radio_EPG;
								break;
							case CSApp_Applet_FAV_Radio_EPG:
								servicelistType = CSApp_Applet_FAV_TV_EPG;
								break;
							case CSApp_Applet_SAT_TV_EPG:
								servicelistType = CSApp_Applet_SAT_Radio_EPG;
								break;
							case CSApp_Applet_SAT_Radio_EPG:
								servicelistType = CSApp_Applet_SAT_TV_EPG;
								break;
							default:
								break;
						}

						servicelistType = MV_Set_EPG_Current_List(servicelistType, EpgSatIndex);
						if (servicelistType == prevServiceType)
						{
							/* No Changes */
							break;
						}
						prevServiceType = servicelistType;

						if(EpgTotalService>0)
						{
							EpgCurrentService = CS_DB_GetCurrentService_OrderIndex();
							EpgCurrentFocus = get_focus_line(&EpgCurrentPage, EpgCurrentService, SERVICES_NUM_PER_PAGE_SIMPLE);
							EpgCurrentPageStartIndex = EpgCurrentPage * SERVICES_NUM_PER_PAGE_SIMPLE;
							channelChangeOn = 1;
						}
						
						hdc = BeginPaint(hwnd);
						MV_Draw_Epg_Title(hdc, servicelistType);
						MvDrawEpgCurrentTime(hdc);
						EpgDrawCnBox(hdc, SERVICES_NUM_PER_PAGE_SIMPLE, EPG_DRAW_PAGE);
						EndPaint(hwnd,hdc);
						
						break;
						
					case CSAPP_KEY_CH_UP:
					case CSAPP_KEY_REW:
						hdc=BeginPaint(hwnd);
						EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_SCROLL_DOWN);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_CH_DOWN:
					case CSAPP_KEY_FF:
						hdc=BeginPaint(hwnd);
						EpgDrawDetailedBox(hdc, EN_EPG_TYPE_SIMPLE, EPG_DRAW_SCROLL_UP);
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_GREEN:
#ifdef DAILY_EPG
						switch( servicelistType )
						{
							case CSApp_Applet_TV_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_TV_EPG;
								break;
							case CSApp_Applet_Radio_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_Radio_EPG;
								break;
							case CSApp_Applet_FAV_TV_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_FAV_TV_EPG;
								break;
							case CSApp_Applet_FAV_Radio_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_FAV_Radio_EPG;
								break;
							case CSApp_Applet_SAT_TV_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_SAT_TV_EPG;
								break;
							case CSApp_Applet_SAT_Radio_EPG:
								CSApp_Epg_Applets = CSApp_Applet_Daily_SAT_Radio_EPG;
								break;
							default:
								break;
						}
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
#endif
						break;

					case CSAPP_KEY_BLUE:
						break;

					case CSAPP_KEY_RED:

					case CSAPP_KEY_ENTER:
						break;
						
					case CSAPP_KEY_IDLE:
						CSApp_Epg_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_EPG:
						CSApp_Epg_Applets = CSApp_Applet_Desktop;
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



