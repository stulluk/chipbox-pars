
#include "linuxos.h"

#include "userdefine.h"
#include "database.h"
#include "mwpublic.h"

#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "sys_setup.h"
#include "ctype.h"
#include "e2p.h"
#include "date_time.h"

#include "mvosapi.h" // by kb : 2010.04.06

/* For EPG Display Problem By KB Kim : 2010.08.17 */
#define ASCII_NULL         '\0'
#define ASCII_SP           32
#define ASCII_STAR         42
#define ASCII_PLUS         43
#define ASCII_COMMA        44
#define ASCII_MINUS        45
#define ASCII_DOT          46
#define ASCII_SLASH        47
#define ASCII_COLONE       58
#define ASCII_SEMI_COLONE  59
#define ASCII_I_SLASH      92
#define ASCII_UNDERLINE    95
#define ASCII_DEL          127
#define ASCII_MAX          128

#define ASCII_L_A          65
#define ASCII_L_F          70
#define ASCII_L_X          88
#define ASCII_L_Z          90
#define ASCII_S_A          97
#define ASCII_S_F          102
#define ASCII_S_X          120
#define ASCII_S_Z          122

/* For New Rating function By KB KIm 2011.08.01 */
#include "dvbtuner.h"
#define RATING_CONFIG  "/usr/work1/rating.txt"
#define RATING_CONTROL "/usr/work0/app/rating_enable.txt"
#define MAX_SITE_LENGTH  512
extern U32			*Tuner_HandleId; // For New Rating function By KB KIm 2011.08.01

U32							ScreenWidth = CSAPP_OSD_MAX_WIDTH;
U32							ScreenHeigth = CSAPP_OSD_MAX_HEIGHT;
CSAPP_Applet_t				CSApp_Applets_Status;
CSAPP_Applet_t				CSApp_Applets_Prev_Status;

static BOOL					Warning_Report_Window_Status = FALSE;
#ifdef CHECK_CH_WATCH
static U32					NowChannel_Time_Count = 0;
static stSend_Channel_Data	stSend_Data;
#endif // #ifdef CHECK_CH_WATCH

U16			MAIN_MENU_STR[MAIN_ITEM_MAX][MAIN_SUB_ITEM_MAX] = {
								{	CSAPP_STR_INSTALL_SEARCH,
									CSAPP_STR_SATELLITE_SETTING,
									CSAPP_STR_TP_SETTING,
									CSAPP_STR_DISECQ_SETTING,
									CSAPP_STR_USALS_SETTING,
									CSAPP_STR_UNICABLE_SETTING,
									0},

								{	CSAPP_STR_MENU_LANG,
									CSAPP_STR_TIME_SETTING,
									CSAPP_STR_SYSTEM_SETTING,
									CSAPP_STR_AV_SETTING,
									CSAPP_STR_PIN_SETTING,
									CSAPP_STR_NET_SETTING,
									CSAPP_STR_SYSTEM_INFO},

								{	CSAPP_STR_REC_FILE,
#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
									CSAPP_STR_REC_CONFIG,
#endif
									CSAPP_STR_FILE_TOOL,
									CSAPP_STR_USB_REMOVE,
									CSAPP_STR_STORAGE_INFO,
									CSAPP_STR_MEDIA_PLAY,
									0},

								{	CSAPP_STR_UPGRADE,
									CSAPP_STR_CI,
									CSAPP_STR_CAS,
									CSAPP_STR_BACKUP,
									CSAPP_STR_BACKUP_RE,
									CSAPP_STR_PLUG_IN,
									CSAPP_STR_DEFAULT_FACTORY}
							};

char	IP_Check_String[4][16] = {
									"address",
									"netmask",
									"gateway",
									"nameserver"
							};
// kb : Mar 25
U16 EPG_MenuString[EN_EPG_TYPE_MAX] =
{
	CSAPP_STR_EPG_SIMPLE,
	CSAPP_STR_EPG_SCHEDULED
};

void Mv_CopyLine(char *dst, char *src, U32 length)
{
	memcpy(dst, src, length);
	dst += length;
	*dst++ = ASCII_NULL;
	*dst   = ASCII_NULL;
}

/* For EPG Display Problem By KB Kim : 2011.08.17 */
BOOL MvCheckSyllable(char data)
{
	if (data == 0x27)
	{
		return FALSE;
	}

	if (((data >= ASCII_SP) && (data < ASCII_0)) ||
		((data > ASCII_9) && (data < ASCII_L_A)) ||
		((data > ASCII_L_Z) && (data < ASCII_S_A)) ||
		((data > ASCII_S_Z) && (data < ASCII_DEL)) ||
		(data == ASCII_NULL) ||
		(data == ASCII_LF) ||
		(data == ASCII_CR))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL MvCompareBlankChar(char data)
{
	if ((data == ASCII_NULL) ||	(data == ASCII_LF) || (data == ASCII_CR) || (data == ASCII_SP))
	{
		return TRUE;
	}

	return FALSE;
}

/* For EPG Display Problem By KB Kim : 2011.08.17 */
BOOL MvCheckLfChar(char data)
{
	if ((data == ASCII_NULL) ||	(data == ASCII_LF) || (data == ASCII_CR))
	{
		return TRUE;
	}

	return FALSE;
}

/*
 *  Name : MvConvertTextforLine
 *  Description
 *     Convert One line text string for window display.
 *  INPUT Arg
 *     char *src           : Data source pointer
 *     U32   charPerLine    : Max number of characters in one line
 *     U32   dataLength    : Total length of Data
 *  OUTPUT Arg
 *     NONE
 *  RETURN : U32
 *     Total number of characters in this line
 */
U32 MvConvertTextforLine(char *src, U32 charPerLine, U32 dataLength)
{
	U32 numberOfChar;
	U32 pointer;      /* For EPG Display Problem By KB Kim : 2011.08.17 */

	numberOfChar = charPerLine - 1;
	if (dataLength < numberOfChar)
	{
		numberOfChar = dataLength;
	}

	/* For EPG Display Problem By KB Kim : 2011.08.17 */
	pointer = 0;
	while(pointer < numberOfChar)
	{
		if (MvCheckLfChar(src[pointer]))
		{
			numberOfChar = pointer;

			return numberOfChar;
		}

		pointer++;
	}

	pointer = numberOfChar - 1;

	/* Find end of word */
	while (!MvCheckSyllable(src[pointer]) && (pointer > 0))
	{
		pointer--;
	}

	/* Remove Blank char from end of line */
	while (MvCompareBlankChar(src[pointer]) && (pointer > 0))
	{
		pointer--;
	}

	numberOfChar = pointer + 1; /* Length need to Plus 1 to position */

	return numberOfChar;
}

/*
 *  Name : MvConvertTextforWindow
 *  Description
 *     Convert text string for window display.
 *  INPUT Arg
 *     char *src         : Data source pointer)
 *     U32   charPerLine : Unmber of chatacter in one line
 *     U32   dataLength  : Total Length of source data
 *  OUTPUT Arg
 *     char **dest       : converted text return pointer
 *  RETURN : U32
 *     Total number of Lines in converted text
 */
U32 MvConvertTextforWindow(char *src, char **dest, U32 charPerLine, U32 dataLength)
{
	U32 numberOfData;
	U32 maxNumberOfLine;
	U32 numberOfLine;
	char *buf;

	if (*dest != NULL)
	{
		/* Allocated data left : Free data */
		OsMemoryFree(*dest);
	}

	/* Allocate 2 * length + 10 : for the safety */
	maxNumberOfLine  = (dataLength / (charPerLine - 2)) + 6;
	maxNumberOfLine += maxNumberOfLine;

	buf = OsMemoryAllocate(charPerLine * maxNumberOfLine);
	if (buf == NULL)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		OsDebugPrtf("MvConvertTextforWindow Error : Can not allocate destination buffer!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		return 0;
	}

	memset(buf, 0x00, (charPerLine * maxNumberOfLine));

	// destDataLength = 0;
	numberOfLine   = 0;
	*dest = buf;

	/* Remove first Blank Charactors */
	while (MvCompareBlankChar(src[0]) && (dataLength > 0))
	{
		src++;
		dataLength--;
	}

	while ((dataLength > 0) && (numberOfLine < maxNumberOfLine))
	{
		numberOfData = MvConvertTextforLine(src, charPerLine, dataLength);
		Mv_CopyLine(buf, src, numberOfData);
		numberOfLine++;
		// destDataLength += charPerLine;
		buf            += charPerLine;
		src            += numberOfData;
		dataLength     -= numberOfData;
		while (MvCompareBlankChar(src[0]) && (dataLength > 0))
		{
			src++;
			dataLength--;
		}
	}

	return numberOfLine;
}

/**************/

inline U32	ScalerWidthPixel(U32	Pixel)
{
	return ((Pixel*ScreenWidth)/CSAPP_OSD_MAX_WIDTH);
}

inline U32	ScalerHeigthPixel(U32 Pixel)
{
	return ((Pixel*ScreenHeigth)/CSAPP_OSD_MAX_HEIGHT);
}

CSAPP_Applet_t get_windown_status(void)
{
	return CSApp_Applets_Status;
}

CSAPP_Applet_t get_prev_windown_status(void)
{
	return CSApp_Applets_Prev_Status;
}

void set_prev_windown_status(CSAPP_Applet_t win_status)
{
	CSApp_Applets_Prev_Status = win_status;
}

void set_windown_status(CSAPP_Applet_t win_status)
{
	CSApp_Applets_Prev_Status = CSApp_Applets_Status;
	CSApp_Applets_Status = win_status;
}

U16 get_page_count(U16 total, U8 nbItemPerPage)
{
	U16	page_count;

	page_count = total / nbItemPerPage;
	page_count += (total % nbItemPerPage) ? 1 : 0;

	return page_count;
}

U16 get_focus_line(U16 *page, U16 current, U8 nbItemPerPage)
{
	U16	focus;
	U16	page_count;

	focus = current%nbItemPerPage;

	page_count = current / nbItemPerPage;

	*page = page_count;

	return(focus);
}

static tComboList	Combo_List;
static RECT 		update_prc = {0, 0, 0, 0};

void ComboList_Create( const tComboList_Element * first, U8 element_num )
{
	Combo_List.element_num = element_num;
	Combo_List.first_element = first;
}

void ComboList_UpdateAll( HWND hWnd )
{
	update_prc.left = ScalerWidthPixel(Combo_List.first_element->element.x);
	update_prc.top = ScalerHeigthPixel(Combo_List.first_element->element.y);
	update_prc.right = ScalerWidthPixel(Combo_List.first_element->element.x + Combo_List.first_element->element.dx);
	update_prc.bottom = ScalerHeigthPixel(Combo_List.first_element->element.y + Combo_List.first_element->element.dy *Combo_List.element_num);
	InvalidateRect(hWnd, &update_prc, FALSE);
}

void ComboList_Update_Element( HWND hWnd, U8 element )
{
	if(element < Combo_List.element_num)
	{
		update_prc.left = ScalerWidthPixel(Combo_List.first_element->element.x);
		update_prc.top =ScalerHeigthPixel(Combo_List.first_element->element.y + Combo_List.first_element->element.dy * element);
		update_prc.right = ScalerWidthPixel(Combo_List.first_element->element.x + Combo_List.first_element->element.dx);
		update_prc.bottom = ScalerHeigthPixel(Combo_List.first_element->element.y + Combo_List.first_element->element.dy * (element+1));
		InvalidateRect(hWnd, &update_prc, FALSE);
	}
}

void ComboList_Update_Field( HWND hWnd, U8 element, U8 field )
{
	if((element < Combo_List.element_num)&&(field < Combo_List.first_element->field_num))
	{
		update_prc.left = ScalerWidthPixel(Combo_List.first_element->element_fields[field].x);
		update_prc.top = ScalerHeigthPixel(Combo_List.first_element->element.y + Combo_List.first_element->element.dy * element);
		update_prc.right = ScalerWidthPixel(Combo_List.first_element->element_fields[field].x + Combo_List.first_element->element_fields[field].dx);
		update_prc.bottom = ScalerHeigthPixel(Combo_List.first_element->element.y + Combo_List.first_element->element.dy * (element+1));
		InvalidateRect(hWnd, &update_prc, FALSE);
	}
}

void ComboList_Destroy( void )
{
	memset(&Combo_List, 0, sizeof(tComboList));
}

static BOOL	Pin_Dialog_Status = FALSE;

static BOOL	Pin_Input_Status = FALSE;

BOOL PinDlg_GetStatus(void)
{
	//APP_PRINT("Pin_Dialog_Status = %d\n", Pin_Dialog_Status);
	return(Pin_Dialog_Status);
}

void PinDlg_SetStatus(BOOL status)
{
	Pin_Dialog_Status = status;
}

char Key_to_Ascii(U32 key)
{
	char    ascii_value;
	switch(key)
	{
		case CSAPP_KEY_0:
			ascii_value = '0';
			break;
		case CSAPP_KEY_1:
			ascii_value = '1';
			break;
		case CSAPP_KEY_2:
			ascii_value = '2';
			break;
		case CSAPP_KEY_3:
			ascii_value = '3';
			break;
		case CSAPP_KEY_4:
			ascii_value = '4';
			break;
		case CSAPP_KEY_5:
			ascii_value = '5';
			break;
		case CSAPP_KEY_6:
			ascii_value = '6';
			break;
		case CSAPP_KEY_7:
			ascii_value = '7';
			break;
		case CSAPP_KEY_8:
			ascii_value = '8';
			break;
		case CSAPP_KEY_9:
			ascii_value = '9';
			break;
		default:
			ascii_value = 0;
			break;
	}

	return(ascii_value);
}


void PinDlg_TreatKey(HWND hWnd, U32 key, int *input_count, char * input_keys)
{
	switch(key)
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
			input_keys[*input_count] = Key_to_Ascii(key);
			++(*input_count);
			SendMessage(hWnd, MSG_PIN_INPUT, 0, 0);
			break;

		case CSAPP_KEY_LEFT:
			if((*input_count)>0)
			{
			--(*input_count);
			}
			SendMessage(hWnd, MSG_PIN_INPUT, 0, 0);
			break;

		//case CSAPP_KEY_DOWN:
		//case CSAPP_KEY_UP:
		case CSAPP_KEY_MENU:
		case CSAPP_KEY_ESC:
			{

			SendMessage(hWnd, MSG_PAINT, param_paint_all, 0);
			//ComboList_UpdateAll(hWnd);
			Pin_Dialog_Status = FALSE;
			}
			break;
		default:
			break;
	}
}

BOOL    Pin_Verify(char * inputs)
{
	if(inputs == NULL)
		return FALSE;

	if((!memcmp(CS_DBU_GetPinCode(), inputs, 4)) ||(!memcmp(MASTER_PIN, inputs, 4)))
		return TRUE;
	else
		return FALSE;
}

BOOL    Pin_CheckValid(char * inputs)
{
	if(inputs == NULL)
		return FALSE;

	if(((inputs[0]>='0')&&(inputs[0]<='9'))
		&&((inputs[1]>='0')&&(inputs[1]<='9'))
		&&((inputs[2]>='0')&&(inputs[2]<='9'))
		&&((inputs[3]>='0')&&(inputs[3]<='9')))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void Pin_Set_Input_Status(BOOL status)
{
	Pin_Input_Status = status;
}

BOOL Pin_Get_Input_Status(void)
{
	return(Pin_Input_Status);
}
#if 0
static tCS_MW_VIDEO_FORMAT current_format;
void VideoFormatNotify(eCS_MW_PIPE_TYPE PipeType,tCS_MW_MSG_INFO MsgInfo)
{


	if(PipeType==CS_MW_PIPE_VIDEO_FORMAT)
	{
		memcpy(&current_format, MsgInfo.WParam, sizeof(tCS_MW_VIDEO_FORMAT));
		//CS_MW_GetVideoSize(&VideoFmt);
		ScreenWidth=current_format.Width;
		ScreenHeigth=current_format.Height;
		BroadcastMessage ( MSG_VIDEO_FORFMAT_UPDATE, &current_format, 0);
		//pVideoFormat=(tCS_MW_VIDEO_FORMAT*)MsgInfo.WParam;
		//printf("get[%d,%d]\n", current_format.Width,current_format.Height);

	}

	return;
}
#endif
static BITMAP       Bmp_MainMenu;
static BITMAP       Bmp_EPGMainMenu;
static BITMAP       Bmp_TVListMainMenu;
static BITMAP       Bmp_Banner;

BITMAP *GetBitMapData(eBitMapIndex idx)
{
	switch(idx)
	{
		case CSBMP_MAIN_MENU:
			return  &Bmp_MainMenu;
			break;
		case CSBMP_EPG_MENU:
			return &Bmp_EPGMainMenu;
			break;
		case CSBMP_TVLIST_MENU:
		case CSBMP_RDLIST_MENU:
			return &Bmp_TVListMainMenu;
			break;
		case CSBMP_BANNER:
			return &Bmp_Banner;
			break;
		default:
			return NULL;
	}

	return NULL;
}

BOOL    In_Main_Menu_Status = FALSE;

void APP_SetMainMenuStatus(BOOL status)
{
    In_Main_Menu_Status = status;
}

BOOL APP_GetMainMenuStatus(void)
{
	return(In_Main_Menu_Status);
}

U16 g_last_unlock_service = 0xffff;

U16 CS_APP_GetLastUnlockServiceIndex(void)
{
    return g_last_unlock_service;
}

void CS_APP_SetLastUnlockServiceIndex(U16 last_service)
{
    g_last_unlock_service = last_service;
}

BOOL g_first_in_desktop = TRUE;

BOOL CS_APP_GetFirstInDesktop(void)
{
    return g_first_in_desktop;
}

void CS_APP_SetFirstInDesktop(BOOL first_in)
{
    g_first_in_desktop = first_in;
}

void MV_DRAWING_MENUBACK(HWND hwnd, U8 MenuKind, U8 SubKind)
{
	HDC 			hdc;
	char			Temp_Str[50];
	RECT			Title_Rect;
	U32             epgBoxH; // kb : Mar 25
	U32             epgBoxX; // kb : Mar 25

	switch( MenuKind )
	{
		case CSAPP_MAINMENU_INSTALL:
			sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_INSTALLATION), CS_MW_LoadStringByIdx(MAIN_MENU_STR[MenuKind][SubKind]));
			break;
		case CSAPP_MAINMENU_SYSTEM:
			sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_SETTING), CS_MW_LoadStringByIdx(MAIN_MENU_STR[MenuKind][SubKind]));
			break;
		case CSAPP_MAINMENU_MEDIA:
			sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_MEDIA), CS_MW_LoadStringByIdx(MAIN_MENU_STR[MenuKind][SubKind]));
			break;
		case CSAPP_MAINMENU_TOOL:
			if ( SubKind == EN_ITEM_FOCUS_TOOLS_MAX )
				sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_TOOL), CS_MW_LoadStringByIdx(CSAPP_STR_OSCAM));
			else
				sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_TOOL), CS_MW_LoadStringByIdx(MAIN_MENU_STR[MenuKind][SubKind]));
			break;
		case CSAPP_MAINMENU_CH_EDIT:
			sprintf(Temp_Str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_EDIT_PROGRAM));
			break;

		// kb : Mar 25
		case CSAPP_MAINMENU_EPG :
			sprintf(Temp_Str, "%s - %s", CS_MW_LoadStringByIdx(CSAPP_STR_EPG), CS_MW_LoadStringByIdx(EPG_MenuString[SubKind]));
			break;
		/***************/
		case CSAPP_MAINMENU_GAME :
			sprintf(Temp_Str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_GAME));
			break;

		case CSAPP_MAINMENU_CALENDAR :
			sprintf(Temp_Str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_CALENDAR));
			break;

		default:
			break;
	}

	hdc = MV_BeginPaint(hwnd);
	//MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
#if 0
	MV_SetBrushColor( hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_R, CFG_Menu_Back_Color.MV_G, CFG_Menu_Back_Color.MV_B, 0xFF) );
	MV_FillBox( hdc, ScalerWidthPixel(0), ScalerHeigthPixel(0), ScalerWidthPixel(ScreenWidth), ScalerHeigthPixel(ScreenHeigth) );
#else
	{
		int i = 0, j = 0;

		for ( i = 0 ; ( i * MV_BMP[MVBMP_MENU_BACK].bmHeight ) < 720 ; i++ )
		{
			for ( j = 0 ; ( j * MV_BMP[MVBMP_MENU_BACK].bmWidth ) < 1280 ; j++ )
				FillBoxWithBitmap(hdc, ScalerWidthPixel(j * MV_BMP[MVBMP_MENU_BACK].bmWidth), ScalerHeigthPixel(i * MV_BMP[MVBMP_MENU_BACK].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_MENU_BACK].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_BACK].bmHeight), &MV_BMP[MVBMP_MENU_BACK]);
		}
	}
#endif

	 // kb : Mar 25
	if (MenuKind == CSAPP_MAINMENU_EPG)
	{
		if (SubKind == EN_EPG_TYPE_SIMPLE)
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR );
			epgBoxH = (MV_INSTALL_MENU_BAR_H * 11) + 18;
			MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y + 44), ScalerWidthPixel(MV_EPG_CN_BAR_DX + 20), ScalerHeigthPixel(epgBoxH) );
			epgBoxX = MV_MENU_BACK_X + MV_MENU_BACK_DX - MV_EPG_CN_PIG_DX - 20;
			MV_FillBox( hdc, ScalerWidthPixel(epgBoxX), ScalerHeigthPixel(MV_MENU_BACK_Y + 44), ScalerWidthPixel(MV_EPG_CN_PIG_DX + 20), ScalerHeigthPixel(MV_EPG_CN_PIG_DY + 20) );
			MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
			epgBoxX += 10;
			MV_FillBox( hdc, ScalerWidthPixel(epgBoxX), ScalerHeigthPixel(MV_MENU_BACK_Y + 54), ScalerWidthPixel(MV_EPG_CN_PIG_DX), ScalerHeigthPixel(MV_EPG_CN_PIG_DY) );
		} else {
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR );
			epgBoxH = (25 * 11) + 18;
			MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y + MV_EPG_CN_PIG_DY + 70), ScalerWidthPixel(1100), ScalerHeigthPixel(epgBoxH) );
			epgBoxX = MV_MENU_BACK_X + 460;
			MV_FillBox( hdc, ScalerWidthPixel(epgBoxX), ScalerHeigthPixel(MV_MENU_BACK_Y + 44), ScalerWidthPixel(MV_EPG_CN_PIG_DX + 20), ScalerHeigthPixel(MV_EPG_CN_PIG_DY + 20) );
			MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
			epgBoxX += 10;
			MV_FillBox( hdc, ScalerWidthPixel(epgBoxX), ScalerHeigthPixel(MV_MENU_BACK_Y + 54), ScalerWidthPixel(MV_EPG_CN_PIG_DX), ScalerHeigthPixel(MV_EPG_CN_PIG_DY) );
		}
	}
	else if ( MenuKind == CSAPP_MAINMENU_CH_EDIT)
	{
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LIST_LEFT), ScalerHeigthPixel(MV_PIG_LIST_TOP), ScalerWidthPixel(MV_PIG_LIST_DX), ScalerHeigthPixel(MV_PIG_LIST_DY) );
		MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT), ScalerHeigthPixel(MV_PIG_TOP), ScalerWidthPixel(MV_PIG_DX), ScalerHeigthPixel(MV_PIG_DY) );
		MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT + MV_PIG_OUTGAP), ScalerHeigthPixel(MV_PIG_TOP + MV_PIG_OUTGAP), ScalerWidthPixel(MV_PIG_DX - MV_PIG_OUTGAP*2), ScalerHeigthPixel(MV_PIG_DY - MV_PIG_OUTGAP*2) );
	}
	else if ( MenuKind == CSAPP_MAINMENU_GAME || MenuKind == CSAPP_MAINMENU_CALENDAR )
	{
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT), ScalerHeigthPixel(MV_PIG_TOP), ScalerWidthPixel(MV_PIG_DX), ScalerHeigthPixel(MV_PIG_DY) );
		MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT + MV_PIG_OUTGAP), ScalerHeigthPixel(MV_PIG_TOP + MV_PIG_OUTGAP), ScalerWidthPixel(MV_PIG_DX - MV_PIG_OUTGAP*2), ScalerHeigthPixel(MV_PIG_DY - MV_PIG_OUTGAP*2) );
	}
	else
	{
		if ( (MenuKind == CSAPP_MAINMENU_MEDIA && ( SubKind == EN_ITEM_FOCUS_RECORD_FILE || SubKind == EN_ITEM_FOCUS_FILE_TOOL || SubKind == EN_ITEM_FOCUS_MEDIA_PLAYER ) ))
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LIST_LEFT), ScalerHeigthPixel(MV_PIG_LIST_TOP), ScalerWidthPixel(MV_PIG_LIST_DX), ScalerHeigthPixel(MV_PIG_LIST_DY) );
			MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT), ScalerHeigthPixel(MV_PIG_TOP), ScalerWidthPixel(MV_PIG_DX), ScalerHeigthPixel(MV_PIG_DY) );
			MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_PIG_LEFT + MV_PIG_OUTGAP), ScalerHeigthPixel(MV_PIG_TOP + MV_PIG_OUTGAP), ScalerWidthPixel(MV_PIG_DX - MV_PIG_OUTGAP*2), ScalerHeigthPixel(MV_PIG_DY - MV_PIG_OUTGAP*2) );
		}
		else
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(MV_MENU_BACK_DY) );
		}
	}
	/***************/

	Title_Rect.top 		= MV_MENU_BACK_Y - 60;
	Title_Rect.bottom	= Title_Rect.top + MV_INSTALL_MENU_BAR_H;
	Title_Rect.left		= MV_MENU_BACK_X;
	Title_Rect.right	= MV_MENU_BACK_X + MV_MENU_BACK_DX;

	/*
	MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(Title_Rect.left), ScalerHeigthPixel(Title_Rect.top - 4), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	*/
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect.left), ScalerHeigthPixel(Title_Rect.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight + 8), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect.left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Title_Rect.top), ScalerWidthPixel(MV_MENU_BACK_DX - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight + 8), &MV_BMP[MVBMP_MENU_TITLE_MID]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect.right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Title_Rect.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight + 8), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);

	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);


	Title_Rect.top = Title_Rect.top + 16 - ( CS_DBU_GetFont_Size() - 10 )/2;
	Title_Rect.left = Title_Rect.left + 4;
	CS_MW_DrawText(hdc, Temp_Str, -1, &Title_Rect, DT_CENTER);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Title_Rect.top = Title_Rect.top - 2;
	Title_Rect.left = Title_Rect.left - 2;
	CS_MW_DrawText(hdc, Temp_Str, -1, &Title_Rect, DT_CENTER);

	MV_EndPaint(hwnd,hdc);
}

void MV_System_draw_help_banner(HDC hdc, EN_SYSTEM_TOP_FOCUS emvkind)
{
	switch(emvkind)
	{
		case EN_ITEM_FOCUS_LANGUAGE:
/*
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_INFO));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_TIME_SETTING));
*/
			break;
		case EN_ITEM_FOCUS_TIME:
#if 0
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MENU_LANG));
			SetBrushColor(hdc, CSAPP_BLACK_COLOR);
			FillBox(hdc,ScalerWidthPixel(MV_HELP_ICON_X + (MV_HELP_ICON_DX4 * 1.5)), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(200), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight));
			if ( Current_TimeMode == eCS_DBU_TIME_INTERNET )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + (MV_HELP_ICON_DX4 * 1.5)), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + (MV_HELP_ICON_DX4 * 1.5) + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DHCP));

				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_SETTING));
			} else {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_SETTING));
			}

			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_SETTING));
#endif
			break;
		case EN_ITEM_FOCUS_SYS:
			{
				char		acTemp_Str[64];

				memset(acTemp_Str, 0x00, 64);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				sprintf(acTemp_Str, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_FAV_KEY) , CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), acTemp_Str);
			}
/*
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_AV_SETTING));
*/
			break;
		case EN_ITEM_FOCUS_AV:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
/*
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_PIN_SETTING));
*/
			break;
		case EN_ITEM_FOCUS_PARENTAL:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
/*
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_AV_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_NET_SETTING));
*/
			break;
		case EN_ITEM_FOCUS_NETWORK:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
/*
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_PIN_SETTING));
*/
			SetBrushColor(hdc, CSAPP_BLACK_COLOR);
			FillBox(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(300), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight));
			if ( CS_DBU_GetDHCP_Type() == CSAPP_DHCP_ON )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_TRY_DHCP));
			} else {
#if 0
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + (MV_HELP_ICON_DX4 * 1.5)), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + (MV_HELP_ICON_DX4 * 1.5) + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
#endif
			}
/*
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SYSTEM_INFO));
*/
			break;
		case EN_ITEM_FOCUS_SYSINFO:
/*
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_NET_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MENU_LANG));
*/
			break;
		default:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			break;
	}
}

void MV_Media_draw_help_banner(HDC hdc, EN_MEDIA_TOP_FOCUS emvkind)
{
	switch(emvkind)
	{
		case EN_ITEM_FOCUS_RECORD_FILE:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_OK_ICON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MEDIA_PLAY));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_EXIT_ICON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_REC_CONFIG));
			break;
#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
		case EN_ITEM_FOCUS_RECORD_CONF:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_REC_FILE));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FILE_TOOL));
			break;
#endif
		case EN_ITEM_FOCUS_FILE_TOOL:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_COPY));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MOVE));
			break;
		case EN_ITEM_FOCUS_USB_REMOVE:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FILE_TOOL));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_STORAGE_INFO));
			break;
		case EN_ITEM_FOCUS_STORAGE_INFO:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_USB_FORMAT));
			//FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MEDIA_PLAY));
			break;
		case EN_ITEM_FOCUS_MEDIA_PLAYER:

			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			if ( acUsbDevCount > 0 )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X - 40), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X - 40 + MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X - 40 + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 3), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CHANGE_PARTITION));
			}
#if 0
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_REC_FILE));
#endif
			break;
		default:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			break;
	}
}

void MV_TOOLS_draw_help_banner(HDC hdc, EN_TOOLS_TOP_FOCUS emvkind)
{
	switch(emvkind)
	{
		case EN_ITEM_FOCUS_UPGRADE:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DEFAULT_FACTORY));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CI));
			break;
		case EN_ITEM_FOCUS_CI:
			//SetTextColor(hdc,CSAPP_WHITE_COLOR);
			//SetBkMode(hdc,BM_TRANSPARENT);
			//FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CAS_DETAIL_INFO));
			//FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CAS));
			break;
		case EN_ITEM_FOCUS_CAS:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CAS_DETAIL_INFO));

			/* By KB Kim for Plugin Setting : 2011.05.07 */
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_OSCAM));
			break;
		case EN_ITEM_FOCUS_BACKUP:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CAS));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP_RE));
			break;
		case EN_ITEM_FOCUS_RESTORE:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_PLUG_IN));
			break;
		case EN_ITEM_FOCUS_PLUG_IN:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP_RE));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DEFAULT_FACTORY));
			break;
		case EN_ITEM_FOCUS_FAC_RESET:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y - 30), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 30), CS_MW_LoadStringByIdx(CSAPP_STR_SAT_CH_DELETE));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y - 30), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y - 30), CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CH_DELETE));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FAV_CH_DELETE));
			break;
		default:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			break;
	}
}

void MV_Draw_LevelBar(HDC hdc, RECT *TmpRect, U16 LevelValue, EN_LEVEL_BAR_KIND LevelKind)
{
	U16		STR_DX = 70;
	RECT	Value_Rect;
	U32		bar_x, bar_y, bar_dx, bar_dy;
	U32		Level_x, Level_dx;
	char	temp_str[5];

	if ( LevelKind != EN_TTEM_PROGRESS_NO_IMG_BMP )
	{
		bar_x 		= TmpRect->left + MV_BMP[MVBMP_LEFT_ARROW].bmWidth;
		bar_y		= TmpRect->top;

		Value_Rect.top 		= TmpRect->top;
		Value_Rect.bottom	= TmpRect->bottom;
		Value_Rect.left		= TmpRect->right - STR_DX - MV_BMP[MVBMP_LEFT_ARROW].bmWidth;
		Value_Rect.right	= TmpRect->right;
	} else {
		bar_x 		= TmpRect->left;
		bar_y		= TmpRect->top;

		Value_Rect.top 		= TmpRect->top;
		Value_Rect.bottom	= TmpRect->bottom;
		Value_Rect.left		= TmpRect->right - STR_DX;
		Value_Rect.right	= TmpRect->right;
	}

	if ( LevelKind != EN_TTEM_PROGRESS_NO_IMG && LevelKind != EN_TTEM_PROGRESS_NO_IMG_MP3 && LevelKind != EN_TTEM_PROGRESS_NO_IMG_BMP )
	{
		if ( LevelKind == EN_ITEM_10_BAR_LEVEL_NONAME )
		{
			bar_dx 		= (TmpRect->right - TmpRect->left) - MV_BMP[MVBMP_LEFT_ARROW].bmWidth*2;
			bar_dy		= MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
		} else {
			bar_dx 		= (TmpRect->right - TmpRect->left) - STR_DX - MV_BMP[MVBMP_LEFT_ARROW].bmWidth*2;
			bar_dy		= MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
		}
	}
	else
	{
		bar_dx 		= (TmpRect->right - TmpRect->left);
		bar_dy		= (TmpRect->bottom - TmpRect->top);
	}

	switch( LevelKind )
	{
		case EN_ITEM_5_BAR_LEVEL:
			{
				U16 	Temp_Value;
				Temp_Value = LevelValue * 20;
				Level_dx = MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
				Level_x = bar_x + ( ( bar_dx - Level_dx ) * Temp_Value/100 );
			}
			break;
		case EN_ITEM_10_BAR_LEVEL:
		case EN_ITEM_10_BAR_LEVEL_NONAME:
			{
				U16 	Temp_Value;
				Temp_Value = LevelValue * 10;
				Level_dx = MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
				Level_x = bar_x + ( ( bar_dx - Level_dx ) * Temp_Value/100 );
			}
			break;

		case EN_ITEM_20_BAR_LEVEL:
			LevelValue = LevelValue * 5;
			Level_dx = MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
			Level_x = bar_x + ( ( bar_dx - Level_dx ) * LevelValue/100 ) - Level_dx/2;
			break;

		case EN_ITEM_SIGNAL_LEVEL:
		case EN_ITEM_CHEDIT_SIGNAL_LEVEL:
		case EN_ITEM_SIGNAL_LEVEL_NONAME:
		case EN_ITEM_PROGRESS_BAR_LEVEL:
			Level_dx = ( bar_dx * LevelValue )/100;
			Level_x = bar_x;
			break;

		case EN_TTEM_PROGRESS_NO_IMG:
		case EN_TTEM_PROGRESS_NO_IMG_MP3:
		case EN_TTEM_PROGRESS_NO_IMG_BMP:
			Level_dx = ( bar_dx * LevelValue )/100;
			Level_x = bar_x;
			break;

		case EN_ITEM_NORMAL_BAR_LEVEL:
		default:
			Level_dx = MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;
			Level_x = bar_x + ( ( bar_dx - Level_dx ) * LevelValue/100 ) - Level_dx/2;
			break;
	}

	if ( LevelKind == EN_ITEM_SIGNAL_LEVEL || LevelKind == EN_ITEM_CHEDIT_SIGNAL_LEVEL)
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel( bar_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( bar_dx ), ScalerHeigthPixel( bar_dy ), &MV_BMP[MVBMP_GRAY_SIGNAL]);

		if ( LevelValue < 50 )
			FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_RED_SIGNAL]);
		else if ( LevelValue >= 50 && LevelValue < 80 )
			FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_ORANGE_SIGNAL]);
		else
			FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_GREEN_SIGNAL]);

		sprintf(temp_str, "%d %%", LevelValue);
	} else if ( LevelKind == EN_TTEM_PROGRESS_NO_IMG || LevelKind == EN_TTEM_PROGRESS_NO_IMG_MP3 || LevelKind == EN_TTEM_PROGRESS_NO_IMG_BMP ) {
		if ( LevelKind == EN_TTEM_PROGRESS_NO_IMG_MP3 || LevelKind == EN_TTEM_PROGRESS_NO_IMG_BMP )
			MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
		else
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(bar_x - 4), ScalerHeigthPixel(bar_y - 4), ScalerWidthPixel(bar_dx + 8), ScalerHeigthPixel(bar_dy + 8) );
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(Level_x), ScalerHeigthPixel(bar_y), ScalerWidthPixel(Level_dx), ScalerHeigthPixel(bar_dy) );
		sprintf(temp_str, "%d %%", LevelValue);
	} else if ( LevelKind == EN_ITEM_PROGRESS_BAR_LEVEL ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel( bar_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( bar_dx ), ScalerHeigthPixel( bar_dy ), &MV_BMP[MVBMP_GRAY_SIGNAL]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_GREEN_SIGNAL]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel( bar_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( bar_dx ), ScalerHeigthPixel( bar_dy ), &MV_BMP[MVBMP_GRAY_SIGNAL]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_RED_SIGNAL]);
		sprintf(temp_str, "%d", LevelValue);
	}

	if ( LevelKind != EN_TTEM_PROGRESS_NO_IMG && LevelKind != EN_ITEM_10_BAR_LEVEL_NONAME && LevelKind != EN_ITEM_SIGNAL_LEVEL_NONAME && LevelKind != EN_TTEM_PROGRESS_NO_IMG_MP3 && LevelKind != EN_TTEM_PROGRESS_NO_IMG_BMP && LevelKind != EN_ITEM_PROGRESS_BAR_LEVEL )
	{
		if ( LevelKind == EN_ITEM_CHEDIT_SIGNAL_LEVEL)
			MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
		else
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );

		if( (LevelKind != EN_ITEM_10_BAR_LEVEL) && (LevelKind != EN_ITEM_5_BAR_LEVEL) )
			MV_FillBox( hdc, ScalerWidthPixel(Value_Rect.left), ScalerHeigthPixel(Value_Rect.top), ScalerWidthPixel(Value_Rect.right - Value_Rect.left), ScalerHeigthPixel(Value_Rect.bottom - Value_Rect.top) );

		CS_MW_DrawText(hdc, temp_str, -1, &Value_Rect, DT_CENTER);
	}
}

void MV_Draw_Time_Progress_Bar(HDC hdc, RECT *TmpRect, U16 LevelValue, U16 Move_Value, char *acStart, char *acEnd, time_t Pogress_Time)
{
	RECT		Text_Rect;
	U32			bar_x, bar_y, bar_dx, bar_dy;
	U32			Level_x, Level_dx;
	U32			Move_x, Move_dx;
	char		acTemp_Str[50];
	struct tm	tm_time;

	bar_x  = TmpRect->left;
	bar_y  = TmpRect->top + 4;
	bar_dx = (TmpRect->right - TmpRect->left);
	bar_dy = MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight;

	Level_dx = ( bar_dx * LevelValue )/100;
	Level_x = bar_x;

	Move_dx = ( bar_dx * Move_Value )/100;
	Move_x = bar_x;

	FillBoxWithBitmap(hdc,ScalerWidthPixel( bar_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( bar_dx ), ScalerHeigthPixel( bar_dy ), &MV_BMP[MVBMP_GRAY_SIGNAL]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel( Level_x ), ScalerHeigthPixel( bar_y ), ScalerWidthPixel( Level_dx ), ScalerHeigthPixel( MV_BMP[MVBMP_GRAY_SIGNAL].bmHeight ), &MV_BMP[MVBMP_GREEN_SIGNAL]);

	memcpy(&tm_time, localtime(&Pogress_Time), sizeof(tm_time));

	if ( Pogress_Time >= 3600 )
		sprintf(acTemp_Str, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
	else
		sprintf(acTemp_Str, "%02d", tm_time.tm_min);

	Text_Rect.top = TmpRect->top - 10;
	Text_Rect.bottom = TmpRect->top + 20;
	Text_Rect.left = ( Level_x + Level_dx ) - 70 ;
	Text_Rect.right = Text_Rect.left + 140;
	FillBoxWithBitmap(hdc,ScalerWidthPixel( ( Level_x + Level_dx ) - (MV_BMP[MVBMP_POSITION].bmWidth/2) ), ScalerHeigthPixel( Text_Rect.top ), ScalerWidthPixel( MV_BMP[MVBMP_POSITION].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_POSITION].bmHeight ), &MV_BMP[MVBMP_POSITION]);

	if ( Move_Value > 0 )
	{
		Text_Rect.top = TmpRect->top + 20;
		Text_Rect.bottom = TmpRect->top + 20;
		Text_Rect.left = ( Move_x + Move_dx ) - 70 ;
		Text_Rect.right = Text_Rect.left + 140;
		FillBoxWithBitmap(hdc,ScalerWidthPixel( ( Move_x + Move_dx ) - (MV_BMP[MVBMP_POSITION].bmWidth/2) ), ScalerHeigthPixel( Text_Rect.top ), ScalerWidthPixel( MV_BMP[MVBMP_POSITION_BLUE].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_POSITION_BLUE].bmHeight ), &MV_BMP[MVBMP_POSITION_BLUE]);
	}
//	CS_MW_DrawText(hdc, acTemp_Str, -1, &Text_Rect, DT_CENTER);

	Text_Rect.top = TmpRect->top;
	Text_Rect.bottom = TmpRect->bottom;
	Text_Rect.left = TmpRect->left - 150;
	Text_Rect.right = TmpRect->left - 10;
	CS_MW_DrawText(hdc, acStart, -1, &Text_Rect, DT_RIGHT);

	Text_Rect.left = TmpRect->right + 10;
	Text_Rect.right = TmpRect->right + 150;
	CS_MW_DrawText(hdc, acEnd, -1, &Text_Rect, DT_LEFT);
}

void MV_Warning_Report_Window_Open( HWND hwnd, ePopupIndex MenuKind /*, U8 SubKind*/ )
{
	HDC 	hdc;
	RECT	Capture_Rect;
	char	temp_str[50];

	memset (&Capture_bmp, 0, sizeof (BITMAP));
	memset (temp_str, 0x00, 50);

	Warning_Report_Window_Status = TRUE;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(CAPTURE_LEFT), ScalerHeigthPixel(CAPTURE_TOP), ScalerWidthPixel(CAPTURE_DX), ScalerHeigthPixel(CAPTURE_DY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(CAPTURE_LEFT), ScalerHeigthPixel(CAPTURE_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CAPTURE_LEFT + CAPTURE_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CAPTURE_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CAPTURE_LEFT), ScalerHeigthPixel(CAPTURE_TOP + CAPTURE_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CAPTURE_LEFT + CAPTURE_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CAPTURE_TOP + CAPTURE_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(CAPTURE_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CAPTURE_TOP),ScalerWidthPixel(CAPTURE_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CAPTURE_DY));
	FillBox(hdc,ScalerWidthPixel(CAPTURE_LEFT), ScalerHeigthPixel(CAPTURE_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(CAPTURE_DX),ScalerHeigthPixel(CAPTURE_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Capture_Rect.top 	= CAPTURE_TOP+10;
	Capture_Rect.bottom	= Capture_Rect.top + MV_INSTALL_MENU_BAR_H;
	Capture_Rect.left	= CAPTURE_LEFT+8;
	Capture_Rect.right	= Capture_Rect.left + (CAPTURE_DX-16);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	switch (MenuKind)
	{
		case MV_WINDOW_WARNING:
			MV_Draw_PopUp_Title_Bar_ByName(hdc, &Capture_Rect, CSAPP_STR_WARNING);
		/*
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_WARNING));
			CS_MW_DrawText(hdc, temp_str, -1, &Capture_Rect, DT_CENTER);
		*/
			break;
		case MV_WINDOW_REPORT:
		case MV_WINDOW_USB_MOUNT:
		case MV_WINDOW_USB_UNMOUNT:
		case MV_WINDOW_USB_CONNECT:
		case MV_WINDOW_USB_DISCONNECT:
		case MV_WINDOW_USB_MOUNT_FAIL:
		case MV_WINDOW_USB_UNMOUNT_FAIL:
		case MV_WINDOW_NO_FAV_CHANNEL:
		case MV_WINDOW_SMART_CARD_INSERT:
		case MV_WINDOW_SMART_CARD_REMOVE:
			MV_Draw_PopUp_Title_Bar_ByName(hdc, &Capture_Rect, CSAPP_STR_ATTENTION);
		/*
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ATTENTION));
			CS_MW_DrawText(hdc, temp_str, -1, &Capture_Rect, DT_CENTER);
		*/
			break;
		case MV_WINDOW_CERTIFY:
			break;
		default:
			break;
	}

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(CAPTURE_LEFT+8), ScalerHeigthPixel(CAPTURE_TOP+46), ScalerWidthPixel(CAPTURE_DX-16), ScalerHeigthPixel(CAPTURE_DY-62) );

	Capture_Rect.top 	= CAPTURE_TOP + CAPTURE_DY/2 - 15;
	Capture_Rect.bottom	= Capture_Rect.top + MV_INSTALL_MENU_BAR_H * 2;
	Capture_Rect.left	= CAPTURE_LEFT;
	Capture_Rect.right	= Capture_Rect.left + CAPTURE_DX;

	switch (MenuKind)
	{
		case MV_WINDOW_WARNING:
			sprintf(temp_str, "%s", "PIN Doðru");
			break;
		case MV_WINDOW_REPORT:
			sprintf(temp_str, "%s", "PIN Doðru");
			break;
		case MV_WINDOW_USB_MOUNT:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_USB_MOUNT));
			break;
		case MV_WINDOW_USB_UNMOUNT:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_USB_UNMOUNT));
			break;
		case MV_WINDOW_USB_CONNECT:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_USB_CONNECTING));
			break;
		case MV_WINDOW_USB_DISCONNECT:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_USB_DISCONNECTING));
			break;
		case MV_WINDOW_USB_MOUNT_FAIL:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_USB_CONNECTING));
			break;
		case MV_WINDOW_USB_UNMOUNT_FAIL:
			sprintf(temp_str, "%s", "USB aygýtý düzgün çýkartýlamadý...");
			break;
		case MV_WINDOW_NO_FAV_CHANNEL:
			sprintf(temp_str, "%s", "Favorilerde Kanal Yok ...");
			break;
		case MV_WINDOW_CERTIFY:
			break;
		case MV_WINDOW_SMART_CARD_INSERT:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_SMART_CARD_INSERTED));
			break;
		case MV_WINDOW_SMART_CARD_REMOVE:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_SMART_CARD_REMOVE));
        case MV_WINDOW_PLUGIN_INSTALLED:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_INSTALLED));
			break;
        case MV_WINDOW_STREAMING_STARTED:
			sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_STREAMING_STARTING));
			break;
		default:
			break;
	}

	CS_MW_DrawText(hdc, temp_str, -1, &Capture_Rect, DT_CENTER);
	MV_EndPaint(hwnd,hdc);

}

void MV_Warning_Report_Window_Close( HWND hwnd )
{
	HDC 	hdc;

	Warning_Report_Window_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(CAPTURE_LEFT), ScalerHeigthPixel(CAPTURE_TOP), ScalerWidthPixel(CAPTURE_DX), ScalerHeigthPixel(CAPTURE_DY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

BOOL MV_Get_Report_Window_Status(void)
{
	return Warning_Report_Window_Status;
}
 /* by kb : 20100407 */
void MV_Draw_PageScrollBar(HDC hdc, RECT Scroll_Rect, U16 u16Now_Point, U16 totalNumber, U16 numberInPage)
{
	U16 	iCount = 0;
	U16		u16Scroll_point;

	if (( totalNumber > numberInPage ) && (totalNumber != 0))
	{
		iCount = totalNumber - numberInPage;
		// kb : 100331
		u16Scroll_point = ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight ) + (( (Scroll_Rect.bottom - ( Scroll_Rect.top + SCROLL_BAR_DX + MV_BMP[MVBMP_UPARROW].bmHeight * 2 )) * u16Now_Point ) / iCount);
		// u16Scroll_point = ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight ) + (( (Scroll_Rect.bottom - ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight * 2 )) * u16Now_Point ) / iCount);

		//dprintf("=== X : %d , Y : %d , DX : %d , DY : %d=====\n", Scroll_Rect.left, Scroll_Rect.top, Scroll_Rect.right - Scroll_Rect.left, Scroll_Rect.bottom - Scroll_Rect.top);
		//dprintf("=== TOTAL SERVICE : %d , NOW : %d , SCROLL_POINT : %d =====\n", Mv_Install_SatFocus, u16Now_Point, u16Scroll_point);

		MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel( Scroll_Rect.right - Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.bottom - Scroll_Rect.top ) );
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel(MV_BMP[MVBMP_UPARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_UPARROW].bmHeight),&MV_BMP[MVBMP_UPARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.bottom - MV_BMP[MVBMP_DOWNARROW].bmHeight ), ScalerWidthPixel(MV_BMP[MVBMP_DOWNARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_DOWNARROW].bmHeight),&MV_BMP[MVBMP_DOWNARROW]);

		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left + 2 ),ScalerHeigthPixel( u16Scroll_point - (SCROLL_BAR_DX - 4) / 2 ), ScalerWidthPixel( SCROLL_BAR_DX - 4 ),ScalerHeigthPixel( SCROLL_BAR_DX - 4 ) );

	} else {
		MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel( Scroll_Rect.right - Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.bottom - Scroll_Rect.top ) );
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel(MV_BMP[MVBMP_UPARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_UPARROW].bmHeight),&MV_BMP[MVBMP_UPARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.bottom - MV_BMP[MVBMP_DOWNARROW].bmHeight ), ScalerWidthPixel(MV_BMP[MVBMP_DOWNARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_DOWNARROW].bmHeight),&MV_BMP[MVBMP_DOWNARROW]);
	}
}
/*************************************/

void MV_Draw_ScrollBar(HDC hdc, RECT Scroll_Rect, U16 u16Now_Point, U16 Mv_Install_SatFocus, EN_LIST_KIND enListKind, U8 enScrollKind)
{
	U16 	iCount = 0;
	U16		u16Scroll_point;

	switch( enListKind )
	{
		case EN_ITEM_SAT_LIST:
			iCount = MV_GetSatelliteData_Num();
			break;
		case EN_ITEM_CHANNEL_LIST:
			iCount = Mv_Install_SatFocus;
			break;
		case EN_ITEM_TP_LIST:
			iCount = MV_DB_Get_TPCount_By_Satindex(Mv_Install_SatFocus);
			break;
		case EN_ITEM_EPG_LIST:
			break;
		default:
			break;
	}

	if ( iCount >= LIST_ITEM_NUM )
	{
		if ( enScrollKind == MV_HORIZONTAL )
			u16Scroll_point = Scroll_Rect.left + ( (Scroll_Rect.right - Scroll_Rect.left) * u16Now_Point) / iCount;
		else
		{
			// kb : 100331
			u16Scroll_point = ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight ) + (( (Scroll_Rect.bottom - ( Scroll_Rect.top + SCROLL_BAR_DX + MV_BMP[MVBMP_UPARROW].bmHeight * 2 )) * u16Now_Point ) / iCount);
			// u16Scroll_point = ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight ) + (( (Scroll_Rect.bottom - ( Scroll_Rect.top + SCROLL_BAR_DX/2 + MV_BMP[MVBMP_UPARROW].bmHeight * 2 )) * u16Now_Point ) / iCount);
		}

		//dprintf("=== X : %d , Y : %d , DX : %d , DY : %d=====\n", Scroll_Rect.left, Scroll_Rect.top, Scroll_Rect.right - Scroll_Rect.left, Scroll_Rect.bottom - Scroll_Rect.top);
		//dprintf("=== TOTAL SERVICE : %d , NOW : %d , SCROLL_POINT : %d =====\n", Mv_Install_SatFocus, u16Now_Point, u16Scroll_point);

		MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel( Scroll_Rect.right - Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.bottom - Scroll_Rect.top ) );
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel(MV_BMP[MVBMP_UPARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_UPARROW].bmHeight),&MV_BMP[MVBMP_UPARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.bottom - MV_BMP[MVBMP_DOWNARROW].bmHeight ), ScalerWidthPixel(MV_BMP[MVBMP_DOWNARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_DOWNARROW].bmHeight),&MV_BMP[MVBMP_DOWNARROW]);

		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left + 2 ),ScalerHeigthPixel( u16Scroll_point - (SCROLL_BAR_DX - 4) / 2 ), ScalerWidthPixel( SCROLL_BAR_DX - 4 ),ScalerHeigthPixel( SCROLL_BAR_DX - 4 ) );

	} else {
		MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel( Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel( Scroll_Rect.right - Scroll_Rect.left ),ScalerHeigthPixel( Scroll_Rect.bottom - Scroll_Rect.top ) );
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.top ), ScalerWidthPixel(MV_BMP[MVBMP_UPARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_UPARROW].bmHeight),&MV_BMP[MVBMP_UPARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Scroll_Rect.left),ScalerHeigthPixel( Scroll_Rect.bottom - MV_BMP[MVBMP_DOWNARROW].bmHeight ), ScalerWidthPixel(MV_BMP[MVBMP_DOWNARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_DOWNARROW].bmHeight),&MV_BMP[MVBMP_DOWNARROW]);
	}
}

void MV_Parser_IP_Value( char *Temp, char *tempSection )
{
	U16		i, j;

	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( isdigit(tempSection[i]) )
			break;
	}

	j = i;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;

		Temp[i] = tempSection[i+j];
	}
}

void MV_Parser_IP_DataValue( U8 u8IPIndex , char *Temp )
{
	U16					i, j = 0, k = 0;
	stIPStr_Struct		temp_ipStr[IP_ITEM];

	memset(temp_ipStr, 0x00, sizeof(stIPStr_Struct) * IP_ITEM);

	for ( i = 0 ; i < strlen(Temp) ; i++ )
	{
		if ( Temp[i] == '.' )
		{
			j++;
			k = 0;
			continue;
		}
		else
		{
			temp_ipStr[j].Re_IP[k] = Temp[i];
			k++;
		}
	}

	//for ( i = 0 ; i < IP_ITEM ; i++ )
	//	printf("MV_Parser_IP_DataValue :: temp_ipStr : %d - %s\n", i, temp_ipStr[i].Re_IP);

	switch(u8IPIndex)
	{
		case CSAPP_IP_IPADDR:
			memcpy(&IP_Addr, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			break;

		case CSAPP_IP_SUBNET:
			memcpy(&IP_SubNet, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			break;

		case CSAPP_IP_GATEWAY:
			memcpy(&IP_Gateway, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			break;

		case CSAPP_IP_DNS1:
			memcpy(&IP_DNS1, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			//printf("========= NAME1 : %d.%d.%d.%d , %d.%d.%d.%d===========\n", atoi(IP_DNS1[0].Re_IP), atoi(IP_DNS1[1].Re_IP), atoi(IP_DNS1[2].Re_IP), atoi(IP_DNS1[3].Re_IP), atoi(temp_ipStr[0].Re_IP), atoi(temp_ipStr[1].Re_IP), atoi(temp_ipStr[2].Re_IP), atoi(temp_ipStr[3].Re_IP));
			break;

		case CSAPP_IP_DNS2:
			memcpy(&IP_DNS2, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			//printf("========= NAME2 : %d.%d.%d.%d , %d.%d.%d.%d===========\n", atoi(IP_DNS2[0].Re_IP), atoi(IP_DNS2[1].Re_IP), atoi(IP_DNS2[2].Re_IP), atoi(IP_DNS2[3].Re_IP), atoi(temp_ipStr[0].Re_IP), atoi(temp_ipStr[1].Re_IP), atoi(temp_ipStr[2].Re_IP), atoi(temp_ipStr[3].Re_IP));
			break;

		case CSAPP_IP_DNS3:
			memcpy(&IP_DNS3, &temp_ipStr, sizeof(stIPStr_Struct) * IP_ITEM);
			//printf("========= NAME3 : %d.%d.%d.%d , %d.%d.%d.%d===========\n", atoi(IP_DNS3[0].Re_IP), atoi(IP_DNS3[1].Re_IP), atoi(IP_DNS3[2].Re_IP), atoi(IP_DNS3[3].Re_IP), atoi(temp_ipStr[0].Re_IP), atoi(temp_ipStr[1].Re_IP), atoi(temp_ipStr[2].Re_IP), atoi(temp_ipStr[3].Re_IP));
			break;

		default:
			break;
	}
}

U8 MV_IPData_Namecheck(char *tempSection)
{
	int 	i;
	char	Temp[20];

	for ( i = 0 ; i < CSAPP_DHCP_IP_MAX ; i++ )
	{
		memset (Temp, 0, sizeof(char) * 20);
		strncpy(Temp, tempSection, strlen(IP_Check_String[i]));

		if ( strcmp ( IP_Check_String[i], Temp ) == 0 )
			break;
	}

	return i;
}

MV_File_Return MV_Load_IPData_File(void)
{
	FILE* 		fp;
    char 		tempSection [DHCP_FILE_MAX_COL + 2];
	char		Temp[100];

	if (!(fp = fopen(IP_FILE, "r")))
         return FILE_NOFILE;

	while (!feof(fp)) {
		memset (tempSection, 0, sizeof(char) * (DHCP_FILE_MAX_COL + 2));
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, DHCP_FILE_MAX_COL, fp)) {
			return FILE_READ_FAIL;
        }

		MV_Parser_IP_Value(Temp, tempSection);
		MV_Parser_IP_DataValue( MV_IPData_Namecheck(tempSection) , Temp);
    }

	fclose (fp);
	return FILE_OK;
}

MV_File_Return MV_Save_DNS_File(void)
{
	FILE* 		fp;
	char		Temp[100];
	int 		i;

	if (!(fp = fopen(DHCP_RESOLV_FILE, "w")))
	{
		//printf("\n=== File Open Error : %s ====\n", DHCP_RESOLV_FILE);
        return FILE_NOFILE;
	}

	for ( i = CSAPP_IP_DNS1 ; i < CSAPP_IP_MAX ; i++ )
	{
		memset( Temp, 0x00, 100 );
		switch(i)
		{
			case CSAPP_IP_DNS1:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[CSAPP_IP_DNS1], atoi(IP_DNS1[0].Re_IP), atoi(IP_DNS1[1].Re_IP), atoi(IP_DNS1[2].Re_IP), atoi(IP_DNS1[3].Re_IP));
				//printf("\nCSAPP_IP_DNS1 === %d , %s ====\n", i, Temp);
				break;

			case CSAPP_IP_DNS2:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[CSAPP_IP_DNS1], atoi(IP_DNS2[0].Re_IP), atoi(IP_DNS2[1].Re_IP), atoi(IP_DNS2[2].Re_IP), atoi(IP_DNS2[3].Re_IP));
				//printf("\nCSAPP_IP_DNS2 === %d , %s ====\n", i, Temp);
				break;

			case CSAPP_IP_DNS3:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[CSAPP_IP_DNS1], atoi(IP_DNS3[0].Re_IP), atoi(IP_DNS3[1].Re_IP), atoi(IP_DNS3[2].Re_IP), atoi(IP_DNS3[3].Re_IP));
				//printf("\nCSAPP_IP_DNS3 === %d , %s ====\n", i, Temp);
				break;
		}
		fprintf(fp, "%s", Temp);
	}
	fclose (fp);

	return FILE_OK;
}

MV_File_Return MV_Save_IPData_File(void)
{
	FILE* 		fp;
	char		Temp[100];
	int 		i;

	if (!(fp = fopen(IP_FILE, "w")))
         return FILE_NOFILE;

	fprintf(fp, "auto lo \n");
	fprintf(fp, "iface lo inet loopback \n\n");
	fprintf(fp, "auto eth0 \n");

	if ( CS_DBU_GetDHCP_Type() == CSAPP_DHCP_ON )
		fprintf(fp, "iface eth0 inet dhcp \n\n");
	else
		fprintf(fp, "iface eth0 inet static \n\n");

	for ( i = 0 ; i < CSAPP_IP_DNS1 ; i++ )
	{
		memset( Temp, 0x00, 100 );
		switch(i)
		{
			case CSAPP_IP_IPADDR:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[i], atoi(IP_Addr[0].Re_IP), atoi(IP_Addr[1].Re_IP), atoi(IP_Addr[2].Re_IP), atoi(IP_Addr[3].Re_IP));
				break;

			case CSAPP_IP_SUBNET:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[i], atoi(IP_SubNet[0].Re_IP), atoi(IP_SubNet[1].Re_IP), atoi(IP_SubNet[2].Re_IP), atoi(IP_SubNet[3].Re_IP));
				break;

			case CSAPP_IP_GATEWAY:
				sprintf(Temp, "%s %d.%d.%d.%d\n", IP_Check_String[i], atoi(IP_Gateway[0].Re_IP), atoi(IP_Gateway[1].Re_IP), atoi(IP_Gateway[2].Re_IP), atoi(IP_Gateway[3].Re_IP));
				break;

			default:
				break;
		}
		fprintf(fp, "%s", Temp);
	}

	fclose (fp);

	if ( MV_Save_DNS_File() != FILE_OK )
	{
		//printf("\n===== DNS - resolv.conf Write Error\n");
		return FILE_READ_FAIL;
	}

	return FILE_OK;
}

void MV_Setting_Manual_Network(void)
{
	char		system_Str[256];

	sprintf(system_Str , "ifconfig eth0 %s.%s.%s.%s broadcast %s.%s.%s.255 netmask %s.%s.%s.%s",
			IP_Addr[0].Re_IP, IP_Addr[1].Re_IP, IP_Addr[2].Re_IP, IP_Addr[3].Re_IP,
			IP_Addr[0].Re_IP, IP_Addr[1].Re_IP, IP_Addr[2].Re_IP,
			IP_SubNet[0].Re_IP, IP_SubNet[1].Re_IP, IP_SubNet[2].Re_IP, IP_SubNet[3].Re_IP);
	system(system_Str);
	sprintf(system_Str, "route add default gw %s.%s.%s.%s dev eth0", IP_Gateway[0].Re_IP, IP_Gateway[1].Re_IP, IP_Gateway[2].Re_IP, IP_Gateway[3].Re_IP);
	printf("%s ==>>>> \n", system_Str);
	system(system_Str);
/*
	fp = fopen("/etc/resolv.conf", "w");
	fprintf(fp, "nameserver %d.%d.%d.%d\n", atoi(IP_DNS1[0].Re_IP), atoi(IP_DNS1[1].Re_IP), atoi(IP_DNS1[2].Re_IP), atoi(IP_DNS1[3].Re_IP));
	fclose (fp);
*/
}

int MV_Get_USB_Info(U8 u8Partition_No, struct f_size *stfile_size)
{
	MOUNTP 			*MP;

	if ((MP=dfopen()) == NULL)
	{
		perror("error");
		return 1;
	}

	while(dfget(MP))
	{
		if ( strcmp(stPartition_data[u8Partition_No].acPartition_Dir, MP->mountdir) == 0 )
		{
			stfile_size->blocks = MP->size.blocks;
			stfile_size->avail = MP->size.avail;
		}
		//printf("%-14s%-20s%10lu%10lu\n", MP->mountdir, MP->devname, MP->size.blocks, MP->size.avail);
	}
	dfclose(MP);
	return 0;
}

BOOL MV_Get_Network_Status(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL + 2];
	BOOL			ret = FALSE;

	if (!(fp = fopen(ETH0_STATUS_FILE, "r")))
	{
         ret = FALSE;
		 //printf("=== MV_Get_Network_Status : NO FILE =====\n");
		 return ret;
	}

	if (!fgets(tempSection, CFG_MAX_COL, fp)) {
		fclose (fp);
		ret = FALSE;
		//printf("=== MV_Get_Network_Status : READ FAIL =====\n");
		return ret;
	}

	fclose (fp);

	ret = atoi(tempSection);

	return ret;
}

#ifdef CHECK_CH_WATCH
int MV_Get_Current_Channel_Time(void)
{
	return NowChannel_Time_Count++;
}

void MV_ReSet_Current_Channel_Time(void)
{
	NowChannel_Time_Count = 0;
	memset( &stSend_Data, 0x00, sizeof(stSend_Channel_Data));
}

void MV_Set_Current_Channel_Data(U16 u16Channel_Index, U16 u16Channel_Num)
{
	MV_stSatInfo 		Temp_SatInfo;
	MV_stTPInfo 		Temp_TPInfo;
	MV_stServiceInfo	Temp_ChInfo;
	time_t				Start_Time;
	struct tm			tm_time;
	struct timespec		time_value;
	char				MacAddress[MAC_SIZE];
	TunerSignalState_t 	signalState; // For New Rating function By KB KIm 2011.08.01

	U8					MvappVer = 0;

	printf("======== SET CHANNEL DATA ===================\n");

	memset( &stSend_Data, 0x00, sizeof(stSend_Channel_Data));
	memset(MacAddress, 0x00, MAC_SIZE);

	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0x114, MacAddress, 6);

	sprintf(stSend_Data.acMac_Add, "%02x%02x%02x%02x%02x%02x", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);

	MV_DB_GetServiceDataByIndex(&Temp_ChInfo, u16Channel_Index);
	MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, Temp_ChInfo.u16ChIndex);
	MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, Temp_ChInfo.u16ChIndex);

	clock_gettime(CLOCK_REALTIME, &time_value);
	Start_Time = time_value.tv_sec;
	memcpy(&tm_time, localtime(&Start_Time), sizeof(tm_time));

	/*	For New Rating function By KB KIm 2011.08.01 */
	TunerReadSignalState(Tuner_HandleId[0], &signalState);
	stSend_Data.u8Strength = signalState.Strength;
	stSend_Data.u8Quality  = signalState.Quality;

	strncpy( stSend_Data.acCH_Name, Temp_ChInfo.acServiceName, 16);
	strncpy( stSend_Data.acSat_Name, Temp_SatInfo.acSatelliteName, 16);
	stSend_Data.u16Ch_Index = u16Channel_Index;
	stSend_Data.u16Service_Id = Temp_ChInfo.u16ServiceId;
	stSend_Data.u16Channel_Number = u16Channel_Num;
	stSend_Data.u16TP_Freq = Temp_TPInfo.u16TPFrequency;
	stSend_Data.u8TP_Pol = Temp_TPInfo.u8Polar_H;
	stSend_Data.u16TP_Symbol = Temp_TPInfo.u16SymbolRate;
	stSend_Data.u16Sat_Logitude = Temp_SatInfo.s16Longitude;
	stSend_Data.long_StTime = Start_Time;
	sprintf( stSend_Data.acSt_Time, "%02d.%02d.%04d%%20%02d:%02d:%02d", tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year + 1900, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
	stSend_Data.u8SetID = 1;
	stSend_Data.u8Mvapp_Ver = MvappVer;

}

static pthread_t			hChSend_TaskHandle;

void Send_ch_data_Stop(void)
{
	pthread_cancel( hChSend_TaskHandle );
}

int Send_ch_data_Init(void)
{
	if (access(RATING_CONTROL, F_OK) == 0)
	{
		pthread_create( &hChSend_TaskHandle, NULL, MV_Send_Current_Channel_Data, NULL );
	}
	return( 0 );
}

void *MV_Send_Current_Channel_Data( void *param )
{
	int					i;
	time_t				End_Time;
	struct tm			tm_time;
	struct timespec		time_value;
	char				system_command[512];
	char				Temp_Sat_Name[32];
	int					count, name_count;

	/*	For New Rating function By KB KIm 2011.08.01 */
	FILE*				ratingCfg;
	char 				*readStatus;
	U8   				readBuffer[MAX_SITE_LENGTH];

	memset( system_command, 0x00, 512);
	memset( Temp_Sat_Name, 0x00, 32);

	if ( NowChannel_Time_Count > 30 )
	{
		NowChannel_Time_Count = 0;
		clock_gettime(CLOCK_REALTIME, &time_value);
		End_Time = time_value.tv_sec;

		if ( stSend_Data.long_StTime > End_Time )
		{
			for ( i = 1 ; i < 100 ; i++ )
			{
				clock_gettime(CLOCK_REALTIME, &time_value);
				End_Time = time_value.tv_sec;

				if ( stSend_Data.long_StTime < End_Time )
					break;
				else if ( i == 99 )
				{
					printf("Send Data Error ==== Start_Time Too Big \n");
					return ( param );
				}
			}
		}

		memcpy(&tm_time, localtime(&End_Time), sizeof(tm_time));

		sprintf( stSend_Data.acEnd_Time , "%02d.%02d.%04d%%20%02d:%02d:%02d", tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year + 1900, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
#if 0
		printf("Send Channel Data ==========================\n");
		printf("1. MAC : %s\n", stSend_Data.acMac_Add);
		printf("2. CH_Num : %d\n", stSend_Data.u16Channel_Number);
		printf("3. CH_S_ID : %d\n", stSend_Data.u16Service_Id);
		printf("4. TP_FREQ : %d\n", stSend_Data.u16TP_Freq);
		printf("5. TP_POL : %d\n", stSend_Data.u8TP_Pol);
		printf("6. TP_SYMB : %d\n", stSend_Data.u16TP_Symbol);
		printf("7. SAT_LON : %d\n", stSend_Data.u16Sat_Logitude);
		printf("8. START_T : %s\n", stSend_Data.acSt_Time);
		printf("9. SET_ID : %d\n", stSend_Data.u8SetID);
		printf("10.MVAPP_V : %d\n", stSend_Data.u8Mvapp_Ver);
		printf("11.END_T : %s\n", stSend_Data.acEnd_Time );
		printf("12.Ch Name : %s\n", stSend_Data.acCH_Name);
		printf("13.Sat Name : %s\n", stSend_Data.acSat_Name);
		/*	For New Rating function By KB KIm 2011.08.01 */
		printf("14.Signal Strengh : %d\n", stSend_Data.u8Strength);
		printf("15.Signal Quality : %d\n", stSend_Data.u8Quality);
		printf("============================================\n");
#endif
		for ( count = 0 , name_count = 0 ; count < (int)strlen(stSend_Data.acSat_Name) ; count++ )
		{
			if ( stSend_Data.acSat_Name[count] == ' ' )
			{
				Temp_Sat_Name[name_count++] = '%';
				Temp_Sat_Name[name_count++] = '2';
				Temp_Sat_Name[name_count++] = '0';
			} else {
				Temp_Sat_Name[name_count++] = stSend_Data.acSat_Name[count];
			}
		}

		/*	For New Rating function By KB KIm 2011.08.01 */
		if (access(RATING_CONTROL, F_OK) == 0)
		{
			ratingCfg = fopen(RATING_CONFIG, "r");
			if (ratingCfg != NULL)
			{
				memset(readBuffer, 0, MAX_SITE_LENGTH);
				readStatus = fgets(readBuffer, MAX_SITE_LENGTH, ratingCfg);
				if (readStatus != NULL)
				{
					count = strlen(readBuffer);
					while (count >0)
					{
						count--;
						if ((readBuffer[count] == 0x0D) || (readBuffer[count] == 0x0A))
						{
							readBuffer[count] = 0x00;
						}
						else
						{
							break;
						}
					}
					// printf("Rating Site is [%s]\n", readBuffer);
					sprintf(system_command, "wget -q -O - \"http://%s?ch_s_id=%d&serial=%s&ch_num=%d&sat_long=%d&sat_name=%s&set_id=%d&mv_vers=%d&start=%s&end=%s&tp_freq=%d&tp_sym=%d&tp_pol=%d&signal=%d&level=%d\"",
																										readBuffer,
																										stSend_Data.u16Service_Id,
																										stSend_Data.acMac_Add,
																										stSend_Data.u16Channel_Number,
																										stSend_Data.u16Sat_Logitude,
																										Temp_Sat_Name,
																										stSend_Data.u8SetID,
																										stSend_Data.u8Mvapp_Ver,
																										stSend_Data.acSt_Time,
																										stSend_Data.acEnd_Time,
																										stSend_Data.u16TP_Freq,
																										stSend_Data.u16TP_Symbol,
																										stSend_Data.u8TP_Pol,
																										stSend_Data.u8Quality,   /*	For New Rating function By KB KIm 2011.08.01 */
																										stSend_Data.u8Strength); /*	For New Rating function By KB KIm 2011.08.01 */
					// printf("test server %s\n", system_command);
					if ( MV_Get_Network_Status() == TRUE )
					{
						system(system_command);
						// printf("=== NETWORK IS ALIVE - First Server : %s ===\n", system_command);
					} else {
						printf("=== NETWORK IS DEAD ===\n");
					}
				}
				else
				{
					printf("Cannot Read from[%s] file\n", RATING_CONFIG);
				}

				fclose(ratingCfg);
			}
			else
			{
				printf("Cannot open Rating Config[%s] file\n", RATING_CONFIG);
			}


			sprintf(system_command, "wget -q -O - \"http://www.merihvideo.com.tr/chipbox/yaz.php?ch_s_id=%d&serial=%s&ch_num=%d&sat_long=%d&sat_name=%s&set_id=%d&mv_vers=%d&start=%s&end=%s&tp_freq=%d&tp_sym=%d&tp_pol=%d&signal=%d&level=%d\"",
																								stSend_Data.u16Service_Id,
																								stSend_Data.acMac_Add,
																								stSend_Data.u16Channel_Number,
																								stSend_Data.u16Sat_Logitude,
																								Temp_Sat_Name,
																								stSend_Data.u8SetID,
																								stSend_Data.u8Mvapp_Ver,
																								stSend_Data.acSt_Time,
																								stSend_Data.acEnd_Time,
																								stSend_Data.u16TP_Freq,
																								stSend_Data.u16TP_Symbol,
																								stSend_Data.u8TP_Pol,
																								stSend_Data.u8Quality,   /*	For New Rating function By KB KIm 2011.08.01 */
																								stSend_Data.u8Strength); /*	For New Rating function By KB KIm 2011.08.01 */
			printf("%s\n", system_command);
			if ( MV_Get_Network_Status() == TRUE )
			{
				system(system_command);
				// printf("=== NETWORK IS ALIVE - Second Server : %s ===\n", system_command);
			} else {
				printf("=== NETWORK IS DEAD ===\n");
			}
		}
	}


	memset( &stSend_Data, 0x00, sizeof(stSend_Channel_Data));
	Send_ch_data_Stop();
	return ( param );
}

static U16			u16Chk_Chindex;
static U16			u16Chk_TP_Freq;
static U16			u16Chk_TP_Symb;
static U16			u16Chk_Sat_Longitude;

void MV_Set_Current_Ch_Data(U16 u16Check_index, MV_stSatInfo Temp_SatInfo, MV_stTPInfo Temp_TPInfo)
{
	u16Chk_Chindex = u16Check_index;
	u16Chk_TP_Freq = Temp_TPInfo.u16TPFrequency;
	u16Chk_TP_Symb = Temp_TPInfo.u16SymbolRate;
	u16Chk_Sat_Longitude = (U16)Temp_SatInfo.s16Longitude;
}

BOOL MV_Check_Current_Ch_Data(U16 u16Check_index, MV_stSatInfo Temp_SatInfo, MV_stTPInfo Temp_TPInfo)
{
	if ( u16Chk_Chindex != u16Check_index
		|| u16Chk_TP_Freq != Temp_TPInfo.u16TPFrequency
		|| u16Chk_TP_Symb != Temp_TPInfo.u16SymbolRate
		|| u16Chk_Sat_Longitude != (U16)Temp_SatInfo.s16Longitude )
		return FALSE;

	return TRUE;
}

void MV_OS_Get_Time_to_MJD_UTC_Date_Time(U16 *u16MJD, U16 *u16UTC, tCS_DT_Date *stDate, tCS_DT_Time *stTime)
{
	struct tm 		tm_time;
	struct timespec time_value;

	clock_gettime(CLOCK_REALTIME, &time_value);

	memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

	stDate->year = tm_time.tm_year+1900;
	stDate->month = tm_time.tm_mon + 1;
	stDate->day = tm_time.tm_mday;
	stTime->hour = tm_time.tm_hour;
	stTime->minute = tm_time.tm_min;

	*u16MJD = CS_DT_YMDtoMJD(*stDate);
	*u16UTC = CS_DT_HMtoUTC(*stTime);
}
#endif

